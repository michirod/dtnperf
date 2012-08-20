/*
 * definitions.h
 *
 *  Created on: 13/lug/2012
 *      Author: michele
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

// dir where are saved incoming bundles
#define BUNDLE_DIR_DEFAULT "~/dtnperf/bundles/"

// dir where are saved transfered files
#define FILE_DIR_DEFAULT "~/dtnperf/files/"

// source file for bundle in client with use_file option
#define SOURCE_FILE "dtnperfbuf.src"

/*
 * FIXED SIZE HEADERS
 */
// header size
#define HEADER_SIZE 20
// header of dtnperf server bundle ack (HEADER_SIZE chars)
#define DSA_HEADER "DTNPERF3_SERVER_ACK_"

// header of bundles sent in time mode (HEADER_SIZE chars)
#define TIME_HEADER "DTNPERF3_TIME_MODE__"

// header of bundles sent in data mode (HEADER_SIZE chars)
#define DATA_HEADER "DTNPERF3_DATA_MODE__"

// header of bundles sent in file mode (HEADER_SIZE chars)
#define FILE_HEADER "DTNERF3_FILE_MODE__"

// max payload (in bytes) if bundles are stored into memory
#define MAX_MEM_PAYLOAD 50000

// illegal number of bytes for the bundle payload
#define ILLEGAL_PAYLOAD 0

// default value (in bytes) for bundle payload
#define DEFAULT_PAYLOAD 50000

// default log filename
#define LOG_FILENAME "~/dtnperf/log"

// server endpoint demux string
#define SERV_EP_STRING "/dtnperf:/dest"

// client endpoint demux string
#define CLI_EP_STRING "/dtnperf:/src"

// monitor endpoint demux string
#define MON_EP_STRING "/dtnperf:/mon"

// generic payload pattern
#define PL_PATTERN "0123456789"

#endif /* DEFINITIONS_H_ */
