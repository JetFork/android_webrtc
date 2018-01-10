
#include <time.h>
#include "../../inc/gve_svc_enc.h"

#define  DATA_MAX  3000000

char* InputName = "test720p.yuv";
char* OutputName = "str.264";
char* RtpSizeName = "size.size";
char* RtpCountName = "count.count";

int main(int argc, const char * argv[])
{
    clock_t start,end;
    double cost_time = 0;
    double pertime;
    //int i = 0;
    //int j = 0, number = 0, discard = 0;
    //int write_size = 0;
    //int relcount = 0;
    unsigned int frameNum = 0;
    long file_len = 0,read_len = 0;
    short size[MAXRTPNUM];
    FILE * fpInfile = NULL;
    FILE * fpOutfile = NULL;
    FILE * fpRtpSizefile = NULL;
    FILE * fpRtpCountfile = NULL;
    unsigned long	GVE_H264Enc_Handle = 0;
    GVE_SVCEnc_OperatePar  OperatePar = {0};
    GVE_SVCEnc_OutPutInfo  OutPutInfo = {0};
    GVE_SVCEnc_ConfigPar   ConfigPar  = {/*{640,320,160},*/{1280,640,320},
        /*{480,240,120},*/{720,360,180},
        15,
        33,
        3,
        1000,
        {550,300,150},
        {{20,35},{20,35},{20,35}},
        6,
        40,
        28,
        0,
        3,
        0,
        1260,
        1,
        1,
        /*32*/16,
        0,
        2,
        /*1,*/2,
        /*5*/3,
        0,
        1,
        0};
    
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
    
    
    
    OperatePar.InPutLen = ConfigPar.Inwidth[0] * ConfigPar.InHeight[0] * 3/2;
    
    
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
    
    GVE_SVC_Encoder_Create(&GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
    
    do
    {
        frameNum++;
        read_len += fread(OperatePar.InBuf,1,OperatePar.InPutLen,fpInfile);
        
        start = clock();
        GVE_SVC_Encoder_Encoder(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
        end = clock();
        cost_time += (double)(end - start);
        
        if (OperatePar.OutputLen)
        {
            //  			discard  = 0;
            //  			write_size = 0;
            //  			for (j = 0;j < OperatePar.rtpcount;j++)
            //  			{
            //  				if (((number + j) % 10)||((number + j) == 0))
            //  				{
            //  					fwrite(&OperatePar.rtpsize[j],1,sizeof(short),fpRtpSizefile);
            //  					fwrite(OperatePar.OutBuf + write_size,1,OperatePar.rtpsize[j],fpOutfile);
            //  				}
            //  				else
            //  				{
            //  					discard += 1;
            //  				}
            //  				write_size += OperatePar.rtpsize[j];
            //  			}
            //  			number += j;
            //  			relcount = OperatePar.rtpcount - discard;
            //  			fwrite(&relcount,1,4,fpRtpCountfile);
            fwrite(OperatePar.OutBuf,1,OperatePar.OutputLen,fpOutfile);
            fwrite(&OperatePar.rtpcount,1,4,fpRtpCountfile);
            fwrite(OperatePar.rtpsize,1,OperatePar.rtpcount*sizeof(short),fpRtpSizefile);
        }
        
        
    } while (read_len < file_len);
    
    cost_time = cost_time/CLOCKS_PER_SEC;
    
    pertime = cost_time/frameNum;
    
    printf("cost total time = %f s\n, tatal frame  = %d \n,every fame cost number is %f s\n",cost_time,frameNum,pertime);
    
    GVE_SVC_Encoder_Destroy(GVE_H264Enc_Handle);
    
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