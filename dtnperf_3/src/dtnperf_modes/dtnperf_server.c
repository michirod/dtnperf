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

#include <bp_abstraction_api.h>


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

	bp_handle_t handle;
	bp_endpoint_id_t local_eid;
	bp_reg_info_t reginfo;
	bp_reg_id_t regid;
	bp_bundle_payload_location_t pl_location;
	bp_endpoint_id_t bundle_source_addr;
	bp_endpoint_id_t bundle_dest_addr;
	bp_endpoint_id_t bundle_replyto_addr;
	bp_error_t error;
	bp_bundle_object_t bundle_object;
	bp_bundle_object_t bundle_ack_object;
	bp_bundle_delivery_opts_t bundle_ack_dopts;
	bp_timestamp_t bundle_creation_timestamp;
	size_t bundle_payload_len;
	dtnperf_server_ack_payload_t server_ack_payload;
	time_t current;
	char* command = NULL;
	char* pl_filename = NULL;
	size_t pl_filename_len = 0;
	char* pl_buffer = NULL;
	size_t pl_buffer_size = 0;


	/* ------------------------
	 * initialize variables
	 * ------------------------ */



	boolean_t debug = perf_g_opt->perf_opt->debug;
	int debug_level =  perf_g_opt->perf_opt->debug_level;


	// show requested options (debug)
	if (debug)
	{
		printf("\nOptions;\n");
		printf("\tendpoint		 : %s\n", SERV_EP_STRING);
		printf("\tsave bundles to: %s\n", perf_opt->use_file ? "file":"memory");
		printf("\tdestination dir: %s\n", perf_opt->dest_dir);
		printf("\tsend acks      : %s\n", perf_opt->no_acks ? "no":"yes");
		if (!perf_opt->no_acks)
		{
			printf("\tsend acks to monitor: %s\n", perf_opt->acks_to_mon ? "yes":"no");
			printf("\tacks expiration time: %d\n", (int) conn_opt->expiration);
			char * prior;
			switch(conn_opt->priority)
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

	//open the connection to the bundle protocol router
	if(debug && debug_level > 0)
		printf("[debug] opening connection to bundle protocol router...");
	if (perf_opt->use_ip)
		error = bp_open_with_ip(perf_opt->ip_addr, perf_opt->ip_port, &handle);
	else
		error = bp_open(&handle);
	if (error != BP_SUCCESS)
	{
		fflush(stdout);
		fprintf(stderr, "fatal error opening bp handle: %s\n", bp_strerror(error));
		exit(1);
	}
	if(debug && debug_level > 0)
		printf("done\n");

	//build a local eid
	if(debug && debug_level > 0)
		printf("[debug] building a local eid...");
	bp_build_local_eid(handle, &local_eid, SERV_EP_STRING);
	if(debug && debug_level > 0)
		printf("done\n");
	if (debug)
		printf("local_eid = %s\n", local_eid.uri);

	//create a new registration to the local router based on this eid
	if(debug && debug_level > 0)
		printf("[debug] registering to local daemon...");
	memset(&reginfo, 0, sizeof(reginfo));
	bp_copy_eid(&reginfo.endpoint, &local_eid);
	reginfo.flags = BP_REG_DEFER;
	reginfo.regid = BP_REGID_NONE;
	reginfo.expiration = 0;
	if ((error = bp_register(handle, &reginfo, &regid)) != 0)
	{
		fflush(stdout);
		fprintf(stderr, "error creating registration: %d (%s)\n",
				error, bp_strerror(bp_errno(handle)));
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

	if ((debug) && (debug_level > 0))
		printf("[debug] entering infinite loop...\n");



	// start infinite loop
	while(1)
	{
		// create a bundle object
		if ((debug) && (debug_level > 0))
			printf("[debug] initiating memory for bundles...\n");
		error = bp_bundle_create(&bundle_object);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "fatal error initiating memory for bundles: %s\n", bp_strerror(error));
			exit(1);
		}
		if(debug && debug_level > 0)
			printf("done\n");

		// initiate server ack payload
		server_ack_payload.header = DSA_STRING;

		// wait until receive a bundle
		if ((debug) && (debug_level > 0))
			printf("[debug] waiting for bundles...\n");
		error = bp_bundle_receive(handle, bundle_object, pl_location, -1);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting recv reply: %d (%s)\n",
					error, bp_strerror(bp_errno(handle)));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
			printf(" bundle received\n");

		// find payload size
		if ((debug) && (debug_level > 0))
			printf("[debug] calculating bundle payload size...");
		error = bp_bundle_get_payload_size(bundle_object, &bundle_payload_len);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle payload size: %s\n",
					bp_strerror(error));
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
		error = bp_bundle_get_source(bundle_object, &bundle_source_addr);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle source eid: %s\n",
					bp_strerror(error));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
		{
			printf(" done:\n");
			printf("\tbundle_source_addr = %s\n", bundle_source_addr.uri);
			printf("\n");
		}
		// set server ack payload source
		server_ack_payload.bundle_source = bundle_source_addr;

		// get DEST eid
		if ((debug) && (debug_level > 0))
			printf("[debug]\tgetting destination eid...");
		error = bp_bundle_get_dest(bundle_object, &bundle_dest_addr);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle destination eid: %s\n",
					bp_strerror(error));
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
		error = bp_bundle_get_replyto(bundle_object, &bundle_replyto_addr);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle reply to eid: %s\n",
					bp_strerror(error));
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
		error = bp_bundle_get_creation_timestamp(bundle_object, &bundle_creation_timestamp);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting bundle creation timestamp: %s\n",
					bp_strerror(error));
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
		// set server ack payload timestamp
		server_ack_payload.bundle_creation_ts = bundle_creation_timestamp;


		// get bundle payload filename
		if(perf_opt->use_file)
		{
			if ((debug) && (debug_level > 0))
				printf("[debug]\tgetting bundle payload filename...");
			error = bp_bundle_get_payload_file(bundle_object, &pl_filename, &pl_filename_len);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "error getting bundle payload filename: %s\n",
						bp_strerror(error));
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
		}


		if(!perf_opt->no_acks)
		{
			// create bundle ack to send to client
			if ((debug) && (debug_level > 0))
				printf("[debug] initiating memory for bundle ack...");
			error = bp_bundle_create(&bundle_ack_object);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error initiating memory for bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");


			// preparing the bundle ack payload
			if ((debug) && (debug_level > 0))
				printf("[debug] preparing the payload of the bundle ack...");
			error = prepare_server_ack_payload(server_ack_payload, &pl_buffer, &pl_buffer_size);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error preparing the payload of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			// setting the bundle ack payload
			if ((debug) && (debug_level > 0))
				printf("[debug] setting the payload of the bundle ack...");
			error = bp_bundle_set_payload_mem(&bundle_ack_object, pl_buffer, pl_buffer_size);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the payload of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			// setting the bundle ack options
			if (debug && debug_level > 0)
			{
				printf("[debug] setting source of the bundle ack: %s ...", bundle_source_addr.uri);
			}
			error = bp_bundle_set_source(& bundle_ack_object, local_eid);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the source of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting destination of the bundle ack: %s ...", bundle_source_addr.uri);
			}
			error = bp_bundle_set_dest(& bundle_ack_object, bundle_source_addr);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the destination of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting replyto eid of the bundle ack: %s ...", bundle_replyto_addr.uri);
			}
			bp_bundle_set_replyto(& bundle_ack_object, bundle_replyto_addr);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting the reply to eid of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting priority of the bundle ack...");
			}
			bp_bundle_set_priority(& bundle_ack_object, conn_opt->priority);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting priority of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting expiration time of the bundle ack...");
			}
			bp_bundle_set_expiration(& bundle_ack_object, conn_opt->expiration);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting expiration time of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			if (debug && debug_level > 0)
			{
				printf("[debug] setting delivery options of the bundle ack...");
			}
			bundle_ack_dopts = BP_DOPTS_CUSTODY;
			bp_bundle_set_delivery_opts(& bundle_ack_object, bundle_ack_dopts);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "fatal error setting delivery options of the bundle ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if(debug && debug_level > 0)
				printf("done\n");

			// send the bundle ack to the client
			if ((debug) && (debug_level > 0))
				printf("[debug] sending bundle ack to client...");
			error = bp_bundle_send(handle, regid, & bundle_ack_object);
			if (error != BP_SUCCESS)
			{
				fflush(stdout);
				fprintf(stderr, "error sending bundle ack to client: %d (%s)\n",
						error, bp_strerror(bp_errno(handle)));
				exit(1);
			}
			if ((debug) && (debug_level > 0))
				printf(" bundle ack sent to client\n");

			// send the bundle ack to the monitor
			if (perf_opt->acks_to_mon)
			{
				bp_bundle_set_dest(& bundle_ack_object, bundle_replyto_addr);
				if ((debug) && (debug_level > 0))
					printf("[debug] sending bundle ack to monitor...");
				error = bp_bundle_send(handle, regid, & bundle_ack_object);
				if (error != BP_SUCCESS)
				{
					fflush(stdout);
					fprintf(stderr, "error sending bundle ack to monitor: %d (%s)\n",
							error, bp_strerror(bp_errno(handle)));
					exit(1);
				}
				if ((debug) && (debug_level > 0))
					printf(" bundle ack sent to monitor\n");
			}
			//free memory for bundle ack
			bp_bundle_free(&bundle_ack_object);
			free(pl_buffer);
			pl_buffer_size = 0;
		}

		// free memory for bundle
		bp_bundle_free(&bundle_object);
		free(pl_filename);
		pl_filename_len = 0;


	}// while(1)

	bp_close(handle);


}
// end server code

void print_server_usage(char * progname)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "DtnPerf3 server mode\n");
	fprintf(stderr, "SYNTAX: %s --server [options]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n"
			"     --ip-addr <addr>   Ip address of the bp daemon api. Default is 127.0.0.1\n"
			"     --ip-port <port>   Ip port of the bp daemon api. Default is 5010\n"
			"     --ddir <dir>       Destination directory (if not using -M), if dir is not indicated assume %s.\n"
			"     --debug[=level]    Debug messages [0-1], if level is not indicated assume level=0.\n"
			" -M, --memory           Save bundles into memory.\n"
			" -e, --expiration <sec> Bundle acks expiration time. Default is 3600\n"
			" -P, --priority <val>   Bundle acks priority [bulk|normal|expedited|reserved]. Default is normal\n"
			"     --acks-to-mon      Send bundle acks to the monitor too\n"
			"     --no-acks          Do not send acks (for using with dtnperf2)\n"
			" -v, --verbose          Print some information message during the execution.\n"
			" -h, --help             This help.\n",
			BUNDLE_DIR_DEFAULT);
	fprintf(stderr, "\n");
	exit(1);
}

void parse_server_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt)
{
	char c, done = 0;
	dtnperf_options_t * perf_opt = perf_g_opt->perf_opt;
	dtnperf_connection_options_t * conn_opt = perf_g_opt->conn_opt;

	while (!done)
	{
		static struct option long_options[] =
		{
				{"help", no_argument, 0, 'h'},
				{"verbose", no_argument, 0, 'v'},
				{"memory", no_argument, 0, 'M'},
				{"expiration", required_argument, 0, 'e'},
				{"debug", optional_argument, 0, 33}, 			// 33 because D is for data mode
				{"priority", required_argument, 0, 'P'},
				{"ddir", required_argument, 0, 34},
				{"acks-to-mon", no_argument, 0, 35},			// server only option
				{"no-acks", no_argument, 0, 36},				// server only option
				{"ip-addr", required_argument, 0, 37},
				{"ip-port", required_argument, 0, 38},
				{0,0,0,0}	// The last element of the array has to be filled with zeros.

		};
		int option_index = 0;
		c = getopt_long(argc, argv, "hvMe:P:", long_options, &option_index);

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

		case 'e':
			conn_opt->expiration = atoi(optarg);
			break;

		case 'P':
			if (!strcasecmp(optarg, "bulk"))   {
				conn_opt->priority = BP_PRIORITY_BULK;
			} else if (!strcasecmp(optarg, "normal")) {
				conn_opt->priority = BP_PRIORITY_NORMAL;
			} else if (!strcasecmp(optarg, "expedited")) {
				conn_opt->priority = BP_PRIORITY_EXPEDITED;
			} else if (!strcasecmp(optarg, "reserved")) {
				conn_opt->priority = BP_PRIORITY_RESERVED;
			} else {
				fprintf(stderr, "Invalid priority value %s\n", optarg);
				exit(1);
			}
			break;

		case 33: // debug
			perf_opt->debug = TRUE;
			if (optarg != NULL){
				int debug_level = atoi(optarg);
				if (debug_level >= 0 && debug_level <= 2)
					perf_opt->debug_level = atoi(optarg);
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

		case '?':
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
}

