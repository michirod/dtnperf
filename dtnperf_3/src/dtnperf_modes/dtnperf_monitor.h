/*
 * dtnperf_monitor.h
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */

#ifndef DTNPERF_MONITOR_H_
#define DTNPERF_MONITOR_H_

#include "../dtnperf_types.h"
#include <stdio.h>

typedef struct monitor_parameters
{
	dtnperf_global_options_t * perf_g_opt;
	boolean_t dedicated_monitor;
	int client_id;
} monitor_parameters_t;

typedef enum
{
	NONE,
	STATUS_REPORT,
	SERVER_ACK,
	CLIENT_START,
	CLIENT_STOP
} bundle_type_t;

typedef struct session
{
	bp_endpoint_id_t client_eid;
	FILE * file;
	struct timeval * start;
	struct session * next;
	struct session * prev;
}session_t;

typedef struct session_list
{
	session_t * first;
	session_t * last;
	int count;
}session_list_t;

session_list_t session_list_create();
void session_list_destroy(session_list_t * list);

session_t * session_create(bp_endpoint_id_t client_eid, FILE * file, struct timeval start);
void session_destroy(session_t * session);

void session_put(session_list_t * list, session_t * session);

session_t * session_get(session_list_t * list, bp_endpoint_id_t client);

void session_del(session_list_t * list, session_t * session);

void run_dtnperf_monitor(monitor_parameters_t * parameters);
void print_monitor_usage(char* progname);
void parse_monitor_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt);

#endif /* DTNPERF_MONITOR_H_ */
