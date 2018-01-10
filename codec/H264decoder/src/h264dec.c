#include <stdio.h>
#include <stdlib.h>
//#include "libh264_dec_api.h"
#include "h264decrtp.h"
int GVE_H264_Decoder_Create(unsigned long           *GVE_H264Dec_Handle,
							GVE_H264Dec_OperatePar *OperatePar,
							GVE_H264Dec_ConfigPar  *ConfigPar,
							GVE_H264Dec_OutPutInfo *OutPutInfo)
{
	decode264_handle *handle = NULL;

	if ((handle = malloc(sizeof(decode264_handle))) == NULL)
	{
		return -1;
	}
	memset(handle,0,sizeof(decode264_handle));

	*GVE_H264Dec_Handle = (unsigned long/*int*/)handle;

	 avcodec_register_all();

	av_init_packet(&handle->avpkt);

	handle->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!handle->codec) 
	{
		return 1;	
	}

	handle->c = avcodec_alloc_context3(handle->codec);
	if (!handle->c) 
	{
		return 1;	
	}

	if (avcodec_open2(handle->c, handle->codec, NULL) < 0) 
	{
		return 1;
	}

	handle->frame = av_frame_alloc();
	if (!handle->frame) 
	{
		return 1;
	}

	//初始化RTP包数据
	if(H264dec_avInitRtPacket(&handle->RtpPacket) < 0)
		return 1;

	return 0;
}

void GVE_H264_Decoder_Destroy(unsigned long           GVE_H264Dec_Handle)
{
	decode264_handle *handle = (decode264_handle *)GVE_H264Dec_Handle;

	avcodec_close(handle->c);
	av_free(handle->c);
	av_frame_free(&handle->frame);

	//释放RTP包结构
	H264dec_avFreePacket(&handle->RtpPacket);
	//结束

	if (handle != NULL)
	{
		free(handle);
	}
}

FILE* fyuv=NULL;
FILE* f264= NULL;
int GVE_H264_Decoder_Decoder(unsigned long            GVE_H264Dec_Handle,
							 GVE_H264Dec_OperatePar *OperatePar,
							 GVE_H264Dec_ConfigPar  *ConfigPar,
							 GVE_H264Dec_OutPutInfo *OutPutInfo)
{
	decode264_handle *handle = (decode264_handle *)GVE_H264Dec_Handle;
	int i = 0;

	handle->avpkt.data = OperatePar->InBuf;
	OperatePar->OutputLen = 0;

	//RTP解包
	H264dec_avResetRtPacket(&handle->RtpPacket,(char *)OperatePar->InBuf,(unsigned short *)OperatePar->rtpsize,OperatePar->rtpcount);
	handle->RtpPacket.rtpLen = OperatePar->InPutLen;
	handle->RtpPacket.pos = 0;
	H264dec_avRtp2RawStream(&handle->RtpPacket, &handle->avpkt);
	//结束
#if 0
	if(f264 == NULL)
				f264= fopen("/sdcard/test264.264","wb");
			if(f264){
				fwrite(handle->avpkt.data, 1, handle->avpkt.size, f264);
				fflush(f264);
				}
#endif
	OperatePar->InPutUsed = avcodec_decode_video2(handle->c, handle->frame, &handle->got_frame, &handle->avpkt);
	if (OperatePar->InPutUsed < 0) {
		printf("Error while decoding frame\n");
		return 1;
	}
	if (handle->got_frame) 
	{
		if(fyuv == NULL)
			fyuv = fopen("/sdcard/testyuv.yuv","wb");
		if(fyuv){
		//	fwrite(handle->frame->data[0], 1, handle->frame->linesize[0]*handle->frame->height, fyuv);
		//	fwrite(handle->frame->data[1], 1, handle->frame->linesize[1]*handle->frame->height/2,fyuv);
		//	fwrite(handle->frame->data[2], 1, handle->frame->linesize[2]*handle->frame->height/2,fyuv);
		//	fflush(fyuv);
			}
//			__android_log_print(ANDROID_LOG_ERROR, "yyf","decoder decode output width =  %d, height=%d, linesize = %d",handle->c->width, handle->c->height, handle->frame->linesize[0]);
		OutPutInfo->width = handle->c->width;
		OutPutInfo->height = handle->c->height;
		OperatePar->OutputLen = (OutPutInfo->width * OutPutInfo->height*3)>>1;
		if (handle->c->width==handle->frame->linesize[0])
		{
			memcpy(OperatePar->OutBuf[0],handle->frame->data[0],handle->frame->linesize[0]*handle->frame->height);
			memcpy(OperatePar->OutBuf[1],handle->frame->data[1],handle->frame->linesize[1]*handle->frame->height/2);
			memcpy(OperatePar->OutBuf[2],handle->frame->data[2],handle->frame->linesize[2]*handle->frame->height/2);	
		}
		else
		{
			for (i=0;i<handle->c->height/2;i++)
			{
				memcpy(OperatePar->OutBuf[0]+i*2*handle->c->width,handle->frame->data[0]+i*2*handle->frame->linesize[0],handle->c->width);
				memcpy(OperatePar->OutBuf[0]+(i*2+1)*handle->c->width,handle->frame->data[0]+(i*2+1)*handle->frame->linesize[0],handle->c->width);
				memcpy(OperatePar->OutBuf[1]+i*handle->c->width/2,handle->frame->data[1]+i*handle->frame->linesize[1],handle->c->width/2);
				memcpy(OperatePar->OutBuf[2]+i*handle->c->width/2,handle->frame->data[2]+i*handle->frame->linesize[1],handle->c->width/2);
			}
			
		}
		
	}

	return 0;
}
