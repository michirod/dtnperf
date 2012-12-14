################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bundle_tools.c \
../src/csv_tools.c \
../src/dtnperf_main.c \
../src/file_transfer_tools.c \
../src/utils.c 

OBJS += \
./src/bundle_tools.o \
./src/csv_tools.o \
./src/dtnperf_main.o \
./src/file_transfer_tools.o \
./src/utils.o 

C_DEPS += \
./src/bundle_tools.d \
./src/csv_tools.d \
./src/dtnperf_main.d \
./src/file_transfer_tools.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/home/annetta/test_VM/abstraction_layer_bundle_protocol" -I/home/annetta/DTN/bp_abstraction_layer/bp_abstraction_layer/src -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


