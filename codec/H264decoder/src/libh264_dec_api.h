#ifndef __LIBH264_DEC_API_H__
#define __LIBH264_DEC_API_H__

#include "libavcodec/avcodec.h"

#define MAXRTPSIZE			512000
#define MAXRTPNUM			256//64

typedef struct
{
	char *buf;//[MAXRTPSIZE];
	char *tmpBuf;
	short size[MAXRTPNUM];
	//int flags;
	//int mtu_size;
	//int ml_err_resilience;
	//unsigned short videoSeqnum;
	int count;
	int pos;
	//int NalNum;//Len;
	int rtpLen;
	//int redundancy;
	//int payLoadType;
	//int codecType;
	//int sliceType[4];
	//int encIDX;//编码分辨率索引；
	//int gop;
	//int allocIDX;//内存分辨率索引；
	//int shareIDX;
	//void *packetInfo;
	//void *rsvData;//预留数据区，主要用于获取预存的SPS/PPS
}H264dec_RTPacket;

typedef struct
{
	int got_frame;
	AVCodec *codec;
	AVCodecContext *c;
	AVPacket avpkt;
	AVFrame *frame;
	H264dec_RTPacket RtpPacket;

}decode264_handle;
#endif