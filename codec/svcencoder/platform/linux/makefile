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

CFLAGS= -O2 -static -DSVC_ENCODER_LINUX
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


OBJ += $(OUT_DIR)svcenclib.o
OBJ += $(OUT_DIR)svcenc_rtp.o

OUTPUT_TARGET=../../bin/linux/libsvcenc.a 


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



