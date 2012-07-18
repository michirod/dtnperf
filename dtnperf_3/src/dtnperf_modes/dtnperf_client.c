/*
 * dtnperf_client.c
 *
 *  Created on: 10/lug/2012
 *      Author: michele
 */


#include <bp_abstraction_api.h>
#include "dtnperf_client.h"
#include "../includes.h"

/*  ----------------------------
 *          CLIENT CODE
 *  ---------------------------- */
void run_dtnperf_client(dtnperf_global_options_t * perf_g_opt)
{

}
// end client code

void print_client_usage(char* progname)
{
	printf("ERROR: dtnperf3 client operative mode not yet implemented\n");
	exit(1);
}

void parse_client_options(int argc, char ** argv, dtnperf_global_options_t * perf_g_opt)
{
	printf("ERROR: dtnperf3 client operative mode not yet implemented\n");
	exit(1);

	char c, done = 0;
		dtnperf_options_t * perf_opt = &(perf_g_opt->perf_opt);
		dtnperf_connection_options_t * conn_opt = &(perf_g_opt->conn_opt);

		while (!done)
			{
				static struct option long_options[] =
				    {
					    {"help", no_argument, 0, 'h'},
					    {"verbose", no_argument, 0, 'v'},
					    {"memory", no_argument, 0, 'M'},
					    {"custody", optional_argument, 0, 'C'},
					    {"window", required_argument, 0, 'w'},
						{"destination", required_argument, 0, 'd'},
					    {"monitor", required_argument, 0, 'm'},
					    {"intervalbeforeexit", required_argument, 0, 'i'},
					    {"time", required_argument, 0, 'T'},
					    {"data", required_argument, 0, 'D'},
					    {"file", required_argument, 0, 'F'},
					    {"payload", required_argument, 0, 'p'},
					    {"expiration", required_argument, 0, 'e'},
					    {"rate", required_argument, 0, 'r'},
					    {"debug", optional_argument, 0, 33}, 				// 200 because D is for data mode
					    {"priority", required_argument, 0, 'P'},
					    {"dest-dir", required_argument, 0, 34}, 			// server only option
					    {"acks-to-src", no_argument, 0, 35},	 			// server only option
					    {0,0,0,0}	// The last element of the array has to be filled with zeros.

				    };

				int option_index = 0;
				c = getopt_long(argc, argv, "hvMC::w:d:m:i:T:D:F:p:e:r:P:", long_options, &option_index);

				switch (c)
				{
				case 'h':
					print_client_usage(argv[0]);
					exit(0);
					return ;

				case 'v':
					perf_opt->verbose = TRUE;
					break;

				case 'M':
					perf_opt->use_file = 0;
					perf_opt->payload_type = BP_PAYLOAD_MEM;
					break;

				case 'C':
					conn_opt->custody_transfer = 1;
					conn_opt->custody_receipts = 1;
					if ((optarg!=NULL && (strncmp(optarg, "SONC", 4)==0||strncmp(optarg, "Slide_on_Custody", 16)==0))||((argv[optind]!=NULL)&&(strncmp(argv[optind], "SONC", 4)==0||strncmp(argv[optind], "Slide_on_Custody", 16)==0))){
						//perf_opt->slide_on_custody=1;
					}
					break;

				case 'w':
					perf_opt->congestion_ctrl = 'w';
					perf_opt->window = atoi(optarg);
					break;

				case 'd':
					perf_opt->dest_eid = optarg;
					break;

				case 'm':
					perf_opt->mon_eid = optarg;
					break;

				case 'i':
					perf_opt->wait_before_exit = atoi(optarg)*1000;
					break;

				case 'T':
					perf_opt->op_mode = 'T';
					perf_opt->transmission_time = atoi(optarg);
					break;

				case 'D':
					perf_opt->op_mode = 'D';
					perf_opt->D_arg = strdup(optarg);
					perf_opt->data_unit = find_data_unit(perf_opt->D_arg);

					switch (perf_opt->data_unit)
					{
					case 'B':
						perf_opt->data_qty = atol(perf_opt->D_arg);
						break;
					case 'K':
						perf_opt->data_qty = kilo2byte(atol(perf_opt->D_arg));
						break;
					case 'M':
						perf_opt->data_qty = mega2byte(atol(perf_opt->D_arg));
						break;
					default:
						printf("\nWARNING: (-n option) invalid data unit, assuming 'M' (MBytes)\n\n");
						perf_opt->data_qty = mega2byte(atol(perf_opt->D_arg));
						break;
					}
					break;

				case 'F':
					perf_opt->op_mode = 'F';
					perf_opt->F_arg = strdup(optarg);
					break;

				case 'p':
					perf_opt->p_arg = optarg;
					perf_opt->data_unit = find_data_unit(perf_opt->p_arg);
					switch (perf_opt->data_unit)
					{
					case 'B':
						perf_opt->bundle_payload = atol(perf_opt->p_arg);
						break;
					case 'K':
						perf_opt->bundle_payload = kilo2byte(atol(perf_opt->p_arg));
						break;
					case 'M':
						perf_opt->bundle_payload = mega2byte(atol(perf_opt->p_arg));

						break;
					default:
						printf("\nWARNING: (-p option) invalid data unit, assuming 'K' (KBytes)\n\n");
						perf_opt->bundle_payload = kilo2byte(atol(perf_opt->p_arg));
						break;
					}
					break;

				case 'e':
					conn_opt->expiration = atoi(optarg);
					break;

				case 'r':
					perf_opt->rate_arg = strdup(optarg);
					perf_opt->rate_unit = find_rate_unit(perf_opt->rate_arg);
					perf_opt->rate = atoi(perf_opt->rate_arg);
					perf_opt->congestion_ctrl = 'r';
					break;

				case 'P':
					if (!strcasecmp(optarg, "bulk"))   {
					    conn_opt->priority = BP_PRIORITY_BULK;
					} else if (!strcasecmp(optarg, "normal")) {
						conn_opt->priority = BP_PRIORITY_NORMAL;
					} else if (!strcasecmp(optarg, "expedited")) {
						conn_opt->priority = BP_PRIORITY_EXPEDITED;
					} else if (!strcasecmp(optarg, "reserved")) {
						conn_opt->priority = BP_PRIORITY_RESERVED;
					} else {
					    fprintf(stderr, "Invalid priority value %s\n", optarg);
					    exit(1);
					}
					break;

				case 33: // debug
					perf_opt->debug = TRUE;
					if (optarg != NULL){
						int debug_level = atoi(optarg);
						if (debug_level >= 0 && debug_level <= 2)
							perf_opt->debug_level = atoi(optarg);
						else {
							fprintf(stderr, "wrong --debug argument\n");
							exit(1);
							return;
						}
					}
					else
						perf_opt->debug_level = 2;
					break;

				case 34: //server destination directory
					perf_opt->dest_dir = strdup(optarg);
					break;

				case 35: //server send acks to source
					perf_opt->acks_to_mon = TRUE;
					break;

				case '?':
					break;

				case (char)(-1):
					done = 1;
					break;

				default:
					// getopt already prints an error message for unknown option characters
					print_client_usage(argv[0]);
					exit(1);
				} // --switch
			} // -- while

			// if mode is client and monitor eid is not specified, client node must be also monitor.
			if(perf_g_opt->mode == DTNPERF_CLIENT && perf_opt->mon_eid == NULL)
			{
				perf_g_opt->mode = DTNPERF_CLIENT_MONITOR;
			}

		#define CHECK_SET(_arg, _what)                                          	\
		    if (_arg == 0) {                                                    	\
		        fprintf(stderr, "\nSYNTAX ERROR: %s must be specified\n", _what);   \
		        print_client_usage(argv[0]);                                               \
		        exit(1);                                                        	\
		    }

			CHECK_SET(perf_opt->dest_eid, "destination eid");
			CHECK_SET(perf_opt->op_mode, "-T or -D or -F");

}
