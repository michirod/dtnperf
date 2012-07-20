#!/bin/bash 
# script for compiling dtnperf 3
# indicate as first parameter the bp_abstraction_layer source dir
# and as second parameter the libbp_abstraction_layer.a dir
# eg: ./compile.sh /home/user/dtn/DTN2/

if [ $# -ne 2 ]; then
	echo "indicate as first parameter the bp_abstraction_layer source dir"
	echo "and as second parameter the libbp_abstraction_layer.a dir"
	exit
fi

gcc -o dtnperf -I$1 -O3 -Wall src/*.c src/dtnperf_modes/*.c -O3 -Wall -fmessage-length=0 -L$2 -ldtnapi -lbp_abstraction_layer -lpthread
