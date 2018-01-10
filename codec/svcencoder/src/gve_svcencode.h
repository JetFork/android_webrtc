#ifndef __GVE_SVCENCODE_H_
#define __GVE_SVCENCODE_H_


#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#ifdef SVC_ENCODER_WIN32
#include "x264/stdint.h"
#endif
#ifdef SVC_ENCODER_LINUX
#include <stdint.h>
#endif

#include "svcenc.h"
#include "x264/x264.h"

typedef struct
{
	unsigned char *data;
	unsigned int size;
	unsigned int pts;

}SVCPacket;

typedef struct
{
	x264_t *x264_info_t;
	unsigned int frameoffset;
	x264_picture_t pic;
	x264_picture_t pic_out;

}x264_info;

typedef struct
{
	x264_info *x264_handle[MAXLAYER];

	RTPacket eRtpPacket;
	SVCPacket *ePacket[MAXLAYER];

	int layer;
	unsigned int frameNum;
	int gopSize;
	int bframenum;
	int encIDX;//编码分辨率索引；
	int allocIDX;//内存分辨率索引；
	int scalefactor[3];
	int startInum[3];
	char *Inbuf[3];
	int (*pfDownResamplefun)(char *src[3], char *dst[3],int iw, int ih, int ow, int oh, int step);

	DebugInfo debugInfo[3];

	//FILE *fptst_wyh;
	//end wyh
}GVESVC_Handle;


#endif//__GVE_H264ENCODE_H_


