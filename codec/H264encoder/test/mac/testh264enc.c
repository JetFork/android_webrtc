#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../../inc/h264_api.h"

#define  DATA_MAX  3000000

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

char* InputName = "test720p.yuv";
char* OutputName = "str.264";
char* RtpSizeName = "size.size";
char* RtpCountName = "count.count";

static int avGetSliceType(int gopSize, int bframenum,int start,int frameNum, int  idx)
{
    int delta=idx-start;
    if (delta <0)
    {
        return -1;
    }
    delta = frameNum-8*delta;
    if ( delta <0)
    {
        return -1;
    }
    
    if (delta%gopSize == 0)
    {
        return PIC_TYPE_IDR;
    }
    else
    {
        return PIC_TYPE_AUTO;
    }
}

int main(int argc, const char * argv[])
{
    int i;
    clock_t start,end;
    double cost_time = 0;
    double pertime;
    unsigned int frameNum = 0;
    long file_len = 0;
    unsigned int read_len = 0;
    short size[MAXRTPNUM];
    FILE * fpInfile = NULL;
    FILE * fpOutfile = NULL;
    FILE * fpRtpSizefile = NULL;
    FILE * fpRtpCountfile = NULL;
    unsigned long GVE_H264Enc_Handle = 0;
    GVE_H264Enc_OperatePar  OperatePar = {0};
    GVE_H264Enc_ConfigPar   ConfigPar  = {0};
    GVE_H264Enc_OutPutInfo  OutPutInfo = {0};
    
    if((fpInfile = fopen(InputName,"rb")) == NULL)
    {
        printf("Fail to open input yuv file!\n");
        return 1;
    }
    if((fpOutfile = fopen(OutputName,"wb")) == NULL)
    {
        printf("Fail to open output stream file!\n");
        return 1;
    }
    if((fpRtpSizefile = fopen(RtpSizeName,"wb")) == NULL)
    {
        printf("Fail to open RTP Size file!\n");
        return 1;
    }
    if((fpRtpCountfile = fopen(RtpCountName,"wb")) == NULL)
    {
        printf("Fail to open RTP count file!\n");
        return 1;
    }
    OperatePar.rtpsize = size;
    
    // 	ConfigPar->CodeType = 1;
    // 	if (ConfigPar->CodeType)
    // 	{
    // 	}
    // 	else
    // 	{
    ConfigPar.InPut_Width = 1280;
    ConfigPar.InPut_Height = 720;
    ConfigPar.FrameRate = 15;
    ConfigPar.GOPSize = 33;
    ConfigPar.BFrameNum = 3;
    ConfigPar.BitsRate = 273;
    ConfigPar.QPMin = 20;
    ConfigPar.QPMax = 35;
    ConfigPar.Mult_Slice = 1;
    ConfigPar.Mtu_Size = 1260;
    ConfigPar.Me_Range = 16;
    ConfigPar.Gen_Threads = 1;
    ConfigPar.RC_Method = 2;
    ConfigPar.Subpel_Refine = 5;
    ConfigPar.Gen_Threads =2;  //ø™∆Ù2∏ˆœﬂ≥Ã
    //}
    
    
    OperatePar.InPutLen = ConfigPar.InPut_Width * ConfigPar.InPut_Height * 3/2;
    
    
    if(NULL == (OperatePar.InBuf = (unsigned char *)malloc(OperatePar.InPutLen)))
    {
        printf("Fail to initialize yuv buffer! \n");
        return 1;
    }
    
    if(NULL == (OperatePar.OutBuf = (unsigned char *)malloc(sizeof(unsigned char)*DATA_MAX)))
    {
        printf("Fail to initialize stream buffer! \n");
        return 1;
    }
    
    fseek(fpInfile,0,SEEK_END);
    file_len = ftell(fpInfile);
    fseek(fpInfile,0,SEEK_SET);
    
    GVE_H264_Encoder_Create(&GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    
    do
    {
        read_len += fread(OperatePar.InBuf,1,OperatePar.InPutLen,fpInfile);
        
        OutPutInfo.abMark= GVE_H264_Encoder_GetSliceType(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
        start = clock();
        GVE_H264_Encoder_Encoder(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
        end = clock();
        cost_time += (double)(end - start);
        frameNum++;
        
        printf("frame = %3d\n",frameNum);
        for (i = 0; i< OperatePar.rtpcount;i++)
        {
            printf("RTP count %3d size is %5d\n ",i,OperatePar.rtpsize[i]);
        }
        printf("\n");
        
        if (OperatePar.OutputLen)
        {
            fwrite(OperatePar.OutBuf,1,OperatePar.OutputLen,fpOutfile);
            fwrite(&OperatePar.rtpcount,1,4,fpRtpCountfile);
            fwrite(OperatePar.rtpsize,1,OperatePar.rtpcount*sizeof(short),fpRtpSizefile);
        }
        
    } while (read_len < file_len);
    
    cost_time = cost_time/CLOCKS_PER_SEC;
    pertime = cost_time/frameNum;
    printf("cost total time = %f s\n, tatal frame  = %d \n,every fame cost number is %f s\n",cost_time,frameNum,pertime);
    GVE_H264_Encoder_Destroy(GVE_H264Enc_Handle);
    
    if (OperatePar.InBuf != NULL)
    {
        free(OperatePar.InBuf);
    }
    
    if (OperatePar.OutBuf != NULL)
    {
        free(OperatePar.OutBuf);
    }
    
    if (fpInfile)
    {
        fclose(fpInfile);
    }
    if (fpOutfile)
    {
        fclose(fpOutfile);
    }
    if (fpRtpCountfile)
    {
        fclose(fpRtpCountfile);
    }
    if (fpRtpSizefile)
    {
        fclose(fpRtpSizefile);
    }
    getchar();
    return 0;
}