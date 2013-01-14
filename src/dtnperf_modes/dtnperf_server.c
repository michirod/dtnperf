/*
 * dtnperf_server.c
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */

#include "dtnperf_server.h"
#include "../includes.h"
#include "../definitions.h"
#include "../bundle_tools.h"
#include "../file_transfer_tools.h"
#include "../utils.h"

#include <al_bp_api.h>

/*
 * Global variables
 */

// pthread variables
pthread_t file_exp_timer;
pthread_mutex_t mutexdata;

file_transfer_info_list_t file_transfer_info_list;
al_bp_handle_t handle;
al_bp_reg_id_t regid;
al_bp_endpoint_id_t local_eid;

// flags to exit cleanly
boolean_t bp_handle_open;


/*  ----------------------------
 *          SERVER CODE
 *  ---------------------------- */
void run_dtnperf_server(dtnperf_global_options_t * perf_g_opt)
{
	/* ------------------------
	 * variables
	 * ------------------------ */

	dtnperf_options_t * perf_opt = perf_g_opt->perf_opt;
	dtnperf_connection_options_t * conn_opt = perf_g_opt->conn_opt;

	al_bp_reg_info_t reginfo;
	al_bp_bundle_payload_location_t pl_location;
	al_bp_endpoint_id_t bundle_source_addr;
	al_bp_endpoint_id_t bundle_dest_addr;
	al_bp_endpoint_id_t bundle_replyto_addr;
	al_bp_error_t error;
	al_bp_bundle_object_t bundle_object;
	al_bp_bundle_object_t bundle_ack_object;
	al_bp_bundle_delivery_opts_t bundle_ack_dopts;
	al_bp_timestamp_t bundle_creation_timestamp;
	al_bp_timeval_t bundle_expiration;
	size_t bundle_payload_len;
	dtnperf_server_ack_payload_t server_ack_payload;
	HEADER_TYPE bundle_header;
	dtnperf_bundle_ack_options_t bundle_ack_options;
	time_t current;
	char* command = NULL;
	char* pl_filename = NULL;
	size_t pl_filename_len = 0;
	char* pl_buffer = NULL;
	size_t pl_buffer_size = 0;
	boolean_t is_file_transfer_bundle;
	int indicator; // for file transfer functions purposes


	/* ------------------------
	 * initialize variables
	 * ------------------------ */
	boolean_t debug = perf_g_opt->perf_opt->debug;
	int debug_level =  perf_g_opt->perf_opt->debug_level;

	perf_opt->dest_dir = correct_dirname(perf_opt->dest_dir);
	perf_opt->file_dir = correct_dirname(perf_opt->file_dir);

	bp_handle_open = FALSE;

	// initialize structures for file transfers
	file_transfer_info_list = file_transfer_info_list_create();

	// set out buffer size if daemon
	if (perf_opt->daemon)
	{
		setlinebuf(stdout);
		setlinebuf(stderr);
	}

	// show requested options (debug)
	if (debug)
	{
		printf("\nOptions;\n");
		printf("\tendpoint:\t%s\n", al_bp_get_implementation() == BP_ION ? SERV_EP_NUM_SERVICE : SERV_EP_STRING);
		printf("\tsave bundles to:\t%s\n", perf_opt->use_file ? "file":"memory");
		if(perf_opt->use_file)
			printf("\tdestination dir:\t%s\n", perf_opt->dest_dir);
		printf("\tsend acks:\t%s\n", perf_opt->no_acks ? "no":"yes");
		if (!perf_opt->no_acks)
		{
			//printf("\tsend acks to monitor: %s\n", perf_opt->acks_to_mon ? "yes":"no");
			printf("\tacks expiration time: %d\n", (int) conn_opt->expiration);
			char * prior;
			switch(conn_opt->priority.priority)
			{
			case BP_PRIORITY_BULK:
				prior = "bulk";
				break;
			case BP_PRIORITY_NORMAL:
				prior = "normal";
				break;
			case BP_PRIORITY_EXPEDITED:
				prior = "expedited";
				break;
			case BP_PRIORITY_RESERVED:
				prior = "reserved";
				break;
			default:
				prior = "unknown";
				break;
			}
			printf("\tacks priority       : %s\n", prior);
		}
		printf("\n");

	}

	//Ctrl+C handler
	signal(SIGINT, server_handler);

	// create dir where dtnperf server will save incoming bundles
	// command should be: mkdir -p "dest_dir"
	if(debug && debug_level > 0)
		printf("[debug] initializing shell command...");
	command = malloc(sizeof(char) * (10 + strlen(perf_opt->dest_dir)));
	sprintf(command, "mkdir -p %s", perf_opt->dest_dir);
	if(debug && debug_level > 0)
		printf("done. Shell command = %s\n", command);

	// execute shell command
	if(debug && debug_level > 0)
		printf("[debug] executing shell command...");
	if (system(command) < 0)
	{
		perror("Error opening bundle destination dir");
		exit(-1);
	}
	free(command);
	if(debug && debug_level > 0)
		printf("done\n");

	// create dir where dtnperf server will save incoming files
	// command should be: mkdir -p "file_dir"
	if(debug && debug_level > 0)
		printf("[debug] initializing shell command...");
	command = malloc(sizeof(char) * (10 + strlen(perf_opt->file_dir)));
	sprintf(command, "mkdir -p %s", perf_opt->file_dir);
	if(debug && debug_level > 0)
		printf("done. Shell command = %s\n", command);

	// execute shell command
	if(debug && debug_level > 0)
		printf("[debug] executing shell command...");
	if (system(command) < 0)
	{
		perror("Error opening transfered files destination dir");
		exit(-1);
	}
	free(command);
	if(debug && debug_level > 0)
		printf("done\n");

	//open the connection to the bundle protocol router
	if(debug && debug_level > 0)
		printf("[debug] opening connection to bundle protocol router...");
	if (perf_opt->use_ip)
		error = al_bp_open_with_ip(perf_opt->ip_addr, perf_opt->ip_port, &handle);
	else
		error = al_bp_open(&handle);
	if (error != BP_SUCCESS)
	{
		fflush(stdout);
		fprintf(stderr, "fatal error opening bp handle: %s\n", al_bp_strerror(error));
		exit(1);
	}
	else
	{
		bp_handle_open = TRUE;
	}
	if(debug && debug_level > 0)
		printf("done\n");

	//build a local eid
	if(debug && debug_level > 0)
		printf("[debug] building a local eid...");

	if(al_bp_get_implementation() == BP_ION)
		al_bp_build_local_eid(handle, &local_eid, SERV_EP_NUM_SERVICE,"Server-CBHE",NULL);
	else if(al_bp_get_implementation() == BP_DTN)
		al_bp_build_local_eid(handle, &local_eid, SERV_EP_STRING,"Server-DTN",NULL);

	if(debug && debug_level > 0)
		printf("done\n");
	if (debug)
		printf("local_eid = %s\n", local_eid.uri);

	// checking if there is already a registration
	if(debug && debug_level > 0)
		printf("[debug] checking for existing registration...");
	error = al_bp_find_registration(handle, &local_eid, &regid);
	if (error == BP_SUCCESS)
	{
		fflush(stdout);
		fprintf(stderr, "error: there is a registration with the same eid.\n");
		fprintf(stderr, "regid 0x%x\n", (unsigned int) regid);
		al_bp_close(handle);
		exit(1);
	}
	if ((debug) && (debug_level > 0))
		printf(" done\n");

	//create a new registration to the local router based on this eid
	if(debug && debug_level > 0)
		printf("[debug] registering to local daemon...");
	memset(&reginfo, 0, sizeof(reginfo));
	al_bp_copy_eid(&reginfo.endpoint, &local_eid);
	reginfo.flags = BP_REG_DEFER;
	reginfo.regid = BP_REGID_NONE;
	reginfo.expiration = 0;
	if ((error = al_bp_register(&handle, &reginfo, &regid)) != 0)
	{
		fflush(stdout);
		fprintf(stderr, "error creating registration: %d (%s)\n",
				error, al_bp_strerror(al_bp_errno(handle)));
		exit(1);
	}
	if ((debug) && (debug_level > 0))
		printf(" done\n");
	if (debug)
		printf("regid 0x%x\n", (unsigned int) regid);

	// set bundle destination type
	if ((debug) && (debug_level > 0))
		printf("[debug] choosing bundle destination type...");
	if (perf_opt->use_file)
		pl_location = BP_PAYLOAD_FILE;
	else
		pl_location = BP_PAYLOAD_MEM;
	if ((debug) && (debug_level > 0))
		printf(" done. Bundles will be saved into %s\n", perf_opt->use_file ? "file" : "memory");

	// start thread
	pthread_mutex_init (&mutexdata, NULL);
	pthread_create(&file_exp_timer, NULL, file_expiration_timer, NULL);



	if ((debug) && (debug_level > 0))
		printf("[debug] entering infinite loop...\n");

	// start infinite loop
	while(1)
	{

		// create a bundle object
		if ((debug) && (debug_level > 0))
			printf("[debug] initiating memory for bundles...\n");

		error = al_bp_bundle_create(&bundle_object);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "fatal error initiating memory for bundles: %s\n", al_bp_strerror(error));
			exit(1);
		}
		if(debug && debug_level > 0)
			printf("done\n");

		// reset file transfer indicators
		is_file_transfer_bundle = FALSE;

		// wait until receive a bundle
		if ((debug) && (debug_level > 0))
			printf("[debug] waiting for bundles...\n");

		error = al_bp_bundle_receive(handle, bundle_object, pl_location, -1);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting recv reply: %d (%s)\n",
					error, al_bp_strerror(al_bp_errno(handle)));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
			printf(" bundle received\n");

		// find payload size
		if ((debug) && (debug_level > 0))
			printf("[debug] calculating bundle payload size...");
		error = al_bp_bundle_get_payload_size(bundle_object, (u32_t *) &bundle_payload_len);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle payload size: %s\n",
					al_bp_strerror(error));
			exit(1);
		}
		if(debug && debug_level > 0)
			printf("done\n");

		// mark current time

		if ((debug) && (debug_level > 0))
			printf("[debug] marking time...");
		current = time(NULL);
		if ((debug) && (debug_level > 0))
			printf(" done\n");

		// print bundle arrival
		printf("%s : %zu bytes from %s\n",
				ctime(&current),
				bundle_payload_len,
				bundle_object.spec->source.uri);


		// get SOURCE eid
		if ((debug) && (debug_level > 0))
			printf("[debug]\tgetting source eid...");
		error = al_bp_bundle_get_source(bundle_object, &bundle_source_addr);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle source eid: %s\n",
					al_bp_strerror(error));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done:\n");
			printf("\tbundle_source_addr = %s\n", bundle_source_addr.uri);
			printf("\n");
		}

		// get DEST eid
		if ((debug) && (debug_level > 0))
			printf("[debug]\tgetting destination eid...");
		error = al_bp_bundle_get_dest(bundle_object, &bundle_dest_addr);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle destination eid: %s\n",
					al_bp_strerror(error));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done:\n");
			printf("\tbundle_dest_eid = %s\n", bundle_dest_addr.uri);
			printf("\n");
		}

		// get REPLY TO eid
		if ((debug) && (debug_level > 0))
			printf("[debug]\tgetting reply to eid...");
		error = al_bp_bundle_get_replyto(bundle_object, &bundle_replyto_addr);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle reply to eid: %s\n",
					al_bp_strerror(error));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done:\n");
			printf("\tbundle_replyto_eid = %s\n", bundle_replyto_addr.uri);
			printf("\n");
		}

		// get bundle CREATION TIMESTAMP
		if ((debug) && (debug_level > 0))
			printf("[debug]\tgetting bundle creation timestamp...");
		error = al_bp_bundle_get_creation_timestamp(bundle_object, &bundle_creation_timestamp);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle creation timestamp: %s\n",
					al_bp_strerror(error));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done:\n");
			printf("\tbundle creation timestamp:\n"
					"\tsecs = %d\n\tseqno= %d\n",
					(int)bundle_creation_timestamp.secs, (int)bundle_creation_timestamp.seqno);
			printf("\n");
		}


		// get bundle payload filename
		if(perf_opt->use_file)
		{
			if ((debug) && (debug_level > 0))
				printf("[debug]\tgetting bundle payload filename...");
			error = al_bp_bundle_get_payload_file(bundle_object, &pl_filename, (u32_t *) &pl_filename_len);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "error getting bundle payload filename: %s\n",
						al_bp_strerror(error));
				exit(1);
			}
			if ((debug) && (debug_level > 0))
			{
				printf(" done:\n");
			}
		}

		if ((debug))
		{
			printf ("======================================\n");
			printf (" Bundle received at %s\n", ctime(&current));
			printf ("  source: %s\n", bundle_source_addr.uri);
			if (perf_opt->use_file)
			{
				printf ("  saved into    : %s\n", pl_filename);
			}

			printf ("--------------------------------------\n");
		};
		// get bundle header and options
		if ((debug) && (debug_level > 0))
			printf("[debug]\tgetting bundle header and options...");
		if (get_bundle_header_and_options(&bundle_object, &bundle_header, &bundle_ack_options) < 0)
		{
			printf("Error in getting bundle header and options\n");
			continue;
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done.\n");
		}

		// check if is file transfer bundle
		if ((debug) && (debug_level > 0))
			printf("[debug]\tchecking if this is a file transfer bundle...");
		if (bundle_header == FILE_HEADER)
		{
			is_file_transfer_bundle = TRUE;
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done.\n");
			printf("\tbundle is%sa file transfer bundle\n",
					is_file_transfer_bundle ? " " : " not ");
			printf("\n");
		}

		// process file transfer bundle
		if(is_file_transfer_bundle)
		{
			if ((debug) && (debug_level > 0))
				printf("[debug]\tprocessing file transfer bundle...");

			pthread_mutex_lock(&mutexdata);

			indicator = process_incoming_file_transfer_bundle(&file_transfer_info_list,
					&bundle_object, perf_opt->file_dir);

			pthread_mutex_unlock(&mutexdata);
			sched_yield();

			if (indicator < 0) // error in processing bundle
			{
				fprintf(stderr, "Error in processing file transfer bundle: %s\n", strerror(errno));
			}
			if ((debug) && (debug_level > 0))
			{
				printf("done.");
				if (indicator == 1)
					printf("Transfer Completed\n");
			}
		}
		// get bundle expiration time and priority
		if (bundle_ack_options.set_ack_expiration)
		{
			al_bp_bundle_get_expiration(bundle_object, &bundle_expiration);
		}

		// send acks to the client only if requested by client
		// send acks to the monitor if:
		// ack requested by client AND ack-to-monitor option set AND bundle_ack_options.ack_to_mon == ATM_NORMAL
		// OR client forced server to send ack to monitor

		boolean_t send_ack_to_client = bundle_ack_options.ack_to_client;
		boolean_t send_ack_to_monitor = FALSE;
		send_ack_to_monitor = (bundle_ack_options.ack_to_client && (bundle_ack_options.ack_to_mon == ATM_NORMAL) && bundle_ack_options.ack_to_client && perf_opt->acks_to_mon)
				|| (bundle_ack_options.ack_to_mon == ATM_FORCE_YES);
		if (send_ack_to_client || send_ack_to_monitor)
		{

			// create bundle ack to send
			if ((debug) && (debug_level > 0))
				printf("[debug] initiating memory for bundle ack...");
			error = al_bp_bundle_create(&bundle_ack_object);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error initiating memory for bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");


			// initiate server ack payload
			// set server ack payload source
			server_ack_payload.bundle_source = bundle_source_addr;
			// set server ack payload timestamp
			server_ack_payload.bundle_creation_ts = bundle_creation_timestamp;
			// preparing the bundle ack payload
			if ((debug) && (debug_level > 0))
				printf("[debug] preparing the payload of the bundle ack...");
			error = prepare_server_ack_payload(server_ack_payload, &pl_buffer, &pl_buffer_size);

			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error preparing the payload of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			// setting the bundle ack payload
			if ((debug) && (debug_level > 0))
				printf("[debug] setting the payload of the bundle ack...");
			error = al_bp_bundle_set_payload_mem(&bundle_ack_object, pl_buffer, pl_buffer_size);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the payload of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			// setting the bundle ack options
			if (debug && debug_level > 0)
			{
				printf("[debug] setting source of the bundle ack: %s ...", bundle_source_addr.uri);
			}
			error = al_bp_bundle_set_source(& bundle_ack_object, local_eid);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the source of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting destination of the bundle ack: %s ...", bundle_source_addr.uri);
			}
			error = al_bp_bundle_set_dest(& bundle_ack_object, bundle_source_addr);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the destination of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting replyto eid of the bundle ack: %s ...", bundle_replyto_addr.uri);
			}
			al_bp_bundle_set_replyto(& bundle_ack_object, bundle_replyto_addr);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the reply to eid of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting priority of the bundle ack...");
			}
			if (bundle_ack_options.set_ack_priority == TRUE)
			{
				al_bp_bundle_set_priority(& bundle_ack_object, bundle_ack_options.priority);
			}
			else
			{
				al_bp_bundle_set_priority(& bundle_ack_object, conn_opt->priority);
			}
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting priority of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting expiration time of the bundle ack...");
			}
			if (bundle_ack_options.set_ack_expiration)
				al_bp_bundle_set_expiration(& bundle_ack_object, bundle_expiration);
			else
				al_bp_bundle_set_expiration(& bundle_ack_object, conn_opt->expiration);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting expiration time of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting delivery options of the bundle ack...");
			}
			bundle_ack_dopts = BP_DOPTS_CUSTODY;
			al_bp_bundle_set_delivery_opts(& bundle_ack_object, bundle_ack_dopts);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting delivery options of the bundle ack: %s\n", al_bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			// send the bundle ack to the client
			if (send_ack_to_client)
			{
				if ((debug) && (debug_level > 0))
					printf("[debug] sending bundle ack to client...");
				error = al_bp_bundle_send(handle, regid, & bundle_ack_object);
				if (error != BP_SUCCESS)
				{
					fflush(stdout);
					fprintf(stderr, "error sending bundle ack to client: %d (%s)\n",
							error, al_bp_strerror(al_bp_errno(handle)));
					exit(1);
				}
				if ((debug) && (debug_level > 0))
					printf(" bundle ack sent to client\n");
			}
			printf("Send bundle to client ok\n");

			// send the bundle ack to the monitor
			if (send_ack_to_monitor)
			{
				al_bp_bundle_set_dest(& bundle_ack_object, bundle_replyto_addr);
				if ((debug) && (debug_level > 0))
					printf("[debug] sending bundle ack to monitor...");
				error = al_bp_bundle_send(handle, regid, & bundle_ack_object);
				if (error != BP_SUCCESS)
				{
					fflush(stdout);
					fprintf(stderr, "error sending bundle ack to monitor: %d (%s)\n",
							error, al_bp_strerror(al_bp_errno(handle)));
					exit(1);
				}
				if ((debug) && (debug_level > 0))
					printf(" bundle ack sent to monitor\n");
			}

			//free memory for bundle ack
			al_bp_bundle_free(&bundle_ack_object);
			free(pl_buffer);
			pl_buffer_size = 0;

		}

		// free memory for bundle
		al_bp_bundle_free(&bundle_object);

		free(pl_filename);
		pl_filename_len = 0;

	}// while(1)

	al_bp_close(handle);
	al_bp_unregister(handle,regid,local_eid);
	bp_handle_open = FALSE;


}
// end server code

// file expiration timer thread
void * file_expiration_timer(void * opt)
{
	u32_t current_dtn_time;
	file_transfer_info_list_item_t * item, * next;

	while(1)
	{
		pthread_sleep(10);
		current_dtn_time = get_current_dtn_time();

		pthread_mutex_lock(&mutexdata);

		for(item = file_transfer_info_list.first; item != NULL; item = next)
		{
			next = item->next;
/*			if (item->info->last_bundle_time + item->info->expiration < current_dtn_time)
			{
				char* filename = (char*) malloc(item->info->filename_len + strlen(item->info->full_dir) +1);
				strcpy(filename, item->info->full_dir);
				strcat(filename, item->info->filename);
				if (remove(filename) < 0)
					perror("Error eliminating expired file:");
				printf("Eliminated file %s because timer has expired\n", filename);
				file_transfer_info_list_item_delete(&file_transfer_info_list, item);
				free(filename);
			}*/
		}
		pthread_mutex_unlock(&mutexdata);
		sched_yield();
	}
	pthread_exit(NULL);
}

void print_server_usage(char * progname)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "DtnPerf3 server mode\n");
	fprintf(stderr, "SYNTAX: %s %s [options]\n", progname, SERVER_STRING);
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n"
			" -a, --daemon           Start the server as a daemon. Output is redirected to %s.\n"
			" -o, --output <file>    Change the default output file (only with -a option).\n"
			" -s, --stop             Stop a demonized instance of server.\n"
			"     --ip-addr <addr>   Ip address of the bp daemon api. Default: 127.0.0.1\n"
			"     --ip-port <port>   Ip port of the bp daemon api. Default: 5010\n"
			"     --fdir <dir>       Destination directory of transfered files. Default is %s .\n"
			"     --debug[=level]    Debug messages [1-2], if level is not indicated level = 1.\n"
			" -M, --memory         	 Save bundles into memory.\n"
			" -l, --lifetime <sec>   Bundle acks lifetime (s). Default is 3600\n"
			" -p, --priority <val>   Bundle acks priority [bulk|normal|expedited|reserved]. Default: normal\n"
			//"     --acks-to-mon      Send bundle acks to the monitor too\n"
			//"     --eid [URI|CBHE]   Set type of eid format. CBHE only for ION implementation. Default: URI\n",
			" -v, --verbose          Print some information message during the execution.\n"
			" -h, --help             This help.\n",
			SERVER_OUTPUT_FILE, FILE_DIR_DEFAULT);
	fprintf(stderr, "\n");
	exit(1);
}

void parse_server_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt)
{
	char c, done = 0;
	boolean_t output_set = FALSE;
	dtnperf_options_t * perf_opt = perf_g_opt->perf_opt;
	dtnperf_connection_options_t * conn_opt = perf_g_opt->conn_opt;
	// kill daemon variables
	int pid;
	char cmd[256];

	while (!done)
	{
		static struct option long_options[] =
		{
				{"help", no_argument, 0, 'h'},
				{"verbose", no_argument, 0, 'v'},
				{"memory", no_argument, 0, 'M'},
				{"lifetime", required_argument, 0, 'l'},
				{"debug", optional_argument, 0, 33}, 			// 33 because D is for data mode
				{"priority", required_argument, 0, 'p'},
				{"ddir", required_argument, 0, 34},
				{"fdir", required_argument, 0, 39},
				{"acks-to-mon", no_argument, 0, 35},		// server only option
				{"ip-addr", required_argument, 0, 37},
				{"ip-port", required_argument, 0, 38},
				{"daemon", no_argument, 0, 'a'},
				{"output", required_argument, 0, 'o'},
				{"stop", no_argument, 0, 's'},
				{0,0,0,0}	// The last element of the array has to be filled with zeros.

		};
		int option_index = 0;
		c = getopt_long(argc, argv, "hvMl:p:ao:s", long_options, &option_index);

		switch (c)
		{
		case 'h':
			print_server_usage(argv[0]);
			exit(0);
			return ;

		case 'v':
			perf_opt->verbose = TRUE;
			break;

		case 'M':
			perf_opt->use_file = 0;
			perf_opt->payload_type = BP_PAYLOAD_MEM;
			break;

		case 'l':
			conn_opt->expiration = atoi(optarg);
			break;

		case 'p':
			if (!strcasecmp(optarg, "bulk"))   {
				conn_opt->priority.priority = BP_PRIORITY_BULK;
			} else if (!strcasecmp(optarg, "normal")) {
				conn_opt->priority.priority = BP_PRIORITY_NORMAL;
			} else if (!strcasecmp(optarg, "expedited")) {
				conn_opt->priority.priority = BP_PRIORITY_EXPEDITED;
			} else if (!strcasecmp(optarg, "reserved")) {
				conn_opt->priority.priority = BP_PRIORITY_RESERVED;
			} else {
				fprintf(stderr, "Invalid priority value %s\n", optarg);
				exit(1);
			}
			break;

		case 33: // debug
			perf_opt->debug = TRUE;
			if (optarg != NULL){
				int debug_level = atoi(optarg);
				if (debug_level >= 1 && debug_level <= 2)
					perf_opt->debug_level = atoi(optarg) - 1;
				else {
					fprintf(stderr, "wrong --debug argument\n");
					exit(1);
					return;
				}
			}
			else
				perf_opt->debug_level = 2;
			break;

		case 34: //incoming bundles destination directory
			perf_opt->dest_dir = strdup(optarg);
			break;

		case 35: //server send acks to monitor
			perf_opt->acks_to_mon = TRUE;
			break;

		case 36: //server do not send acks
			perf_opt->no_acks = TRUE;
			break;

		case 37:
			perf_opt->ip_addr = strdup(optarg);
			perf_opt->use_ip = TRUE;
			break;

		case 38:
			perf_opt->ip_port = atoi(optarg);
			perf_opt->use_ip = TRUE;
			break;

		case 39:
			perf_opt->file_dir = strdup(optarg);
			break;

		case 'a':
			perf_opt->daemon = TRUE;
			break;

		case 'o':
			perf_opt->server_output_file = strdup(optarg);
			output_set = TRUE;
			break;

		case 's':
			memset(cmd, 0, sizeof(cmd));
			sprintf(cmd, "%s %s", argv[0], SERVER_STRING);
			pid = find_proc(cmd);
			if (pid)
			{
				printf("Closing dtnperf server pid: %d\n", pid);
				kill(pid, SIGINT);
			}
			else
			{
				fprintf(stderr, "ERROR: cannot find a running instance of dtnperf server\n");
			}
			exit(0);
			break;

		case '?':
			fprintf(stderr, "Unknown option: %c\n", optopt);
			exit(1);
			break;

		case (char)(-1):
			done = 1;
			break;

		default:
			// getopt already prints an error message for unknown option characters
			print_server_usage(argv[0]);
			exit(1);
		}
	}
	if (output_set && !perf_opt->daemon)
	{
		fprintf(stderr, "\nSYNTAX ERROR: -o option can be used only with -a option\n");   \
		print_server_usage(argv[0]);                                               \
		exit(1);
	}
}


// Ctrl+C handler
void server_handler(int sig)
{
	printf("\nDTNperf server received SIGINT: Exiting\n");
	server_clean_exit(0);
}

void server_clean_exit(int status)
{
	file_transfer_info_list_item_t * item;

	// terminate immediately all child threads
	pthread_cancel(file_exp_timer);

	// delete all incomplete files
	for(item = file_transfer_info_list.first; item != NULL; item = item->next)
	{

		char* filename = (char*) malloc(item->info->filename_len + strlen(item->info->full_dir) +1);
		strcpy(filename, item->info->full_dir);
		strcat(filename, item->info->filename);
		if (remove(filename) < 0)
			perror("Error eliminating incomplete file:");
		printf("Eliminated file %s because incomplete\n", filename);
		file_transfer_info_list_item_delete(&file_transfer_info_list, item);
		free(filename);
	}

	if (bp_handle_open)
	{
		al_bp_close(handle);
		al_bp_unregister(handle,regid,local_eid);
	}
	printf("DTNperf server: Exit.\n");
	exit(status);
}
