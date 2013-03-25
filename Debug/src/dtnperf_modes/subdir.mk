################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dtnperf_modes/dtnperf_client.c \
../src/dtnperf_modes/dtnperf_monitor.c \
../src/dtnperf_modes/dtnperf_server.c 

OBJS += \
./src/dtnperf_modes/dtnperf_client.o \
./src/dtnperf_modes/dtnperf_monitor.o \
./src/dtnperf_modes/dtnperf_server.o 

C_DEPS += \
./src/dtnperf_modes/dtnperf_client.d \
./src/dtnperf_modes/dtnperf_monitor.d \
./src/dtnperf_modes/dtnperf_server.d 


# Each subdirectory must supply rules for building sources it contributes
src/dtnperf_modes/%.o: ../src/dtnperf_modes/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/annetta/test_VM/abstraction_layer_bundle_protocol" -I/home/annetta/DTN/bp_abstraction_layer/bp_abstraction_layer/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


