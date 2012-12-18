#!/bin/bash 

if [ $# -ne 3 -a $# -ne 4 ] ; then
	echo "use :"
	echo "	- Only DTN2:	./compile DTN2 <dtn2_dir> <al_bpdir>"
	echo "	- Only ION:	./compile ION <ion_dir> <al_bpdir>"
	echo "	- Both Impl:	./compile BOTH <al_bpdir> <dtn2_dir> <ion_dir>"
	exit
fi
if [ $# -eq 3 ] ; then
	if [ $1 = DTN2 ] ; then
		AL_BPDIR=$3
		DTN2DIR=$2
		gcc -o dtnperf_vION -L/usr/local/lib -L$AL_BPDIR -I$AL_BPDIR/src/bp_implementations -I$AL_BPDIR/src -I$DTN2DIR -I$DTN2DIR/applib src/*.c src/dtnperf_modes/*.c -fmessage-length=0 -lal_bp -ldtnapi -lpthread
		exit
	fi
	if [ $1 = ION ] ; then
		AL_BPDIR=$3
		IONDIR=$2
		gcc -o dtnperf_vION -L/usr/local/lib -L$AL_BPDIR -I$AL_BPDIR/src/bp_implementations -I$AL_BPDIR/src -I$IONDIR/include -I$IONDIR/library src/*.c src/dtnperf_modes/*.c  -fmessage-length=0 -lal_bp -lbp -lici -lpthread
		exit
	fi
fi
if [ $# -eq 4 -a $1 = BOTH ] ; then
	AL_BPDIR=$1
	DTN2DIR=$2
	IONDIR=$3
	gcc -o dtnperf_vION -L/usr/local/lib -L$AL_BPDIR -I$AL_BPDIR/src/bp_implementations -I$AL_BPDIR/src -I$DTN2DIR -I$DTN2DIR/applib -I$IONDIR/include -I$IONDIR/library -O3 src/*.c src/dtnperf_modes/*.c -fmessage-length=0 -lal_bp -ldtnapi -lbp -lici -lpthread
	exit
fi
echo "No Compile"
exit

