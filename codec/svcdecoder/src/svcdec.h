#ifndef SVCODEC_H
#define SVCODEC_H

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

#define PAYLOADTYPE_H264 96
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
	int encRatio;
	int gop;
	int allocIDX;//内存分辨率索引；
	int shareIDX;
	void *packetInfo;
	void *rsvData;//预留数据区，主要用于获取预存的SPS/PPS
}RTPacket;

/** 
* @ingroup	SVCCodec
* @brief	一帧图像相关参数
*
* 包含层数，图像每行存储宽度，宽，高，图像大小，图像数据
*/
typedef struct
{
	int layer;
	int stride;
	int width;
	int height;
	int imgsize;
	char *buf[3];
}ImgInfo;

/** 
* @ingroup	SVCCodec
* @brief	debug相关参数
*/
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
	int lastPts;//上一帧PTS
	int lostFlag;//是否有丢包
	int waitIDR;//是否需要等待I帧，需要则不显示，不需要则可以显示当前帧
	int lostType;//丢包类型：bit0-3:分别表示IPB是否丢失
	int repeatCnt;//上一帧和此帧PTS相同的次数
	int startDecode;
	int decFrmCnt;//解码器初始化以后，可以解码的帧数。主要用来解决解码器初始化以后第一帧不显示
	int decodedPts;
}Encoded_Decoded_Info;

#endif
