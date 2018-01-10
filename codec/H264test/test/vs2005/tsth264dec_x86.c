#include <stdio.h>
#include <stdlib.h>
#include "../../inc/h264_dec_api.h"

#define INBUF_SIZE  40960

char *filename = "layer0.stream";
char *outfilename = "test.yuv";

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

int main()
{
    int readlen;
	int width,height;
	int frame_cout = 0;
	FILE *Infp,*Outfp;
	unsigned int	GVE_H264Dec_Handle = 0;
	GVE_H264Dec_OperatePar  OperatePar = {0};
	GVE_H264Dec_ConfigPar   ConfigPar  = {0};
	GVE_H264Dec_OutPutInfo  OutPutInfo = {0};


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

	OperatePar.InBuf = malloc(INBUF_SIZE *sizeof(char));
	if(OperatePar.InBuf == NULL)
	{
		exit(1);
	}
	OperatePar.OutBuf = malloc(width * height *3 /2);
	if(OperatePar.OutBuf == NULL)
	{
		exit(1);
	}

	fseek(Infp,0,SEEK_END);
	ftell(Infp);
	fseek(Infp,0,SEEK_SET);

	GVE_H264_Decoder_Create(&GVE_H264Dec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);


	fread(OperatePar.InBuf, 1, INBUF_SIZE, Infp);  //wyh

    do 
	{
		OperatePar.InPutLen = find_head(OperatePar.InBuf,INBUF_SIZE);
 
		GVE_H264_Decoder_Decoder(GVE_H264Dec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
		frame_cout ++;
		printf("decode frame %d\n",frame_cout);

		if (OperatePar.OutputLen > 0)
		{
			fwrite(OperatePar.OutBuf, 1,OperatePar.OutputLen, Outfp);
		}

		memcpy(OperatePar.InBuf,OperatePar.InBuf + OperatePar.InPutLen,INBUF_SIZE - OperatePar.InPutLen);
		readlen = fread(OperatePar.InBuf + INBUF_SIZE - OperatePar.InPutLen, 1, OperatePar.InPutLen, Infp);  //wyh

    }while(readlen > 0);

	GVE_H264_Decoder_Destroy(GVE_H264Dec_Handle);

	if (OperatePar.InBuf != NULL)
	{
		free(OperatePar.InBuf);
	}
	if (OperatePar.OutBuf != NULL)
	{
		free(OperatePar.OutBuf);
	}
	
    fclose(Infp);
	fclose(Outfp);

    return 0;
}
