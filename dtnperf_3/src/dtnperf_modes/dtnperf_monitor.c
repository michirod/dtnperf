/*
 * dtnperf_monitor.c
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */

#include <bp_errno.h>
#include <bp_abstraction_api.h>

#include "dtnperf_monitor.h"
#include "../includes.h"
#include "../utils.h"
#include "../bundle_tools.h"
#include "../csv_tools.h"

/*
 * Global Variables
 */

session_list_t * session_list;
bp_handle_t handle;

// flags to exit cleanly
boolean_t dedicated_monitor;
boolean_t bp_handle_open;


/*  ----------------------------
 *          MONITOR CODE
 *  ---------------------------- */
void run_dtnperf_monitor(monitor_parameters_t * parameters)
{
	/* ------------------------
	 * variables
	 * ------------------------ */
	dtnperf_options_t * perf_opt = parameters->perf_g_opt->perf_opt;

	bp_error_t error;
	bp_endpoint_id_t local_eid;
	bp_reg_info_t reginfo;
	bp_reg_id_t regid;
	bp_bundle_object_t bundle_object;
	bp_bundle_status_report_t * status_report;
	bp_endpoint_id_t bundle_source_addr;
	bp_timestamp_t bundle_creation_timestamp;
	bp_endpoint_id_t relative_source_addr;
	bp_timestamp_t relative_creation_timestamp;

	session_t * session;
	bundle_type_t bundle_type;
	struct timeval current, start;
	char * command;
	char temp[256];
	char * filename;
	int filename_len;
	char * full_filename;
	FILE * file;


	/* ------------------------
	 * initialize variables
	 * ------------------------ */
	boolean_t debug = perf_opt->debug;
	int debug_level = perf_opt->debug_level;

	dedicated_monitor = parameters->dedicated_monitor;
	bp_handle_open = FALSE;

	perf_opt->logs_dir = correct_dirname(perf_opt->logs_dir);

	status_report = NULL;
	session_list = session_list_create();


	// create dir where dtnperf monitor will save logs
	// command should be: mkdir -p "logs_dir"
	if(debug && debug_level > 0)
		printf("[debug] initializing shell command...");
	command = malloc(sizeof(char) * (10 + strlen(perf_opt->logs_dir)));
	sprintf(command, "mkdir -p %s", perf_opt->logs_dir);
	if(debug && debug_level > 0)
		printf("done. Shell command = %s\n", command);

	// execute shell command
	if(debug && debug_level > 0)
		printf("[debug] executing shell command...");
	if (system(command) < 0)
	{
		perror("Error opening monitor logs dir");
		exit(-1);
	}
	free(command);
	if(debug && debug_level > 0)
		printf("done\n");

	// signal handlers
	signal(SIGINT, monitor_handler);
	signal(SIGUSR1, monitor_handler);

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
	else
	{
		bp_handle_open = TRUE;
	}
	if(debug && debug_level > 0)
		printf("done\n");

	//build a local eid
	if(debug && debug_level > 0)
		printf("[debug] building a local eid...");
	if (parameters->dedicated_monitor)
		sprintf(temp, "%s_%d", MON_EP_STRING, parameters->client_id);
	else
		sprintf(temp, "%s", MON_EP_STRING);
	bp_build_local_eid(handle, &local_eid, temp);
	if(debug && debug_level > 0)
		printf("done\n");
	if (debug)
		printf("local_eid = %s\n", local_eid.uri);

	// checking if there is already a registration
	if(debug && debug_level > 0)
		printf("[debug] checking for existing registration...");
	error = bp_find_registration(handle, &local_eid, &regid);
	if (error == BP_SUCCESS)
	{
		fflush(stdout);
		fprintf(stderr, "error: there is a registration with the same eid.\n");
		fprintf(stderr, "regid 0x%x\n", (unsigned int) regid);
		bp_close(handle);
		exit(1);
	}
	if ((debug) && (debug_level > 0))
		printf(" done\n");

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

	// start infinite loop
	while(1)
	{
		// reset variables
		bundle_type = NONE;

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


		// wait until receive a bundle
		if ((debug) && (debug_level > 0))
			printf("[debug] waiting for bundles...\n");
		error = bp_bundle_receive(handle, bundle_object, BP_PAYLOAD_FILE, -1);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error getting recv reply: %d (%s)\n",
					error, bp_strerror(bp_errno(handle)));
			exit(1);
		}
		if ((debug) && (debug_level > 0))
			printf(" bundle received\n");

		// mark current time
		if ((debug) && (debug_level > 0))
			printf("[debug] marking time...");
		gettimeofday(&current, NULL);
		if ((debug) && (debug_level > 0))
			printf(" done\n");

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

		// check if bundle is a status report
		if ((debug) && (debug_level > 0))
			printf("[debug] check if bundle is a status report...\n");
		error = bp_bundle_get_status_report(bundle_object, &status_report);
		if (error != BP_SUCCESS)
		{
			fflush(stdout);
			fprintf(stderr, "error checking if bundle is a status report: %d (%s)\n",
					error, bp_strerror(bp_errno(handle)));
			continue;
		}
		if ((debug) && (debug_level > 0))
			printf(" %s\n", status_report == NULL ? "no" : "yes");

		// check for other bundle types
		if (status_report != NULL)
			bundle_type = STATUS_REPORT;
		else if (is_header(&bundle_object, START_HEADER))
			bundle_type = CLIENT_START;
		else if (is_header(&bundle_object, STOP_HEADER))
			bundle_type = CLIENT_STOP;
		else if (is_header(&bundle_object, DSA_HEADER))
			bundle_type = SERVER_ACK;
		else // unknown bundle type
		{
			fprintf(stderr, "error: unknown bundle type\n");
			continue;
		}

		// retrieve or open log file
		if (bundle_type == CLIENT_START)
		{
			// mark start time
			start = current;
			filename_len = strlen(bundle_source_addr.uri) - strlen("dtn://") + 15;
			filename = (char *) malloc(filename_len);
			memset(filename, 0, filename_len);
			strncpy(temp, bundle_source_addr.uri, strlen(bundle_source_addr.uri) + 1);
			strtok(temp, "/");
			sprintf(filename, "%lu_", bundle_creation_timestamp.secs);
			strcat(filename, strtok(NULL, "/"));
			strcat(filename, ".csv");
			full_filename = (char *) malloc(strlen(perf_opt->logs_dir) + strlen(filename) + 2);
			sprintf(full_filename, "%s/%s", perf_opt->logs_dir, filename);
			file = fopen(full_filename, "w");
			session = session_create(bundle_source_addr, full_filename, file, start);
			session_put(session_list, session);
			// write header in csv log file
			fprintf(file,"RX_TIME;BUNDLE_SOURCE;BUNDLE_TIMESTAMP;BUNDLE_SEQNO;"
					"BUNDLE_TYPE;REL_SOURCE;REL_TIMESTAMP;REL_SEQNO;"
					"REL_FRAG_OFFSET;REL_PAYLOAD;");
			csv_print_status_report_timestamps_header(file);
			csv_end_line(file);

		}
		else
		{
			if (bundle_type == STATUS_REPORT)
			{
				bp_copy_eid(&relative_source_addr, &(status_report->bundle_id.source));
				relative_creation_timestamp = status_report->bundle_id.creation_ts;
				session = session_get(session_list, status_report->bundle_id.source);
			}
			else if (bundle_type == SERVER_ACK)
			{
				get_info_from_ack(&bundle_object, &relative_source_addr, &relative_creation_timestamp);
				session = session_get(session_list, relative_source_addr);
			}
			else
				session = session_get(session_list, bundle_source_addr);
			if (session == NULL)
			{
				fprintf(stderr, "error: bundle received out of session\n");
				continue;
			}
			file = session->file;
			memcpy(&start, session->start, sizeof(struct timeval));
		}

		// print rx time in csv log
		csv_print_rx_time(file, current, start);

		// print bundle source in csv log
		csv_print_eid(file, bundle_source_addr);

		//print bundle creation timestamp in csv log
		csv_print_timestamp(file, bundle_creation_timestamp);

		// print bundle type in csv log
		switch (bundle_type)
		{
		case CLIENT_START:
			csv_print(file, "CLIENT_START");
			break;
		case CLIENT_STOP:
			csv_print(file, "CLIENT_STOP");
			break;
		case SERVER_ACK:
			csv_print(file, "SERVER_ACK");
			break;
		case STATUS_REPORT:
			csv_print(file, "STATUS_REPORT");
			break;
		default:
			csv_print(file, "UNKNOWN");
			break;
		}

		// print relative source and timestamp
		if (bundle_type == SERVER_ACK || bundle_type == STATUS_REPORT)
		{
			csv_print_eid(file, relative_source_addr);
			csv_print_timestamp(file, relative_creation_timestamp);
		}

		// print status report infos in csv log
		if (bundle_type == STATUS_REPORT)
		{
			csv_print_ulong(file, status_report->bundle_id.frag_offset);
			csv_print_ulong(file, status_report->bundle_id.orig_length);
			csv_print_status_report_timestamps(file, * status_report);
		}

		// end line in csv log
		csv_end_line(file);

		// close file
		if (bundle_type == CLIENT_STOP)
		{
			fclose(file);
			fprintf(stdout, "\nDTNperf monitor: saved log file: %s\n", session->full_filename);
			session_del(session_list, session);

			// close monitor if dedicated
			if (parameters->dedicated_monitor)
			{
				break;
			}
		}


	} // end loop

	session_list_destroy(session_list);
	bp_close(handle);
	bp_handle_open = FALSE;
	exit(0);

}
// end monitor code

void print_monitor_usage(char * progname)
{
	//printf("ERROR: dtnperf3 monitor operative mode not yet implemented\n");
	//exit(1);
}

void parse_monitor_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt)
{
	//printf("ERROR: dtnperf3 monitor operative mode not yet implemented\n");
	//exit(1);
}

session_list_t * session_list_create()
{
	session_list_t * list;
	list = (session_list_t *) malloc(sizeof(session_list_t));
	list->first = NULL;
	list->last = NULL;
	list->count = 0;
	return list;
}
void session_list_destroy(session_list_t * list)
{
	free(list);
}

session_t * session_create(bp_endpoint_id_t client_eid, char * full_filename, FILE * file, struct timeval start)
{
	session_t * session;
	session = (session_t *) malloc(sizeof(session_t));
	session->start = (struct timeval *) malloc(sizeof(struct timeval));
	bp_copy_eid(&(session->client_eid), &client_eid);
	session->full_filename = strdup(full_filename);
	session->file = file;
	memcpy(session->start, &start, sizeof(struct timeval));
	session->next = NULL;
	session->prev = NULL;
	return session;
}
void session_destroy(session_t * session)
{
	free(session);
}

void session_put(session_list_t * list, session_t * session)
{
	if (list->first == NULL) // empty list
	{
		list->first = session;
		list->last = session;
	}
	else
	{
		session->prev = list->last;
		list->last->next = session;
		list->last = session;
	}
	list->count ++;

}

session_t *  session_get(session_list_t * list, bp_endpoint_id_t client)
{
	session_t * item = list->first;
	while (item != NULL)
		{
			if (strcmp(item->client_eid.uri, client.uri) == 0)
			{
				return item;
			}
			item = item->next;
		}
		return NULL;
}

void session_del(session_list_t * list, session_t * session)
{
	if (session->next == NULL && session->prev == NULL) // unique element of the list
	{
		list->first = NULL;
		list->last = NULL;
	}
	else if (session->next == NULL) // last item of list
	{
		list->last = session->prev;
		session->prev->next = NULL;
	}
	else if (session->prev == NULL) // first item of the list
	{
		list->first = session->next;
		session->next->prev = NULL;
	}
	else // generic element of list
	{
		session->next->prev = session->prev;
		session->prev->next = session->next;
	}
	session_destroy(session);
	list->count --;

}

void monitor_clean_exit(int status)
{
	session_t * session;

	// close all log files and delete all sessions
	if (dedicated_monitor)
	{
		session = session_list->first;
		fclose(session->file);
		fprintf(stdout, "\nDTNperf monitor: saved log file: %s\n", session->full_filename);
		session_del(session_list, session);
	}
	else
	{
		for (session = session_list->first; session != NULL; session = session->next)
		{
			fclose(session->file);
			if (session->prev != NULL)
				session_destroy(session->prev);
			if (session->next == NULL)
			{
				session_destroy(session);
				break;
			}
		}
	}

	session_list_destroy(session_list);

	// close bp_handle
	if (bp_handle_open)
		bp_close(handle);
	printf("Dtnperf Monitor: exit.\n");
	exit(status);
}

void monitor_handler(int signo)
{
	if (dedicated_monitor)
	{
		if (signo == SIGUSR1)
		{
			printf("\nDtnperf monitor: received signal from client. Exiting\n");
			monitor_clean_exit(0);
		}
	}
	else
	{
		if (signo == SIGINT)
		{
			printf("\nDtnperf monitor: received SIGINT. Exiting\n");
			monitor_clean_exit(0);
		}
	}
}
