/*
 * definitions.h
 *
 *  Created on: 13/lug/2012
 *      Author: michele
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <stdint.h>

// dtnperf version
#define DTNPERF_VERSION "3.0.b2"

// dtnperf server mode string
#define SERVER_STRING "--server"

// dtnperf client mode string
#define CLIENT_STRING "--client"

// dtnperf monitor mode string
#define MONITOR_STRING "--monitor"

// dir where are saved incoming bundles
#define BUNDLE_DIR_DEFAULT "/tmp/"

// dir where are saved transfered files
#define FILE_DIR_DEFAULT "~/dtnperf/files/"

// dir where are saved client and monitor logs
#define LOGS_DIR_DEFAULT "."

// source file for bundle in client with use_file option
#define SOURCE_FILE "/tmp/dtnperfbuf"

// source file for bundle ack in server [ONLY ION impl]
#define SOURCE_FILE_ACK "/tmp/dtnperfack"

// default client log filename
#define LOG_FILENAME "dtnperf_client.log"

// output file: stdin and stderr redirect here if daemon is TRUE;
#define SERVER_OUTPUT_FILE "dtnperf_server.log"
#define MONITOR_OUTPUT_FILE "dtnperf_monitor.log"

/*
 * FIXED SIZE HEADERS
 */
// header type
#define HEADER_TYPE uint32_t

// header size
#define HEADER_SIZE sizeof(HEADER_TYPE)

// header of bundles sent in time mode
#define TIME_HEADER 0x1

// header of bundles sent in data mode
#define DATA_HEADER 0x2

// header of bundles sent in file mode
#define FILE_HEADER 0x4

// header of dtnperf server bundle ack
#define DSA_HEADER 0x8

// header of force stop bundle sent by client to monitor
#define FORCE_STOP_HEADER 0x10

// header of stop bundle sent by client to monitor
#define STOP_HEADER 0x20

// bundle options type
#define BUNDLE_OPT_TYPE uint16_t

// bundle options size
#define BUNDLE_OPT_SIZE sizeof(uint16_t)

// congestion control options
// window based
#define BO_ACK_CLIENT_YES 0x8000
// rate based
#define BO_ACK_CLIENT_NO 0x0000
// congestion control option mask (first 2 bits)
#define BO_ACK_CLIENT_MASK 0xC000

// acks to monitor options
// default behavior
#define BO_ACK_MON_NORMAL 0x0000
// force server to send acks to monitor
#define BO_ACK_MON_FORCE_YES 0x2000
// force server to not send acks to monitor
#define BO_ACK_MON_FORCE_NO 0x1000
// acks to monitor options mask
#define BO_ACK_MON_MASK 0x3000

// set ack expiration time as this bundle one
#define BO_SET_EXPIRATION 0x0080

// ack priority options
// set ack priority bit
#define BO_SET_PRIORITY 0x0040
// priorities indicated
#define BO_PRIORITY_BULK 0x0000
#define BO_PRIORITY_NORMAL 0x0010
#define BO_PRIORITY_EXPEDITED 0x0020
#define BO_PRIORITY_RESERVED 0x0030
// priority mask
#define BO_PRIORITY_MASK 0x0030

// max payload (in bytes) if bundles are stored into memory
#define MAX_MEM_PAYLOAD 50000

// illegal number of bytes for the bundle payload
#define ILLEGAL_PAYLOAD 0

// default value (in bytes) for bundle payload
#define DEFAULT_PAYLOAD 50000

// server endpoint demux string
#define SERV_EP_STRING "/dtnperf:/dest"
#define SERV_EP_NUM_SERVICE "2000"

// client endpoint demux string
#define CLI_EP_STRING "/dtnperf:/src"

// monitor endpoint demux string
#define MON_EP_STRING "/dtnperf:/mon"
#define MON_EP_NUM_SERVICE "1000"

// generic payload pattern
#define PL_PATTERN "0123456789"

// unix time of 1/1/2000
#define DTN_EPOCH 946684800

#endif /* DEFINITIONS_H_ */
