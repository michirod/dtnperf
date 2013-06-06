#!/bin/bash
echo AGGIORNAMENTO AL
cd ../abstraction_layer_bundle_protocol
git pull
echo 	GIT OK
make ION_DIR=/root/ion-3.1.2
echo 	MAKE ION OK
make DTN2_DIR=/root/dtn
echo 	MAKE DTN2 OK
make ION_DIR=/root/ion-3.1.2 DTN2_DIR=/root/dtn 
echo 	MAKE DTN2 ION OK
make install
echo AGGIORNAMENT DTNPERF
cd ../dtnperf_vION
git pull
echo 	GIT OK
make ION_DIR=/root/ion-3.1.2 AL_BP_DIR=/root/adamico/abstraction_layer_bundle_protocol
echo 	MAKE ION OK
make DTN2_DIR=/root/dtn AL_BP_DIR=/root/adamico/abstraction_layer_bundle_protocol
echo 	MAKE DTN2 OK
make ION_DIR=/root/ion-3.1.2 DTN2_DIR=/root/dtn AL_BP_DIR=/root/adamico/abstraction_layer_bundle_protocol
echo 	MAKE DTN2 ION OK
make install
