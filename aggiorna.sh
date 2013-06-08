#!/bin/bash
# declaration var dir
al_dir_src=/root/adamico/abstraction_layer_bundle_protocol
dtn2_dir_src=/root/dtn
ion_dir_src=/root/ion-3.1.2
#
echo al dir: $al_dir_src
echo dtn2 dir: $dtn2_dir_src
echo ion dir: $ion_dir_src
#
echo
echo AGGIORNAMENTO AL
cd ../abstraction_layer_bundle_protocol
git pull
echo 	GIT OK
make ION_DIR=$ion_dir_src
echo 	MAKE ION OK
make DTN2_DIR=$dtn2_dir_src
echo 	MAKE DTN2 OK
make ION_DIR=$ion_dir_src DTN2_DIR=$dtn2_dir_src 
echo 	MAKE DTN2 ION OK
make install
echo AGGIORNAMENT DTNPERF
cd ../dtnperf_vION
git pull
echo 	GIT OK
make ION_DIR=$ion_dir_src AL_BP_DIR=$al_dir_src
echo 	MAKE ION OK
make DTN2_DIR=$dtn2_dir_src AL_BP_DIR=$al_dir_src
echo 	MAKE DTN2 OK
make ION_DIR=$ion_dir_src DTN2_DIR=$dtn2_dir_src AL_BP_DIR=$al_dir_src
echo 	MAKE DTN2 ION OK
make install
