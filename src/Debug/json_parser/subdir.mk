################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../json_parser/json_parser.c 

OBJS += \
./json_parser/json_parser.o 

C_DEPS += \
./json_parser/json_parser.d 


# Each subdirectory must supply rules for building sources it contributes
json_parser/%.o: ../json_parser/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


