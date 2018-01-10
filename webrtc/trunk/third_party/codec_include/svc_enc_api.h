#ifndef _SVC_ENC_API_H_
#define _SVC_ENC_API_H_


#include <stdio.h> 

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	unsigned char *InBuf;
	unsigned char *OutBuf;     
	unsigned int  InPutLen;
	unsigned int  InPutUsed;
	unsigned int  OutputLen;
	short 	*rtpsize;
	int    rtpcount;

}GVE_SVCEnc_OperatePar;

typedef struct
{
	int Inwidth[3];//当前最大编码宽，或解码目标宽
	int InHeight[3];//当前最大编码高，或解码目标高
	int framerate;
	int gop;
	int bframenum;
	int bitsrate;//kbps
	int bitrateLayer[3];
	int qp[3][2];
	int qpmin;
	int qpmax;
	int rf_constant;//qp_constant;
	int psnr;
	int layer;
	int redundancy;
	int mtu_size;
	int mult_slice;
	int ml_err_resilience;
	int me_range;
	int sliced_threads;
	int gen_threads;//default:0
	int rc_method;//CQP(0)CRF(1)ABR(2)
	int subpel_refine;//--subme
	int me_method;//--me//DIA(0)HEX(1)UMH(2)ESA(3)TESA(4)
	int enable_cabac;
	int enable_8x8dct;
	unsigned int intra;
	unsigned int inter;
	char *preset;
	char *tune;
	int frame_reference;
	int lookahead;

	//for debug
	FILE *debugfp;
	char *debugfilename;
	int debuglayer;
	int debugflag;
	//end debug

}GVE_SVCEnc_ConfigPar;

typedef struct 
{
	int    abMark;
	int    pts;

}GVE_SVCEnc_OutPutInfo;


int GVE_SVC_Encoder_Create(unsigned long           *GVE_SVCEnc_Handle,
						   GVE_SVCEnc_OperatePar *OperatePar,
						   GVE_SVCEnc_ConfigPar  *ConfigPar,
						   GVE_SVCEnc_OutPutInfo *OutPutInfo);

int GVE_SVC_Encoder_Encoder(unsigned long            GVE_SVCEnc_Handle,
							GVE_SVCEnc_OperatePar *OperatePar,
							GVE_SVCEnc_ConfigPar  *ConfigPar,
							GVE_SVCEnc_OutPutInfo *OutPutInfo);

void GVE_SVC_Encoder_ResetBitrate(unsigned long            GVE_SVCEnc_Handle,
								  GVE_SVCEnc_OperatePar *OperatePar,
								  GVE_SVCEnc_ConfigPar  *ConfigPar,
								  GVE_SVCEnc_OutPutInfo *OutPutInfo);

void GVE_SVC_Encoder_SetDebugFile(unsigned long            GVE_SVCEnc_Handle,
								  GVE_SVCEnc_OperatePar *OperatePar,
								  GVE_SVCEnc_ConfigPar  *ConfigPar,
								  GVE_SVCEnc_OutPutInfo *OutPutInfo);

void GVE_SVC_Encoder_Destroy(unsigned long           GVE_SVCEnc_Handle);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif//_SVC_ENC_API_H_
