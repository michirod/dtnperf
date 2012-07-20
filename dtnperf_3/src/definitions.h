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

// source file for bundle in client with use_file option
#define SOURCE_FILE "dtnperfbuf.src"

// header of dtnperf server bundle ack
#define DSA_STRING "DTNPERF3_SERVER_BUNDLE_ACK"

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

#endif /* DEFINITIONS_H_ */
