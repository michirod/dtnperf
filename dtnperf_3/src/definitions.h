/*
 * definitions.h
 *
 *  Created on: 13/lug/2012
 *      Author: michele
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

// dtnperf server mode string
#define SERVER_STRING "--server"

// dtnperf client mode string
#define CLIENT_STRING "--client"

// dtnperf monitor mode string
#define MONITOR_STRING "--monitor"

// dir where are saved incoming bundles
#define BUNDLE_DIR_DEFAULT "~/dtnperf/bundles/"

// dir where are saved transfered files
#define FILE_DIR_DEFAULT "~/dtnperf/files/"

// dir where are saved monitor logs
#define LOGS_DIR_DEFAULT "~/dtnperf/logs/"

// source file for bundle in client with use_file option
#define SOURCE_FILE "dtnperfbuf.src"

// default client log filename
#define LOG_FILENAME "~/dtnperf/log"

// output file: stdin and stderr redirect here if daemon is TRUE;
#define SERVER_OUTPUT_FILE "server_output.txt"
#define MONITOR_OUTPUT_FILE "monitor_output.txt"

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
#define FILE_HEADER "DTNPERF3_FILE_MODE__"

// header of start bundle sent by client to monitor (HEADER_SIZE chars)
#define START_HEADER "DTNPERF3_MON_START__"

// header of stop bundle sent by client to monitor (HEADER_SIZE chars)
#define STOP_HEADER "DTNPERF3_MON_STOP___"

// max payload (in bytes) if bundles are stored into memory
#define MAX_MEM_PAYLOAD 50000

// illegal number of bytes for the bundle payload
#define ILLEGAL_PAYLOAD 0

// default value (in bytes) for bundle payload
#define DEFAULT_PAYLOAD 50000

// server endpoint demux string
#define SERV_EP_STRING "/dtnperf:/dest"

// client endpoint demux string
#define CLI_EP_STRING "/dtnperf:/src"

// monitor endpoint demux string
#define MON_EP_STRING "/dtnperf:/mon"

// generic payload pattern
#define PL_PATTERN "0123456789"

// unix time of 1/1/2000
#define DTN_EPOCH 946684800

#endif /* DEFINITIONS_H_ */
