/*
 * dtnperf_monitor.h
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */

#ifndef DTNPERF_MONITOR_H_
#define DTNPERF_MONITOR_H_

#include "../dtnperf_modes.h"

void run_dtnperf_monitor(dtnperf_global_options_t * global_options);
void print_monitor_usage(char* progname);
void parse_monitor_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt);

#endif /* DTNPERF_MONITOR_H_ */
