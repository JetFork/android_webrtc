#ifndef _SVC_DEC_API_H_
#define _SVC_DEC_API_H_


#include <stdio.h> 

typedef struct 
{
	unsigned char *InBuf;
	unsigned char *OutBuf[3];     
	unsigned int  InPutLen;
	unsigned int  InPutUsed;
	unsigned int  OutputLen;
	short 	*rtpsize;
	int    rtpcount;

}GVE_SVCDec_OperatePar;

typedef struct
{
	int Width;
	int Height;
	//int CompensateFlag;

	//for debug
	FILE *debugfp;
	char *debugfilename;
	int debuglayer;
	int debugflag;
	//end

}GVE_SVCDec_ConfigPar;

typedef struct 
{
	int layer;
	int OutPutWidth;
	int OutPutHeight;
}GVE_SVCDec_OutPutInfo;


#ifdef __cplusplus
extern "C" {
#endif

int GVE_SVC_Decoder_Create(unsigned long           *GVE_SVCDec_Handle,
	GVE_SVCDec_OperatePar *OperatePar,
	GVE_SVCDec_ConfigPar  *ConfigPar,
	GVE_SVCDec_OutPutInfo *OutPutInfo);

int GVE_SVC_Decoder_Decode(unsigned long            GVE_SVCDec_Handle,
	GVE_SVCDec_OperatePar *OperatePar,
	GVE_SVCDec_ConfigPar  *ConfigPar,
	GVE_SVCDec_OutPutInfo *OutPutInfo);

void GVE_SVC_Decoder_SetDebugFile(unsigned long            GVE_SVCDec_Handle,
								  GVE_SVCDec_OperatePar *OperatePar,
								  GVE_SVCDec_ConfigPar  *ConfigPar,
								  GVE_SVCDec_OutPutInfo *OutPutInfo);

void GVE_SVC_Decoder_Destroy(unsigned long           GVE_SVCDec_Handle);

#ifdef __cplusplus
}
#endif

#endif//_SVC_DEC_API_H_
