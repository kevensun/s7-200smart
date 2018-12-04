################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../quarkIntegrate.cc \
../quarkMain.cc 

C_SRCS += \
../createSrTemplate.c \
../database.c \
../fx-serial.c \
../msgQueue.c \
../plc-mitsubishi-fx-3u.c \
../plc-siemens-s7-1500.c \
../plc-siemens-s7-200-driver.c \
../plc-siemens-s7-200.c \
../plc-siemens-s7-smart200.c \
../plcMain.c \
../rWebCfgCjson.c \
../rWebCfgCrc32.c \
../rWebCfgHeartbeat.c \
../rWebCfgRegister.c \
../rWebCfgSubscription.c \
../rWebMain.c 

CC_DEPS += \
./quarkIntegrate.d \
./quarkMain.d 

OBJS += \
./createSrTemplate.o \
./database.o \
./fx-serial.o \
./msgQueue.o \
./plc-mitsubishi-fx-3u.o \
./plc-siemens-s7-1500.o \
./plc-siemens-s7-200-driver.o \
./plc-siemens-s7-200.o \
./plc-siemens-s7-smart200.o \
./plcMain.o \
./quarkIntegrate.o \
./quarkMain.o \
./rWebCfgCjson.o \
./rWebCfgCrc32.o \
./rWebCfgHeartbeat.o \
./rWebCfgRegister.o \
./rWebCfgSubscription.o \
./rWebMain.o 

C_DEPS += \
./createSrTemplate.d \
./database.d \
./fx-serial.d \
./msgQueue.d \
./plc-mitsubishi-fx-3u.d \
./plc-siemens-s7-1500.d \
./plc-siemens-s7-200-driver.d \
./plc-siemens-s7-200.d \
./plc-siemens-s7-smart200.d \
./plcMain.d \
./rWebCfgCjson.d \
./rWebCfgCrc32.d \
./rWebCfgHeartbeat.d \
./rWebCfgRegister.d \
./rWebCfgSubscription.d \
./rWebMain.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++0x -I/home/fanxiaobin/cumulocity-sdk-c/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


