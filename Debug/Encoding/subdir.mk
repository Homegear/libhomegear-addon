################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Encoding/BinaryDecoder.cpp \
../Encoding/BinaryEncoder.cpp \
../Encoding/RPCDecoder.cpp \
../Encoding/RPCEncoder.cpp \
../Encoding/RPCHeader.cpp 

OBJS += \
./Encoding/BinaryDecoder.o \
./Encoding/BinaryEncoder.o \
./Encoding/RPCDecoder.o \
./Encoding/RPCEncoder.o \
./Encoding/RPCHeader.o 

CPP_DEPS += \
./Encoding/BinaryDecoder.d \
./Encoding/BinaryEncoder.d \
./Encoding/RPCDecoder.d \
./Encoding/RPCEncoder.d \
./Encoding/RPCHeader.d 


# Each subdirectory must supply rules for building sources it contributes
Encoding/%.o: ../Encoding/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -D__GXX_EXPERIMENTAL_CXX0X__ -D_GLIBCXX_USE_NANOSLEEP -D__cplusplus=201103L -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


