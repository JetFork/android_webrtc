
#include <stdio.h>
#include <stdlib.h>
#include "../../inc/gve_svcdec_api.h"

#define INBUF_SIZE  65536

char *filename = "str.264";
char *outfilename = "test.yuv";
char *RtpCountName = "count.count";
char *RtpSizeName = "size.size";

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

int main(int argc, const char * argv[])
{
    size_t readlen = 0;
    int frame_cout = 0;
    unsigned char *OutBuffer = NULL;
    FILE *Infp,*Outfp;
    FILE *fpRtpCount;
    FILE *fpRtpSize;
    long fpRtpCount_Len = 0,fpRtpSize_Len = 0;
    int *RtpCountBuffer = NULL;
    int *RtpCount_Ptr = NULL;
    short *RtpSizeBuffer  = NULL;
    short *RtpSize_Ptr = NULL;
    unsigned long	GVE_SVCDec_Handle = 0;
    GVE_SVCDec_OperatePar  OperatePar = {0};
    GVE_SVCDec_OutPutInfo  OutPutInfo = {0};
    GVE_SVCDec_ConfigPar   ConfigPar  = {0};
    
    ConfigPar.Width = 640;
    ConfigPar.Height = 360;
    ConfigPar.CompensateFlag = 0;
    
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
    
    //准备RTP包个数和大小数据
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
    //结束
    
    OperatePar.InBuf = malloc(INBUF_SIZE *sizeof(char));
    if(OperatePar.InBuf == NULL)
    {
        exit(1);
    }
    
    fseek(Infp,0,SEEK_END);
    ftell(Infp);
    fseek(Infp,0,SEEK_SET);
    
    fread(OperatePar.InBuf, 1, INBUF_SIZE, Infp);  //wyh
    
    GVE_SVC_Decoder_Create(&GVE_SVCDec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    
    OutBuffer = malloc(ConfigPar.Width * ConfigPar.Height *3 /2);
    if(OutBuffer == NULL)
    {
        exit(1);
    }
    OperatePar.OutBuf[0] = OutBuffer;
    OperatePar.OutBuf[1] = OutBuffer + ConfigPar.Width * ConfigPar.Height;
    OperatePar.OutBuf[2] = OperatePar.OutBuf[1] + ((ConfigPar.Width * ConfigPar.Height) >>2);
    
    do
    {
        //OperatePar.InPutLen = find_head(OperatePar.InBuf,INBUF_SIZE);
        //计算RTP大小和包个数以及总的长度
        OperatePar.rtpcount = *(RtpCount_Ptr++);
        OperatePar.rtpsize = RtpSize_Ptr ;
        RtpSize_Ptr += OperatePar.rtpcount;
        OperatePar.InPutLen = GetFrameLen(OperatePar.rtpsize,OperatePar.rtpcount);
        //结束
        
        GVE_SVC_Decoder_Decode(GVE_SVCDec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
        frame_cout ++;
        printf("decode frame %d\n",frame_cout);
        
        if (OperatePar.OutputLen > 0)
        {
            fwrite(OutBuffer, 1,OperatePar.OutputLen, Outfp);
        }
        
        memcpy(OperatePar.InBuf,OperatePar.InBuf + OperatePar.InPutLen,INBUF_SIZE - OperatePar.InPutLen);
        readlen = fread(OperatePar.InBuf + INBUF_SIZE - OperatePar.InPutLen, 1, OperatePar.InPutLen, Infp);  //wyh
        
    }while(readlen > 0);
    
    GVE_SVC_Decoder_Destroy(GVE_SVCDec_Handle);
    
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
    
    getchar();

    return 0;
}
