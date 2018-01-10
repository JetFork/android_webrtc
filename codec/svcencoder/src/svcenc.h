#ifndef SVCODEC_H
#define SVCODEC_H

#define H264MTU				1100
#define MAXRTPNUM			256//64
#define MAXRTPSIZE			512000

#define MAXLAYER		3
#define FDEC_FACTOR     4
#define FDEC_FACTOR1     3
#define FDEC_STRIDE1     ((1 << FDEC_FACTOR1) - 1)//(16 - 1)//(32 - 1)//
#define MAXWIDTH        1920
#define MINWIDTH        16
#define MAXHEIGHT       1288
#define MINHEITHT       16

#define CLIP3(x,a,b)    (x < a ? a : (x > b ? b : x))

#define PAYLOADTYPE_SVC 37

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
	unsigned int flag;
	void *fp;
	char *filename;
	void *data;
	int dataSize;
}DebugInfo;

typedef struct
{
	char *buf;//[MAXRTPSIZE];
	char *tmpBuf;
	short size[MAXRTPNUM];
	int flags;
	int mtu_size;
	int ml_err_resilience;
	unsigned short videoSeqnum;
	int count;
	int pos;
	int NalNum;//Len;
	int rtpLen;
	int redundancy;
	int payLoadType;
	int codecType;
	int sliceType[4];
	int encIDX;//编码分辨率索引；
	int gop;
	int allocIDX;//内存分辨率索引；
	int shareIDX;
	void *packetInfo;
	void *rsvData;//预留数据区，主要用于获取预存的SPS/PPS
}RTPacket;

#endif
