# This makefile will build a Linux library

ANDROID_NDK_ROOT=/home/zj_work/android-ndk-r9d
PREBUILT=$(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64
PLATFORM=$(ANDROID_NDK_ROOT)/platforms/android-19/arch-arm
ARM_INC=$(PLATFORM)/usr/include
PATH_GNU_BIN=$(PREBUILT)/bin
BIN_DIR := /usr/local/arm_inst/sdk/lib/


#==============================================================================
# GNU 		binaries										(server admin update)
#==============================================================================
CC=$(PATH_GNU_BIN)/arm-linux-androideabi-gcc
CXX=$(PATH_GNU_BIN)/arm-linux-androideabi-gcc
AR=$(PATH_GNU_BIN)/arm-linux-androideabi-ar
AS=$(PATH_GNU_BIN)/arm-linux-androideabi-as
LN=$(PATH_GNU_BIN)/arm-linux-androideabi-gcc
LD=$(PATH_GNU_BIN)/arm-linux-androideabi-ld
RAN=$(PATH_GNU_BIN)/arm-linux-androideabi-ranlib


#==============================================================================
# GNU build options: all										(build engineer update)				
#==============================================================================

CFLAGS= -O2 -static -I$(ARM_INC) -DANDROID
ARFLAGS=rs
CXXFLAGS=-O3 

LNFLAGS= -lpthread -ldl -lm

#==============================================================================
# User root path											(user update)
#==============================================================================
CSRC_DIR := ../../src/
INC_DIR := ../../src/
OUT_DIR := ./#build/obj/

INC := -I$(INC_DIR)

OBJ :=


OBJ += $(OUT_DIR)ffmpegdemuxdec.o

OUTPUT_TARGET=$(BIN_DIR)libffmpegdemux_arm.a

.PHONY:
$(OUT_DIR)%.o: $(CSRC_DIR)%.c 
	$(CC) -c $(CFLAGS) $(INC) -o $@ $<

all : $(OUTPUT_TARGET) 

$(OUTPUT_TARGET) : $(OBJ) 
	$(AR)  $(ARFLAGS) $@ $^
	$(RAN) $@
	rm ./*.o
clean:
	#rm -fr $(OUT_DIR)* $(OUTPUT_TARGET)
	rm *.o



