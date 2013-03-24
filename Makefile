# Makefile for compiling DTNPerf3

# NAME OF BIN USED FOR INSTALL/UNINSTALL (NEVER LEAVE IT BLANK!!!)
BIN_NAME_BASE=dtnperf3
CC=gcc
LIBs=-L/usr/local/lib -L$(AL_BP_DIR)

INSTALLED=$(wildcard /usr/bin/$(BIN_NAME_BASE)*)

ifeq ($(strip $(AL_BP_DIR)),)
all: help
else ifeq ($(or $(ION_DIR),$(DTN2_DIR)),)
# NOTHING
all: help
else 
all: bin
endif 

ifeq ($(strip $(DTN2_DIR)),)
# ION
INC=-I$(AL_BP_DIR)/src/bp_implementations -I$(AL_BP_DIR)/src -I$(ION_DIR)/include -I$(ION_DIR)/library
OPT=-O0 -fmessage-length=0 -lal_bp_vION -lbp -lici -lpthread
BIN_NAME=$(BIN_NAME_BASE)_vION
else ifeq ($(strip $(ION_DIR)),)
# DTN
INC=-I$(AL_BP_DIR)/src/bp_implementations -I$(AL_BP_DIR)/src -I$(DTN2_DIR) -I$(DTN2_DIR)/applib 
OPT=-O0 -fmessage-length=0 -lal_bp_vDTN -ldtnapi -lpthread
BIN_NAME=$(BIN_NAME_BASE)_vDTN
else ifneq ($(and $(ION_DIR),$(DTN2_DIR)),)
# BOTH
INC=-I$(AL_BP_DIR)/src/bp_implementations -I$(AL_BP_DIR)/src -I$(DTN2_DIR) -I$(DTN2_DIR)/applib -I$(ION_DIR)/include -I$(ION_DIR)/library
OPT=-O0 -fmessage-length=0 -lal_bp -ldtnapi -lbp -lici -lpthread
BIN_NAME=$(BIN_NAME_BASE)
endif

bin: 
	$(CC) -o $(BIN_NAME) $(LIBs) $(INC) src/*.c src/dtnperf_modes/*.c $(OPT)

install: 
	cp $(BIN_NAME_BASE)* /usr/bin/

uninstall:
	@if test `echo $(INSTALLED) | wc -w` -eq 1 -a -f "$(INSTALLED)"; then rm -rf $(INSTALLED); else echo "MORE THAN 1 FILE, DELETE THEM MANUALLY: $(INSTALLED)"; fi

help:
	@echo "Usage:"
	@echo "For DTN2: 	make DTN2_DIR=<dtn2_dir> AL_BP_DIR=<al_bp_dir>"
	@echo "For ION:	make ION_DIR=<ion_dir> AL_BP_DIR=<al_bp_dir>"
	@echo "For both: 	make DTN2_DIR=<dtn2_dir> ION_DIR=<ion_dir> AL_BP_DIR=<al_bp_dir>"

clean:
	@rm -rf $(BIN_NAME_BASE)*

