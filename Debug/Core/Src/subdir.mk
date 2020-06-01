################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/main.c \
../Core/Src/rtc.c \
../Core/Src/stm32l0xx_hal_msp.c \
../Core/Src/stm32l0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/system_stm32l0xx.c 

OBJS += \
./Core/Src/main.o \
./Core/Src/rtc.o \
./Core/Src/stm32l0xx_hal_msp.o \
./Core/Src/stm32l0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/system_stm32l0xx.o 

C_DEPS += \
./Core/Src/main.d \
./Core/Src/rtc.d \
./Core/Src/stm32l0xx_hal_msp.d \
./Core/Src/stm32l0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/system_stm32l0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32L071xx -I"/home/thanhlt/sw4stm32/sx1276/Core/Inc" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/STM32L0xx_HAL_Driver/Inc" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"/home/thanhlt/sw4stm32/sx1276/Drivers/CMSIS/Include" -I"/home/thanhlt/sw4stm32/sx1276/Core/Src/sx1276/crypto"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


