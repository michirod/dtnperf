/*
 * dtnperf_client.c
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */


#include "dtnperf_client.h"
#include "../includes.h"
#include "../definitions.h"
#include "../bundle_tools.h"
#include "../file_transfer_tools.h"
#include "../utils.h"
#include <semaphore.h>
#include <libgen.h>
#include <sys/stat.h>
#include <bp_abstraction_api.h>


/* pthread_yield() is not standard,
   so use sched_yield if necessary */
#ifndef HAVE_PTHREAD_YIELD
#   ifdef HAVE_SCHED_YIELD
#       include <sched.h>
#       define pthread_yield    sched_yield
#   endif
#endif

// pthread variables
pthread_t sender;
pthread_t cong_ctrl;
pthread_mutex_t mutexdata;
pthread_cond_t cond_ackreceiver;
sem_t window;						// semaphore for congestion control

// client threads variables
send_information_t * send_info;		// array info of sent bundles
long tot_bundles;					// for data mode
struct timeval start, end, now;			// time variables
struct timeval bundle_sent, ack_recvd;	// time variables
int sent_bundles = 0;					// sent bundles counter
unsigned int sent_data = 0;				// sent byte counter
int close_ack_receiver = 0;			// to signal the ack receiver to close
unsigned int data_written = 0;			// num of bytes written on the source file


FILE * log_file = NULL;
char * source_file_abs;				// absolute path of file SOURCE_FILE
char * transfer_filename;			// basename of the file to transfer
u32_t transfer_filedim;				// size of the file to transfer
int transfer_fd;					// file descriptor

// buffer settings
char* buffer = NULL;            // buffer containing data to be transmitted
size_t bufferLen;                  // lenght of buffer


// BP variables
bp_error_t error;
bp_handle_t handle;
bp_reg_id_t regid;
bp_reg_info_t reginfo;
bp_bundle_id_t * bundle_id;
bp_endpoint_id_t local_eid;
bp_endpoint_id_t dest_eid;
bp_endpoint_id_t mon_eid;
bp_bundle_object_t bundle;
bp_bundle_object_t ack;

void usr(int signo)
{}

/*  ----------------------------
 *          CLIENT CODE
 *  ---------------------------- */
void run_dtnperf_client(dtnperf_global_options_t * perf_g_opt)
{
	/* ------------------------
	 * variables
	 * ------------------------ */
	dtnperf_options_t * perf_opt = perf_g_opt->perf_opt;
	dtnperf_connection_options_t * conn_opt = perf_g_opt->conn_opt;
	char * client_demux_string;
	int pthread_status;

	char temp1[256]; // buffer for different purpose
	char temp2[256];
	FILE * stream; // stream for preparing payolad


	/* ------------------------
	 * initialize variables
	 * ------------------------ */
	boolean_t debug = perf_opt->debug;
	int debug_level =  perf_opt->debug_level;
	boolean_t verbose = perf_opt->verbose;
	boolean_t create_log = perf_opt->create_log;
	stream = NULL;
	tot_bundles = 0;
	perf_opt->log_filename = correct_dirname(perf_opt->log_filename);

	// Create a new log file
	if (create_log)
	{
		if ((log_file = fopen(perf_opt->log_filename, "w")) == NULL)
		{
			fprintf(stderr, "fatal error opening log file\n");
			exit(1);
		}
	}

	// Connect to BP Daemon
	if ((debug) && (debug_level > 0))
		printf("[debug] opening connection to local BP daemon...");

	if (perf_opt->use_file)
		error = bp_open_with_ip(perf_opt->ip_addr,perf_opt->ip_port,&handle);
	else
		error = bp_open(&handle);

	if (error != BP_SUCCESS)
	{
		fprintf(stderr, "fatal error opening bp handle: %s\n", bp_strerror(error));
		if (create_log)
			fprintf(log_file, "fatal error opening bp handle: %s\n", bp_strerror(error));
		exit(1);
	}

	if ((debug) && (debug_level > 0))
		printf("done\n");

	// Ctrl+C handler
	signal(SIGUSR1, &handler);
	signal(SIGUSR2, &usr);

	/* -----------------------------------------------------
	 *   initialize and parse bundle src/dest/replyto EIDs
	 * ----------------------------------------------------- */

	// append process id to the client demux string
	client_demux_string = malloc (strlen(CLI_EP_STRING) + 6);
	sprintf(client_demux_string, "%s_%d", CLI_EP_STRING, getpid());

	//build a local eid
	if(debug && debug_level > 0)
		printf("[debug] building a local eid...");
	bp_build_local_eid(handle, &local_eid, client_demux_string);
	if(debug && debug_level > 0)
		printf("done\n");
	if (debug)
		printf("Source     : %s\n", local_eid.uri);
	if (create_log)
		fprintf(log_file, "\nSource     : %s\n", local_eid.uri);

	// parse SERVER EID
	// append server demux string to destination eid
	strcat(perf_opt->dest_eid, SERV_EP_STRING);

	if (verbose)
		fprintf(stdout, "%s (local)\n", perf_opt->dest_eid);
	// parse
	error = bp_parse_eid_string(&dest_eid, perf_opt->dest_eid);

	if (error != BP_SUCCESS)
	{
		fprintf(stderr, "fatal error parsing bp EID: invalid eid string '%s'\n", perf_opt->dest_eid);
		if (create_log)
			fprintf(log_file, "\nfatal error parsing bp EID: invalid eid string '%s'", perf_opt->dest_eid);
		exit(1);
	}

	if (debug)
		printf("Destination: %s\n", dest_eid.uri);

	if (create_log)
		fprintf(log_file, "Destination: %s\n", dest_eid.uri);


	// parse REPLY-TO (if none specified, same as the source)
	if (strlen(perf_opt->mon_eid) == 0)
	{
		char * ptr;
		ptr = strstr(local_eid.uri, CLI_EP_STRING);
		// copy from local eid only the uri (not the demux string)
		strncpy(perf_opt->mon_eid, local_eid.uri, ptr - local_eid.uri);

	}
	// append monitor demux string to replyto eid
	strcat(perf_opt->mon_eid, MON_EP_STRING);
	// parse
	error = bp_parse_eid_string(&mon_eid, perf_opt->mon_eid);
	if (error != BP_SUCCESS)
	{
		fprintf(stderr, "fatal error parsing bp EID: invalid eid string '%s'\n", perf_opt->dest_eid);
		if (create_log)
			fprintf(log_file, "\nfatal error parsing bp EID: invalid eid string '%s'", perf_opt->dest_eid);
		exit(1);
	}
	if (debug)
		printf("Reply-to   : %s\n\n", mon_eid.uri);

	if (create_log)
		fprintf(log_file, "Reply-to   : %s\n\n", mon_eid.uri);

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
		if (create_log)
			fprintf(log_file, "error creating registration: %d (%s)\n",
					error, bp_strerror(bp_errno(handle)));
		exit(1);
	}
	if ((debug) && (debug_level > 0))
		printf(" done\n");
	if (debug)
		printf("regid 0x%x\n", (unsigned int) regid);
	if (create_log)
		fprintf(log_file, "regid 0x%x\n", (unsigned int) regid);

	// if bundle payload > MAX_MEM_PAYLOAD, then transfer a file
	if (!perf_opt->use_file && perf_opt->bundle_payload > MAX_MEM_PAYLOAD)
	{
		perf_opt->use_file = 1;
		perf_opt->bundle_payload = BP_PAYLOAD_FILE;
		if (verbose)
			printf("Payload %ld > %d: Using file instead of memory\n", perf_opt->bundle_payload, MAX_MEM_PAYLOAD);
		if (create_log)
			fprintf(log_file, "Payload %ld > %d: Using file instead of memory\n", perf_opt->bundle_payload, MAX_MEM_PAYLOAD);
	}

	/* ------------------------------------------------------------------------------
	 * select the operative-mode (between Time_Mode, Data_Mode and File_Mode)
	 * ------------------------------------------------------------------------------ */

	if (perf_opt->op_mode == 'T')	// Time mode
	{

		if (debug)
			printf("Working in Time_Mode\n");

		if (create_log)
			fprintf(log_file, "Working in Time_Mode\n");

		if (debug)
			printf("requested %d second(s) of transmission\n", perf_opt->transmission_time);

		if (create_log)
			fprintf(log_file, "requested %d second(s) of transmission\n", perf_opt->transmission_time);
	}
	else if (perf_opt->op_mode == 'D') // Data mode
	{
		if (debug)
			printf("Working in Data_Mode\n");
		if (create_log)
			fprintf(log_file, "Working in Data_Mode\n");
		if (debug)
			printf("requested transmission of %ld bytes of data\n", perf_opt->data_qty);
		if (create_log)
			fprintf(log_file, "requested transmission of %ld bytes of data\n", perf_opt->data_qty);
	}
	else if (perf_opt->op_mode == 'F') // File mode
	{
		if (debug)
			printf("Working in File_Mode\n");
		if (create_log)
			fprintf(log_file, "Working in File_Mode\n");
		if (debug)
			printf("requested transmission of file %s\n", perf_opt->F_arg);
		if (create_log)
			fprintf(log_file, "requested transmission of file %s\n", perf_opt->F_arg);

	}

	if (debug)
		printf(" transmitting data %s\n", perf_opt->use_file ? "using a file" : "using memory");

	if (create_log)
		fprintf(log_file, " transmitting data %s\n", perf_opt->use_file ? "using a file" : "using memory");


	sent_bundles = 0;

	if (perf_opt->op_mode == 'D' || perf_opt->op_mode == 'F') // Data or File mode
	{
		if ((debug) && (debug_level > 0))
			printf("[debug] calculating how many bundles are needed...");

		if (perf_opt->op_mode == 'F') // File mode
		{
			struct stat file;
			if (stat(perf_opt->F_arg, &file) < 0)
			{
				fprintf(stderr, "couldn't stat file %s : %s", perf_opt->F_arg, strerror(errno));
				if (create_log)
					fprintf(log_file, "couldn't stat file %s : %s", perf_opt->F_arg, strerror(errno));
				exit(1);
			}

			// get transfer file basename
			strcpy(temp1, perf_opt->F_arg);
			strcpy(temp2, basename(temp1));
			transfer_filename = malloc(strlen(temp2) + 1);
			strcpy(transfer_filename, temp2);

			transfer_filedim = file.st_size;
			tot_bundles += bundles_needed(transfer_filedim, get_file_fragment_size(perf_opt->bundle_payload, strlen(transfer_filename)));
		}
		else // Data mode
			tot_bundles += bundles_needed(perf_opt->data_qty, perf_opt->bundle_payload);

		if ((debug) && (debug_level > 0))
			printf(" n_bundles = %ld\n", tot_bundles);

	}


	// Create the file
	if (perf_opt->use_file)
	{
		// create the file
		if ((debug) && (debug_level > 0))
			printf("[debug] creating file %s...", SOURCE_FILE);

		stream = fopen(SOURCE_FILE,	"wb");

		if (stream == NULL)
		{
			fprintf(stderr, "ERROR: couldn't create file %s.\n \b Maybe you don't have permissions\n", SOURCE_FILE);

			if (create_log)
				fprintf(log_file, "ERROR: couldn't create file %s.\n \b Maybe you don't have permissions\n", SOURCE_FILE);

			exit(2);
		}

		fclose(stream);

		if ((debug) && (debug_level > 0))
			printf(" done\n");

		// set the absolute path of the source file
		char buf[256];
		getcwd(buf, 256);
		strcat(buf, "/");
		strcat(buf, SOURCE_FILE);
		source_file_abs = malloc(strlen(buf) + 1);
		strncpy(source_file_abs, buf, strlen(buf) + 1);
	}

	// Create the bundle object
	if ((debug) && (debug_level > 0))
		printf("[debug] creating the bundle object...");
	error = bp_bundle_create(& bundle);
	if (error != BP_SUCCESS)
	{
		fprintf(stderr, "ERROR: couldn't create bundle object\n");

		if (create_log)
			fprintf(log_file, "ERROR: couldn't create bundle object\n");
		exit(1);
	}
	if ((debug) && (debug_level > 0))
		printf(" done\n");

	// Fill the payload
	if ((debug) && (debug_level > 0))
		printf("[debug] filling payload...");

	if (perf_opt->use_file)
		error = bp_bundle_set_payload_file(&bundle, source_file_abs, strlen(source_file_abs));
	else
		error = bp_bundle_set_payload_mem(&bundle, buffer, bufferLen);
	if (error != BP_SUCCESS)
	{
		fprintf(stderr, "ERROR: couldn't set bundle payload\n");

		if (create_log)
			fprintf(log_file, "ERROR: couldn't set bundle payload\n");
		exit(1);
	}
	if ((debug) && (debug_level > 0))
		printf(" done\n");


	// open payload stream in write mode
	if (open_payload_stream_write(bundle, &stream) < 0)
	{
		fprintf(stderr, "ERROR: couldn't open payload stream in write mode");

		if (create_log)
			fprintf(log_file, "ERROR: couldn't open payload stream in write mode");

		exit(2);
	}

	// prepare the payload
	if(perf_opt->op_mode == 'F') // File mode
	{
		// payload will be prepared into send_bundles() cycle

		// open file to transfer in read mode
		if ((transfer_fd = open(perf_opt->F_arg, O_RDONLY)) < 0)
		{
			fprintf(stderr, "couldn't stat file %s : %s", perf_opt->F_arg, strerror(errno));
			if (create_log)
				fprintf(log_file, "couldn't stat file %s : %s", perf_opt->F_arg, strerror(errno));
			exit(2);
		}
	}
	else // Time and Data mode
	{
		error = prepare_generic_payload(perf_opt, stream);
		if (error != BP_SUCCESS)
		{
			fprintf(stderr, "error preparing payload: %s\n", bp_strerror(error));
			if (create_log)
				fprintf(log_file, "error preparing payload: %s\n", bp_strerror(error));
			exit(1);
		}
	}

	// close the stream
	close_payload_stream_write(&bundle, stream);

	if(debug)
		printf("[debug] payload prepared");

	// Create the array for the bundle send info (only for sliding window congestion control)
	if (perf_opt->congestion_ctrl == 'w') {
		if ((debug) && (debug_level > 0))
			printf("[debug] creating structure for sending information...");

		send_info = (send_information_t*) malloc(perf_opt->window * sizeof(send_information_t));
		init_info(send_info, perf_opt->window);

		if ((debug) && (debug_level > 0))
			printf(" done\n");
	}


	// Setting the bundle options
	bp_bundle_set_source(&bundle, local_eid);
	bp_bundle_set_dest(&bundle, dest_eid);
	bp_bundle_set_replyto(&bundle, mon_eid);
	set_bp_options(&bundle, conn_opt);

	if ((debug) && (debug_level > 0))
		printf("[debug] entering in loop\n");

	// Run threads
	if (perf_opt->congestion_ctrl == 'w') // sliding window congestion control
		sem_init(&window, 0, perf_opt->window);
	else								// rate based congestion control
		sem_init(&window, 0, 0);

	pthread_cond_init(&cond_ackreceiver, NULL);
	pthread_mutex_init (&mutexdata, NULL);


	pthread_create(&sender, NULL, send_bundles, (void*)perf_g_opt);
	pthread_create(&cong_ctrl, NULL, congestion_control, (void*)perf_g_opt);

	pthread_join(cong_ctrl, (void**)&pthread_status);
	pthread_join(sender, (void**)&pthread_status);

	pthread_mutex_destroy(&mutexdata);
	sem_destroy(&window);
	pthread_cond_destroy(&cond_ackreceiver);

	if ((debug) && (debug_level > 0))
		printf("[debug] out from loop\n");

	// Get the TOTAL end time
	if ((debug) && (debug_level > 0))
		printf("[debug] getting total end-time...");

	gettimeofday(&end, NULL);

	if ((debug) && (debug_level > 0))
		printf(" end.tv_sec = %u sec\n", (u_int)end.tv_sec);

	// Print final report
	print_final_report(NULL);
	if(perf_opt->create_log)
		print_final_report(log_file);

	// Close the BP handle --
	if ((debug) && (debug_level > 0))
		printf("[debug] closing DTN handle...");

	if (bp_close(handle) != BP_SUCCESS)
	{
		fprintf(stderr, "fatal error closing bp handle: %s\n", strerror(errno));
		if (create_log)
			fprintf(log_file, "fatal error closing bp handle: %s\n", strerror(errno));
		exit(1);
	}

	if ((debug) && (debug_level > 0))
		printf(" done\n");

	if (create_log)
		fclose(log_file);

	// deallocate memory
	if (perf_opt->op_mode == 'F')
	{
		close(transfer_fd);
	}
	free((void*)buffer);
	free(client_demux_string);
	free(source_file_abs);
	free(transfer_filename);
	free(send_info);
	bp_bundle_free(&bundle);

	pthread_exit(NULL);


	// Final carriage return
	printf("\n");

	return;
}




// end client code

/**
 * Client Threads code
 */
void * send_bundles(void * opt)
{
	dtnperf_options_t *perf_opt = ((dtnperf_global_options_t *)(opt))->perf_opt;
	boolean_t debug = perf_opt->debug;
	int debug_level = perf_opt->debug_level;
	boolean_t create_log = perf_opt->create_log;
	boolean_t condition;
	boolean_t eof_reached;
	u32_t actual_payload;
	FILE * stream;

	// Initialize timer
	if ((debug) && (debug_level > 0))
		printf("[debug send thread] initializing timer...");
	if (create_log)
		fprintf(log_file, " initializing timer...");

	gettimeofday(&start, NULL);

	if ((debug) && (debug_level > 0))
		printf(" start.tv_sec = %d sec\n", (u_int)start.tv_sec);
	if (create_log)
		fprintf(log_file, " start.tv_sec = %d sec\n", (u_int)start.tv_sec);

	if (perf_opt->op_mode == 'T')		// TIME MODE
	{									// Calculate end-time
		if ((debug) && (debug_level > 0))
			printf("[debug send thread] calculating end-time...");

		if (create_log)
			fprintf(log_file, " calculating end-time...");

		end = set (0);
		end.tv_sec = start.tv_sec + perf_opt->transmission_time;

		if ((debug) && (debug_level > 0))
			printf(" end.tv_sec = %d sec\n", (u_int)end.tv_sec);

		if (create_log)
			fprintf(log_file, " end.tv_sec = %d sec\n", (u_int)end.tv_sec);
	}

	if ((debug) && (debug_level > 0))
		printf("[debug send thread] entering loop...\n");
	if (create_log)
		fprintf(log_file, " entering loop...\n");

	if (perf_opt->op_mode == 'T') 	// TIME MODE
	{								// init variables for loop and setting condition
		now.tv_sec = start.tv_sec;
		condition = now.tv_sec <= end.tv_sec;
	}
	else							// DATA and FILE MODE
	{								// setting condition for loop
		condition = sent_bundles < tot_bundles;
	}
	while (condition)				//LOOP
	{
		// prepare payload if FILE MODE
		if (perf_opt->op_mode == 'F')
		{
			open_payload_stream_write(bundle, &stream);
			error = prepare_file_transfer_payload(perf_opt, stream, transfer_fd,
					transfer_filename, transfer_filedim, &eof_reached);
			close_payload_stream_write(&bundle, stream);
		}

		// window debug
		if ((debug) && (debug_level > 1))
		{
			int cur;
			sem_getvalue(&window, &cur);
			printf("\t[debug send thread] window is %d\n", cur);
		}
		// wait for the semaphore
		sem_wait(&window);

		// Send the bundle
		if (debug)
			printf("sending the bundle...");

		if (perf_opt->congestion_ctrl == 'w')
			pthread_mutex_lock(&mutexdata);

		if ((error = bp_bundle_send(handle, regid, &bundle)) != 0)
		{
			fprintf(stderr, "error sending bundle: %d (%s)\n", error, bp_strerror(error));
			if (create_log)
				fprintf(log_file, "error sending bundle: %d (%s)\n", error, bp_strerror(error));
			exit(1);
		}
		if ((error = bp_bundle_get_id(bundle, &bundle_id)) != 0)
		{
			fprintf(stderr, "error getting bundle id: %s\n", bp_strerror(error));
			if (create_log)
				fprintf(log_file, "error getting bundle id: %s\n", bp_strerror(error));
			exit(1);
		}
		if (debug)
			printf(" bundle sent\n");
		if ((debug) && (debug_level > 0))
			printf("\t[debug send thread] bundle sent timestamp: %llu.%llu\n", (unsigned long long) bundle_id->creation_ts.secs, (unsigned long long) bundle_id->creation_ts.seqno);
		if (create_log)
			fprintf(log_file, "\t bundle sent timestamp: %llu.%llu\n", (unsigned long long) bundle_id->creation_ts.secs, (unsigned long long) bundle_id->creation_ts.seqno);

		// put bundle id in send_info (only windowed congestion control)
		if (perf_opt->congestion_ctrl == 'w') {
			gettimeofday(&bundle_sent, NULL);
			add_info(send_info, *bundle_id, bundle_sent, perf_opt->window);
			if ((debug) && (debug_level > 1))
				printf("\t[debug send thread] added info for sent bundle\n");
			pthread_cond_signal(&cond_ackreceiver);
			pthread_mutex_unlock(&mutexdata);
		}

		// Increment sent_bundles
		++sent_bundles;

		if ((debug) && (debug_level > 0))
			printf("\t[debug send thread] now bundles_sent is %d\n", sent_bundles);
		if (create_log)
			fprintf(log_file, "\t now bundles_sent is %d\n", sent_bundles);
		// Increment data_qty
		bp_bundle_get_payload_size(bundle, &actual_payload);
		sent_data += actual_payload;

		if (perf_opt->op_mode == 'T')	// TIME MODE
		{								// update time and condition
			gettimeofday(&now, NULL);
			condition = now.tv_sec <= end.tv_sec;
		}
		else							// DATA MODE
		{								// update condition
			condition = sent_bundles < tot_bundles;
		}
	} // while

	if ((debug) && (debug_level > 0))
		printf("[debug send thread] ...out from loop\n");
	if (create_log)
		fprintf(log_file, " ...out from loop\n");

	pthread_mutex_lock(&mutexdata);
	close_ack_receiver = 1;
	if (perf_opt->congestion_ctrl == 'r')
	{
		pthread_kill(cong_ctrl, SIGUSR2);
	}
	else
	{
		pthread_cond_signal(&cond_ackreceiver);
	}
	pthread_mutex_unlock(&mutexdata);
	pthread_exit(NULL);

}

void * congestion_control(void * opt)
{
	dtnperf_options_t *perf_opt = ((dtnperf_global_options_t *)(opt))->perf_opt;
	boolean_t debug = perf_opt->debug;
	int debug_level = perf_opt->debug_level;
	boolean_t create_log = perf_opt->create_log;

	bp_timestamp_t reported_timestamp;
	bp_endpoint_id_t ack_sender;
	bp_copy_eid(&ack_sender, &dest_eid);
	struct timeval temp;

	int position = -1;

	if (debug && debug_level > 0)
		printf("[debug cong crtl] congestion control = %c\n", perf_opt->congestion_ctrl);

	if (perf_opt->congestion_ctrl == 'w') // window based congestion control
	{
		bp_bundle_create(&ack);

		while ((close_ack_receiver == 0) || (gettimeofday(&temp, NULL) == 0 && ack_recvd.tv_sec - temp.tv_sec <= perf_opt->wait_before_exit))
		{
			// if there are no bundles without ack, wait
			pthread_mutex_lock(&mutexdata);
			if (close_ack_receiver == 0 && count_info(send_info, perf_opt->window) == 0)
			{
				pthread_cond_wait(&cond_ackreceiver, &mutexdata);
				pthread_mutex_unlock(&mutexdata);
				// pthread_yield();
				sched_yield();
				continue;
			}

			// Wait for the reply
			if ((debug) && (debug_level > 0))
				printf("\t[debug cong crtl] waiting for the reply...\n");

			if ((error = bp_bundle_receive(handle, ack, BP_PAYLOAD_MEM, count_info(send_info, perf_opt->window) == 0 ? perf_opt->wait_before_exit : -1)) != BP_SUCCESS)
			{
				if(count_info(send_info, perf_opt->window) == 0 && close_ack_receiver == 1)
					// send_bundles is terminated
					break;
				fprintf(stderr, "error getting server ack: %d (%s)\n", error, bp_strerror(bp_errno(handle)));
				if (create_log)
					fprintf(log_file, "error getting server ack: %d (%s)\n", error, bp_strerror(bp_errno(handle)));
				exit(1);
			}
			gettimeofday(&ack_recvd, NULL);
			if ((debug) && (debug_level > 0))
				printf("\t[debug cong crtl] ack received\n");

			// Get ack infos
			error = get_info_from_ack(&ack, &reported_timestamp);
			if (error != BP_SUCCESS)
			{
				fprintf(stderr, "error getting info from ack: %s\n", bp_strerror(error));
				if (create_log)
					fprintf(log_file, "error getting info from ack: %s\n", bp_strerror(error));
				exit(1);
			}
			if ((debug) && (debug_level > 1))
				printf("\t[debug cong crtl] ack received timestamp: %lu %lu\n", reported_timestamp.secs, reported_timestamp.seqno);
			position = is_in_info(send_info, reported_timestamp, perf_opt->window);
			if (position < 0)
			{
				fprintf(stderr, "error removing bundle info\n");
				if (create_log)
					fprintf(log_file, "error removing bundle info\n");
				exit(1);
			}
			remove_from_info(send_info, position);
			if ((debug) && (debug_level > 0))
				printf("\t[debug cong crtl] ack validated\n");
			sem_post(&window);
			if ((debug) && (debug_level > 1))
			{
				int cur;
				sem_getvalue(&window, &cur);
				printf("\t[debug cong crtl] window is %d\n", cur);
			}

			pthread_mutex_unlock(&mutexdata);
			//pthread_yield();
			sched_yield();
		} // end while(n_bundles)

		bp_bundle_free(&ack);
	}
	else if (perf_opt->congestion_ctrl == 'r') // Rate based congestion control
	{
		double interval_secs;
		struct timespec interval, remaining;

		if (perf_opt->rate_unit == 'b') // rate is bundles per second
		{
			interval_secs = 1.0 / perf_opt->rate;
		}
		else 							// rate is bit or kbit per second
		{
			if (perf_opt->rate_unit == 'k') // Rate is kbit per second
			{
				perf_opt->rate = kilo2byte(perf_opt->rate);
			}
			else // rate is Mbit per second
			{
				perf_opt->rate = mega2byte(perf_opt->rate);
			}
			interval_secs = (double)perf_opt->bundle_payload * 8 / perf_opt->rate;
		}

		interval.tv_sec = (long) interval_secs;
		interval.tv_nsec = (long) ((interval_secs - interval.tv_sec) * 1000 * 1000 * 1000);

		if (debug && debug_level > 0)
				printf("[debug cong crtl] wait time for each bundle: %.4f sec\n", interval_secs);

		pthread_mutex_lock(&mutexdata);
		while(close_ack_receiver == 0)
		{
			pthread_mutex_unlock(&mutexdata);
			sem_post(&window);
			//pthread_yield();
			sched_yield();
			if (debug && debug_level > 0)
					printf("[debug cong crtl] increased window size\n");
			nanosleep(&interval, &remaining);
			pthread_mutex_lock(&mutexdata);
		}
	}
	else // wrong char for congestion control
	{
		exit(1);
	}

	pthread_exit(NULL);
	return NULL;
}

void print_final_report(FILE * f)
{
	double goodput, sent;
	struct timeval total;
	double total_secs;
	char * gput_unit, * sent_unit;
	if (f == NULL)
		f = stdout;
	timersub(&end, &start, &total);
	total_secs = (((double)total.tv_sec * 1000 *1000) + (double)total.tv_usec) / (1000 * 1000);

	if (sent_data / (1000 * 1000) >= 1)
	{
		sent = (double) sent_data / (1000 * 1000);
		sent_unit = "Mbytes";
	}
	else if (sent_data / 1000 >= 1)
	{
		sent = (double) sent_data / 1000;
		sent_unit = "Kbytes";
	}
	else
		sent_unit = "bytes";

	goodput = sent_data * 8 / total_secs;
	if (goodput / (1000 * 1000) >= 1)
	{
		goodput /= 1000 * 1000;
		gput_unit = "Mbit/sec";
	}
	else if (goodput / 1000 >= 1)
	{
		goodput /= 1000;
		gput_unit = "Kbit/sec";
	}
	else
		gput_unit = "bit/sec";

	fprintf(f, "Sent %d bundles, total sent data = %.3f %s\n", sent_bundles, sent, sent_unit);
	fprintf(f, "Total execution time = %.1f\n", total_secs);
	fprintf(f, "Goodput = %.3f %s\n", goodput, gput_unit);
}


void print_client_usage(char* progname)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "DtnPerf3 client mode\n");
	fprintf(stderr, "SYNTAX: %s --client -d <dest_eid> <[-T <sec> | -D <num> | -F <filename]> [-w <size> | -r <rate>] [options]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "options:\n"
			" -d, --destination <eid>     Destination eid (required).\n"
			" -m, --monitor <eid>         Monitor eid. Default is same as local eid.\n"
			" -T, --time <seconds>        Time-mode: seconds of transmission.\n"
			" -D, --data <num[B|k|M]>     Data-mode: bytes to transmit; B = Bytes, k = kBytes, M = MBytes. Default 'M' (MB).\n"
			" -F, --file <filename>       File-mode: file to transfer\n"
			" -w, --window <size>         Size of transmission window, i.e. max number of bundles \"in flight\" (not still ACKed by a server ack); default =1.\n"
			" -r, --rate <rate[k|M|b]>    Bitrate of transmission. k = kbit/s, M = Mbit/s, b = bundles/s. Default is kb/s\n"
			" -C, --custody               Enable both custody transfer and \"custody accepted\" status reports.\n"
			" -i, --exitinterval <sec>    Additional interval before exit.\n"
			" -p, --payload <size[B|k|M]> Size of bundle payloads; B = Bytes, k = kBytes, M = MBytes. Default= 'k' (kB).\n"
			" -u, --nofragment            Disable bundle fragmentation.\n"
			" -M, --memory                Store the bundle into memory instead of file (if payload < 50KB).\n"
			" -L, --log[=log_filename]    Create a log file. Default log filename is %s\n"
			"     --ip-addr <addr>        Ip address of the bp daemon api. Default is 127.0.0.1\n"
			"     --ip-port <port>        Ip port of the bp daemon api. Default is 5010\n"
			"     --debug[=level]         Debug messages [0-1], if level is not indicated assume level=2.\n"
			" -e, --expiration <time>     Bundle acks expiration time. Default is 3600\n"
			" -P, --priority <val>        Bundle acks priority [bulk|normal|expedited|reserved]. Default is normal\n"
			" -v, --verbose               Print some information messages during the execution.\n"
			" -h, --help                  This help.\n",
			LOG_FILENAME);
	fprintf(stderr, "\n");
	exit(1);
}

void parse_client_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt)
{
	char c, done = 0;
	dtnperf_options_t * perf_opt = perf_g_opt->perf_opt;
	dtnperf_connection_options_t * conn_opt = perf_g_opt->conn_opt;
	boolean_t w = FALSE, r = FALSE;

	while (!done)
	{
		static struct option long_options[] =
		{
				{"help", no_argument, 0, 'h'},
				{"verbose", no_argument, 0, 'v'},
				{"memory", no_argument, 0, 'M'},
				{"custody", no_argument, 0, 'C'},
				{"window", required_argument, 0, 'w'},
				{"destination", required_argument, 0, 'd'},
				{"monitor", required_argument, 0, 'm'},
				{"exitinterval", required_argument, 0, 'i'},			// interval before exit
				{"time", required_argument, 0, 'T'},			// time mode
				{"data", required_argument, 0, 'D'},			// data mode
				{"file", required_argument, 0, 'F'},			// file mode
				{"payload", required_argument, 0, 'p'},
				{"expiration", required_argument, 0, 'e'},
				{"rate", required_argument, 0, 'r'},
				{"debug", optional_argument, 0, 33}, 				// 33 because D is for data mode
				{"priority", required_argument, 0, 'P'},
				{"log", optional_argument, 0, 'L'},				// create log file
				{"ip-addr", required_argument, 0, 37},
				{"ip-port", required_argument, 0, 38},
				{0,0,0,0}	// The last element of the array has to be filled with zeros.

		};

		int option_index = 0;
		c = getopt_long(argc, argv, "hvMCw:d:m:i:T:D:F:p:e:r:P:L::", long_options, &option_index);

		switch (c)
		{
		case 'h':
			print_client_usage(argv[0]);
			exit(0);
			return ;

		case 'v':
			perf_opt->verbose = TRUE;
			break;

		case 'M':
			perf_opt->use_file = 0;
			perf_opt->payload_type = BP_PAYLOAD_MEM;
			break;

		case 'C':
			conn_opt->custody_transfer = 1;
			conn_opt->custody_receipts = 1;
			break;

		case 'w':
			perf_opt->congestion_ctrl = 'w';
			perf_opt->window = atoi(optarg);
			w = TRUE;
			break;

		case 'd':
			strncpy(perf_opt->dest_eid, optarg, BP_MAX_ENDPOINT_ID);
			break;

		case 'm':
			strncpy(perf_opt->mon_eid, optarg, BP_MAX_ENDPOINT_ID);
			break;

		case 'i':
			perf_opt->wait_before_exit = atoi(optarg)*1000;
			break;

		case 'T':
			perf_opt->op_mode = 'T';
			perf_opt->transmission_time = atoi(optarg);
			break;

		case 'D':
			perf_opt->op_mode = 'D';
			perf_opt->D_arg = strdup(optarg);
			perf_opt->data_unit = find_data_unit(perf_opt->D_arg);

			switch (perf_opt->data_unit)
			{
			case 'B':
				perf_opt->data_qty = atol(perf_opt->D_arg);
				break;
			case 'k':
				perf_opt->data_qty = kilo2byte(atol(perf_opt->D_arg));
				break;
			case 'M':
				perf_opt->data_qty = mega2byte(atol(perf_opt->D_arg));
				break;
			default:
				printf("\nWARNING: (-D option) invalid data unit, assuming 'M' (MBytes)\n\n");
				perf_opt->data_qty = mega2byte(atol(perf_opt->D_arg));
				break;
			}
			break;

		case 'F':
			perf_opt->op_mode = 'F';
			perf_opt->F_arg = strdup(optarg);
			if(!file_exists(perf_opt->F_arg))
			{
				fprintf(stderr, "Unable to open file %s: ", perf_opt->F_arg);
				perror("");
				exit(1);
			}
			break;

		case 'p':
			perf_opt->p_arg = optarg;
			perf_opt->data_unit = find_data_unit(perf_opt->p_arg);
			switch (perf_opt->data_unit)
			{
			case 'B':
				perf_opt->bundle_payload = atol(perf_opt->p_arg);
				break;
			case 'k':
				perf_opt->bundle_payload = kilo2byte(atol(perf_opt->p_arg));
				break;
			case 'M':
				perf_opt->bundle_payload = mega2byte(atol(perf_opt->p_arg));

				break;
			default:
				printf("\nWARNING: (-p option) invalid data unit, assuming 'K' (KBytes)\n\n");
				perf_opt->bundle_payload = kilo2byte(atol(perf_opt->p_arg));
				break;
			}
			break;

		case 'e':
			conn_opt->expiration = atoi(optarg);
			break;

		case 'r':
			perf_opt->rate_arg = strdup(optarg);
			perf_opt->rate_unit = find_rate_unit(perf_opt->rate_arg);
			perf_opt->rate = atoi(perf_opt->rate_arg);
			perf_opt->congestion_ctrl = 'r';
			r = TRUE;
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

		case 'L':
			perf_opt->create_log = TRUE;
			if (optarg != NULL)
				perf_opt->log_filename = strdup(optarg);
			break;

		case 33: // debug
			perf_opt->debug = TRUE;
			if (optarg){
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

		case 34: // incoming bundle destination directory
			perf_opt->dest_dir = strdup(optarg);
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
			print_client_usage(argv[0]);
			exit(1);
		} // --switch
	} // -- while

	// if mode is client and monitor eid is not specified, client node must be also monitor.
	if(perf_g_opt->mode == DTNPERF_CLIENT && perf_opt->mon_eid == NULL)
	{
		perf_g_opt->mode = DTNPERF_CLIENT_MONITOR;
	}


#define CHECK_SET(_arg, _what)                                          	\
		if (_arg == 0) {                                                    	\
			fprintf(stderr, "\nSYNTAX ERROR: %s must be specified\n", _what);   \
			print_client_usage(argv[0]);                                               \
			exit(1);                                                        	\
		}

	CHECK_SET(perf_opt->dest_eid[0], "destination eid");
	CHECK_SET(perf_opt->op_mode, "-T or -D or -F");

	if (w && r)
	{
		fprintf(stderr, "\nSYNTAX ERROR: -w and -r options are mutually exclusive\n");   \
		print_client_usage(argv[0]);                                               \
		exit(1);
	}

}

void handler(int sig)
{
	(void)sig;
	bp_close(handle);
	if(log_file != NULL)
		fclose(log_file);


	exit(0);
}
