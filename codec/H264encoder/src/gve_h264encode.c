#include "h264encrtp.h"


int GVE_H264_Encoder_GetSliceType(unsigned long           GVE_H264Enc_Handle,
								  GVE_H264Enc_OperatePar *OperatePar,
								  GVE_H264Enc_ConfigPar  *ConfigPar,
								  GVE_H264Enc_OutPutInfo *OutPutInfo)
{
	X264_Handle *hd = (X264_Handle *)GVE_H264Enc_Handle;

	if ( hd->frameNum <0)
	{
		return -1;
	}

	if (hd->frameNum%ConfigPar->GOPSize == 0)
	{
		return PIC_TYPE_IDR;
	}
	else
	{
		return PIC_TYPE_AUTO;
	}
}

int GVE_H264_Encoder_Create(unsigned long           *GVE_H264Enc_Handle,
							GVE_H264Enc_OperatePar *OperatePar,
							GVE_H264Enc_ConfigPar  *ConfigPar,
							GVE_H264Enc_OutPutInfo *OutPutInfo)
{

	//malloc hd
	X264_Handle *hd = NULL;
	x264_param_t param;
	hd = (X264_Handle*)malloc(sizeof(X264_Handle));
	if (hd != NULL)
	{
		memset(hd,0,sizeof(X264_Handle));
	}

	*GVE_H264Enc_Handle = (unsigned long/*int*/)hd;

	//初始化默认参数
	x264_param_default( &param );
	x264_param_default_preset( &param, "medium", NULL );


	//输入外部传入的参数
	//1 自己修改的默认参数
	hd->frameNum = 0;
 	param.i_lookahead_threads = X264_THREADS_AUTO;
 	param.i_keyint_min = X264_KEYINT_MIN_AUTO;
 	param.i_bframe_adaptive = X264_B_ADAPT_NONE;// X264_B_ADAPT_FAST modified by wuzhong;
 	param.i_bframe_pyramid = X264_B_PYRAMID_NONE;//X264_B_PYRAMID_NORMAL modified by wuzhong;
 	param.rc.i_lookahead = 4;
 	param.analyse.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
 	param.analyse.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8
 		| X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
 	param.analyse.i_direct_mv_pred = X264_DIRECT_PRED_SPATIAL;
 	param.i_scenecut_threshold = 0;
 	param.i_frame_reference =1;// 3 modified by wuzhong;
 	param.analyse.b_mixed_references = 0;//1; modified by wuzhong
 	param.analyse.b_weighted_bipred = 0;//1;modified by wuzhong
 	param.analyse.i_weighted_pred = X264_WEIGHTP_NONE;//X264_WEIGHTP_SMART;modified by wuzhong
 	param.b_vfr_input = 0;
 	param.vui.b_fullrange = 0;
 	param.b_intra_refresh = 1;
	param.b_interlaced =0;
	
 	param.i_fps_den = param.i_timebase_num = 1000;

	//2 test传入参数
	param.i_width         = ConfigPar->InPut_Width;
	param.i_height        = ConfigPar->InPut_Height;
 	param.i_keyint_max = ConfigPar->GOPSize;
 	param.i_fps_num = param.i_timebase_den = (int)((float)ConfigPar->FrameRate * 1000 + .5);
 	param.i_bframe = ConfigPar->BFrameNum;
 	param.rc.i_qp_min = ConfigPar->QPMin;
 	param.rc.i_qp_max = ConfigPar->QPMax;
	if (ConfigPar->Level)  //add wyh
	{
		param.i_level_idc = ConfigPar->Level;
	}
 
 
 	if(ConfigPar->RC_Method == X264_RC_ABR)
 	{
 		param.rc.i_bitrate = ConfigPar->BitsRate;
 		param.rc.i_qp_min = ConfigPar->QPMin;
 		param.rc.i_qp_max = ConfigPar->QPMax;
 		param.rc.i_vbv_max_bitrate = ConfigPar->BitsRate;
 		param.rc.i_vbv_buffer_size = ConfigPar->BitsRate;
 		param.rc.i_rc_method = X264_RC_ABR;
 	}
 	else if(ConfigPar->RC_Method == X264_RC_CRF)
 	{
 		param.rc.f_rf_constant= ConfigPar->RF_Constant;	
 		param.rc.i_rc_method = X264_RC_CRF;
 	}
 	else if(ConfigPar->RC_Method == X264_RC_CQP)
 	{
 		param.rc.i_qp_constant= ConfigPar->RF_Constant;
 		param.rc.i_rc_method = X264_RC_CQP;
 	}
 
 	if (ConfigPar->Mult_Slice)
 	{
 		param.i_slice_max_size = ConfigPar->Mtu_Size;
 	}
 
 	if(ConfigPar->Me_Range)
 	{
 		param.analyse.i_me_range = ConfigPar->Me_Range;
 	}
 
 	param.b_sliced_threads = ConfigPar->Sliced_Threads;
 	param.i_threads = ConfigPar->Gen_Threads;
 	param.analyse.i_subpel_refine = ConfigPar->Subpel_Refine;
 	param.analyse.i_me_method = ConfigPar->Me_Method;
 	param.analyse.b_transform_8x8 = 0;
 	param.b_cabac = ConfigPar->Enable_Cabac;
 	param.analyse.b_psnr = ConfigPar->Psnr;

	//x264_param_apply_profile(&param, "baseline");


	x264_picture_alloc( &hd->pic,X264_CSP_I420, param.i_width, param.i_height );

	hd->h = x264_encoder_open( &param );
	hd->frameoffset = param.i_width * param.i_height;


	//初始化打包数据
	if(H264_avInitRtPacket(&hd->RtpPacket,&param) < 0)
		return NULL;

	//for test
//    	if((hd->fph264 = fopen("ench264.h264","wb")) == NULL)
//    	{ 
//    		return 1;
//    	}
	//end test

		return 0;
}


int GVE_H264_Encoder_Encoder(unsigned long            GVE_H264Enc_Handle,
							 GVE_H264Enc_OperatePar *OperatePar,
							 GVE_H264Enc_ConfigPar  *ConfigPar,
							 GVE_H264Enc_OutPutInfo *OutPutInfo)
{
	int packetSize;
	x264_nal_t *nal;
	int i_nal;
	X264_Handle *hd = (X264_Handle *)GVE_H264Enc_Handle;
	x264_param_t param;

	memset(&hd->pic_out,0,sizeof(x264_picture_t));

	memcpy(hd->pic.img.plane[0],OperatePar->InBuf/*hd->YUVBuffer[0]*/,hd->frameoffset);
	memcpy(hd->pic.img.plane[1],&OperatePar->InBuf[hd->frameoffset]/*hd->YUVBuffer[1]*/,hd->frameoffset/4);
	memcpy(hd->pic.img.plane[2],&OperatePar->InBuf[hd->frameoffset  + (hd->frameoffset>>2)]/*hd->YUVBuffer[2]*/,hd->frameoffset/4);

	hd->pic.i_type = OutPutInfo->abMark;
	hd->pic.i_qpplus1 = 0;
	hd->pic.i_pic_struct = PIC_STRUCT_AUTO;

	++hd->pic.i_pts ;//PTS递增
	//当PTS即将溢出时，PTS重新从34开始循环
	if(hd->pic.i_pts==65506)
	{ 	
		x264_encoder_parameters(hd->h, &param);
		hd->pic.i_pts = param.i_keyint_max + 1;
	}

	//H264编码
	OperatePar->OutputLen = x264_encoder_encode( hd->h, &nal, &i_nal, &hd->pic, &hd->pic_out );

	//为打包初始化
	H264_avResetRtPacket(&hd->RtpPacket,OperatePar->OutBuf,0,0); ///< 重置RTP packet
	hd->RtpPacket.sliceType = hd->pic_out.i_type;
	//hd->RtpPacket.payLoadType = PAYLOADTYPE_H264;

	//hd->RtpPacket.sliceType[0] = hd->pic_out.i_type;
	//初始化完毕

	if(OperatePar->OutputLen > 0)
	{
		memcpy((void *)OperatePar->OutBuf,(void *)nal->p_payload,OperatePar->OutputLen);
		//fwrite(OperatePar->OutBuf,1,OperatePar->OutputLen,hd->fph264);
		//打包数据
		packetSize = H264enc_avRaw2RtpStream(&hd->RtpPacket,OperatePar->OutBuf,OperatePar->OutputLen,hd->pic_out.i_pts);
		OperatePar->rtpcount = hd->RtpPacket.count;
		memset(OperatePar->rtpsize,0,sizeof(short)*MAXRTPNUM);
		memcpy(OperatePar->rtpsize,&hd->RtpPacket.size,hd->RtpPacket.count*sizeof(short));

		if(packetSize < 0)
		{
			packetSize = 0;
		}
		OperatePar->OutputLen = packetSize;
	}

	if(hd->pic_out.i_type == X264_TYPE_IDR || hd->pic_out.i_type ==X264_TYPE_I)
		OutPutInfo->abMark =1;
	else
		OutPutInfo->abMark =0;

	//OutPutInfo->abMark = hd->pic_out.i_type;

	hd->frameNum++;
	if (hd->frameNum == 65506)//溢出处理
	{
		hd->frameNum = 34;
	}


	return 0;

}

void GVE_H264_Encoder_Destroy(unsigned long   GVE_H264Enc_Handle)
{
	X264_Handle *hd = (X264_Handle *)GVE_H264Enc_Handle;

	H264enc_avFreePacket(&hd->RtpPacket);
	x264_encoder_close(hd->h );
	x264_picture_clean( &hd->pic );

	//for test
	//fclose(hd->fph264);
	//end test

	if (hd != NULL)
	{
		free(hd);
	}
}

void GVE_H264_Encoder_ResetBitrate(unsigned long            GVE_H264Enc_Handle,
								   GVE_H264Enc_OperatePar *OperatePar,
								   GVE_H264Enc_ConfigPar  *ConfigPar,
								   GVE_H264Enc_OutPutInfo *OutPutInfo)
{
	x264_param_t param;
	X264_Handle *hd = (X264_Handle *)GVE_H264Enc_Handle;

	x264_encoder_parameters(hd->h, &param);
	param.rc.i_bitrate = ConfigPar->BitsRate;
	param.rc.i_vbv_buffer_size = ConfigPar->BitsRate;
	param.rc.i_vbv_max_bitrate = ConfigPar->BitsRate;
	x264_encoder_reconfig(hd->h, &param);
}