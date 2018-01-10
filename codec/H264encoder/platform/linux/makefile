# This makefile will build a Linux library

#==============================================================================
# GNU 		binaries										(server admin update)
#==============================================================================
CC=gcc
CXX=gcc
AR=ar
AS=as
LN=gcc
LD=ld
RAN=ranlib


#==============================================================================
# GNU build options: all										(build engineer update)				
#==============================================================================

CFLAGS= -O2 -static -DH264_ENCODER_LINUX
ARFLAGS=rs
CXXFLAGS=-O3 

#ASFLAGS= -k -miwmmxt
LNFLAGS= -lpthread -ldl -lm

#==============================================================================
# User root path											(user update)
#==============================================================================
CSRC_DIR := ../../src/
INC_DIR := ../../src/
OUT_DIR := ./#build/obj/

INC := -I$(INC_DIR)

OBJ :=


OBJ += $(OUT_DIR)gve_h264encode.o
OBJ += $(OUT_DIR)h264encrtp.o


OUTPUT_TARGET=../../bin/linux/libh264enc.a 


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



