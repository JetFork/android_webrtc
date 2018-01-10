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
#include "../inc/svc_enc_api.h"
#include "gve_svcencode.h"
#include "svcencrtp.h"

#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#else
#define GetConsoleTitle(t,n)
#define SetConsoleTitle(t)
#endif

#define FDEC_STRIDE     ((1 << FDEC_FACTOR) - 1)

const unsigned short SvcEncnormResolutionTab[16][3] = 
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
/**
* @brief    对宽度和高度做对齐处理，变为16的倍数
* @param[in] [out]    int *width，图像宽度
* @param[in] [out]    int *height，图像高度
*/
static int SVCEnc_avGetWidthAndHeight(int *width, int *height)
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


//add wh
static int SVCEnc_GetStartIFrame(int gopsize,int layer,int startI[])
{
	int dis = gopsize/layer;

	switch(layer)
	{
// 	case 4:
// 		startI[0] = 0;
// 		startI[1] = dis *2;
// 		startI[2] = dis;
// 		startI[3] = dis *3;
// 		break;
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
//end wyh

 static int SVCEnc_avGetSliceType(int gopSize,int layer,int frameNum, int  idx,int startI[])
 {
 	int delta;
 
 	if (frameNum < 0)
 	{
 		return -1;
 	}

	delta = frameNum - startI[idx];
 
 	//delta = frameNum-8*delta;
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

/**
* @brief    下采样
* @param[in]     char *src[3], 采样前原数据
* @param[in]     char *dst[3], 采样后目的数据
* @param[in]     int iw, 采样前原宽度
* @param[in]     int ih, 采样前原高度
* @param[in]     int ow, 采样后目标宽度
* @param[in]     int oh, 采样后目标高度
* @param[in]     int step, 采样阶数。一般为2，即缩小1倍
* @returns   int，成功返回0
*/
static int SVCEnc_downResample_C(char *src[3], char *dst[3],int iw, int ih, int ow, int oh, int step)
{
	int i, j;
	int ii,jj;
	int ow1 = iw >> step;
	int oh1 = ih >> step;
	int col = 1 << step;
	unsigned char *sY,*sU,*sV,*dY,*dU,*dV,*dU1 ,*dV1;
	int iwc = iw >> 1;
	int owc = ow >> 1;
	int flag = src == dst;
	if(src[1] == NULL || src[2] == NULL)
	{
		sY = &src[0][0];
		sU = &src[0][iw*ih];
		sV = &src[0][iw*ih + (iw*ih >> 2)];
		dY = &dst[0][0];
		dU = &dst[0][ow*oh];
		dV = &dst[0][ow*oh + (ow*oh >> 2)];
		dU1 = &src[0][iw*ih];
		dV1 = &src[0][iw*ih + (iw*ih >> 2)];
	}
	else
	{
		sY = src[0];
		sU = src[1];
		sV = src[2];
		dY = dst[0];
		dU = dst[1];
		dV = dst[2];
		dU1 = src[1];
		dV1 = src[2];
	}
	//不规整分辨率需要扩边；
	dU1 = flag ? dU1 : dU;
	dV1 = flag ? dV1 : dV;
	for (i = 0; i < oh1; i++)
	{
		int I , J;
		for (j = 0; j < ow1; j++)
		{	
			unsigned short value = 0;	
			for (ii = 0; ii < col; ii++)
			{
				for (jj = 0; jj < col; jj++)
				{
					I = ((i << step) + ii) * iw;
					J = (j << step) + jj;
					value += sY[I + J];
				}
			}
			value = (value + (1 << ((step << 1) - 1))) >> (step << 1);
			value = value > 255 ? 255 : value;

			dY[i * ow + j] = (unsigned char)value;
			if(!((i | j) & 1))
			{
				unsigned short value0 = 0;
				unsigned short value1 = 0;
				for (ii = 0; ii < col; ii++)
				{
					for (jj = 0; jj < col; jj++)
					{
						I = (((i << step) >> 1) + ii) * iwc;
						J = ((j << step) >> 1) + jj;
						value0 += sU[I + J];
						value1 += sV[I + J];
					}
				}
				value0 = (value0 + (1 << ((step << 1) - 1))) >> (step << 1);
				value0 = value0 > 255 ? 255 : value0;
				value1 = (value1 + (1 << ((step << 1) - 1))) >> (step << 1);
				value1 = value1 > 255 ? 255 : value1;

				dU1[(i >> 1) * owc + (j >> 1)] = (unsigned char)value0;
				dV1[(i >> 1) * owc + (j >> 1)] = (unsigned char)value1;
			}
		}
		if(ow > ow1)
		{
			for (; j < ow; j++)
			{
				dY[i * ow + j] = dY[i * ow + ow1 - 1];
				if(!((i | j) & 1))
				{
					I = (((i << step) >> 1)) * iwc;
					J = ((j << step) >> 1);

					dU1[(i >> 1) * owc + (j >> 1)] = dU1[(i >> 1) * owc + (ow1 >> 1) - 1];
					dV1[(i >> 1) * owc + (j >> 1)] = dV1[(i >> 1) * owc + (ow1 >> 1) - 1];
				}
			}
		}
	}
	if(oh > oh1)
	{
		for (; i < oh; i++)
		{
			memcpy(&dY[i * ow],&dY[(oh1 - 1) * ow],ow);
			if(!((i) & 1))
			{
				memcpy(&dU1[(i >> 1) * owc],&dU1[((oh1 >> 1) - 1) * owc],owc);
				memcpy(&dV1[(i >> 1) * owc],&dV1[((oh1 >> 1) - 1) * owc],owc);
			}
		}
	}
	if(flag)
	{
		memcpy((void *)dU,(void *)dU1,(ow*oh >> 2));
		memcpy((void *)dV,(void *)dV1,(ow*oh >> 2));
	}
	return 0;
}

#ifdef _WIN32
__inline void SVCEnc_BoxHalfDownResampleMMX(unsigned char *src, unsigned char *dst, int iw, int ih)
{
	long long iValue = 0x0001000100010001;
	unsigned char *s = src;
	unsigned char *d = dst;

	__asm
		{
			mov edx, ih
			shr edx, 1 //for (i = 0; i < oh1; i++)

LOOP_H:
			mov ecx, iw
			shr ecx, 3 //for (j = 0; j < ow1/4; j++)

LOOP_W:
			mov eax, s
			movq mm0,qword ptr [eax]  //mm0=a11,a12,a13,a14,a15,a16,a17,a18

			mov ebx, s
			add ebx, iw
			movq mm1,qword ptr [ebx]  //mm1=a21,a22,a23,a24,a25,a26,a27,a28

			movq mm2, mm0  //mm2=a11,a12,a13,a14,a15,a16,a17,a18
			movq mm3, mm1  //mm3=a21,a22,a23,a24,a25,a26,a27,a28

			pxor mm5, mm5  //mm5=0,0,0,0,0,0,0,0


			PUNPCKHBW  mm0, mm5  //mm0=0,a11,0,a12,0,a13,0,a14
			PUNPCKHBW  mm1, mm5  //mm1=0,a21,0,a22,0,a23,0,a24

			PUNPCKLBW  mm2, mm5  //mm0=0,a15,0,a16,0,a17,0,a18
			PUNPCKLBW  mm3, mm5  //mm1=0,a25,0,a26,0,a27,0,a28

			PADDW mm0, mm1  //mm0=a11+a21,a12+a22,a13+a23,a14+a24
			PADDW mm2, mm3  //mm2=a15+a25,a16+a26,a17+a27,a18+a28

			PMADDWD mm0, iValue  //mm0 = a11+a21+a12+a22, a13+a23+a14+a24
			pmaddwd mm2, iValue  //mm2 = a15+a25+a16+a26,a17+a27+a18+a28

			PSRLD mm0, 2  //mm0 = (a11+a21+a12+a22)/4, (a13+a23+a14+a24)/4
			PSRLD mm2, 2  //mm2 = (a15+a25+a16+a26)/4, (a17+a27+a18+a28)/4

			PACKSSDW mm2, mm0  //mm2 = (a11+a21+a12+a22)/4, (a13+a23+a14+a24)/4, (a15+a25+a16+a26)/4, (a17+a27+a18+a28)/4
			PACKUSWB mm2, mm5  //mm2 = 0,0,0,0,(a11+a21+a12+a22)/4, (a13+a23+a14+a24)/4, (a15+a25+a16+a26)/4, (a17+a27+a18+a28)/4

			mov ebx,d
			movd [ebx],mm2
			add s, 8
			add d, 4

			dec ecx
			jnz LOOP_W

			mov eax, iw
			add s, eax
			dec edx
			jnz LOOP_H

			emms
		}
}

static int SVCEnc_halfDownResampleMMX(char *src[3], char *dst[3],int iw, int ih)
{
	SVCEnc_BoxHalfDownResampleMMX(src[0], dst[0], iw, ih);
	SVCEnc_BoxHalfDownResampleMMX(src[1], dst[1], iw>>1, ih>>1);
	SVCEnc_BoxHalfDownResampleMMX(src[2], dst[2], iw>>1, ih>>1);
	return 0;
}
#endif

#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
extern int downResample_neon(char *src[3], char *dst[3],int iw, int ih, int ow, int oh, int step);
extern int downResample_9_16neon(char *src[3], char *dst[3],int iw, int ih, int ow, int oh, int step);
#endif

int GVE_SVC_Encoder_Create(unsigned long           *GVE_SVCEnc_Handle,
						   GVE_SVCEnc_OperatePar *OperatePar,
						   GVE_SVCEnc_ConfigPar  *ConfigPar,
						   GVE_SVCEnc_OutPutInfo *OutPutInfo) 
{
	//videoInParams Params;
	x264_param_t param;
	GVESVC_Handle *h = NULL;
	int i = 0;

	//  判断参数符合要求，否则返回NULL
	if(ConfigPar->layer <= 0 || (ConfigPar->layer > MAXLAYER))
		return NULL;
	//  为编码器/解码器句柄申请空间
	h = (GVESVC_Handle *)malloc( sizeof(GVESVC_Handle) );
	memset(h, 0, sizeof(GVESVC_Handle));

	*GVE_SVCEnc_Handle = (unsigned long/*int*/)h;

	if(h)
	{	//  根据传进参数对编码器/解码器进行初始化

		if(ConfigPar->layer > MAXLAYER )
			return -1;
		h->layer = ConfigPar->layer;
		h->gopSize = ConfigPar->gop;
		h->bframenum = ConfigPar->bframenum;
		h->frameNum = 0;
		// 		h->displayLayer =0;
		// 		h->displayPts =0;

		//h->shareIDX = 0; //Params->shareIDX;
		for (i = 0; i < 16; i++)
		{
			int width = (int)SvcEncnormResolutionTab[i][0];
			int height = (int)SvcEncnormResolutionTab[i][1];
			if(ConfigPar->Inwidth[0] == width && ConfigPar->InHeight[0] == height)
			{
				h->encIDX =  (int)SvcEncnormResolutionTab[i][2];
				break;
			}
		}
		//add wyh for 9:16 or 16:9
		h->allocIDX = (i & 0x8) >>3 ;
		//end wyh

		//add wyh 添加初始化散列帧的信息
		for (i = 0;i < h->layer -1;i++)
		{
			h->scalefactor[i] = log(ConfigPar->Inwidth[i]/ConfigPar->Inwidth[i+1])/log(2);
		}
		SVCEnc_GetStartIFrame(h->gopSize,h->layer,h->startInum);
		//end wyh


//合并wrapper，原先wrapper中的内容
		for (i = 0; i < h->layer; i++)
		{
			//int width = ConfigPar->Inwidth[i];//Params->widthAlloc >> i;
			//int height = ConfigPar->InHeight[i];//Params->heightAlloc >> i;
			//SVCEnc_avGetWidthAndHeight(&width, &height);

			//encode264Init();
			h->x264_handle[i] = (x264_info*)malloc(sizeof(x264_info));
			x264_param_default( &param );
			x264_param_default_preset( &param, ConfigPar->preset,/*"medium"*//*"ultrafast"*/ ConfigPar->tune/*NULL*/);

			param.i_lookahead_threads = X264_THREADS_AUTO;
			param.i_width         = ConfigPar->Inwidth[i];//width;
			param.i_height        = ConfigPar->InHeight[i];//height;
			param.i_keyint_max = ConfigPar->gop;
			param.i_fps_num = param.i_timebase_den = (int)((float)ConfigPar->framerate * 1000 + .5);
			param.i_fps_den = param.i_timebase_num = 1000;
			param.b_intra_refresh = 1;
			param.i_bframe = ConfigPar->bframenum;
			param.rc.i_qp_min = ConfigPar->qp[i][0];
			param.rc.i_qp_max = ConfigPar->qp[i][1];;
			param.b_vfr_input = 0;
			param.vui.b_fullrange = 0;

			if(ConfigPar->rc_method == X264_RC_ABR)
			{
				param.rc.i_bitrate = ConfigPar->bitrateLayer[i];
				//param.rc.i_qp_min = svcparams->qpmin;
				//param.rc.i_qp_max = svcparams->qpmax;
				param.rc.i_vbv_max_bitrate = ConfigPar->bitrateLayer[i];
				param.rc.i_vbv_buffer_size = ConfigPar->bitrateLayer[i];
				param.rc.i_rc_method = X264_RC_ABR;
			}
			else if(ConfigPar->rc_method == X264_RC_CRF)
			{
				param.rc.f_rf_constant= ConfigPar->rf_constant;	
				param.rc.i_rc_method = X264_RC_CRF;
			}
			else if(ConfigPar->rc_method == X264_RC_CQP)
			{
				param.rc.i_qp_constant= ConfigPar->rf_constant;
				param.rc.i_rc_method = X264_RC_CQP;
			}

			if (ConfigPar->mult_slice)
			{
				param.i_slice_max_size = ConfigPar->mtu_size;//1260;//0; modified by wuzhong
			}

			if(ConfigPar->me_range)
			{
				param.analyse.i_me_range = ConfigPar->me_range;
			}

			param.b_sliced_threads = ConfigPar->sliced_threads;
			param.i_threads =ConfigPar->gen_threads;
			param.analyse.i_subpel_refine = ConfigPar->subpel_refine;
			param.i_frame_reference =ConfigPar->frame_reference;//1;// 3 modified by wuzhong;
			param.i_keyint_min = X264_KEYINT_MIN_AUTO;
			param.i_bframe_adaptive = X264_B_ADAPT_NONE;// X264_B_ADAPT_FAST modified by wuzhong;
			param.i_bframe_pyramid = X264_B_PYRAMID_NONE;//X264_B_PYRAMID_NORMAL modified by wuzhong;
			param.rc.i_lookahead = ConfigPar->lookahead;//4;
			param.analyse.intra = ConfigPar->intra;//X264_ANALYSE_I4x4 /*| X264_ANALYSE_I8x8*/;
			param.analyse.inter = ConfigPar->inter;//X264_ANALYSE_I4x4/* | X264_ANALYSE_I8x8*/ | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
			param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_SPATIAL;
			param.analyse.i_me_method = ConfigPar->me_method;
			param.analyse.b_mixed_references = 0;//1; modified by wuzhong
			param.analyse.b_weighted_bipred = 0;//1;modified by wuzhong
			param.analyse.i_weighted_pred = X264_WEIGHTP_NONE;//X264_WEIGHTP_SMART;modified by wuzhong
			param.analyse.b_transform_8x8 = ConfigPar->enable_8x8dct;
			param.b_cabac = ConfigPar->enable_cabac;
			param.i_scenecut_threshold = 0;
			param.analyse.b_psnr = ConfigPar->psnr;

			x264_picture_alloc( &h->x264_handle[i]->pic,X264_CSP_I420, param.i_width, param.i_height );

			h->x264_handle[i]->x264_info_t = x264_encoder_open( &param );
			h->x264_handle[i]->frameoffset = param.i_width * param.i_height;
			//end encode264Init();

			h->ePacket[i] = (SVCPacket *)malloc(sizeof(SVCPacket)); 
			if(h->ePacket[i] == NULL)
				return NULL;
		}

//结束合并

		if(SVCEnc_avInitRtPacket(&h->eRtpPacket,ConfigPar->mtu_size,ConfigPar->layer,ConfigPar->bframenum,0,0,ConfigPar->mult_slice) < 0)
			return NULL;
		else
		{
			//h->eRtpPacket.payLoadType = h->payLoadType;
			//h->eRtpPacket.codecType = ((h->codec >> 1) & 1);
			h->eRtpPacket.encIDX = h->encIDX;
			h->eRtpPacket.allocIDX = h->allocIDX;
			h->eRtpPacket.shareIDX = 0;//h->shareIDX;
			h->eRtpPacket.gop = ConfigPar->gop;
		}
		if(SVCEnc_avInitPacket(h->ePacket,h->layer) < 0)
			return NULL;

		//add wyh
		for (i = 0;i<h->layer;i++)
		{
			//SVC_X264_Handle *hx =(SVC_X264_Handle *)h->encodeHandle[i];
			h->x264_handle[i]->pic.i_pts = h->startInum[i];
		}
		//end wyh

#if SVC_ENCODER_WIN32
		h->pfDownResamplefun = SVCEnc_halfDownResampleMMX;

#elif (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
		if ((9*ConfigPar->Inwidth[0]) == (16*ConfigPar->InHeight[0]))
		{
			h->pfDownResamplefun = downResample_neon;
		}
		else
		{
			h->pfDownResamplefun = downResample_9_16neon;
		}
#else
		h->pfDownResamplefun =SVCEnc_downResample_C;
#endif

		//addd wyh
		//h->fptst_wyh = fopen("h264_640X480.264","wb");
		//end wyh
	}
	return 0;
}

int GVE_SVC_Encoder_Encoder(unsigned long            GVE_SVCEnc_Handle,
							GVE_SVCEnc_OperatePar *OperatePar,
							GVE_SVCEnc_ConfigPar  *ConfigPar,
							GVE_SVCEnc_OutPutInfo *OutPutInfo)
{
	GVESVC_Handle *h = (GVESVC_Handle *)GVE_SVCEnc_Handle;
	int pic_type[3]={0,0,0};
	int layer = h->layer;	
	int oSize = 0;
	int packetSize = 0;
	//int start = normResolutionTab[h->encIDX][2];//0;//???
	int keyFrame = 0;
	int i;	

// wrapper
	x264_nal_t *nal;
	int i_nal;
	x264_param_t param;
//end 

	h->Inbuf[0] = OperatePar->InBuf;
	h->Inbuf[1] = OperatePar->InBuf + ConfigPar->Inwidth[0] * ConfigPar->InHeight[0];
	h->Inbuf[2] = h->Inbuf[1] + ConfigPar->Inwidth[0] * ConfigPar->InHeight[0]/4;

	//开始编码序号：
	//（共四层）0层：0；1层：16；2层：8；3层：24
	//（共三层）1层：0；2层：11；3层：22
	//（共两层）2层：0；3层：16
	for ( i = 0; i < layer; i++)
	{
		h->ePacket[i]->pts = 0;
		if ((layer == 3 && ((i == 0 && h->frameNum >= h->startInum[0])||(i == 1 && h->frameNum >= h->startInum[1]||(i ==2 && h->frameNum >= h->startInum[2]))))||
			(layer == 2 &&  ((i == 0 && h->frameNum >= h->startInum[0] ||(i == 1 && h->frameNum >= h->startInum[1]))))||
			(layer == 1))
		{ 
			keyFrame= SVCEnc_avGetSliceType(h->gopSize, layer, h->frameNum, i,h->startInum);
			//bsize = encode264encode(h->encodeHandle[i],h->Inbuf,OperatePar->InPutLen,h->ePacket[i]->data, &h->ePacket[i]->size,&keyFrame);//&key);
// encode264encode();
			memset(&h->x264_handle[i]->pic_out,0,sizeof(x264_picture_t));

			memcpy(h->x264_handle[i]->pic.img.plane[0],h->Inbuf[0],h->x264_handle[i]->frameoffset);
			memcpy(h->x264_handle[i]->pic.img.plane[1],h->Inbuf[1],h->x264_handle[i]->frameoffset/4);
			memcpy(h->x264_handle[i]->pic.img.plane[2],h->Inbuf[2],h->x264_handle[i]->frameoffset/4);
			h->x264_handle[i]->pic.i_type = keyFrame;
			h->x264_handle[i]->pic.i_qpplus1 = 0;
			h->x264_handle[i]->pic.i_pic_struct = PIC_STRUCT_AUTO;

			++h->x264_handle[i]->pic.i_pts ;//PTS递增
			//当PTS即将溢出时，PTS重新从34开始循环
			if(h->x264_handle[i]->pic.i_pts==65535/h->gopSize*h->gopSize+1)
			{ 	
				x264_encoder_parameters(h->x264_handle[i]->x264_info_t, &param);
				h->x264_handle[i]->pic.i_pts = param.i_keyint_max + 1;
			}

			h->ePacket[i]->size = x264_encoder_encode( h->x264_handle[i]->x264_info_t, &nal, &i_nal, &h->x264_handle[i]->pic, &h->x264_handle[i]->pic_out );

			if(h->ePacket[i]->size > 0)
			{
				memcpy((void *)h->ePacket[i]->data,(void *)nal->p_payload,h->ePacket[i]->size);
				h->ePacket[i]->pts = h->x264_handle[i]->pic_out.i_pts;
			}
			keyFrame = h->x264_handle[i]->pic_out.i_type;
//end  encode264encode();
			pic_type[i]=keyFrame;
			oSize += h->ePacket[i]->size;

 			//add wyh
//  			if (i == 2)
//  			{
//  				fwrite(h->ePacket[i]->data,1,h->ePacket[i]->size,h->fptst_wyh);
//  			}
 			//end wyh
		}

		if((i + 1) < layer)
		{
			h->pfDownResamplefun(h->Inbuf, h->Inbuf,ConfigPar->Inwidth[i], ConfigPar->InHeight[i], ConfigPar->Inwidth[i]>>1, ConfigPar->InHeight[i]>>1, h->scalefactor[i]);
		}
	}

//add wyh  影响性能
	for (i = 0; i < layer; i++)
	{
		FILE *debugFile = (FILE *)h->debugInfo[i].fp;
		if(!h->debugInfo[i].flag)
			continue;
		if(debugFile == NULL)  
			debugFile = fopen(h->debugInfo[i].filename, "wb"); 
		if(debugFile && h->ePacket[i]->size)
		{   
			int fsize =
				fwrite(h->ePacket[i]->data,1,h->ePacket[i]->size,debugFile);
			fflush(debugFile);

		}
	}
//end wyh

	SVCEnc_avResetRtPacket(&h->eRtpPacket,OperatePar->OutBuf,0,0); ///< 重置RTP packet
	for(i=0;i<layer;i++)
	{
		h->eRtpPacket.sliceType[i] = pic_type[i];
	}

	if(oSize > 0)
	{
		packetSize = SVCEnc_avRaw2RtpStream(&h->eRtpPacket,h->ePacket,h->layer/*start,layer - 1*/);	

		if(packetSize < 0)
		{
			packetSize = 0;
		}
		oSize = packetSize;
		OperatePar->rtpcount = h->eRtpPacket.count;
		memset(OperatePar->rtpsize,0,sizeof(short)*MAXRTPNUM);
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
        memcpy_neon(OperatePar->rtpsize,&h->eRtpPacket.size,h->eRtpPacket.count*sizeof(short));
#else
		memcpy(OperatePar->rtpsize,&h->eRtpPacket.size,h->eRtpPacket.count*sizeof(short));
#endif
	}
	h->frameNum++;
	if (h->frameNum == /*65506*/65535/h->gopSize*h->gopSize+1)//溢出处理
	{
		h->frameNum = h->gopSize +1;/*34*/; //wyh
	}
	OperatePar->OutputLen = oSize;
	return 0;
}


void GVE_SVC_Encoder_Destroy(unsigned long           GVE_SVCEnc_Handle)
{
	int i;
	GVESVC_Handle *h = (GVESVC_Handle *)GVE_SVCEnc_Handle;
	if(h->layer <= 0 || (h->layer > MAXLAYER))
		return;
	if(h)
	{
		SVCEnc_avFreePacket(&h->eRtpPacket);
		for ( i = 0; i < h->layer; i++)
		{
			if(h->x264_handle[i])
			{
				x264_encoder_close(h->x264_handle[i]->x264_info_t );
				x264_picture_clean( &h->x264_handle[i]->pic );
				free(h->x264_handle[i]);
				h->x264_handle[i] = NULL;
			}

			if(h->ePacket[i])
			{
				if (h->ePacket[i]->data)
				{
					free(h->ePacket[i]->data);
					h->ePacket[i]->data = NULL;
				}
				free(h->ePacket[i]);
				h->ePacket[i] = NULL;
			}
		}
		//add wyh
		//fclose(h->fptst_wyh);
		//end wyh
		free(h);

	}
}

void GVE_SVC_Encoder_ResetBitrate(unsigned long            GVE_SVCEnc_Handle,
								  GVE_SVCEnc_OperatePar *OperatePar,
								  GVE_SVCEnc_ConfigPar  *ConfigPar,
								  GVE_SVCEnc_OutPutInfo *OutPutInfo)
{
	int i;
	x264_param_t param;
	GVESVC_Handle *h = (GVESVC_Handle *)GVE_SVCEnc_Handle;

	for (i = 0; i < ConfigPar->layer; i++)
	{
		//encode264resetBitrate(h->encodeHandle[i], ConfigPar->bitrateLayer[i]);

		x264_encoder_parameters(h->x264_handle[i]->x264_info_t, &param);
		param.rc.i_bitrate = ConfigPar->bitrateLayer[i];
		param.rc.i_vbv_buffer_size = ConfigPar->bitrateLayer[i];
		param.rc.i_vbv_max_bitrate = ConfigPar->bitrateLayer[i];
		x264_encoder_reconfig(h->x264_handle[i]->x264_info_t, &param);
	}

}

void GVE_SVC_Encoder_SetDebugFile(unsigned long            GVE_SVCEnc_Handle,
								  GVE_SVCEnc_OperatePar *OperatePar,
								  GVE_SVCEnc_ConfigPar  *ConfigPar,
								  GVE_SVCEnc_OutPutInfo *OutPutInfo)
{
	GVESVC_Handle *h = (GVESVC_Handle *)GVE_SVCEnc_Handle;

	h->debugInfo[ConfigPar->debuglayer].flag = ConfigPar->debugflag;
	h->debugInfo[ConfigPar->debuglayer].fp = ConfigPar->debugfp;
	h->debugInfo[ConfigPar->debuglayer].filename = ConfigPar->debugfilename;
}
