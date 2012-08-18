/*
 * dtnperf_main.c
 *
 *  Created on: 09/mar/2012
 *      Author: michele
 */

#include "includes.h"
#include "utils.h"
#include "dtnperf_types.h"
#include "definitions.h"
#include "dtnperf_modes/dtnperf_client.h"
#include "dtnperf_modes/dtnperf_server.h"
#include "dtnperf_modes/dtnperf_monitor.h"

/* ---------------------------
 * Global variables and options
 * --------------------------- */
dtnperf_global_options_t global_options;
int pid;



/* -------------------------------
 *       function interfaces
 * ------------------------------- */

void parse_options(int argc, char** argv, dtnperf_global_options_t * global_opt);
void print_usage(char* progname);
void init_dtnperf_global_options(dtnperf_global_options_t *, dtnperf_options_t *, dtnperf_connection_options_t *);
void init_dtnperf_options(dtnperf_options_t *);
void init_dtnperf_connection_options(dtnperf_connection_options_t*);
void check_options(dtnperf_global_options_t * global_options);

// CTRL+C handler
void main_handler(int signo);



/* -------------------------------
 *              MAIN
 * ------------------------------- */
int main(int argc, char ** argv)
{
	// variable declarations
	int status;
	dtnperf_global_options_t global_options;
	dtnperf_options_t perf_opt;
	dtnperf_connection_options_t conn_opt;

	// init options
	init_dtnperf_global_options(&global_options, &perf_opt, &conn_opt);

	// parse command line options
	parse_options(argc, argv, &global_options);

	// check command line options SOLO CLIENT
	// check_options(&global_options);

	// sigint handler
	signal(SIGINT, main_handler);

	// if client is also monitor, start a different process for monitor instance
	if (global_options.mode == DTNPERF_CLIENT_MONITOR)
	{	// client is also monitor
		pid = fork();
		if (pid < 0)
		{
			perror("error in child process creation");
			exit(-1);
		}
		if (pid == 0)
		{
			run_dtnperf_monitor(&global_options);
			fprintf(stderr,"error in monitor\n");
			exit(-1);
		}
	}

	switch (global_options.mode)
	{
	case DTNPERF_SERVER:
		run_dtnperf_server(&global_options);
		break;

	case DTNPERF_CLIENT_MONITOR:
	case DTNPERF_CLIENT:
		run_dtnperf_client(&global_options);
		break;

	case DTNPERF_MONITOR:
		run_dtnperf_monitor(&global_options);
		break;

	default:
		fprintf(stderr,"error in switching dtnperf mode");
		exit(-1);
	}


	if (global_options.mode == DTNPERF_CLIENT_MONITOR)
	{
		wait(&status);
		if ((char) status == 0)
			printf("Exited with status %d\n", status>>8);
		else
			printf("Exited by signal %d\n,", (char) status);
	}
	return 0;



}

void print_usage(char* progname){
	fprintf(stderr, "\n");
	fprintf(stderr, "SYNTAX: %s <operative mode> [options]\n", progname);
	fprintf(stderr, "\n");
	fprintf(stderr, "operative modes:\n");
	fprintf(stderr, " --server\n");
	fprintf(stderr, " --client\n");
	fprintf(stderr, " --monitor\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "For more options see\n %s <operative mode> --help\n", progname);
	fprintf(stderr, " %s --help\tPrint this screen.\n", progname);
	fprintf(stderr, "\n");
	exit(1);
}

void parse_options(int argc, char**argv, dtnperf_global_options_t * global_opt)
{
	int i;
	dtnperf_mode_t perf_mode = 0;


	// find dtnperf mode (server, client or monitor)
	if (argc < 2)
	{
		print_usage(argv[0]);
		exit(1);
	}
	if (strcmp(argv[1], "--server") == 0)
	{
		perf_mode = DTNPERF_SERVER;
	}
	else if (strcmp(argv[1], "--client") == 0)
	{
		perf_mode = DTNPERF_CLIENT;
	}
	else if (strcmp(argv[1], "--monitor") == 0)
	{
		perf_mode = DTNPERF_MONITOR;
	}
	else if (strcmp(argv[1], "--help") == 0) // general help option
	{
		print_usage(argv[0]);
		exit(0);
	}
	else
	{
		fprintf(stderr, "dtnperf mode must be specified as first argument\n");
		print_usage(argv[0]);
		exit(1);
	}

	// insert perf_mode in global options
	global_opt->mode = perf_mode;

	//scroll argv array
	for(i = 2; i < argc; i++)
	{
		argv[i-1] = argv[i];
	}
	argc = argc -1;

	switch(perf_mode)
	{
	case DTNPERF_CLIENT:
		parse_client_options(argc, argv, global_opt);
		break;
	case DTNPERF_SERVER:
		parse_server_options(argc, argv, global_opt);
		break;
	case DTNPERF_MONITOR:
		parse_monitor_options(argc, argv, global_opt);
		break;
	default:
		fprintf(stderr, "error in parsing options\n");
		print_usage(argv[0]);
		exit(1);
	}

} // end parse_options


void init_dtnperf_global_options(dtnperf_global_options_t *opt, dtnperf_options_t * perf_opt, dtnperf_connection_options_t * conn_opt)
{
	opt->perf_opt = perf_opt;
	opt->conn_opt = conn_opt;
	init_dtnperf_options(opt->perf_opt);
	init_dtnperf_connection_options(opt->conn_opt);
	opt->mode = 0;
}

void init_dtnperf_options(dtnperf_options_t *opt)
{
	opt->verbose = FALSE;
	opt->debug = FALSE;
	opt->debug_level = 0;
	opt->use_ip = FALSE;
	opt->ip_addr = "127.0.0.1";
	opt->ip_port = 5010;
	memset(opt->dest_eid, 0, BP_MAX_ENDPOINT_ID);
	memset(opt->mon_eid, 0, BP_MAX_ENDPOINT_ID);
	opt->op_mode = 'D';
	opt->data_qty = 0;
	opt->D_arg = NULL;
	opt->F_arg = NULL;
	opt->p_arg = NULL;
	opt->use_file = 1;
	opt->data_unit = 'M';
	opt->transmission_time = 0;
	opt->congestion_ctrl = 'w';
	opt->window = 1;
	opt->rate_arg = NULL;
	opt->rate = 0;
	opt->rate_unit = 'b';
	opt->wait_before_exit = 0;
	opt->bundle_payload = DEFAULT_PAYLOAD;
	opt->payload_type = BP_PAYLOAD_FILE;
	opt->dest_dir = BUNDLE_DIR_DEFAULT;
	opt->file_dir = FILE_DIR_DEFAULT;
	opt->create_log = FALSE;
	opt->log_filename = LOG_FILENAME;
	opt->acks_to_mon = FALSE;
	opt->no_acks = FALSE;
}



void init_dtnperf_connection_options(dtnperf_connection_options_t* opt)
{
	opt->expiration = 3600;				// expiration time (sec) [3600]
	opt->delivery_receipts = TRUE;		// request delivery receipts [1]
	opt->forwarding_receipts = FALSE;   // request per hop departure [0]
	opt->custody_transfer = FALSE;   	// request custody transfer [0]
	opt->custody_receipts = FALSE;   	// request per custodian receipts [0]
	opt->receive_receipts = FALSE;   	// request per hop arrival receipt [0]
	opt->wait_for_report = TRUE;   		// wait for bundle status reports [1]
	opt->disable_fragmentation = FALSE; //disable bundle fragmentation[0]
	opt->priority = BP_PRIORITY_NORMAL; // bundle priority [BP_PRIORITY_NORMAL]
}

/* ----------------------------
 * check_options
 * ---------------------------- */
void check_options(dtnperf_global_options_t * global_options)
{

	dtnperf_options_t * perf_opt = global_options->perf_opt;

	// checks on values
	if ((perf_opt->op_mode == 'D') && (perf_opt->data_qty <= 0))
	{
		fprintf(stderr, "\nSYNTAX ERROR: (-D option) you should send a positive number of MBytes (%ld)\n\n",
		        perf_opt->data_qty);
		exit(2);
	}
	if ((perf_opt->op_mode == 'T') && (perf_opt->transmission_time <= 0))
	{
		fprintf(stderr, "\nSYNTAX ERROR: (-T option) you should specify a positive time\n\n");
		exit(2);
	}
	// checks on options combination
	if ((perf_opt->use_file) && (perf_opt->op_mode == 'T'))
	{
		if (perf_opt->bundle_payload <= ILLEGAL_PAYLOAD)
		{
			perf_opt->bundle_payload = DEFAULT_PAYLOAD;
			fprintf(stderr, "\nWARNING (a): bundle payload set to %ld bytes\n", perf_opt->bundle_payload);
			fprintf(stderr, "(use_file && op_mode=='T' + payload <= %d)\n\n", ILLEGAL_PAYLOAD);
		}
	}
	if ((perf_opt->use_file) && (perf_opt->op_mode == 'D'))
	{
		if ((perf_opt->bundle_payload <= ILLEGAL_PAYLOAD)
		        || ((perf_opt->bundle_payload > perf_opt->data_qty)	&& (perf_opt->data_qty > 0)))
		{
			perf_opt->bundle_payload = perf_opt->data_qty;
			fprintf(stderr, "\nWARNING (b): bundle payload set to %ld bytes\n", perf_opt->bundle_payload);
			fprintf(stderr, "(use_file && op_mode=='D' + payload <= %d or > %ld)\n\n", ILLEGAL_PAYLOAD, perf_opt->data_qty);
		}
	}
	if ((!perf_opt->use_file)
	        && (perf_opt->bundle_payload <= ILLEGAL_PAYLOAD)
	        && (perf_opt->op_mode == 'D'))
	{
		if (perf_opt->data_qty <= MAX_MEM_PAYLOAD)
		{
			perf_opt->bundle_payload = perf_opt->data_qty;
			fprintf(stderr, "\nWARNING (c1): bundle payload set to %ld bytes\n", perf_opt->bundle_payload);
			fprintf(stderr, "(!use_file + payload <= %d + data_qty <= %d + op_mode=='D')\n\n",
			        ILLEGAL_PAYLOAD, MAX_MEM_PAYLOAD);
		}
		if (perf_opt->data_qty > MAX_MEM_PAYLOAD)
		{
			perf_opt->bundle_payload = MAX_MEM_PAYLOAD;
			fprintf(stderr, "(!use_file + payload <= %d + data_qty > %d + op_mode=='D')\n",
			        ILLEGAL_PAYLOAD, MAX_MEM_PAYLOAD);
			fprintf(stderr, "\nWARNING (c2): bundle payload set to %ld bytes\n\n", perf_opt->bundle_payload);
		}
	}
	if ((!perf_opt->use_file) && (perf_opt->op_mode == 'T'))
	{
		if (perf_opt->bundle_payload <= ILLEGAL_PAYLOAD)
		{
			perf_opt->bundle_payload = DEFAULT_PAYLOAD;
			fprintf(stderr, "\nWARNING (d1): bundle payload set to %ld bytes\n\n", perf_opt->bundle_payload);
			fprintf(stderr, "(!use_file + payload <= %d + op_mode=='T')\n\n", ILLEGAL_PAYLOAD);
		}
		if (perf_opt->bundle_payload > MAX_MEM_PAYLOAD)
		{
			fprintf(stderr, "\nWARNING (d2): bundle payload was set to %ld bytes, now set to %ld bytes\n",
			        perf_opt->bundle_payload, (long)DEFAULT_PAYLOAD);
			perf_opt->bundle_payload = DEFAULT_PAYLOAD;
			fprintf(stderr, "(!use_file + payload > %d)\n\n", MAX_MEM_PAYLOAD);
		}
	}

	if (perf_opt->window <= 0)
	{
		fprintf(stderr, "\nSYNTAX ERROR: (-w option) you should specify a positive value of window\n\n");
		exit(2);
	}

	if (!file_exists(perf_opt->F_arg))
	{
		fprintf(stderr, "\nERROR: (-F option) the file specified does not exist: %s\n\n", perf_opt->F_arg);
			exit(2);
	}
} // end check_options

// CTRL + C handler
void main_handler(int signo)
{
	// send signal to the child
	if (global_options.mode == DTNPERF_CLIENT_MONITOR)
	{
		kill(pid, SIGINT);
	}
	kill(getpid(), SIGUSR1);
}
