#ifndef SVCODEC_H
#define SVCODEC_H

#define MAXRTPNUM			256//64
typedef struct
{
	char *buf;//[MAXRTPSIZE];
	char *tmpBuf;
	short size[MAXRTPNUM];
	int flags;
	//unsigned short video_seqnum;
	int mtu_size;
	int ml_err_resilience;
	unsigned short videoSeqnum;
	int count;
	int pos;
	//short priorityMap[MAXRTPNUM];
	int NalNum;//Len;
	int rtpLen;
	int redundancy;
	int payLoadType;
	int codecType;
	int sliceType[4];
	int encIDX;//����ֱ���������
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

/** 
* @ingroup	SVCCodec
* @brief	�������ز���
*/
typedef struct
{
	int widthAlloc;//�ڴ�����
	int heightAlloc;//�ڴ�����
	int width_used;//��ǰ������������Ŀ���
	int height_used;//��ǰ������ߣ������Ŀ���
	int depth;
	int framerate;
	int gop;
	int bframenum;
	int bitsrate;//kbps
	int bitrateLayer[4];
	int qp[4][2];
	int qpmin;
	int qpmax;
	int rf_constant;//qp_constant;
	int psnr;
	//
	int layer;
	int codec;//bit0:0-enc,1-dec;bit1:0-svc,1-h264
	int redundancy;
	//
	int mtu_size;
	int mult_slice;
	int ml_err_resilience;
	//
	int payload_type;
	//
	int me_range;
	int sliced_threads;
	int gen_threads;//default:0
	int rc_method;//CQP(0)CRF(1)ABR(2)
	int subpel_refine;//--subme
	int me_method;//--me//DIA(0)HEX(1)UMH(2)ESA(3)TESA(4)      
	int disable_cabac;
	int no_8x8dct;
	float rate_tolerance;//rate_tolerance//���ʲ���ʵ��ֵͨ��������50%�����ǿ���cbr��ʽ���ϸ��������ƽ��
	//qp_constant;
	//int displayLayer;//��ʾ��ID�ţ�
	int encIDX;//����ֱ���������
	int shareIDX;
	//
	unsigned int resv0;
	unsigned int resv1;
	unsigned int resv2;
	unsigned int resv3;
	//
	//int outmode;//bit0:1-rgb;0-yuv;bit1:1-display;0-off;
	//
	ImgInfo img;
	//
	DebugInfo debugInfo;
}videoInParams;

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

typedef struct 
{
	int wishTotalCnts;
	int realTotalCnts;
	int currPts;
	int downLayer;
	int lostRate;
	int requestLayer;
}Down_Layer_Info;

#ifdef __cplusplus
extern "C" {
#endif

/**
* @ingroup	SVCCodec
* @brief    ������/�������ڴ���估��ʼ��
* 
* 
* @req		GVE-SRS-109
* @req		GVE-SRS-110
* @req		GVE-SRS-111
GVE-SRS-110
* @param[in]     videoInParams *Params: ��������ʼ��ʱΪEncParam����������ϡ���������ʼ��ʱΪDecParam����������ϡ�
* @returns   void *�����������߽����������ʵ��ΪGVESVCodec *�������ʼ���������ʧ�ܣ�����NULL��
*/
void *GVECodecInit(videoInParams *Params );
/**
* @ingroup	SVCCodec
* @brief    �ֲ�����RTP���
* @req		GVE-SRS-113
* @req		GVE-SRS-114
* @param[in]     void *handle: ������������EncParams,���������
* @param[in]     char *apSrcData[3]: ������YUV����
* @param[in]     int aiSrcDataLen: ���������ݳ���
* @param[out]     char *apDstBuff: �����������������
* @param[out]     int *aiDstBuffLen: �������Ĵ�С
* @param[out]     int *abMark: ��EncKeyFrm���Ƿ��ǹؼ�֡
* @returns   ����int, ���ش����ĳ��ȡ����������ʧ�ܣ�����0.
*/
int GVECodecEnc(void *handle,char *apSrcData[3],int aiSrcDataLen,char *apDstBuff,int *aiDstBuffLen, int *abMark);
/**
* @ingroup	SVCCodec
* @brief    RTP����ͷֲ����
* @req		GVE-SRS-122
* @req		GVE-SRS-123
* @param[in]     void *handle: ������������DecParams���������
* @param[in]     char *apSrcData: ��������������
* @param[in]     int aiSrcDataLen: ������������������
* @param[out]     char *apDstBuff[3]: ���������
* @param[out]     int *aiDstBuffLen: �������Ĵ�С
* @param[in]     unsigned short *packetSize: NULL(�˲�������δ��)
* @param[in]     int count: 0(�˲�������δ��)
* @param[in]     int *abMark: ��EncKeyFrm���Ƿ��ǹؼ�֡
* @returns   int, �����ĳ���
*/
int GVECodecDec(void *handle,char *apSrcData,int aiSrcDataLen,char *apDstBuff[3],int *aiDstBuffLen,int *DstWidth, int *DstHeight,unsigned short *packetSize, int count,Encoded_Decoded_Info info[4], Down_Layer_Info flags[1], int *lastenc);

/**
* @ingroup	SVCCodec
* @brief    ������/�������ڴ��ͷ�
* �뺯��GVECodecInit()��Ӧ���ͷ������x264����ĳ�ʼ�����ݲ���������Ҫ�Ŀռ�
* @param[in]     void *handle: ���������߽����������
* @returns   int�����Ǵ�����Ŀǰδ�õ����ء�
*/
int GVECodecClose(void *handle);
/**
* @ingroup	SVCCodec
* @brief    ��������ʼ��ʱ���ã�Ϊ_pRtPacket����ռ䣬���RTP����
* @param[in]     void *handle: �����������
*/
void *GVECodecGetRtpPoint(void *handle);
//�������Ŀǰδ�õ� Shelley 20130518
void *GVECodecGetParams(void *handle);
/**
* @brief    �����ú�����δ��Ҫɾ����
*/
void GVESetDebugFile(void *handle,void *infile,char *filename,int layer,int start,int codec);
//add by zj
void GVEResetBitrate(void *handle,int bitrate[4], int layer);
void GVEResetQP(void *handle, int minQP, int maxQP, int layer);
//void GVEStreamControl(void *handle, int bitrate, int layer);
//add end
#ifdef __cplusplus
}
#endif

#endif
