#!/bin/bash 
# script for compiling dtnperf 3
# indicate as first parameter the bp_abstraction_layer source dir
# eg: ./compile.sh /home/user/dtn/bp_abstraction_layer/src
# if libbp_abstraction_layer.so cannot be found,
# indicate as second parameter the libbp_abstraction_layer.so dir
# eg: ./compile.sh /home/user/dtn/bp_abstraction_layer/src /home/user/dtn/bp_abstraction_layer/

if [ $# -ne 1 -a $# -ne 2 ]; then
	echo "indicate as first parameter the bp_abstraction_layer source dir"
	echo "if libbp_abstraction_layer.so cannot be found,"
	echo "indicate as second parameter the libbp_abstraction_layer.a dir"
	exit
fi

if [ $# -eq 1 ]; then
	gcc -o dtnperf -I$1 -O3 -Wall src/*.c src/dtnperf_modes/*.c -fmessage-length=0 -lbp_abstraction_layer -lpthread
elif [ $# -eq 2 ]; then
	gcc -o dtnperf -static -I$1 -O3 -Wall src/*.c src/dtnperf_modes/*.c -fmessage-length=0 -L$2 -lbp_abstraction_layer -lpthread
fi

