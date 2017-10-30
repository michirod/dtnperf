/*
 * csv_tools.h
 *
 *  Created on: 30/ago/2012
 *      Author: michele
 */

#ifndef CSV_TOOLS_H_
#define CSV_TOOLS_H_

#include <al_bp_types.h>

void csv_print_rx_time(FILE * file, struct timeval time, struct timeval start_time);
void csv_print_eid(FILE * file, al_bp_endpoint_id_t eid);
void csv_print_timestamp(FILE * file, al_bp_timestamp_t timestamp);
void csv_print_status_report_timestamps_header(FILE * file);
void csv_print_status_report_timestamps(FILE * file, al_bp_bundle_status_report_t status_report);
void csv_print_long(FILE * file, long num);
void csv_print_ulong(FILE * file, unsigned long num);
void csv_print(FILE * file, char * string);
void csv_end_line(FILE * file);

#endif /* CSV_TOOLS_H_ */
