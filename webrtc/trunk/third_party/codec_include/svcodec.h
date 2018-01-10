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
	int encIDX;//编码分辨率索引；
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

/** 
* @ingroup	SVCCodec
* @brief	编解码相关参数
*/
typedef struct
{
	int widthAlloc;//内存分配宽
	int heightAlloc;//内存分配高
	int width_used;//当前最大编码宽，或解码目标宽
	int height_used;//当前最大编码高，或解码目标高
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
	float rate_tolerance;//rate_tolerance//码率波动实际值通常在上下50%，除非开启cbr方式可严格控制码率平稳
	//qp_constant;
	//int displayLayer;//显示层ID号；
	int encIDX;//编码分辨率索引；
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
	int lastPts;//上一帧PTS
	int lostFlag;//是否有丢包
	int waitIDR;//是否需要等待I帧，需要则不显示，不需要则可以显示当前帧
	int lostType;//丢包类型：bit0-3:分别表示IPB是否丢失
	int repeatCnt;//上一帧和此帧PTS相同的次数
	int startDecode;
	int decFrmCnt;//解码器初始化以后，可以解码的帧数。主要用来解决解码器初始化以后第一帧不显示
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
* @brief    编码器/解码器内存分配及初始化
* 
* 
* @req		GVE-SRS-109
* @req		GVE-SRS-110
* @req		GVE-SRS-111
GVE-SRS-110
* @param[in]     videoInParams *Params: 编码器初始化时为EncParam编码参数集合。解码器初始化时为DecParam解码参数集合。
* @returns   void *，编码器或者解码器句柄，实际为GVESVCodec *。如果初始化编解码器失败，返回NULL。
*/
void *GVECodecInit(videoInParams *Params );
/**
* @ingroup	SVCCodec
* @brief    分层编码和RTP打包
* @req		GVE-SRS-113
* @req		GVE-SRS-114
* @param[in]     void *handle: 编码句柄，包含EncParams,即编码参数
* @param[in]     char *apSrcData[3]: 待编码YUV数据
* @param[in]     int aiSrcDataLen: 待编码数据长度
* @param[out]     char *apDstBuff: 编码打包后的码流数据
* @param[out]     int *aiDstBuffLen: 输出缓存的大小
* @param[out]     int *abMark: 即EncKeyFrm，是否是关键帧
* @returns   类型int, 返回打包后的长度。如果编码打包失败，返回0.
*/
int GVECodecEnc(void *handle,char *apSrcData[3],int aiSrcDataLen,char *apDstBuff,int *aiDstBuffLen, int *abMark);
/**
* @ingroup	SVCCodec
* @brief    RTP拆包和分层解码
* @req		GVE-SRS-122
* @req		GVE-SRS-123
* @param[in]     void *handle: 解码句柄，包含DecParams即编码参数
* @param[in]     char *apSrcData: 待拆包解码的码流
* @param[in]     int aiSrcDataLen: 待拆包解码的码流长度
* @param[out]     char *apDstBuff[3]: 解码后数据
* @param[out]     int *aiDstBuffLen: 输出缓存的大小
* @param[in]     unsigned short *packetSize: NULL(此参数现在未用)
* @param[in]     int count: 0(此参数现在未用)
* @param[in]     int *abMark: 即EncKeyFrm，是否是关键帧
* @returns   int, 解码后的长度
*/
int GVECodecDec(void *handle,char *apSrcData,int aiSrcDataLen,char *apDstBuff[3],int *aiDstBuffLen,int *DstWidth, int *DstHeight,unsigned short *packetSize, int count,Encoded_Decoded_Info info[4], Down_Layer_Info flags[1], int *lastenc);

/**
* @ingroup	SVCCodec
* @brief    编码器/解码器内存释放
* 与函数GVECodecInit()对应，释放其调用x264本身的初始化根据参数申请需要的空间
* @param[in]     void *handle: 编码器或者解码器句柄。
* @returns   int，但是代码中目前未用到返回。
*/
int GVECodecClose(void *handle);
/**
* @ingroup	SVCCodec
* @brief    编码器初始化时调用，为_pRtPacket申请空间，存放RTP包。
* @param[in]     void *handle: 编码器句柄。
*/
void *GVECodecGetRtpPoint(void *handle);
//这个函数目前未用到 Shelley 20130518
void *GVECodecGetParams(void *handle);
/**
* @brief    测试用函数，未来要删除。
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
