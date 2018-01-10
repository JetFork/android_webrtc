# This makefile will build a Linux library

#PATH_GNU_BIN=/home/quanshi/carol/Sourcery_G++_Lite/bin
#PATH_GNU_BIN=/home/quanshi/work/android-ndk-r9d/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64/bin

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

CFLAGS= -O2 -static -I$(ARM_INC) -DSVC_ENCODER_ANDROID
ARFLAGS=rs
CXXFLAGS=-O3 

LNFLAGS= -lpthread -ldl -lm

#==============================================================================
# User root path											(user update)
#==============================================================================
SSRC_DIR := ./asm/
OUT_DIR := ./#build/obj/

OBJ :=
OBJ += $(OUT_DIR)downsample_1_android32.o
OBJ += $(OUT_DIR)downsample_2_android32.o
OBJ += $(OUT_DIR)memcpy_armv7_android32.o
OBJ += $(OUT_DIR)ScalePlaneBox_1.5_neon.o

OUTPUT_TARGET=$(BIN_DIR)libcommonandroidneon.a 


.PHONY:

$(OUT_DIR)%.o: $(SSRC_DIR)%.s
	$(AS) -mfpu=neon  $< -o $@

all : $(OUTPUT_TARGET) 

$(OUTPUT_TARGET) : $(OBJ) 
	$(AR)  $(ARFLAGS) $@ $^
	$(RAN) $@
	rm ./*.o
clean:
	#rm -fr $(OUT_DIR)* $(OUTPUT_TARGET)
	rm *.o



