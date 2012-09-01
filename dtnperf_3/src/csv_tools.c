/*
 * csv_tools.c
 *
 *  Created on: 29/ago/2012
 *      Author: michele
 */

#include "utils.h"

#include <bp_abstraction_api.h>

void csv_print_rx_time(FILE * file, struct timeval time, struct timeval start_time)
{
	struct timeval * result = malloc(sizeof(struct timeval));
	char buf[50];
	sub_time(time, start_time, result);
	sprintf(buf, "%ld.%ld;", result->tv_sec, result->tv_usec);
	fwrite(buf, strlen(buf), 1, file);
}

void csv_print_eid(FILE * file, bp_endpoint_id_t eid)
{
	char buf[256];
	sprintf(buf, "%s;", eid.uri);
	fwrite(buf, strlen(buf), 1, file);
}

void csv_print_timestamp(FILE * file, bp_timestamp_t timestamp)
{
	char buf[50];
	sprintf(buf, "%lu;%lu;", timestamp.secs, timestamp.seqno);
	fwrite(buf, strlen(buf), 1, file);
}

void csv_print_status_report_flags_header(FILE * file)
{
	char buf[256];
	memset(buf, 0, 256);
	strcat(buf, "STATUS_RECEIVED;");
	strcat(buf, "STATUS_CUSTODY_ACCEPTED;");
	strcat(buf, "STATUS_FORWARDED;");
	strcat(buf, "STATUS_DELETED;");
	strcat(buf, "STATUS_DELIVERED;");
	strcat(buf, "STATUS_ACKED_BY_APP;");
	fwrite(buf, strlen(buf), 1, file);
}
void csv_print_status_report_flags(FILE * file, bp_status_report_flags_t flags)
{
	char buf[256];
	memset(buf, 0, 256);
	strcat(buf, flags & BP_STATUS_RECEIVED ? "STATUS_RECEIVED" : " ");
	strcat(buf, ";");
	strcat(buf, flags & BP_STATUS_CUSTODY_ACCEPTED ? "STATUS_CUSTODY_ACCEPTED" : " ");
	strcat(buf, ";");
	strcat(buf, flags & BP_STATUS_FORWARDED ? "STATUS_FORWARDED" : " ");
	strcat(buf, ";");
	strcat(buf, flags & BP_STATUS_DELETED ? "STATUS_DELETED" : " ");
	strcat(buf, ";");
	strcat(buf, flags & BP_STATUS_DELIVERED ? "STATUS_DELIVERED" : " ");
	strcat(buf, ";");
	strcat(buf, flags & BP_STATUS_ACKED_BY_APP ? "STATUS_ACKED_BY_APP" : " ");
	strcat(buf, ";");
	fwrite(buf, strlen(buf), 1, file);
}

void csv_print_status_report_timestamps_header(FILE * file)
{
	char buf[300];
	memset(buf, 0, 300);
	strcat(buf, "RECEIVED_TIMESTAMP;RECEIVED_SEQNO;");
	strcat(buf, "CUSTODY_ACCEPTED_TIMESTAMP;CUSTODY_ACCEPTED_SEQNO;");
	strcat(buf, "FORWARDED_TIMESTAMP;FORWARDED_SEQNO;");
	strcat(buf, "DELETED_TIMESTAMP;DELETED_SEQNO;");
	strcat(buf, "DELIVERED_TIMESTAMP;DELIVERED_SEQNO;");
	strcat(buf, "ACKED_BY_APP_TIMESTAMP;ACKED_BY_APP_SEQNO;");
	fwrite(buf, strlen(buf), 1, file);
}
void csv_print_status_report_timestamps(FILE * file, bp_bundle_status_report_t status_report)
{
	char buf1[256];
	char buf2[50];
	memset(buf1, 0, 256);
	if (status_report.flags & BP_STATUS_RECEIVED)
		sprintf(buf2, "%lu;%lu;", status_report.receipt_ts.secs, status_report.receipt_ts.seqno);
	else
		sprintf(buf2, " ; ;");
	strcat(buf1, buf2);

	if (status_report.flags & BP_STATUS_CUSTODY_ACCEPTED)
		sprintf(buf2, "%lu;%lu;", status_report.custody_ts.secs, status_report.custody_ts.seqno);
	else
		sprintf(buf2, " ; ;");
	strcat(buf1, buf2);

	if (status_report.flags & BP_STATUS_FORWARDED)
		sprintf(buf2, "%lu;%lu;", status_report.forwarding_ts.secs, status_report.forwarding_ts.seqno);
	else
		sprintf(buf2, " ; ;");
	strcat(buf1, buf2);

	if (status_report.flags & BP_STATUS_DELETED)
		sprintf(buf2, "%lu;%lu;", status_report.deletion_ts.secs, status_report.deletion_ts.seqno);
	else
		sprintf(buf2, " ; ;");
	strcat(buf1, buf2);

	if (status_report.flags & BP_STATUS_DELIVERED)
		sprintf(buf2, "%lu;%lu;", status_report.delivery_ts.secs, status_report.delivery_ts.seqno);
	else
		sprintf(buf2, " ; ;");
	strcat(buf1, buf2);

	if (status_report.flags & BP_STATUS_ACKED_BY_APP)
		sprintf(buf2, "%lu;%lu;", status_report.ack_by_app_ts.secs, status_report.ack_by_app_ts.seqno);
	else
		sprintf(buf2, " ; ;");
	strcat(buf1, buf2);

	fwrite(buf1, strlen(buf1), 1, file);
}

void csv_print(FILE * file, char * string)
{
	char buf[256];
	memset(buf, 0, 256);
	strcat(buf, string);
	if (buf[strlen(buf) -1] != ';')
		strcat(buf, ";");
	fwrite(buf, strlen(buf), 1, file);
}

void csv_end_line(FILE * file)
{
	char c = '\n';
	fwrite(&c, 1, 1, file);
}

