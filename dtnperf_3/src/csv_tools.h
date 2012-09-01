/*
 * csv_tools.h
 *
 *  Created on: 30/ago/2012
 *      Author: michele
 */

#ifndef CSV_TOOLS_H_
#define CSV_TOOLS_H_

void csv_print_rx_time(FILE * file, struct timeval time, struct timeval start_time);
void csv_print_eid(FILE * file, bp_endpoint_id_t eid);
void csv_print_timestamp(FILE * file, bp_timestamp_t timestamp);
void csv_print_status_report_flags_header(FILE * file);
void csv_print_status_report_flags(FILE * file, bp_status_report_flags_t flags);
void csv_print_status_report_timestamps_header(FILE * file);
void csv_print_status_report_timestamps(FILE * file, bp_bundle_status_report_t status_report);
void csv_print(FILE * file, char * string);
void csv_end_line(FILE * file);

#endif /* CSV_TOOLS_H_ */
