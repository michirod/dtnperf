/*
 * dtnperf_server.h
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */

#ifndef DTNPERF_SERVER_H_
#define DTNPERF_SERVER_H_

#include <bp_abstraction_api.h>
#include "../dtnperf_modes.h"
#include "../includes.h"
#include "../definitions.h"

void run_dtnperf_server(dtnperf_global_options_t * global_options );
void print_server_usage(char* progname);
void parse_server_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt);
bp_error_t prepare_server_ack_payload(dtnperf_server_ack_payload_t ack, char ** payload, size_t * payload_size);

#endif /* DTNPERF_SERVER_H_ */
