dtnperf
=======
$ ./dtnperf_3 -h
dtnperf mode must be specified as first argument

SYNTAX: ./dtnperf_3 <operative mode> [options]

operative modes:
 --server
 --client
 --monitor

For more options see
 ./dtnperf_3 <operative mode> --help
 ./dtnperf_3 --help  Print this screen.

==============
DTNPERF client
==============
$ ./dtnperf_3 --client -h

DtnPerf3 client mode
SYNTAX: ./dtnperf_3 --client -d <dest_eid> <[-T <sec> | -D <num> | -F <filename]> [options]

options:
 -d, --destination <eid>   Destination eid (required).
 -m, --monitor <eid>       Monitor eid. Default is same as local eid.
 -T, --time <sec>          Time-mode: seconds of transmission.
 -D, --data <num[BKM]>     Data-mode: bytes to transmit, data unit default 'M' (Mbytes).
 -F, --file <filename>     File-mode: file to transfer -w, --window <size[BKb]>  Size of transmission window, i.e. max number of bundles "in flight" (not still ACKed by a server ack); default =1.
 -r, --rate <rate>         Bitrate of transmission. Bytes/sec, KBytes/sec, bundles/sec. Default is B
 -C, --custody             Enable both custody transfer and "custody accepted" status reports.
 -i, --exitinterval <sec>  Additional interval before exit.
 -p, --payload <size[BKM]> Size of bundle payloads; data unit default= 'K' (Kbytes).
 -u, --nofragment          Disable bundle fragmentation.
 -M, --memory              Store the bundle into memory instead of file (if payload < 50KB).
 -L, --log[=log_filename]  Create a log file. Default log filename is LOG_FILENAME
     --ip-addr <addr>      Ip address of the bp daemon api. Default is 127.0.0.1
     --ip-port <port>      Ip port of the bp daemon api. Default is 5010
     --debug[=level]       Debug messages [0-1], if level is not indicated assume level=2.
 -e, --expiration <time>   Bundle acks expiration time. Default is 3600
 -P, --priority <val>      Bundle acks priority [bulk|normal|expedited|reserved]. Default is normal
 -v, --verbose             Print some information messages during the execution.
 -h, --help                This help.

==============
DTNPERF server
==============
$ ./dtnperf_3 --server -h

DtnPerf3 server mode
SYNTAX: ./dtnperf_3 --server [options]

options:
     --ip-addr <addr>   Ip address of the bp daemon api. Default is 127.0.0.1
     --ip-port <port>   Ip port of the bp daemon api. Default is 5010
     --ddir <dir>       Destination directory of bundles (if not using -M), if dir is not indicated assume ~/dtnperf/bundles/.
     --fdir <dir>       Destination directory of transfered files
     --debug[=level]    Debug messages [0-1], if level is not indicated assume level=0.
 -M, --memory           Save bundles into memory.
 -e, --expiration <sec> Bundle acks expiration time. Default is 3600
 -P, --priority <val>   Bundle acks priority [bulk|normal|expedited|reserved]. Default is normal
     --acks-to-mon      Send bundle acks to the monitor too
     --no-acks          Do not send acks (for using with dtnperf2)
 -v, --verbose          Print some information message during the execution.
 -h, --help             This help.
