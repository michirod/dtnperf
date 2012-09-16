#ifndef BUNDLE_TOOLS_H_
#define BUNDLE_TOOLS_H_

#include "includes.h"
#include "dtnperf_types.h"
#include <bp_types.h>
#include <bp_errno.h>


typedef struct
{
    bp_bundle_id_t bundle_id;
    struct timeval send_time;
    u_int relative_id;
}
send_information_t;



long bundles_needed (long data, long pl);
void print_eid(char * label, bp_endpoint_id_t *eid);


void init_info(send_information_t *send_info, int window);
long add_info(send_information_t* send_info, bp_bundle_id_t bundle_id, struct timeval p_start, int window);
int is_in_info(send_information_t* send_info, bp_timestamp_t timestamp, int window);
int count_info(send_information_t* send_info, int window);
void remove_from_info(send_information_t* send_info, int position);
void set_bp_options(bp_bundle_object_t *bundle, dtnperf_connection_options_t *opt);

int open_payload_stream_read(bp_bundle_object_t bundle, FILE ** f);
int close_payload_stream_read(FILE * f);
int open_payload_stream_write(bp_bundle_object_t bundle, FILE ** f);
int close_payload_stream_write(bp_bundle_object_t * bundle, FILE * f);

bp_error_t prepare_payload_header(dtnperf_options_t *opt, FILE * f);
bp_error_t prepare_generic_payload(dtnperf_options_t *opt, FILE * f);
bp_error_t prepare_start_bundle(bp_bundle_object_t * start, bp_endpoint_id_t monitor,
		bp_timeval_t expiration, bp_bundle_priority_t priority);
bp_error_t prepare_stop_bundle(bp_bundle_object_t * stop, bp_endpoint_id_t monitor,
		bp_timeval_t expiration, bp_bundle_priority_t priority, int sent_bundles);
bp_error_t get_info_from_stop(bp_bundle_object_t * stop, int * sent_bundles);
bp_error_t prepare_server_ack_payload(dtnperf_server_ack_payload_t ack, char ** payload, size_t * payload_size);

/**
 * Get reported eid and timestamp from bundle ack
 * If you don't need either eid or timestamp, just put NULL in eid or timestamp.
 */
bp_error_t get_info_from_ack(bp_bundle_object_t * ack, bp_endpoint_id_t * reported_eid, bp_timestamp_t * report_timestamp);


boolean_t is_header(bp_bundle_object_t * bundle, const char * header_string);
boolean_t is_congestion_ctrl(bp_bundle_object_t * bundle, char mode);

u32_t get_current_dtn_time();

#endif /*BUNDLE_TOOLS_H_*/
