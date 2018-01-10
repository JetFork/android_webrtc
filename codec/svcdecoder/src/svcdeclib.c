/*****************************************************************************
* x264: top-level x264cli functions
*****************************************************************************
* Copyright (C) 2003-2012 x264 project
*
* Authors: Loren Merritt <lorenm@u.washington.edu>
*          Laurent Aimar <fenrir@via.ecp.fr>
*          Steven Walters <kemuri9@gmail.com>
*          Jason Garrett-Glaser <darkshikari@gmail.com>
*          Kieran Kunhya <kieran@kunhya.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
*
* This program is also available under a commercial proprietary license.
* For more information, contact us at licensing@x264.com.
*****************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include "svcdecrtp.h"

#define FDEC_STRIDE     ((1 << FDEC_FACTOR) - 1)

//add wyh
//extern int *DEC_SVCGop;
//extern int64_t *DEC_SVCGopPTS;
//end wyh

const unsigned short ResolutionTab[16][3] = 
{
	{1920,1080, 0},    //1080p mode
	{960,	544,	1},
	{960,	540,	1},
	{480,	272,	2},
	{1280	,720	,0},	//720p 16:9mode
	{640	,360	,1},
	{320	,180	,2},
	{160	,90	,3},
	{640    ,480, 0},	//640 4:3mode
	{320    ,240, 1},
	{160	,120, 2},
	{80		,60,	 3},
	{720	,1280	,0},  //720p 9:16mode
	{360	,640	,1},
	{180	,320	,2},
	{90	,160	,3},
};

static int SVCDec_GetStartIFrame(int gopsize,int layer,int startI[])
{
	int dis = gopsize/layer;

	switch(layer)
	{
	case 3:
		startI[0] = 0;
		startI[1] = dis;
		startI[2] = dis *2;
		break;
	case 2:
		startI[0] =  0;
		startI[1] = dis;
		break;
	case 1:
		startI[0] = 0;
		break;
	default:
		printf("layer must greater than 1\n");
	}
	return 0;
}

static int SVCDec_avGetWidthAndHeight(int *width, int *height)
{
	int w = *width;
	int h = *height;

	w = ((w + FDEC_STRIDE) >> FDEC_FACTOR) << FDEC_FACTOR;
	h = ((h + FDEC_STRIDE1) >> FDEC_FACTOR1) << FDEC_FACTOR1;//在IOS上height必须被16整除，否则会异常，此处需要再次确认；
	w = CLIP3(w,MINWIDTH,MAXWIDTH);
	h = CLIP3(h,MINHEITHT,MAXHEIGHT);

	*width = w;
	*height = h;
	return 0;
}

/**
* @details	处理流程：
*
* @verbatim
STEP1:判断参数符合要求，否则返回NULL;
STEP2:为编码器/解码器句柄申请空间
STEP3:根据传进参数对编码器/解码器进行初始化
STEP4:调用x264本身的初始化根据参数申请需要的空间
@endverbatim 
*/
int GVE_SVC_Decoder_Create(unsigned long           *GVE_SVCDec_Handle,
						   GVE_SVCDec_OperatePar *OperatePar,
						   GVE_SVCDec_ConfigPar  *ConfigPar,
						   GVE_SVCDec_OutPutInfo *OutPutInfo)
{
	int x,y;
	SVCdec_Handle *h = NULL;
	int i = 0;
	RTPEXTENDHEADER *rtpexten = NULL;

	//  为编码器/解码器句柄申请空间
	h = (SVCdec_Handle *)malloc( sizeof(SVCdec_Handle) );
	memset(h, 0, sizeof(SVCdec_Handle));

	*GVE_SVCDec_Handle = (unsigned long/*int*/)h;

	//添加从码流中获得需要的参数
	h->decIDX = -1;
	rtpexten = (RTPEXTENDHEADER *)&OperatePar->InBuf[RTPHEADSIZE];
	h->encIDX = rtpexten->rtp_extend_globInfo & 0xF;
	h->encRatio =(rtpexten->rtp_extend_globInfo & 0xF0) >> 4;
	h->layer = (rtpexten->rtp_extend_profile & 0x1C0) >> 6;
	h->bframenum = ((rtpexten->rtp_extend_profile & 0xE00) >> 9);
	h->gopSize = (rtpexten->rtp_extend_svcheader & 0xFE00) >> 9;
	h->width[0] = (int)ResolutionTab[h->encIDX][0];
	h->height[0] = (int)ResolutionTab[h->encIDX][1];
	h->width[1] = h->width[0] >>1;
	h->width[2] = h->width[0] >>2;
	h->height[1] = h->height[0] >>1;
	h->height[2] = h->height[0] >>2;
	//h->displayLayer = ConfigPar->ShowLayer;

// 	if ((ConfigPar->ShowLayer >= h->layer)||(ConfigPar->ShowLayer < 0))
// 	{
// 		printf("error layer is input\n");
// 		return -1;
// 	}

	SVCDec_GetStartIFrame(h->gopSize,h->layer,h->startInum);
	//end

	if(h)
	{	//  根据传进参数对编码器/解码器进行初始化
		//注意：解码端设置的帧率可能会与编码端实际帧率不一致；因此要强制使得编码端的GOP是解码端给出的；

		h->lastEnc = h->lastEncRatio = -1;
		memset(&h->info,0,3 *sizeof(Encoded_Decoded_Info));
		for (i=0; i<MAXLAYER/*h->layer*/; i++)
		{
			int size;
			int width = 1920 >>i/*h->width[i]*/;
			int height = 1088 >> i/*h->height[i]*/;
			SVCDec_avGetWidthAndHeight(&width,&height);
// 			if (i == h->displayLayer)
// 			{
// 				OutPutInfo->OutPutWidth = h->width[i];//width;
// 				OutPutInfo->OutPutHeight = h->height[i];//height;
// 			}
			size = width * height;
			h->DecBuf[i][0] = (char *)malloc(size);
			h->DecBuf[i][1] = (char *)malloc(size >>2);
			h->DecBuf[i][2] = (char *)malloc(size>>2);
		}

		if( h->layer > MAXLAYER )
			return -1;
		h->decFrameNum = 0;

		for (i = 0;i<16;i++)
		{
			int width = (int)ResolutionTab[i][0];
			int height = (int)ResolutionTab[i][1];
			if (ConfigPar->Width == width && ConfigPar->Height == height)
			{
				h->decIDX = (int)ResolutionTab[i][2];//i;
				break;
			}
		}
		if (h->decIDX == -1)
		{
			h->decIDX = h->encIDX;
		}

		avcodec_register_all();
			
		//解码分辨率和层级由观看端即解码端确定
		for ( i = 0; i < /*h->layer*/3; i++)
		{	
//decode264Init();
			if ((h->ffmpegdec_handle[i] = malloc(sizeof(ffmpeg_info))) == NULL)
			{
				return -1;
			}
			memset(h->ffmpegdec_handle[i],0,sizeof(ffmpeg_info));
			av_init_packet(&h->ffmpegdec_handle[i]->avpkt);
			h->ffmpegdec_handle[i]->codec = avcodec_find_decoder(AV_CODEC_ID_H264);
			if (!h->ffmpegdec_handle[i]->codec) 
			{
				return -1;	
			}
			h->ffmpegdec_handle[i]->c = avcodec_alloc_context3(h->ffmpegdec_handle[i]->codec);
			if (!h->ffmpegdec_handle[i]->c) 
			{
				return -1;	
			}
			if (avcodec_open2(h->ffmpegdec_handle[i]->c, h->ffmpegdec_handle[i]->codec, NULL) < 0) 
			{
				return -1;
			}
			h->ffmpegdec_handle[i]->frame = av_frame_alloc();
			if (!h->ffmpegdec_handle[i]->frame) 
			{
				return -1;
			}
//end decode264Init();

			h->dPacket[i] = (SVCDecPacket *)malloc(sizeof(SVCDecPacket)); 
			if(h->dPacket[i] == NULL)
				return -1;
			memset(h->dPacket[i],0,sizeof(SVCDecPacket));
			h->dPacket[i]->pts =0;
		}
		if(SVCDec_avInitRtPacket(&h->dRtpPacket,ConfigPar) < 0)
			return -1;
		else
		{
			h->dRtpPacket.payLoadType = PAYLOADTYPE_SVC;//h->payLoadType;
			h->dRtpPacket.encIDX = h->encIDX;
			h->dRtpPacket.encRatio = h->encRatio;
			h->dRtpPacket.gop = h->gopSize;
		}
		if(SVCDec_avInitPacket(h->dPacket,/*h->layer*/3) < 0)
			return -1;

		//add wyh生成丢包检测的数组
		x = ((h->gopSize - 1)/4) <<2;
		y = (h->gopSize - 1)%4;
		if ((h->DEC_SVCGop = (int *)malloc(h->gopSize * sizeof(int))) == NULL)
		{
			return -1;
		}
		memset(h->DEC_SVCGop,0,h->gopSize *sizeof(int));
		if ((h->DEC_SVCGopPTS = (int *)malloc(h->gopSize * sizeof(int64_t))) == NULL)
		{
			return -1;
		}
		memset(h->DEC_SVCGopPTS,0,h->gopSize *sizeof(int64_t));
		h->DEC_SVCGop[0] = PIC_TYPE_I;
		h->DEC_SVCGopPTS[0] = 1;
		for (i = 1;i < x/*ConfigPar->gop*/; i += 4)
		{
			h->DEC_SVCGop[i] = PIC_TYPE_P;
			h->DEC_SVCGop[i+1] = h->DEC_SVCGop[i+2] = h->DEC_SVCGop[i+3] = PIC_TYPE_B;
			h->DEC_SVCGopPTS[i] = i +4;
			h->DEC_SVCGopPTS[i + 1] = i +1;
			h->DEC_SVCGopPTS[i + 2] = i +2;
			h->DEC_SVCGopPTS[i + 3] = i +3;
		}
		if (y)
		{
			h->DEC_SVCGop[i] = PIC_TYPE_P;
			h->DEC_SVCGopPTS[i++] = i +4;
			for (;i < h->gopSize;i++)
			{
				h->DEC_SVCGop[i] = PIC_TYPE_B;
				h->DEC_SVCGopPTS[i] = i+1;
			}
		}
		//end wyh
	}
	return 0;
}

/**
* @brief    编码器/解码器内存释放
* 与函数GVECodecInit()对应，释放其调用x264本身的初始化根据参数申请需要的空间
* @param[in]     void *handle: 编码器或者解码器句柄。
* @returns   int，但是代码中目前未用到返回。
*/
void GVE_SVC_Decoder_Destroy(unsigned long GVE_SVCDec_Handle)
{
	int i;
	SVCdec_Handle *h = (SVCdec_Handle *) GVE_SVCDec_Handle;
	if(h->layer <= 0 || (h->layer > MAXLAYER))
		return;
	if(h)
	{
		SVCDec_avFreePacket(&h->dRtpPacket);
		for ( i = 0; i < 3/*h->layer*/; i++)
		{
			if(h->ffmpegdec_handle[i])
			{	
//decode264close();
				av_free_packet(&h->ffmpegdec_handle[i]->avpkt);
				avcodec_close(h->ffmpegdec_handle[i]->c);
				av_free(h->ffmpegdec_handle[i]->c);
				av_frame_free(&h->ffmpegdec_handle[i]->frame);
//end decode264close();
				free(h->ffmpegdec_handle[i]);
				h->ffmpegdec_handle[i] = NULL;
			}
			if(h->dPacket[i])
			{
				if(h->dPacket[i]->data)
				{
					free(h->dPacket[i]->data);
					h->dPacket[i]->data = NULL;
				}
				free(h->dPacket[i]);
				h->dPacket[i] = NULL;
			}
			free(h->DecBuf[i][0]);
			h->DecBuf[i][0] = NULL;
			free(h->DecBuf[i][1]);
			h->DecBuf[i][1] = NULL;
			free(h->DecBuf[i][2]);
			h->DecBuf[i][2] = NULL;
		}
		//add wyh
		if (h->DEC_SVCGop != NULL)
		{
			free(h->DEC_SVCGop);
			h->DEC_SVCGop = NULL;
		}
		if(h->DEC_SVCGopPTS != NULL)
		{
			free(h->DEC_SVCGopPTS);
			h->DEC_SVCGopPTS = NULL;
		}
		//end wyh
		free(h);
		h = NULL;

	}
	return;
}

int GVE_SVC_Decoder_Decode(unsigned long            GVE_SVCDec_Handle,
						   GVE_SVCDec_OperatePar *OperatePar,
						   GVE_SVCDec_ConfigPar  *ConfigPar,
						   GVE_SVCDec_OutPutInfo *OutPutInfo)
{
	SVCdec_Handle *h = (SVCdec_Handle *)GVE_SVCDec_Handle;
	int packetSize = 0;
	int i,j = 0;
	int showLevel;
	int maxPts=0;
	int maxSize=0;
	int maxW=0;
	int maxH=0;
	int maxLayer =0;
	int lastlayer;
	int len;
	int display = 0;
	int UVwidth,UVheight;
	RTPEXTENDHEADER *rtpexten;
	//int end = min(showLevel+2,h->layer);

	//RtpNalu *nal = (RtpNalu *)h->dRtpPacket.packetInfo;
	SVCDec_avResetRtPacket(&h->dRtpPacket,OperatePar->InBuf,OperatePar->rtpsize,OperatePar->rtpcount);
	h->dRtpPacket.rtpLen = OperatePar->InPutLen;
	h->dRtpPacket.pos = 0;
	h->dRtpPacket.redundancy = 0/*(short)h->redundancy*/;
	OperatePar->OutputLen = 0;

	for (i=0; i<MAXLAYER; i++)
	{
		h->info[i].decodedPts = 0;
	}

	rtpexten = (RTPEXTENDHEADER *)&OperatePar->InBuf[RTPHEADSIZE];
	h->encIDX = rtpexten->rtp_extend_globInfo & 0xF;
	h->encRatio = (rtpexten->rtp_extend_globInfo & 0xF0) >>4;
	lastlayer = (rtpexten->rtp_extend_profile & 0x1C0) >> 6;
	if (h->layer != lastlayer)
	{
		h->layer = lastlayer;
		SVCDec_GetStartIFrame(h->gopSize,h->layer,h->startInum);
		for (i = 0;i < h->layer;i++)
		{
			h->width[i] = (int)ResolutionTab[h->encIDX][0] >> i;
			h->height[i] = (int)ResolutionTab[h->encIDX][1] >>i;
		}
	}
	//showLevel = h->decIDX - h->encIDX;
    //showLevel = showLevel>=0?showLevel:0;
    showLevel = 0;

	packetSize =SVCDec_avRtp2RawStream(&h->dRtpPacket, h->dPacket, /*start, end*/h->layer, h->info, &h->lastEnc, h->bframenum,h->gopSize,h->startInum,h->DEC_SVCGop,h->DEC_SVCGopPTS,&h->lastEncRatio);

	if(showLevel >= 0 && h->layer <= MAXLAYER && packetSize)
	{
		for (i = showLevel; i < h->layer; i++)
		{
			FILE *debugFile = (FILE *)h->debugInfo[i].fp;
			if(!h->debugInfo[i].flag)
				continue;
			if(debugFile == NULL)  
				debugFile = fopen(h->debugInfo[i].filename, "wb"); 

			if(debugFile && h->dPacket[i]->size)
			{   
				int fsize =fwrite(h->dPacket[i]->data,1,h->dPacket[i]->size,debugFile);
			}
		}
	}

	for ( i = showLevel; i <h->layer; i++)
//	for ( i = h->layer-1; i <h->layer; i++)
	{
		//char * inbuf_ptr = h->dPacket[i]->data;
		int size = h->dPacket[i]->size;
		int tmpPts=0;
		int tmpSize=0;
		int tmpW=0;
		int tmpH=0;

		if(!h->info[i].lostFlag)
		{
			h->info[i].decFrmCnt++;
			//decode264decodeSVC(h->decodeHandle[i],inbuf_ptr, size, h->DecBuf[i], &tmpSize,  i, showLevel,h->dPacket[i]->pts,&tmpPts, h->info, &tmpW, &tmpH);
//decode264decodeSVC();
			h->ffmpegdec_handle[i]->avpkt.data = h->dPacket[i]->data;
			h->ffmpegdec_handle[i]->avpkt.size = h->dPacket[i]->size;
			h->ffmpegdec_handle[i]->avpkt.pts= h->dPacket[i]->pts;
			len = avcodec_decode_video2(h->ffmpegdec_handle[i]->c, h->ffmpegdec_handle[i]->frame, &h->ffmpegdec_handle[i]->got_frame, &h->ffmpegdec_handle[i]->avpkt);

			if(h->ffmpegdec_handle[i]->frame->pict_type == AV_PICTURE_TYPE_I)
			{
				//if(h->info[i].startDecode)
				{
					h->info[i].waitIDR = 0;
				}
			}

			if(!h->info[i].waitIDR)
			{
				display = 1;
			}

			h->info[i].decodedPts = (int)h->ffmpegdec_handle[i]->frame->pkt_pts;
			//更新显示PTS
			if(display)
			{
				tmpPts= (unsigned short)h->ffmpegdec_handle[i]->frame->pkt_pts;
			}

			//输出
			if(len > 0)
			{

				if(h->ffmpegdec_handle[i]->got_frame )
				{
					//add by zj@2013
					if(1 == display)
						//add end
					{
						tmpW = h->ffmpegdec_handle[i]->frame->width;
						tmpH = h->ffmpegdec_handle[i]->frame->height;
						tmpSize = h->ffmpegdec_handle[i]->frame->width*h->ffmpegdec_handle[i]->frame->height * 3 / 2;
					
						UVwidth = h->ffmpegdec_handle[i]->frame->width >>1;
						UVheight = h->ffmpegdec_handle[i]->frame->height >>1;
						for (j = 0;j < UVheight;j++)
						{
							memcpy(&h->DecBuf[i][0][(j<<1)* tmpW],&h->ffmpegdec_handle[i]->frame->data[0][(j<<1)*h->ffmpegdec_handle[i]->frame->linesize[0]],tmpW);
							memcpy(&h->DecBuf[i][0][((j<<1)+1)*tmpW],&h->ffmpegdec_handle[i]->frame->data[0][((j<<1)+1)*h->ffmpegdec_handle[i]->frame->linesize[0]],tmpW);
							memcpy(&h->DecBuf[i][1][j *UVwidth],&h->ffmpegdec_handle[i]->frame->data[1][j  * h->ffmpegdec_handle[i]->frame->linesize[1]],UVwidth);
							memcpy(&h->DecBuf[i][2][j *UVwidth],&h->ffmpegdec_handle[i]->frame->data[2][j  * h->ffmpegdec_handle[i]->frame->linesize[2]],UVwidth);	
						}
					}
 					else
 					{
 						tmpSize = 0;
 					}
				}
				else
				{
					tmpSize = 0;
				}
			}
			else
			{
				tmpSize = 0;
			}
//end decode264decodeSVC();
			if (tmpSize!=0 && tmpPts > maxPts)
			{
				int len ;
				maxPts = tmpPts;
				maxSize = tmpSize;
				maxLayer = i;
				maxW = tmpW;
				maxH = tmpH;
			}
		}
	}

	if (maxSize != 0)
	{
		int len = /*h->width[maxLayer]*h->height[maxLayer];*/maxW * maxH;
		OutPutInfo->OutPutWidth = /*h->width[maxLayer];*/maxW;
		OutPutInfo->OutPutHeight = /*h->height[maxLayer];*/maxH;
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
        memcpy_neon(OperatePar->OutBuf[0], h->DecBuf[maxLayer][0], len);
        memcpy_neon(OperatePar->OutBuf[1], h->DecBuf[maxLayer][1], len>>2);
        memcpy_neon(OperatePar->OutBuf[2], h->DecBuf[maxLayer][2], len>>2);
#else
        memcpy(OperatePar->OutBuf[0], h->DecBuf[maxLayer][0], len);
        memcpy(OperatePar->OutBuf[1], h->DecBuf[maxLayer][1], len>>2);
        memcpy(OperatePar->OutBuf[2], h->DecBuf[maxLayer][2], len>>2);
#endif
		OperatePar->OutputLen = (len *3) >>1;
	}


	h->lastEnc = h->dRtpPacket.encIDX;//记录上次的编码分辨率索引，以区分编码器是否发生初始化
	h->lastEncRatio = h->dRtpPacket.encRatio;
exit:
	h->decFrameNum += (OperatePar->OutputLen > 0);
	//OperatePar->OutputLen = dstSize;
	return 0;
}

void GVE_SVC_Decoder_SetDebugFile(unsigned long            GVE_SVCDec_Handle,
								  GVE_SVCDec_OperatePar *OperatePar,
								  GVE_SVCDec_ConfigPar  *ConfigPar,
								  GVE_SVCDec_OutPutInfo *OutPutInfo)
{
	SVCdec_Handle *h = (SVCdec_Handle *)GVE_SVCDec_Handle;

	h->debugInfo[ConfigPar->debuglayer].flag = ConfigPar->debugflag;
	h->debugInfo[ConfigPar->debuglayer].fp = ConfigPar->debugfp;
	h->debugInfo[ConfigPar->debuglayer].filename = ConfigPar->debugfilename;
}
