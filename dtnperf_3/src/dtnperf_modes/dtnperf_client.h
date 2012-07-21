/*
 * dtnperf_client.h
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */

#ifndef DTNPERF_CLIENT_H_
#define DTNPERF_CLIENT_H_

#include "../dtnperf_types.h"
#include <stdio.h>

void run_dtnperf_client(dtnperf_global_options_t * global_options);
void print_client_usage(char* progname);
void parse_client_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt);
void print_final_report(FILE * f);
void handler(int sig);

//pthread functions
void * send_bundles(void * opt);
void * congestion_control(void * opt);
#endif /* DTNPERF_CLIENT_H_ */
