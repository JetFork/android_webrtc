#ifndef __H264_DEC_API_H__
#define __H264_DEC_API_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct 
{
	unsigned char *InBuf;
	unsigned char *OutBuf[3];     
	unsigned int  InPutLen;
	unsigned int  InPutUsed;
	unsigned int  OutputLen;
	short 	*rtpsize;
	int    rtpcount;

}GVE_H264Dec_OperatePar;

typedef struct
{
	int temp;

}GVE_H264Dec_ConfigPar;

typedef struct 
{
	int width;
	int height;

}GVE_H264Dec_OutPutInfo;


int GVE_H264_Decoder_Create(unsigned long           *GVE_H264Dec_Handle,
							GVE_H264Dec_OperatePar *OperatePar,
							GVE_H264Dec_ConfigPar  *ConfigPar,
							GVE_H264Dec_OutPutInfo *OutPutInfo);

int GVE_H264_Decoder_Decoder(unsigned long            GVE_H264Dec_Handle,
							 GVE_H264Dec_OperatePar *OperatePar,
							 GVE_H264Dec_ConfigPar  *ConfigPar,
							 GVE_H264Dec_OutPutInfo *OutPutInfo); 

void GVE_H264_Decoder_Destroy(unsigned long           GVE_H264Dec_Handle);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif

