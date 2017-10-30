#ifndef BUNDLE_TOOLS_H_
#define BUNDLE_TOOLS_H_

#include "includes.h"
#include "dtnperf_types.h"
#include <al_bp_types.h>


typedef struct
{
    al_bp_bundle_id_t bundle_id;
    struct timeval send_time;
    u_int relative_id;
}
send_information_t;



long bundles_needed (long data, long pl);
void print_eid(char * label, al_bp_endpoint_id_t *eid);


void init_info(send_information_t *send_info, int window);
long add_info(send_information_t* send_info, al_bp_bundle_id_t bundle_id, struct timeval p_start, int window);
int is_in_info(send_information_t* send_info, al_bp_timestamp_t timestamp, int window);
int count_info(send_information_t* send_info, int window);
void remove_from_info(send_information_t* send_info, int position);
void set_bp_options(al_bp_bundle_object_t *bundle, dtnperf_connection_options_t *opt);

int open_payload_stream_read(al_bp_bundle_object_t bundle, FILE ** f);
int close_payload_stream_read(FILE * f);
int open_payload_stream_write(al_bp_bundle_object_t bundle, FILE ** f);
int close_payload_stream_write(al_bp_bundle_object_t * bundle, FILE * f);

al_bp_error_t prepare_payload_header_and_ack_options(dtnperf_options_t *opt, FILE * f);
int get_bundle_header_and_options(al_bp_bundle_object_t * bundle, HEADER_TYPE * header, dtnperf_bundle_ack_options_t * options);

al_bp_error_t prepare_generic_payload(dtnperf_options_t *opt, FILE * f);
al_bp_error_t prepare_force_stop_bundle(al_bp_bundle_object_t * start, al_bp_endpoint_id_t monitor,
				al_bp_timeval_t expiration, al_bp_bundle_priority_t priority);
al_bp_error_t prepare_stop_bundle(al_bp_bundle_object_t * stop, al_bp_endpoint_id_t monitor,
		al_bp_timeval_t expiration, al_bp_bundle_priority_t priority, int sent_bundles);
al_bp_error_t get_info_from_stop(al_bp_bundle_object_t * stop, int * sent_bundles);
al_bp_error_t prepare_server_ack_payload(dtnperf_server_ack_payload_t ack, char ** payload, size_t * payload_size);

/**
 * Get reported eid and timestamp from bundle ack
 * If you don't need either eid or timestamp, just put NULL in eid or timestamp.
 */
al_bp_error_t get_info_from_ack(al_bp_bundle_object_t * ack, al_bp_endpoint_id_t * reported_eid, al_bp_timestamp_t * report_timestamp);

u32_t get_current_dtn_time();

#endif /*BUNDLE_TOOLS_H_*/
