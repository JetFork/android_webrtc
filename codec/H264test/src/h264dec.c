#include <stdio.h>
#include <stdlib.h>
#include "libh264_dec_api.h"

int GVE_H264_Decoder_Create(unsigned int           *GVE_H264Dec_Handle,
							GVE_H264Dec_OperatePar *OperatePar,
							GVE_H264Dec_ConfigPar  *ConfigPar,
							GVE_H264Dec_OutPutInfo *OutPutInfo)
{
	decode264_handle *handle = NULL;

	if ((handle = malloc(sizeof(decode264_handle))) == NULL)
	{
		return NULL;
	}
	memset(handle,0,sizeof(decode264_handle));

	*GVE_H264Dec_Handle = (unsigned int)handle;

	 avcodec_register_all();

	av_init_packet(&handle->avpkt);

	handle->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!handle->codec) 
	{
		return NULL;	
	}

	handle->c = avcodec_alloc_context3(handle->codec);
	if (!handle->c) 
	{
		return NULL;	
	}

	if (avcodec_open2(handle->c, handle->codec, NULL) < 0) 
	{
		return NULL;
	}

	handle->frame = av_frame_alloc();
	if (!handle->frame) 
	{
		return NULL;
	}

	return 0;
}

void GVE_H264_Decoder_Destroy(unsigned int           GVE_H264Dec_Handle)
{
	decode264_handle *handle = (decode264_handle *)GVE_H264Dec_Handle;

	avcodec_close(handle->c);
	av_free(handle->c);
	av_frame_free(&handle->frame);
	if (handle != NULL)
	{
		free(handle);
	}
}

int GVE_H264_Decoder_Decoder(unsigned int            GVE_H264Dec_Handle,
							 GVE_H264Dec_OperatePar *OperatePar,
							 GVE_H264Dec_ConfigPar  *ConfigPar,
							 GVE_H264Dec_OutPutInfo *OutPutInfo)
{
	int len;
	char *OBuf[3];
	decode264_handle *handle = (decode264_handle *)GVE_H264Dec_Handle;

	handle->avpkt.data = OperatePar->InBuf;
	handle->avpkt.size = OperatePar->InPutLen;

	OperatePar->InPutUsed = avcodec_decode_video2(handle->c, handle->frame, &handle->got_frame, &handle->avpkt);
	if (OperatePar->InPutUsed < 0) {
		printf("Error while decoding frame\n");
		return 1;
	}
	if (handle->got_frame) 
	{
		OutPutInfo->width = handle->c->width;
		OutPutInfo->height = handle->c->height;
		OperatePar->OutputLen = (OutPutInfo->width * OutPutInfo->height*3)>>1;
		OBuf[0] = OperatePar->OutBuf;
		OBuf[1] = OperatePar->OutBuf + OutPutInfo->width * OutPutInfo->height;
		OBuf[2] = OBuf[1] + ((OutPutInfo->width * OutPutInfo->height) >>2);
		memcpy(OBuf[0],handle->frame->data[0],handle->frame->linesize[0]*handle->frame->height);
		memcpy(OBuf[1],handle->frame->data[1],handle->frame->linesize[1]*handle->frame->height/2);
		memcpy(OBuf[2],handle->frame->data[2],handle->frame->linesize[2]*handle->frame->height/2);	
	}

	return 0;
}
