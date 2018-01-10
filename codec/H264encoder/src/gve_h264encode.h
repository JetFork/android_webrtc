#ifndef __GVE_H264ENCODE_H_
#define __GVE_H264ENCODE_H_


#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#define DATA_MAX 3000000
#define MAXRTPSIZE			512000
#define PAYLOADTYPE_H264 96
#define H264MTU				1100
#define MAXRTPNUM			256//64

#include "stdint.h"
#include <stdio.h>
#include <signal.h>
#include "x264.h"

enum PicType
{
	PIC_TYPE_AUTO = 0,        
	PIC_TYPE_IDR,           
	PIC_TYPE_I,             
	PIC_TYPE_P,             
	PIC_TYPE_BREF,            
	PIC_TYPE_B,             
	PIC_TYPE_KEYFRAME,   
};

typedef struct
{
	char *buf;//[MAXRTPSIZE];
	char *tmpBuf;
	short size[MAXRTPNUM];
	//int flags;
	int mtu_size;
	//int ml_err_resilience;
	//unsigned short videoSeqnum;
	int count;
	int pos;
	int NalNum;//Len;
	int rtpLen;
	//int redundancy;
	//int payLoadType;
	//int codecType;
	int sliceType;
	//int encIDX;//编码分辨率索引；
	//int gop;
	//int allocIDX;//内存分辨率索引；
	//int shareIDX;
	//void *packetInfo;
	//void *rsvData;//预留数据区，主要用于获取预存的SPS/PPS
}H264enc_RTPacket;

typedef struct
{
	unsigned int frameNum;
	x264_t *h;
	H264enc_RTPacket RtpPacket;
	unsigned int frameoffset;
	x264_picture_t pic;
	x264_picture_t pic_out;
	//for test
	//FILE *fph264;
	//end test
}X264_Handle;

#endif//__GVE_H264ENCODE_H_


