/*
*  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "modules/video_coding/codecs/i420/main/interface/i420.h"


#include "math.h"


#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "../../system_wrappers/interface/trace.h"
#include <android/log.h>


#define X264_ANALYSE_I4x4       0x0001  /* Analyse i4x4 */
#define X264_ANALYSE_I8x8       0x0002  /* Analyse i8x8 (requires 8x8 transform) */
#define X264_ANALYSE_PSUB16x16  0x0010  /* Analyse p16x8, p8x16 and p8x8 */
#define X264_ANALYSE_PSUB8x8    0x0020  /* Analyse p8x4, p4x8, p4x4 */
#define X264_ANALYSE_BSUB16x16  0x0100  /* Analyse b16x8, b8x16 and b8x8 */

#define GOPSIZE_H264   33
#define GOPSIZE_SVC   66
#define GOPSIZE_SVC_P2P   100

#ifdef _WIN32
#include < windows.h >
#ifdef __cplusplus
extern "C" {
#ifdef __cplusplus
}
#endif
#endif//_WIN32
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern void	memcpy_neon(void *, const void *, size_t);
#ifdef __cplusplus
}
#endif




namespace webrtc
{
FILE* fpOutfile_dec = NULL;
FILE* fpRtpCountfile_dec = NULL;
FILE* fpRtpSizefile_dec = NULL;
I420Encoder::I420Encoder():
_inited(false),
_encodedImage(),
_encodedCompleteCallback(NULL)
{
	int i;
	_pRtPacket = NULL;
	_maxImgBufCnt = 0;
	_inImgBufIdx = 0;
	_outImgBufIdx = 0;
	_lastOutImgBufIdx = -1;
	GVE_CodecEnc_Handle = 0;
	memset(&OperatePar,0,sizeof(GVE_H264Enc_OperatePar));
	memset(&ConfigPar,0,sizeof(GVE_H264Enc_ConfigPar));
	memset(&OutPutInfo,0,sizeof(GVE_H264Enc_OutPutInfo));
	memset(&SVCOperatePar,0,sizeof(GVE_SVCEnc_OperatePar));
	memset(&SVCConfigPar,0,sizeof(GVE_SVCEnc_ConfigPar));
	memset(&SVCOutPutInfo,0,sizeof(GVE_SVCEnc_OutPutInfo));
	for (i = 0; i < MAX_IMG_BUF; i++)
	{
		_encImg[i]._buffer = NULL;
		_encImg[i]._timeStamp = 0;
		_encImg[i].capture_time_ms_ = 0;
		_encImg[i]._encodedHeight = 0;
		_encImg[i]._encodedWidth = 0;
	}
	_inEncBuf = NULL;
	
	_isRunning = false;
}

I420Encoder::~I420Encoder() {
	_inited = false;	
	_isRunning = false;
	if (_encodedImage._buffer != NULL) {
		delete [] _encodedImage._buffer;
		_encodedImage._buffer = NULL;
	}

	delete _pSvc_event;
	delete _pSvc_critsect;
	delete _pSvc_thread;
	_pSvc_event = NULL;
	_pSvc_critsect = NULL;
	_pSvc_thread = NULL;
}

int I420Encoder::Release() {
	// Should allocate an encoded frame and then release it here, for that we
	// actually need an init flag.
    int i;
	SVCStopThread();
	if (_encodedImage._buffer != NULL) {
		delete [] _encodedImage._buffer;
		_encodedImage._buffer = NULL;
	}
	if (_pRtPacket->payLoadType == PAYLOADTYPE_H264)
	{
		if (GVE_CodecEnc_Handle)
		{
			GVE_H264_Encoder_Destroy(GVE_CodecEnc_Handle);
		}	
	}
	else if(_pRtPacket->payLoadType == PAYLOADTYPE_SVC_19)
	{
		if(GVE_CodecEnc_Handle)
		GVE_SVC_Encoder_Destroy(GVE_CodecEnc_Handle);
	}
	for (i = 0; i < MAX_IMG_BUF; i++)
	{
		if(_encImg[i]._buffer)
		{
			delete [] _encImg[i]._buffer;
			_encImg[i]._buffer = NULL;
		}
	}

	if( _pRtPacket)
	{
		delete _pRtPacket;
		_pRtPacket =NULL;
	}
	if(_inEncBuf)
	{
		delete [] _inEncBuf;
		_inEncBuf = NULL;
	}
	_inited = false;
	return WEBRTC_VIDEO_CODEC_OK;
}

WebRtc_Word32 I420Encoder::InitSVCEnc(const webrtc::VideoCodec* inst)
{
	int i = 0;
	//add wyh
	_pRtPacket = new WebRtc_RTPacket;
	_pRtPacket->payLoadType = inst->plType;
	//end wyh

	__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Encoder::InitSVCEnc,payLoadThpe =  %d",inst->plType);


	if(_pRtPacket->payLoadType == PAYLOADTYPE_H264)
	{
		//将编码参数传给H264编码器
		ConfigPar.InPut_Width = inst->width_used;
		ConfigPar.InPut_Height = inst->height_used;
		ConfigPar.FrameRate = 12;
		ConfigPar.GOPSize = GOPSIZE_H264;
		ConfigPar.BFrameNum = 0;
		ConfigPar.BitsRate = inst->bitrates[0];//512
		ConfigPar.QPMin = 20;
		ConfigPar.QPMax = 35;
		ConfigPar.Mult_Slice = 0;
		ConfigPar.Mtu_Size = H264MTU;
		ConfigPar.Me_Range = 32;
		ConfigPar.Gen_Threads = 1;
		ConfigPar.RC_Method = 2;
		ConfigPar.Subpel_Refine = 5;
		ConfigPar.Level = 30;
		//调用H264编码器初始化函数
		GVE_H264_Encoder_Create(&GVE_CodecEnc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

		//将H264编码输出参数初始化为webrtc中的RTPakect结构体
		//_pRtPacket = new RTPacket;
		_pRtPacket->payLoadType = PAYLOADTYPE_H264;
		_pRtPacket->buf = (char *)_encodedImage._buffer;  //?
		OperatePar.rtpsize = _pRtPacket->size; //?

		//给编码原始图像及码流存储分配空间及初始化宽高参数
		_width_used = ConfigPar.InPut_Width;
		_height_used = ConfigPar.InPut_Height;
		_maxImgBufCnt = 4;
		_width = ConfigPar.InPut_Width;
		_height = ConfigPar.InPut_Height;

		int size = (_width * _height * 3) >> 1;

		for (i = 0; i < _maxImgBufCnt; i++)
		{
			if(_encImg[i]._buffer == NULL)
				_encImg[i]._buffer = new uint8_t[size];
		}

		if(_inEncBuf == NULL)
			_inEncBuf = new uint8_t[size];

	}
	else if (_pRtPacket->payLoadType == PAYLOADTYPE_SVC_19)
	{
		//将编码参数传给SVC编码器
		SVCConfigPar.gop = GOPSIZE_SVC;
		SVCConfigPar.layer = inst->enclayer;
		for (i=0; i< SVCConfigPar.layer ; i++)
		{
			SVCConfigPar.Inwidth[i] = inst->width_used>>i;
			SVCConfigPar.InHeight[i] = inst->height_used>>i;
			SVCConfigPar.qp[i][0] = 23;
			SVCConfigPar.qp[i][1] = 40;
			SVCConfigPar.bitrateLayer[i] = inst->bitrates[i];//i=0,表示最大层

		}
		
		if (inst->width_used == 1280 && inst->height_used == 720)
		{
			switch(inst->encLevel)
			{
			case 1://低配电脑
				SVCConfigPar.preset = "ultrafast";
				SVCConfigPar.framerate = 13;
				SVCConfigPar.bframenum = 0;
				SVCConfigPar.subpel_refine = 3;//5;
				SVCConfigPar.me_method = 0;
				SVCConfigPar.enable_cabac = 0;//1
				SVCConfigPar.enable_8x8dct = 0;
				SVCConfigPar.intra = X264_ANALYSE_I4x4/* | X264_ANALYSE_I8x8*/;
				SVCConfigPar.inter =/* X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | */X264_ANALYSE_PSUB16x16 /*| X264_ANALYSE_BSUB16x16*/;
				break;
			case 2://标配电脑
				SVCConfigPar.preset = "medium";
				SVCConfigPar.framerate = 15;
				SVCConfigPar.bframenum = inst->bFrameNum;
				SVCConfigPar.subpel_refine = 3;//5;
				SVCConfigPar.me_method = 0;
				SVCConfigPar.enable_cabac = 1;
				SVCConfigPar.enable_8x8dct = 0;
				if (SVCConfigPar.bframenum ==0)
				{
					SVCConfigPar.intra = X264_ANALYSE_I4x4 /*| X264_ANALYSE_I8x8*/;
					SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16/* | X264_ANALYSE_BSUB16x16*/;
				}
				else if (SVCConfigPar.bframenum ==3)
				{
					SVCConfigPar.intra = X264_ANALYSE_I4x4 /*| X264_ANALYSE_I8x8*/;
					SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
				}
				break;
			case 3://高配电脑
				SVCConfigPar.preset = "medium";
				SVCConfigPar.framerate = 15;
				SVCConfigPar.bframenum = inst->bFrameNum;;
				SVCConfigPar.subpel_refine = 5;
				SVCConfigPar.me_method = 1;//0;
				SVCConfigPar.enable_cabac = 1;
				SVCConfigPar.enable_8x8dct = 1;//0
				if (SVCConfigPar.bframenum ==0)
				{
					SVCConfigPar.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
					SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16/* | X264_ANALYSE_BSUB16x16*/;
				}
				else if (SVCConfigPar.bframenum ==3)
				{
					SVCConfigPar.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
					SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
				}
				break;
			default://标配电脑
				SVCConfigPar.preset = "medium";
				SVCConfigPar.framerate = 15;
				SVCConfigPar.bframenum = inst->bFrameNum;;
				SVCConfigPar.subpel_refine = 3;//5;
				SVCConfigPar.me_method = 0;
				SVCConfigPar.enable_cabac = 1;
				SVCConfigPar.enable_8x8dct = 0;
				if (SVCConfigPar.bframenum ==0)
				{
					SVCConfigPar.intra = X264_ANALYSE_I4x4 /*| X264_ANALYSE_I8x8*/;
					SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16 /*| X264_ANALYSE_BSUB16x16*/;
				}
				else if (SVCConfigPar.bframenum ==3)
				{
					SVCConfigPar.intra = X264_ANALYSE_I4x4 /*| X264_ANALYSE_I8x8*/;
					SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
				}

				break;
			}
		}
		else//最大层非高清
		{
			SVCConfigPar.preset = "medium";
			SVCConfigPar.framerate = 15;
			SVCConfigPar.bframenum = inst->bFrameNum;;
			SVCConfigPar.subpel_refine = 5;
			SVCConfigPar.me_method = 1;//0;
			SVCConfigPar.enable_cabac = 1;
			SVCConfigPar.enable_8x8dct = 1;//0
			if (SVCConfigPar.bframenum ==0)
			{
				SVCConfigPar.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
				SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16/* | X264_ANALYSE_BSUB16x16*/;
			}
			else if (SVCConfigPar.bframenum ==3)
			{
				SVCConfigPar.intra = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8;
				SVCConfigPar.inter = X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16 | X264_ANALYSE_BSUB16x16;
			}
		}


		 if (inst->encLevel == 100)
		 {
			 	SVCConfigPar.gop = GOPSIZE_SVC_P2P;
		 }


		SVCConfigPar.gen_threads = 2;//2
		SVCConfigPar.tune = NULL;
		SVCConfigPar.psnr = 0;
		SVCConfigPar.rf_constant = 28;
		SVCConfigPar.mtu_size = H264MTU;//RTP包大小
		SVCConfigPar.mult_slice = 0;
		SVCConfigPar.ml_err_resilience = 1;
		SVCConfigPar.sliced_threads = 0;
		SVCConfigPar.rc_method = 2;
		SVCConfigPar.me_range = 16;
		SVCConfigPar.frame_reference = 3;
		SVCConfigPar.lookahead = SVCConfigPar.bframenum ==0 ? 1: 4;

		WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
			"Current pre enc resolution: %d x %d, payload type:%d, enc layernum:%d, enc level:%d, bitrate:%d, %d, %d",
			SVCConfigPar.Inwidth[0],SVCConfigPar.InHeight[0], PAYLOADTYPE_SVC_19,SVCConfigPar.layer, inst->encLevel, SVCConfigPar.bitrateLayer[0], SVCConfigPar.bitrateLayer[1], SVCConfigPar.bitrateLayer[2]);
		//锟斤拷锟斤拷SVC锟斤拷锟斤拷锟斤拷锟斤拷始锟斤拷锟斤拷锟斤拷
		int ret = GVE_SVC_Encoder_Create(&GVE_CodecEnc_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

		if(ret < 0)
		{		
			WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
				"I420Decoder::InitSVCEnc GVE_SVC_Encoder_Create fail ret = %d.", ret );
			return -1;
		}

		//将SVC编码输出参数初始化为webrtc中的RTPakect结构体
		//_pRtPacket = new RTPacket;
		_pRtPacket->payLoadType = PAYLOADTYPE_SVC_19;
		_pRtPacket->buf = (char *)_encodedImage._buffer;  //?
		SVCOperatePar.rtpsize = _pRtPacket->size; //?

		//给编码原始图像及码流存储分配空间及初始化宽高参数
		_width_used = SVCConfigPar.Inwidth[0];
		_height_used = SVCConfigPar.InHeight[0];
		_maxImgBufCnt = 4;
		_width = SVCConfigPar.Inwidth[0];
		_height = SVCConfigPar.InHeight[0];

		int size = (_width * _height * 3) >> 1;

		for (i = 0; i < _maxImgBufCnt; i++)
		{
			if(_encImg[i]._buffer == NULL)
				_encImg[i]._buffer = new uint8_t[size];
		}

		if(_inEncBuf == NULL)
			_inEncBuf = new uint8_t[size];


#if 0  //delete wyh
	if (_EncCfg.m_wirteEncfile)
	{
		//0 layer
		SVCConfigPar.debugfilename = "D:\\quanshi_video\\x264_enc_0.264";
		SVCConfigPar.debugfp = fopen(SVCConfigPar.debugfilename, "wb");
		SVCConfigPar.debuglayer = 0;
		SVCConfigPar.debugflag = 1;
		GVE_SVC_Encoder_SetDebugFile(GVE_CodecEnc_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

		//1 layer
		SVCConfigPar.debugfilename = "D:\\quanshi_video\\x264_enc_1.264";
		SVCConfigPar.debugfp = fopen(SVCConfigPar.debugfilename, "wb");
		SVCConfigPar.debuglayer = 1;
		SVCConfigPar.debugflag = 1;
		GVE_SVC_Encoder_SetDebugFile(GVE_CodecEnc_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

		//2 layer
		SVCConfigPar.debugfilename = "D:\\quanshi_video\\x264_enc_2.264";
		SVCConfigPar.debugfp = fopen(SVCConfigPar.debugfilename, "wb");
		SVCConfigPar.debuglayer = 2;
		SVCConfigPar.debugflag = 1;
		GVE_SVC_Encoder_SetDebugFile(GVE_CodecEnc_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);
	}
#endif
	}
	//end codec

	_pSvc_event = EventWrapper::Create();
	_pSvc_critsect = CriticalSectionWrapper::CreateCriticalSection();
	_pSvc_thread = ThreadWrapper::CreateThread(SVCThreadProc, this,kNormalPriority, "SVC");

	unsigned int tid = 0;
	if(_pSvc_thread->Start(tid))
		_isRunning = true;

	return WEBRTC_VIDEO_CODEC_OK;
}

WebRtc_Word32
I420Encoder::GetEncodedPartitions() 
{
	int part_idx = 0;
	_encodedImage._length = 0;
	webrtc::RTPFragmentationHeader frag_info;

	WebRtc_RTPacket *rtPacket = _pRtPacket;
	int i = 0;
	int count = rtPacket->count;

	if(rtPacket->rtpLen > 0)
	{
		frag_info.VerifyAndAllocateFragmentationHeader(count);

	}
	//add by zj@2013-06-21
	if(PAYLOADTYPE_H264 == rtPacket->payLoadType)
	{
		for (i = 0; i < count; i++)
		{
			if(rtPacket->size[i] > 0)
			{				  
				memcpy_neon(&_encodedImage._buffer[_encodedImage._length],
                       &rtPacket->buf[_encodedImage._length],
                       rtPacket->size[i]);
//				memcpy(&_encodedImage._buffer[_encodedImage._length],
//					&rtPacket->buf[_encodedImage._length],
//					rtPacket->size[i]);
				frag_info.fragmentationOffset[part_idx] = _encodedImage._length;
				frag_info.fragmentationLength[part_idx] =  rtPacket->size[i];
				frag_info.fragmentationPlType[part_idx] = 0;
				frag_info.fragmentationTimeDiff[part_idx] = 0;
				_encodedImage._length += rtPacket->size[i];

				if(_encodedImage._length > _encodedImage._size)
				{
					break;
				}
				++part_idx;
				rtPacket->size[i] = 0;
			}
		}	
	}
	else
	{
		int send_length = 0;

		for (i = 0; i < count; i++)
		{
			if(rtPacket->size[i] > 0)
			{		
                    memcpy_neon(&_encodedImage._buffer[send_length],
                           &rtPacket->buf[_encodedImage._length],
                           rtPacket->size[i]);
/*					memcpy(&_encodedImage._buffer[send_length],
						&rtPacket->buf[_encodedImage._length],
						rtPacket->size[i]);*/

					frag_info.fragmentationOffset[part_idx] = send_length;
					frag_info.fragmentationLength[part_idx] =  rtPacket->size[i];
					frag_info.fragmentationPlType[part_idx] = 0;
					frag_info.fragmentationTimeDiff[part_idx] = 0;
					send_length += rtPacket->size[i];
					++part_idx;
				
				_encodedImage._length += rtPacket->size[i];
				if(_encodedImage._length > _encodedImage._size)
				{
					break;
				}
				rtPacket->size[i] = 0;
			}
		}

		frag_info.fragmentationVectorSize = part_idx;
	}
	//add end
	if (_encodedImage._length > 0) 
	{ 
		_encodedCompleteCallback->Encoded(_encodedImage, NULL,&frag_info);

	}
	return WEBRTC_VIDEO_CODEC_OK;
}

bool I420Encoder::SVCThreadProc(void* obj)
{
	return static_cast<I420Encoder*> (obj)->SVCProcess();
}

bool I420Encoder::SVCProcess()
{
	static int svc_thread_cnt = 0;
	int time_limit = 1000 / 30;
	int flag = 0;
	int size = _width_used * _height_used;

	_pSvc_event->Wait(time_limit);

	if (!_pSvc_thread || !_isRunning)
	{
		//stop the thread
		return false;
	}
	if(_outImgBufIdx != _lastOutImgBufIdx)
	{
		_pSvc_critsect->Enter();
		flag = abs(_outImgBufIdx - _inImgBufIdx) >= 1;
		if(flag)
		{
            memcpy_neon(_inEncBuf,_encImg[_outImgBufIdx]._buffer,(size * 3) >> 1);
//			memcpy(_inEncBuf,_encImg[_outImgBufIdx]._buffer,(size * 3) >> 1);
			_encodedImage._frameType = kKeyFrame; // No coding.
			_encodedImage._timeStamp = _encImg[_outImgBufIdx]._timeStamp;
			_encodedImage.capture_time_ms_ = _encImg[_outImgBufIdx].capture_time_ms_;
			_encodedImage._encodedHeight = _encImg[_outImgBufIdx]._encodedHeight;
			_encodedImage._encodedWidth = _encImg[_outImgBufIdx]._encodedWidth;
			_lastOutImgBufIdx = _outImgBufIdx;

		}
		_pSvc_critsect->Leave();
		//CriticalSectionScoped cs(_pSvc_critsect);
		if(flag)
		{
			char *outImgBuf[3];
			outImgBuf[0] = (char *)_inEncBuf;
			outImgBuf[1] = &outImgBuf[0][size];
			outImgBuf[2] = &outImgBuf[1][size >> 2];




			SVCEnc(outImgBuf);
			_outImgBufIdx++;
			_outImgBufIdx = _outImgBufIdx >= _maxImgBufCnt ? 0 : _outImgBufIdx;
			svc_thread_cnt++;
		}
	}
	return true;
}

bool I420Encoder::SVCStopThread()
{
	_pSvc_thread->SetNotAlive();
	// Make sure the thread finishes as quickly as possible (instead of having
	// to wait for the timeout).
	_isRunning = false;
	_pSvc_event->Reset();
	  __android_log_print(ANDROID_LOG_ERROR, "yyf","I420Encoder::SVCStopThread start");
	bool stopped = _pSvc_thread->Stop();
	  __android_log_print(ANDROID_LOG_ERROR, "yyf","I420Encoder::SVCStopThread end");
	_pSvc_thread = NULL;

	return stopped;
}
WebRtc_Word32 
I420Encoder::ImgCopy(const webrtc::I420VideoFrame& inputImage)
{
	int size =  inputImage.width() * inputImage.height();
	char *inY = (char *)inputImage.buffer(webrtc::kYPlane);
	char *inU = (char *)inputImage.buffer(webrtc::kUPlane);
	char *inV = (char *)inputImage.buffer(webrtc::kVPlane);
	char *outImgBuf[3];
	outImgBuf[0] = (char *)_encImg[_inImgBufIdx]._buffer;
	outImgBuf[1] = &outImgBuf[0][size];
	outImgBuf[2] = &outImgBuf[1][size >> 2];

    memcpy_neon(outImgBuf[0],inY,size);
    memcpy_neon(outImgBuf[1],inU,size >> 2);
    memcpy_neon(outImgBuf[2],inV,size >> 2);

//	memcpy(outImgBuf[0],inY,size);
//	memcpy(outImgBuf[1],inU,size >> 2);
//	memcpy(outImgBuf[2],inV,size >> 2);


	_encImg[_inImgBufIdx]._timeStamp = inputImage.timestamp();
	_encImg[_inImgBufIdx].capture_time_ms_ = inputImage.render_time_ms();
	_encImg[_inImgBufIdx]._encodedHeight = inputImage.height();
	_encImg[_inImgBufIdx]._encodedWidth = inputImage.width();

	return  WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}

WebRtc_Word32
I420Encoder::SVCEnc(char *apSrcData[3])
{
	//__android_log_print(ANDROID_LOG_ERROR, "yyf","SVCENC START");
	if(GVE_CodecEnc_Handle && _pRtPacket->payLoadType == PAYLOADTYPE_H264)
	{
        

		//初始化传入图像目的图像数据
		int size = _width_used * _height_used;

		OperatePar.OutBuf = ( unsigned char *)_encodedImage._buffer;
		int len = size * 3 >> 1;
		OperatePar.InPutLen = len;
		OutPutInfo.abMark = GVE_H264_Encoder_GetSliceType(GVE_CodecEnc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
		if(OperatePar.InBuf == NULL)
		{
			OperatePar.InBuf = (unsigned char *)apSrcData[0];//(unsigned char *)_encImg[_inImgBufIdx]._buffer;
		}

		OperatePar.OutputLen = 0;
		OperatePar.rtpcount = 0;
		//调用编码函数
		GVE_H264_Encoder_Encoder(GVE_CodecEnc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
		_encodedImage._length = OperatePar.OutputLen;
		_pRtPacket->count = OperatePar.rtpcount;
		_pRtPacket->rtpLen = OperatePar.OutputLen;

		__android_log_print(ANDROID_LOG_ERROR, "yyf","encoded size = %d", OperatePar.OutputLen);


		if (OutPutInfo.abMark == PIC_TYPE_IDR || OutPutInfo.abMark == PIC_TYPE_I || OutPutInfo.abMark == PIC_TYPE_KEYFRAME)
			_encodedImage._frameType = kKeyFrame;
		else
			_encodedImage._frameType = kDeltaFrame;

		if(_encodedImage._length > 0)
			GetEncodedPartitions();
		return WEBRTC_VIDEO_CODEC_OK; 
	}
	else
	{
		//初始化输入图像数据，输入图像Buffer，长度，输出图像Buffer，长度
		int size = _width_used * _height_used;
		SVCOperatePar.OutBuf = (unsigned char *)_encodedImage._buffer;
		SVCOperatePar.InPutLen /*= SVCOperatePar.OutputLen*/ = size *3 >>1;
		SVCOperatePar.InBuf = (unsigned char *)apSrcData[0];
		SVCOutPutInfo.abMark = 0;

		SVCOperatePar.OutputLen = 0;
		SVCOperatePar.rtpcount = 0;
		GVE_SVC_Encoder_Encoder(GVE_CodecEnc_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

	//	__android_log_print(ANDROID_LOG_ERROR, "kxw","encoded size = %d", SVCOperatePar.OutputLen);
		_encodedImage._length = SVCOperatePar.OutputLen;
		_pRtPacket->count = SVCOperatePar.rtpcount;
		_pRtPacket->rtpLen = SVCOperatePar.OutputLen;

		if (SVCOutPutInfo.abMark == PIC_TYPE_IDR || SVCOutPutInfo.abMark == PIC_TYPE_I || SVCOutPutInfo.abMark == PIC_TYPE_KEYFRAME)
			_encodedImage._frameType = kKeyFrame;
		else
			_encodedImage._frameType = kDeltaFrame;

		
		//add end
//__android_log_print(ANDROID_LOG_DEBUG, "kxw","%d ok, %d", __LINE__, _encodedImage._length);
		if(_encodedImage._length > 0)
			GetEncodedPartitions();
//__android_log_print(ANDROID_LOG_DEBUG, "kxw","%d ok", __LINE__);
		return WEBRTC_VIDEO_CODEC_OK; 
	}

	return  WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}


int I420Encoder::InitEncode(const VideoCodec* codecSettings,
							int /*numberOfCores*/,
							uint32_t /*maxPayloadSize */)
{
	if (codecSettings == NULL) 
	{
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}
	if (codecSettings->width_used < 1 || codecSettings->height_used < 1) 
	{
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}

	// Allocating encoded memory.
	if (_encodedImage._buffer != NULL) 
	{
		delete [] _encodedImage._buffer;
		_encodedImage._buffer = NULL;
		_encodedImage._size = 0;
	}
	const uint32_t newSize = CalcBufferSize(kI420,codecSettings->width_used,codecSettings->height_used);
	uint8_t* newBuffer = new uint8_t[newSize];
	if (newBuffer == NULL)
	{
		return WEBRTC_VIDEO_CODEC_MEMORY;
	}
	_encodedImage._size = newSize;
	_encodedImage._buffer = newBuffer;
	InitSVCEnc(codecSettings);

	// If no memory allocation, no point to init.
	_inited = true;
	return WEBRTC_VIDEO_CODEC_OK;
}



int I420Encoder::Encode(const I420VideoFrame& inputImage,
						const CodecSpecificInfo* /*codecSpecificInfo*/,
						const std::vector<VideoFrameType>* /*frame_types*/) 
{
	if (!_inited) 
	{
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}
	if (_encodedCompleteCallback == NULL) 
	{
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}

	int req_length = CalcBufferSize(kI420, inputImage.width(),
		inputImage.height());



	WebRtc_Word32 runflag = WEBRTC_VIDEO_CODEC_OK;


	if(GVE_CodecEnc_Handle)
	{
		CriticalSectionScoped cs(_pSvc_critsect);
		ImgCopy(inputImage);


		_inImgBufIdx++;
		_inImgBufIdx = _inImgBufIdx >= _maxImgBufCnt ? 0 : _inImgBufIdx;

		_pSvc_event->Set();
	}
	else
		runflag = WEBRTC_VIDEO_CODEC_NO_OUTPUT;

	return runflag;
}

int I420Encoder::RegisterEncodeCompleteCallback(EncodedImageCallback* callback) 
{
	_encodedCompleteCallback = callback;
	return WEBRTC_VIDEO_CODEC_OK;
}


I420Decoder::I420Decoder():
_decodedImage(),
_width(0),
_height(0),
_inited(false),
_decodeCompleteCallback(NULL),
_isH264(false),
GVE_CodecDec_Handle(0)
{
	//add wyh
	memset(&OperatePar,0,sizeof(GVE_H264Dec_OperatePar));
	memset(&ConfigPar,0,sizeof(GVE_H264Dec_ConfigPar));
	memset(&OutPutInfo,0,sizeof(GVE_H264Dec_OutPutInfo));
	memset(&SVCOperatePar,0,sizeof(GVE_SVCDec_OperatePar));
	memset(&SVCConfigPar,0,sizeof(GVE_SVCDec_ConfigPar));
	memset(&SVCOutPutInfo,0,sizeof(GVE_SVCDec_OutPutInfo));
    tmpo[0] = new unsigned char[1920*1080];
 	tmpo[1] = new unsigned char[1920*1080/4];
 	tmpo[2] = new unsigned char[1920*1080/4];
}

I420Decoder::~I420Decoder()
{
	if (tmpo[0] != NULL)
	{
		delete tmpo[0];
	}
	if (tmpo[1] != NULL)
	{
		delete tmpo[1];
	}
	if (tmpo[2] != NULL)
	{
		delete tmpo[2];
	}
	Release();
	if(fpOutfile_dec)
		fclose(fpOutfile_dec);
	if(fpRtpCountfile_dec)
		fclose(fpRtpCountfile_dec);
	if(fpRtpSizefile_dec)
		fclose(fpRtpSizefile_dec);
}

int I420Decoder::Reset()
{
	return WEBRTC_VIDEO_CODEC_OK;
}

int  I420Decoder::InitSVCDec(webrtc::VideoCodec* inst)
{
	if (inst->plType == PAYLOADTYPE_H264)
	{
		int ret;
		_isH264 = true;
		ret = GVE_H264_Decoder_Create(&GVE_CodecDec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

		if(ret < 0)
		{
			WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
			"I420Decoder::InitSVCDec GVE_H264_Decoder_Create fail ret = %d.", ret );
			return -1;			
		}
	}
	else
	{

		SVCConfigPar.Width = inst->width_used;
		SVCConfigPar.Height = inst->height_used;
        SVCOperatePar.InBuf = (unsigned char *)inst->buffer;

		WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
			"Current pre decoded resolution: %d x %d, payload type:%d",SVCConfigPar.Width,SVCConfigPar.Height,PAYLOADTYPE_SVC_19);

		int ret;
		_isH264 = false;

		__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder::InitSVCDec,svc decoder create, width = %d, height = %d, buf = %d", inst->width_used, inst->height_used, inst->buffer);
		ret = GVE_SVC_Decoder_Create(&GVE_CodecDec_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

		if(ret)
		{
			if(ret < 0)
			{
				WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
				"I420Decoder::InitSVCDec GVE_SVC_Decoder_Create fail ret = %d.", ret );
				return -1;			
			}
			__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder::InitSVCDec,svc decoder create faile");
			return -1;
		}
	}

#if 0  //delete wyh
	if (_decCfg.m_wirteDecfile)
	{
		SVCConfigPar.debugfilename = "D:\\quanshi_video\\x264_dec_0.264";
		SVCConfigPar.debugfp = fopen(SVCConfigPar.debugfilename, "wb");
		SVCConfigPar.debuglayer = 0;
		SVCConfigPar.debugflag = 1;
		GVE_SVC_Decoder_SetDebugFile(GVE_CodecDec_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

		SVCConfigPar.debugfilename = "D:\\quanshi_video\\x264_dec_1.264";
		SVCConfigPar.debugfp = fopen(SVCConfigPar.debugfilename, "wb");
		SVCConfigPar.debuglayer = 1;
		SVCConfigPar.debugflag = 1;
		GVE_SVC_Decoder_SetDebugFile(GVE_CodecDec_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);

		SVCConfigPar.debugfilename = "D:\\quanshi_video\\x264_dec_2.264";
		SVCConfigPar.debugfp = fopen(SVCConfigPar.debugfilename, "wb");
		SVCConfigPar.debuglayer = 2;
		SVCConfigPar.debugflag = 1;
		GVE_SVC_Decoder_SetDebugFile(GVE_CodecDec_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);
	}
#endif
		__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder::InitSVCDec,svc decoder create success");
		return 0;

}

WebRtc_Word32 I420Decoder::SVCDec(const EncodedImage& inputImage)
{
	int ret = 0;

	if (inputImage._length)
	{
		if (_isH264)
		{
			char *obuf[3];
			OperatePar.OutBuf[0] = (unsigned char *)tmpo[0];
			OperatePar.OutBuf[1] = (unsigned char *)tmpo[1];
			OperatePar.OutBuf[2] = (unsigned char *)tmpo[2];
			OperatePar.InBuf = (unsigned char *)inputImage._buffer;
			OperatePar.InPutLen = inputImage._length;
			OperatePar.rtpsize = (short *)inputImage._packetSize;
			OperatePar.rtpcount = inputImage._count;

			if (inputImage._packetSize <= 0 || inputImage._count <=0 || OperatePar.InPutLen<=0)
			{
				return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
			}
#if 0	
			//for test
			if(fpOutfile_dec == NULL)
				fpOutfile_dec = fopen("/sdcard/gnet_dec.bin","wb");
			if(fpRtpCountfile_dec == NULL)
				fpRtpCountfile_dec = fopen("/sdcard/gnet_rtpCount.bin","wb");
			if(fpRtpSizefile_dec == NULL)
				fpRtpSizefile_dec = fopen("/sdcard/gnet_rtpSize.bin","wb");
			if(fpOutfile_dec)
				fwrite(OperatePar.InBuf,1,OperatePar.InPutLen,fpOutfile_dec);
			if(fpRtpCountfile_dec){
				fwrite(&OperatePar.rtpcount,1,4,fpRtpCountfile_dec);
				fflush(fpRtpCountfile_dec);
}
			if(fpRtpSizefile_dec)
				fwrite(OperatePar.rtpsize,1,OperatePar.rtpcount*sizeof(short),fpRtpSizefile_dec);
			//end test
#endif			
			memset(&OutPutInfo,0,sizeof(OutPutInfo));
			GVE_H264_Decoder_Decoder(GVE_CodecDec_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

	//__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder:,outputsize =  %d", OperatePar.OutputLen);
			if (OperatePar.OutputLen >0 && OutPutInfo.width >0 && OutPutInfo.height >0)
			{
				_width = OutPutInfo.width;
				_height = OutPutInfo.height;

				int half_width = (_width + 1) / 2;
				_decodedImage.CreateEmptyFrame(_width, _height,
					_width, half_width, half_width);
				obuf[0] = (char *)_decodedImage.buffer(webrtc::kYPlane);
				obuf[1] = (char *)_decodedImage.buffer(webrtc::kUPlane);
				obuf[2] = (char *)_decodedImage.buffer(webrtc::kVPlane);
				memcpy_neon(obuf[0], tmpo[0], _decodedImage.allocated_size(kYPlane));
				memcpy_neon(obuf[1], tmpo[1], _decodedImage.allocated_size(kUPlane));
				memcpy_neon(obuf[2], tmpo[2], _decodedImage.allocated_size(kVPlane));

//				memcpy(obuf[0], tmpo[0], _decodedImage.allocated_size(kYPlane));
//				memcpy(obuf[1], tmpo[1], _decodedImage.allocated_size(kUPlane));
//				memcpy(obuf[2], tmpo[2], _decodedImage.allocated_size(kVPlane));
				
			}
			else
			{
				return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
			}
			
		}
		else
		{
			char *obuf[3];
			SVCOperatePar.OutBuf[0] = (unsigned char *)tmpo[0];
			SVCOperatePar.OutBuf[1] = (unsigned char *)tmpo[1];
			SVCOperatePar.OutBuf[2] = (unsigned char *)tmpo[2];
			SVCOperatePar.InBuf = (unsigned char *)inputImage._buffer;
			SVCOperatePar.InPutLen = inputImage._length;
			SVCOperatePar.rtpsize = (short *)inputImage._packetSize;
			SVCOperatePar.rtpcount = inputImage._count;

			if (inputImage._packetSize <= 0 || inputImage._count <=0 || SVCOperatePar.InPutLen<=0)
			{
				return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
			}

			//for test
//  			fwrite(SVCOperatePar.InBuf,1,SVCOperatePar.InPutLen,fpOutfile_dec);
//  			fwrite(&SVCOperatePar.rtpcount,1,4,fpRtpCountfile_dec);
//  			fwrite(SVCOperatePar.rtpsize,1,SVCOperatePar.rtpcount*sizeof(short),fpRtpSizefile_dec);
			//end test

//			__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder:SVC,input size =  %d", inputImage._length);
			memset(&SVCOutPutInfo,0,sizeof(GVE_SVCEnc_OutPutInfo));
			GVE_SVC_Decoder_Decode(GVE_CodecDec_Handle,&SVCOperatePar,&SVCConfigPar,&SVCOutPutInfo);
//__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder:SVC,outputsize =  %d, outputwidth = %d, outputheight = %d", SVCOperatePar.OutputLen, SVCOutPutInfo.OutPutWidth, SVCOutPutInfo.OutPutHeight);

			if(SVCOperatePar.OutputLen >0 && SVCOutPutInfo.OutPutWidth >0 && SVCOutPutInfo.OutPutHeight >0)
			{
                SVCOutPutInfo.OutPutHeight = ((SVCOutPutInfo.OutPutHeight == 184) ? 180: ((SVCOutPutInfo.OutPutHeight == 96) ? 90:SVCOutPutInfo.OutPutHeight));
                SVCOperatePar.OutputLen = SVCOutPutInfo.OutPutWidth * SVCOutPutInfo.OutPutHeight *3/2;

				_width = SVCOutPutInfo.OutPutWidth;
				_height = SVCOutPutInfo.OutPutHeight;

				int half_width = (_width + 1) / 2;
				_decodedImage.CreateEmptyFrame(_width, _height,
					_width, half_width, half_width);
				obuf[0] = (char *)_decodedImage.buffer(webrtc::kYPlane);
				obuf[1] = (char *)_decodedImage.buffer(webrtc::kUPlane);
				obuf[2] = (char *)_decodedImage.buffer(webrtc::kVPlane);

				memcpy(obuf[0], tmpo[0], _decodedImage.allocated_size(kYPlane));
				memcpy(obuf[1], tmpo[1], _decodedImage.allocated_size(kUPlane));
				memcpy(obuf[2], tmpo[2], _decodedImage.allocated_size(kVPlane));
                    
            }
            else
            {
            	WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCoding, 0,	"I420Decoder::SVCDEC lost");
                return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
            }
        }
    }

	return WEBRTC_VIDEO_CODEC_OK;
}

int I420Decoder::InitDecode(VideoCodec* codecSettings,
							int /*numberOfCores */)
{
	if (codecSettings == NULL)
	{
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	} else if (codecSettings->width_used < 1 || codecSettings->height_used< 1)
	{
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}
	_width = codecSettings->width_used;
	_height = codecSettings->height_used;

	if(-1== InitSVCDec(codecSettings))
	{
		WEBRTC_TRACE(
			kTraceError,
			kTraceVideoCoding,
			-1,
			"InitSVCDec is failed!!!"); 
		return -1;
	}
	_inited = true;
	WEBRTC_TRACE(
		kTraceInfo,
		kTraceVideoCoding,
		-1,
		"InitSVCDec is successful"); 
	return WEBRTC_VIDEO_CODEC_OK;
}

int I420Decoder::Decode(const EncodedImage& inputImage,
						bool /*missingFrames*/,
						const RTPFragmentationHeader* /*fragmentation*/,
						const CodecSpecificInfo* /*codecSpecificInfo*/,
						int64_t /*renderTimeMs*/) 
{
	if (inputImage._buffer == NULL) 
	{
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}
	if (_decodeCompleteCallback == NULL) 
	{
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}
	if (inputImage._length <= 0) 
	{
		return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
	}

	if (!_inited) {
		return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
	}
//	__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder::Decode,packetSize =  %d", inputImage._length);
__android_log_print(ANDROID_LOG_ERROR, "yyf","I420Decoder::Decode,packetSize =  %d:, count:%d", inputImage._length, inputImage._count);
	int ret =0;
	unsigned short * pInputImage =NULL;
	pInputImage=(unsigned short *)inputImage._packetSize;
	if(pInputImage&&inputImage._count>0) 
	{
		ret = SVCDec(inputImage);

		if(ret==WEBRTC_VIDEO_CODEC_OK) //modified by wuzhong 20130701
		{
			_decodedImage.set_timestamp(inputImage._timeStamp);

			_decodeCompleteCallback->Decoded(_decodedImage);
			return WEBRTC_VIDEO_CODEC_OK;
		}
		else
		{
			return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
		}
	}
	else
		return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}

int I420Decoder::RegisterDecodeCompleteCallback(DecodedImageCallback* callback) 
{
	_decodeCompleteCallback = callback;
	return WEBRTC_VIDEO_CODEC_OK;
}

int I420Decoder::Release() 
{
	_inited = false;

	if (_isH264)
	{
		if (GVE_CodecDec_Handle)
		{
			GVE_H264_Decoder_Destroy(GVE_CodecDec_Handle);
			GVE_CodecDec_Handle = NULL;
		}
	}
	else
	{
		if(GVE_CodecDec_Handle)
		{
			GVE_SVC_Decoder_Destroy(GVE_CodecDec_Handle);
			GVE_CodecDec_Handle = NULL;
		}
	}
	return WEBRTC_VIDEO_CODEC_OK;
}
    
}
