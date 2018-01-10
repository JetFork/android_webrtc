#ifndef __H264_ENC_API_H_
#define __H264_ENC_API_H_

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

}GVE_H264Enc_OperatePar;

typedef struct
{
	unsigned int    InPut_Width;
	unsigned int	  InPut_Height;
	unsigned int    GOPSize;
	unsigned int    FrameRate;
	unsigned int    BFrameNum;
	unsigned int    QPMin;
	unsigned int    QPMax;
	int                   RC_Method;
	unsigned int     BitsRate;
	unsigned int     RF_Constant;
	int                   Sliced_Threads;
	int  Mult_Slice;
	int  Mtu_Size;
	int  Me_Range;
	int  Gen_Threads;
	int  Subpel_Refine;
	int  Me_Method;
	int  Enable_Cabac;
	int  Psnr;
	int	  Level;
	//int  CodeType; //1:baseline 0:main

}GVE_H264Enc_ConfigPar;

typedef struct 
{
	int    abMark;
	int    pts;

}GVE_H264Enc_OutPutInfo;


int GVE_H264_Encoder_Create(unsigned long           *GVE_H264Enc_Handle,
							GVE_H264Enc_OperatePar *OperatePar,
							GVE_H264Enc_ConfigPar  *ConfigPar,
							GVE_H264Enc_OutPutInfo *OutPutInfo);

int GVE_H264_Encoder_Encoder(unsigned long            GVE_H264Enc_Handle,
							 GVE_H264Enc_OperatePar *OperatePar,
							 GVE_H264Enc_ConfigPar  *ConfigPar,
							 GVE_H264Enc_OutPutInfo *OutPutInfo); 

void GVE_H264_Encoder_ResetBitrate(unsigned long            GVE_H264Enc_Handle,
								   GVE_H264Enc_OperatePar *OperatePar,
								   GVE_H264Enc_ConfigPar  *ConfigPar,
								   GVE_H264Enc_OutPutInfo *OutPutInfo);

int GVE_H264_Encoder_GetSliceType(unsigned long           GVE_H264Enc_Handle,
								  GVE_H264Enc_OperatePar *OperatePar,
								  GVE_H264Enc_ConfigPar  *ConfigPar,
								  GVE_H264Enc_OutPutInfo *OutPutInfo);

void GVE_H264_Encoder_Destroy(unsigned long           GVE_H264Enc_Handle);


#ifdef __cplusplus
}
#endif//__cplusplus


#endif//__H264_ENC_API_H_

