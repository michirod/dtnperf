################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dtnperf_main.c \
../src/utils.c 

OBJS += \
./src/dtnperf_main.o \
./src/utils.o 

C_DEPS += \
./src/dtnperf_main.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/michele/Dropbox/workspace/bp_abstraction_layer/src -I/home/michele/Dropbox/git/bp-abstraction-api/bp_abstraction_layer/src -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


