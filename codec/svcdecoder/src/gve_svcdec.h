#ifndef __GVE_SVCDEC_H_
#define __GVE_SVCDEC_H_


#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

#define inline __inline
#define __inline__ __inline

#include "svcdec.h"
#include "libavcodec/avcodec.h"

typedef struct
{
	AVCodec *codec;
	AVCodecContext *c;
	AVPacket avpkt;
	AVFrame *frame;
	int got_frame;

}ffmpeg_info;

typedef struct
{
	char *data;
	unsigned int pts;
	unsigned int size;

}SVCDecPacket;

typedef struct
{
	RTPacket dRtpPacket;
	//void *decodeHandle[MAXLAYER];

	ffmpeg_info *ffmpegdec_handle[MAXLAYER];

	SVCDecPacket *dPacket[MAXLAYER];

	int width[3];
	int height[3];
	int layer;
	int decFrameNum;
	int gopSize;
	int bframenum;
	int displayLayer;
	int encIDX;//编码分辨率索引；
	int encRatio;
	//int allocIDX;//内存分辨率索引；
	int decIDX;
	//unsigned short displayPts;

	//add wyh
	int lastEnc;
	int lastEncRatio;
	char *DecBuf[3][3];
	Encoded_Decoded_Info info[3];
	//int scalefactor[3];
	int startInum[3];

	//add wyh
	int *DEC_SVCGop ;
	int64_t *DEC_SVCGopPTS;
	//end wyh

	DebugInfo debugInfo[3];

	//FILE *fptst_wyh;
	//end wyh
}SVCdec_Handle;

#endif//__GVE_H264ENCODE_H_
