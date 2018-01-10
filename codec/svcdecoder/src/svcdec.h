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
	int encIDX;//����ֱ���������
	int encRatio;
	int gop;
	int allocIDX;//�ڴ�ֱ���������
	int shareIDX;
	void *packetInfo;
	void *rsvData;//Ԥ������������Ҫ���ڻ�ȡԤ���SPS/PPS
}RTPacket;

/** 
* @ingroup	SVCCodec
* @brief	һ֡ͼ����ز���
*
* ����������ͼ��ÿ�д洢��ȣ����ߣ�ͼ���С��ͼ������
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
* @brief	debug��ز���
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
	int lastPts;//��һ֡PTS
	int lostFlag;//�Ƿ��ж���
	int waitIDR;//�Ƿ���Ҫ�ȴ�I֡����Ҫ����ʾ������Ҫ�������ʾ��ǰ֡
	int lostType;//�������ͣ�bit0-3:�ֱ��ʾIPB�Ƿ�ʧ
	int repeatCnt;//��һ֡�ʹ�֡PTS��ͬ�Ĵ���
	int startDecode;
	int decFrmCnt;//��������ʼ���Ժ󣬿��Խ����֡������Ҫ���������������ʼ���Ժ��һ֡����ʾ
	int decodedPts;
}Encoded_Decoded_Info;

#endif
