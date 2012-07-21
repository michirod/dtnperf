/*
 * dtnperf_types.h
 *
 *  Created on: 09/mar/2012
 *      Author: michele
 */

#ifndef DTNPERF_TYPES_H_
#define DTNPERF_TYPES_H_

#include <bp_types.h>

// max payload (in bytes) if bundles are stored into memory
#define MAX_MEM_PAYLOAD 50000

// illegal number of bytes for the bundle payload
#define ILLEGAL_PAYLOAD 0

// default value (in bytes) for bundle payload
#define DEFAULT_PAYLOAD 50000

typedef enum {
	DTNPERF_SERVER = 1,
	DTNPERF_CLIENT,
	DTNPERF_MONITOR,
	DTNPERF_CLIENT_MONITOR
} dtnperf_mode_t;

/**
 * To change default values go to init_dtnperf_options()
 */
typedef struct dtnperf_options
{
	//general options
	boolean_t verbose;		// if true, execution becomes verbose [FALSE]
	boolean_t debug;		// if true, debug messages are shown [FALSE]
	int debug_level;		// set the debug level 0|1|2 [0]
	int use_file;			// if set to 1, a file is used instead of memory [1]
	boolean_t use_ip;		// set different values of ip address and port [FALSE]
	char * ip_addr;			// daemon ip address [127.0.0.1]
	short ip_port;			// daemon port [5010]
	//client options
	char dest_eid[BP_MAX_ENDPOINT_ID];	// destination eid
	char mon_eid[BP_MAX_ENDPOINT_ID];	// monitor eid
	char op_mode;    		// operative mode (T = time_mode, D = data_mode, F = file_mode) [D]
	long data_qty;			// data to be transmitted (bytes) [0]
	char * D_arg;			// arguments of -D option
	char * F_arg;			// argument of -F option (filename)
	char * p_arg;			// arguments of -p option
	char data_unit;			// B = bytes, K = kilobytes, M = megabytes [M]
	int transmission_time;	// seconds of transmission [0]
	char congestion_ctrl;	// w = window based, r = rate based [w]
	int window;				// transmission window (bundles) [1]
	char * rate_arg;		// argument of -r option
	long rate;				// transmission rate [0]
	char rate_unit;			// b = bit/sec; B = bundle/sec [b]
	int wait_before_exit;	// additional interval before exit [0]
	long bundle_payload;  	// quantity of data (in bytes) to send (-p option) [DEFAULT_PAYLOAD]
	bp_bundle_payload_location_t payload_type;	// the type of data source for the bundle [DTN_PAYLOAD_FILE]
	boolean_t create_log;	// create log file [FALSE]
	char * log_filename;	// log filename [LOG_FILENAME]
	//server options
	char * dest_dir;		// destination dir of bundles [dtnperf]
	boolean_t acks_to_mon;	// send ACKs to both source and monitor (if monitor is not the source) [FALSE]
	boolean_t no_acks;		// do not send ACKs (for retro-compatibility purpose)

} dtnperf_options_t;

/**
 * To change default values go to init_dtnperf_connection_options()
 */
typedef struct dtnperf_connection_options
	{
		bp_timeval_t expiration;			// bundle expiration time (sec)
		bp_bundle_priority_t priority;		// bundle priority
		boolean_t delivery_receipts;
		boolean_t forwarding_receipts;
		boolean_t custody_transfer;
		boolean_t custody_receipts;
		boolean_t receive_receipts;
		boolean_t wait_for_report;
		boolean_t disable_fragmentation;
	} dtnperf_connection_options_t;

typedef struct dtnperf_global_options
{
	dtnperf_mode_t mode;
	dtnperf_options_t * perf_opt;
	dtnperf_connection_options_t * conn_opt;
} dtnperf_global_options_t;

typedef struct dtnperf_server_ack_payload
{
	char* header;
	bp_endpoint_id_t bundle_source;
	bp_timestamp_t bundle_creation_ts;
} dtnperf_server_ack_payload_t;

#endif /* DTNPERF_TYPES_H_ */
