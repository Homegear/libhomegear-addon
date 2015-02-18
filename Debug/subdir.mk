################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../GD.cpp \
../Output.cpp \
../RPCClient.cpp \
../RPCMethod.cpp \
../RPCMethods.cpp \
../RPCServer.cpp \
../Server.cpp \
../Variable.cpp \
../main.cpp 

OBJS += \
./GD.o \
./Output.o \
./RPCClient.o \
./RPCMethod.o \
./RPCMethods.o \
./RPCServer.o \
./Server.o \
./Variable.o \
./main.o 

CPP_DEPS += \
./GD.d \
./Output.d \
./RPCClient.d \
./RPCMethod.d \
./RPCMethods.d \
./RPCServer.d \
./Server.d \
./Variable.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -D__GXX_EXPERIMENTAL_CXX0X__ -D_GLIBCXX_USE_NANOSLEEP -D__cplusplus=201103L -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


