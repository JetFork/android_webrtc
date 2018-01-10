#include <stdio.h>
#include <stdlib.h>
#include "../../inc/h264_dec_api.h"

#define INBUF_SIZE  81920

char *filename = "str.264";
char *outfilename = "test.yuv";
char *RtpCountName = "count.count";
char *RtpSizeName = "size.size";

int find_head(unsigned char*buffer, int len)  
{  
	int i;  

	for(i=256;i<len;i++)  
	{  
		if(buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 0&& buffer[i+3] == 1)  
			break;  
	}  
	if (i ==len)  
		return 0;  
	return i;  
}

int GetFrameLen(short *rtpsize,int rtpcount)
{
	int i,len = 0;
	short *rtpsize_ptr = rtpsize;

	for (i = 0;i < rtpcount;i++)
	{
		len += *(rtpsize_ptr++);
	}

	return len;
}

int main()
{
    int readlen;
	int width,height;
	int frame_cout = 0;
	char *OutBuffer;
	FILE *Infp,*Outfp;
	FILE *fpRtpCount;
	FILE *fpRtpSize;
	unsigned int fpRtpCount_Len,fpRtpSize_Len;
	int *RtpCountBuffer = NULL;
	int *RtpCount_Ptr = NULL;
	short *RtpSizeBuffer  = NULL;
	short *RtpSize_Ptr = NULL;
	unsigned int	GVE_H264Dec_Handle = 0;
	GVE_H264Dec_OperatePar  OperatePar = {0};
	GVE_H264Dec_ConfigPar   ConfigPar  = {0};
	GVE_H264Dec_OutPutInfo  OutPutInfo = {0};
	long int x;


	width = 1280;
	height = 720;

    Infp = fopen(filename, "rb");
    if (!Infp) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

	Outfp = fopen(outfilename, "wb");
	if (!Outfp) {
		fprintf(stderr, "Could not open %s\n", outfilename);
		exit(1);
	}

	//׼��RTP�������ʹ�С����
	fpRtpCount = fopen(RtpCountName, "rb");
	if (!fpRtpCount) {
		fprintf(stderr, "Could not open %s\n", RtpCountName);
		exit(1);
	}
	fpRtpSize = fopen(RtpSizeName, "rb");
	if (!fpRtpSize) {
		fprintf(stderr, "Could not open %s\n", RtpSizeName);
		exit(1);
	}
	fseek(fpRtpCount,0,SEEK_END);
	fpRtpCount_Len = ftell(fpRtpCount);
	fseek(fpRtpCount,0,SEEK_SET);
	fseek(fpRtpSize,0,SEEK_END);
	fpRtpSize_Len = ftell(fpRtpSize);
	fseek(fpRtpSize,0,SEEK_SET);
	RtpCountBuffer = (int *)malloc(fpRtpCount_Len *sizeof(char));
	if(RtpCountBuffer == NULL)
	{
		exit(1);
	}
	RtpSizeBuffer = (short *)malloc(fpRtpSize_Len *sizeof(short));
	if(RtpSizeBuffer == NULL)
	{
		exit(1);
	}
	fread(RtpCountBuffer, 1, fpRtpCount_Len, fpRtpCount);
	fread(RtpSizeBuffer,1,fpRtpSize_Len,fpRtpSize);
	RtpCount_Ptr = RtpCountBuffer;
	RtpSize_Ptr = RtpSizeBuffer;
	//����

	OperatePar.InBuf = malloc(INBUF_SIZE *sizeof(char));
	if(OperatePar.InBuf == NULL)
	{
		exit(1);
	}
	OutBuffer = malloc(width * height *3 /2);
	if(OutBuffer == NULL)
	{
		exit(1);
	}
	OperatePar.OutBuf[0] = OutBuffer;
	OperatePar.OutBuf[1] = OutBuffer + width * height;
	OperatePar.OutBuf[2] = OperatePar.OutBuf[1] + ((width * height) >>2);

	fseek(Infp,0,SEEK_END);
	x = ftell(Infp);
	fseek(Infp,0,SEEK_SET);

	GVE_H264_Decoder_Create(&GVE_H264Dec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);


	x = fread(OperatePar.InBuf, 1, INBUF_SIZE, Infp);  //wyh

    do 
	{
		//OperatePar.InPutLen = find_head(OperatePar.InBuf,INBUF_SIZE);
		//����RTP��С�Ͱ������Լ��ܵĳ���
		OperatePar.rtpcount = *(RtpCount_Ptr++);
		OperatePar.rtpsize = RtpSize_Ptr ;
		RtpSize_Ptr += OperatePar.rtpcount;
		OperatePar.InPutLen = GetFrameLen(OperatePar.rtpsize,OperatePar.rtpcount);
		//����
 
		GVE_H264_Decoder_Decoder(GVE_H264Dec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
		frame_cout ++;
		printf("decode frame %d\n",frame_cout);

		if (OperatePar.OutputLen > 0)
		{
			//fwrite(OutBuffer, 1,OperatePar.OutputLen, Outfp);
			fwrite(OperatePar.OutBuf[0], 1, OutPutInfo.width*OutPutInfo.height, Outfp);
			fwrite(OperatePar.OutBuf[1], 1, OutPutInfo.width*OutPutInfo.height>>2, Outfp);
			fwrite(OperatePar.OutBuf[2], 1, OutPutInfo.width*OutPutInfo.height>>2, Outfp);
		}

		memcpy(OperatePar.InBuf,OperatePar.InBuf + OperatePar.InPutLen,INBUF_SIZE - OperatePar.InPutLen);
		readlen = fread(OperatePar.InBuf + INBUF_SIZE - OperatePar.InPutLen, 1, OperatePar.InPutLen, Infp);  //wyh

    }while(readlen > 0);

	GVE_H264_Decoder_Destroy(GVE_H264Dec_Handle);

	if (OperatePar.InBuf != NULL)
	{
		free(OperatePar.InBuf);
	}
	if (OutBuffer != NULL)
	{
		free(OutBuffer);
	}

	if (RtpCountBuffer != NULL)
	{
		free(RtpCountBuffer);
	}
	if (RtpSizeBuffer != NULL)
	{
		free(RtpSizeBuffer);
	}
	
    fclose(Infp);
	fclose(Outfp);
	fclose(fpRtpCount);
	fclose(fpRtpSize);

    return 0;
}
