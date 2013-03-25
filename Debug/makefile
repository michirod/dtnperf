################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/dtnperf_modes/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: dtnperf_3_vION

dependents:
	-cd /home/annetta/test_VM/abstraction_layer_bundle_protocol/Debug && $(MAKE) clean all

# Tool invocations
dtnperf_3_vION: $(OBJS) $(USER_OBJS) /home/annetta/test_VM/abstraction_layer_bundle_protocol/Debug/libbp_abstraction_layer.a
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc -L"/home/annetta/test_VM/abstraction_layer_bundle_protocol/Debug" -pthread -o "dtnperf_3_vION" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) dtnperf_3_vION
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/annetta/test_VM/abstraction_layer_bundle_protocol/Debug/libbp_abstraction_layer.a:

-include ../makefile.targets
