#!/bin/bash 
# script for compiling dtnperf 3
# indicate as first parameter the bp_abstraction_layer source dir
# and as second parameter the libbp_abstraction_layer.a dir
# eg: ./compile.sh /home/user/dtn/bp_abstraction_layer/src /home/user/dtn/bp_abstraction_layer/
# if libdtnapi cannot be found, try to indicate the libdtnapi dir as third parameter
# eg ./compile.sh /home/user/dtn/bp_abstraction_layer/src /home/user/dtn/bp_abstraction_layer/ /home/user/dtn/DTN2/applib/

if [ $# -ne 2 -a $# -ne 3 ]; then
	echo "indicate as first parameter the bp_abstraction_layer source dir"
	echo "and as second parameter the libbp_abstraction_layer.a dir"
	echo "if libdtnapi cannot be found, try to indicate the libdtnapi dir as third parameter"
	exit
fi

if [ $# -eq 2 ]; then
	gcc -o dtnperf -I$1 -O3 -Wall src/*.c src/dtnperf_modes/*.c -fmessage-length=0 -L$2 -lbp_abstraction_layer -ldtnapi -lpthread
else
	gcc -o dtnperf -I$1 -O3 -Wall src/*.c src/dtnperf_modes/*.c -fmessage-length=0 -L$2 -L$3 -lbp_abstraction_layer -ldtnapi -lpthread
fi

