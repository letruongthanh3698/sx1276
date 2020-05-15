################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/sx1276/lora.c 

OBJS += \
./Core/Src/sx1276/lora.o 

C_DEPS += \
./Core/Src/sx1276/lora.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/sx1276/%.o: ../Core/Src/sx1276/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32L071xx -I"/home/thanhlt/sw4stm32/sx1276/Core/Inc" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/STM32L0xx_HAL_Driver/Inc" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


