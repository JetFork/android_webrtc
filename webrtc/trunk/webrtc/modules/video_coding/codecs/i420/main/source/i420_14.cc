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

#define MINLAYERBITRATE  34
const unsigned short normResolutionTab[16][3] = 
{
	{640	,480	,0},
	{320	,240	,1},
	{160	,120	,2},
	{704	,576	,0},
	{352	,288	,1},
	{176	,144	,2},
	{1920	,1080	,0},
	{1280	,720	,0},
};

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
I420Encoder_14::I420Encoder_14():
_inited(false),
_encodedImage(),
_encodedCompleteCallback(NULL)
{
	int i;
	_pEncHandle = NULL;
	_pRtPacket = NULL;
	_maxImgBufCnt = 0;
	_inImgBufIdx = 0;
	_outImgBufIdx = 0;
	_lastOutImgBufIdx = -1;
	_lastTime = 0;
	_frameRate = 30;
	_bitsrate = 0;
	for (i = 0; i < MAX_IMG_BUF; i++)
	{
		_encImg[i]._buffer = NULL;
		_encImg[i]._timeStamp = 0;
		_encImg[i].capture_time_ms_ = 0;
		_encImg[i]._encodedHeight = 0;
		_encImg[i]._encodedWidth = 0;
	}
	_inEncBuf = NULL;
}

I420Encoder_14::~I420Encoder_14() {
	_inited = false;
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

int I420Encoder_14::Release() {
	// Should allocate an encoded frame and then release it here, for that we
	// actually need an init flag.
	SVCStopThread();
	if (_encodedImage._buffer != NULL) {
		delete [] _encodedImage._buffer;
		_encodedImage._buffer = NULL;
	}
	int i;
	if(_pEncHandle)
	{
		GVECodecClose(_pEncHandle);
		_pEncHandle = NULL;
		_pRtPacket = NULL;
	}
	for (i = 0; i < MAX_IMG_BUF; i++)
	{
		if(_encImg[i]._buffer)
		{
			delete [] _encImg[i]._buffer;
			_encImg[i]._buffer = NULL;
		}
	}
	if(_inEncBuf)
	{
		delete [] _inEncBuf;
		_inEncBuf = NULL;
	}
	_inited = false;
	return WEBRTC_VIDEO_CODEC_OK;
}


WebRtc_Word32 I420Encoder_14::InitSVCEnc(const webrtc::VideoCodec* inst)
{
	int i = 0;
	videoInParams EncParams ;
	memset(&EncParams,0,sizeof(videoInParams));

	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"enc read the codec.ini: bFrame=%d.",
		_EncCfg.m_bframebum);


	EncParams.bframenum = _EncCfg.m_bframebum;//3;
	EncParams.redundancy = 0;
	EncParams.codec = 0;  //bit0:0-enc,1-dec;bit1:0-svc,1-h264
	EncParams.depth = 12;
	_frameRate = EncParams.framerate = inst->maxFramerate;
	EncParams.gop = 33;
	EncParams.widthAlloc = inst->widthAlloc;
	EncParams.heightAlloc = inst->heightAlloc;
	EncParams.width_used = inst->width_used;
	EncParams.height_used = inst->height_used;

    EncParams.bitrateLayer[0] = 300;//inst->Bitrates[0];
	if(EncParams.bitrateLayer[0]<=0)
		EncParams.bitrateLayer[0] = 273; 
    EncParams.bitrateLayer[1] = 200;//inst->Bitrates[1];
	if (EncParams.bitrateLayer[1]<=0)
		EncParams.bitrateLayer[1] =137;
    EncParams.bitrateLayer[2] = 68;//inst->Bitrates[2];
	if(EncParams.bitrateLayer[2]<=0)
		EncParams.bitrateLayer[2] =68;
	EncParams.bitrateLayer[3] = MINLAYERBITRATE;
	_bitsrate = EncParams.bitrateLayer[0] +EncParams.bitrateLayer[1] +EncParams.bitrateLayer[2] +EncParams.bitrateLayer[3];


	for (i=0; i<4; i++)
	{
		EncParams.qp[i][0] = _EncCfg.m_qp[i][0]; //min
		if(EncParams.qp[i][0]<=0)
			EncParams.qp[i][0] =23;
		EncParams.qp[i][1] = _EncCfg.m_qp[i][1]; //max 
		if(EncParams.qp[i][1]<=0)
			EncParams.qp[i][1] =40;

	}



	EncParams.layer=4;
	EncParams.psnr = 0;
	EncParams.rf_constant = 28;
	EncParams.mtu_size = H264MTU;
	EncParams.mult_slice = 1;
	EncParams.ml_err_resilience = 1;
	EncParams.payload_type = inst->plType;
	EncParams.me_range = 32;
	EncParams.sliced_threads = 0;
	EncParams.gen_threads = 1;
	EncParams.rc_method = 2;
	EncParams.subpel_refine = 5;
	EncParams.me_method = 1;
	EncParams.disable_cabac = 0;
	EncParams.no_8x8dct=0;
	if(EncParams.payload_type == PAYLOADTYPE_H264)
	{
		EncParams.codec |= 2;
		EncParams.widthAlloc = inst->width_used;
		EncParams.heightAlloc = inst->height_used;
		EncParams.width_used = inst->width_used;
		EncParams.height_used = inst->height_used;
		EncParams.bitrateLayer[0] = 512;
		EncParams.qp[0][0] = 20;
		EncParams.qp[0][1] = 35;
		if(inst->h264SubType <0 )
			return -1;

		if(inst->h264SubType ==0) //baseline 
		{
			EncParams.disable_cabac =1;
			EncParams.bframenum =0;
			EncParams.no_8x8dct=1;
		}
		else if(inst->h264SubType ==1)  
		{
#if 0
			EncParams.disable_cabac =0;
			EncParams.bframenum =_EncCfg.m_bframebum;//3;
			EncParams.no_8x8dct=1;
#endif 
		}
	}
	//__android_log_print(ANDROID_LOG_ERROR, "kmm","I420encoder::InitSVCenc,svc encoder create, width = %d, height = %d, [%d]", EncParams.width_used,EncParams.height_used,EncParams.payload_type);
	
	_pEncHandle = GVECodecInit(&EncParams);
	
	//__android_log_print(ANDROID_LOG_ERROR, "kmm","InitSVCEnc %p ok", _pEncHandle);
	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"Current encoded resolution: %d x %d, payload type:%d,framerate:%d,bitrate(4 layers if pt is 36):%d",
		EncParams.width_used,EncParams.height_used,EncParams.payload_type,EncParams.framerate,_bitsrate);


	if (_EncCfg.m_wirteEncfile)
	{

		char *filename0 = "/sdcard/x264_enc_0.264";
		char *filename1 = "/sdcard/x264_enc_1.264";
		char *filename2 = "/sdcard/x264_enc_2.264";
		char *filename3 = "/sdcard/x264_enc_3.264";
		FILE *pf0 = fopen(filename0, "wb"); 
		FILE *pf1 = fopen(filename1, "wb"); 
		FILE *pf2 = fopen(filename2, "wb"); 
		FILE *pf3 = fopen(filename3, "wb");
		GVESetDebugFile(_pEncHandle,(void*)pf0,filename0,0,1,0);
		GVESetDebugFile(_pEncHandle,(void*)pf1,filename1,1,2,0);
		GVESetDebugFile(_pEncHandle,(void*)pf2,filename2,2,3,0);
		GVESetDebugFile(_pEncHandle,(void*)pf3,filename3,3,4,0);
	}


	if(_pEncHandle == NULL)
		return WEBRTC_VIDEO_CODEC_ERROR;
	if(_pRtPacket == NULL)
		_pRtPacket = (RTPacket *)GVECodecGetRtpPoint(_pEncHandle);

	_width_used = EncParams.width_used;
	_height_used = EncParams.height_used;
	_maxImgBufCnt =4;
	_width = EncParams.widthAlloc;
	_height = EncParams.heightAlloc;
	int size = (_width * _height * 3) >> 1;
	for (i = 0; i < _maxImgBufCnt; i++)
	{
		if(_encImg[i]._buffer == NULL)
			_encImg[i]._buffer = new uint8_t[size];
	}
	if(_inEncBuf == NULL)
		_inEncBuf = new uint8_t[size];

	_pSvc_event = EventWrapper::Create();
	_pSvc_critsect = CriticalSectionWrapper::CreateCriticalSection();
	_pSvc_thread = ThreadWrapper::CreateThread(SVCThreadProc, this,kNormalPriority, "SVC");

	unsigned int tid = 0;
	_pSvc_thread->Start(tid);
	


	return WEBRTC_VIDEO_CODEC_OK;
}

WebRtc_Word32
I420Encoder_14::GetEncodedPartitions() 
{
	int part_idx = 0;
	_encodedImage._length = 0;
	webrtc::RTPFragmentationHeader frag_info;

	RTPacket *rtPacket = _pRtPacket;
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
				/*memcpy(&_encodedImage._buffer[_encodedImage._length],
					&rtPacket->buf[_encodedImage._length],
					rtPacket->size[i]);*/
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
					/*memcpy(&_encodedImage._buffer[send_length],
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

bool I420Encoder_14::SVCThreadProc(void* obj)
{
	return static_cast<I420Encoder_14*> (obj)->SVCProcess();
}

bool I420Encoder_14::SVCProcess()
{
	static int svc_thread_cnt = 0;
	int time_limit = 1000 / _frameRate;
	int flag = 0;
	int size = _width_used * _height_used;

	_pSvc_event->Wait(time_limit);

	if (!_pSvc_thread)
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
			//memcpy(_inEncBuf,_encImg[_outImgBufIdx]._buffer,(size * 3) >> 1);
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

bool I420Encoder_14::SVCStopThread()
{
	_pSvc_thread->SetNotAlive();
	// Make sure the thread finishes as quickly as possible (instead of having
	// to wait for the timeout).
	bool stopped = _pSvc_thread->Stop();

	return stopped;
}
WebRtc_Word32 
I420Encoder_14::ImgCopy(const webrtc::I420VideoFrame& inputImage)
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

	/*memcpy(outImgBuf[0],inY,size);
	memcpy(outImgBuf[1],inU,size >> 2);
	memcpy(outImgBuf[2],inV,size >> 2);*/


	_encImg[_inImgBufIdx]._timeStamp = inputImage.timestamp();
	_encImg[_inImgBufIdx].capture_time_ms_ = inputImage.render_time_ms();
	_encImg[_inImgBufIdx]._encodedHeight = inputImage.height();
	_encImg[_inImgBufIdx]._encodedWidth = inputImage.width();

	return  WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}

//FILE *f1[3]={NULL, NULL, NULL};
WebRtc_Word32
I420Encoder_14::SVCEnc(char *apSrcData[3])
{
	//__android_log_print(ANDROID_LOG_ERROR, "kmm","SVCEnc start");
	if(_pEncHandle)
	{
		int size = _width_used * _height_used;

		char *apDstBuff = (char *)_encodedImage._buffer;
		int len = size * 3 >> 1;
		int aiDstBuffLen = len;
		int EncKeyFrm = 0;
		if(apSrcData[0] == NULL || apSrcData[1] == NULL || apSrcData[2] == NULL)
		{
			apSrcData[0] = (char *)_encImg[_inImgBufIdx]._buffer;
			apSrcData[1] = &apSrcData[0][size];
			apSrcData[2] = &apSrcData[1][size >> 2];
		}



		/*
		int layer =0;
		if (_width_used==640)
		{
		layer = 0;
		if (f1[layer] == NULL)
		{
		f1[layer]=fopen("d:\\or640.yuv", "wb");
		}
		}
		else if (_width_used==320)
		{
		layer = 1;
		if (f1[layer] == NULL)
		{
		f1[layer]=fopen("d:\\or320.yuv", "wb");
		}
		}
		else if (_width_used==160)
		{
		layer = 2;
		if (f1[layer] == NULL)
		{
		f1[layer]=fopen("d:\\or160.yuv", "wb");
		}
		}

		fwrite(apSrcData[0], size, 1, f1[layer]);
		fwrite(apSrcData[1], size/4, 1, f1[layer]);
		fwrite(apSrcData[2], size/4, 1, f1[layer]);
		fflush(f1[layer]);*/
        //struct timeval t_s, t_e;
        //long cost_time = 0;
		//gettimeofday(&t_s, NULL);
		//__android_log_print(ANDROID_LOG_ERROR, "kmm","%d ok", __LINE__);
		_encodedImage._length = GVECodecEnc(_pEncHandle,apSrcData,len,apDstBuff,&aiDstBuffLen,&EncKeyFrm);
		//__android_log_print(ANDROID_LOG_ERROR, "kmm","%d ok", __LINE__);
        //gettimeofday(&t_e, NULL);

	    //cost_time = t_e.tv_usec - t_s.tv_usec;
		//WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		//					        "Current frame encode cost : %ld",cost_time);

		if (EncKeyFrm == PIC_TYPE_IDR || EncKeyFrm == PIC_TYPE_I || EncKeyFrm == PIC_TYPE_KEYFRAME)
			_encodedImage._frameType = kKeyFrame;
		else
			_encodedImage._frameType = kDeltaFrame;


		if(_encodedImage._length > 0)
			GetEncodedPartitions();
		//__android_log_print(ANDROID_LOG_DEBUG, "kxw","%d ok", __LINE__);
		return WEBRTC_VIDEO_CODEC_OK; 
	}

	return  WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}


int I420Encoder_14::InitEncode(const VideoCodec* codecSettings,
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



int I420Encoder_14::Encode(const I420VideoFrame& inputImage,
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
	if(_pEncHandle)
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

int I420Encoder_14::RegisterEncodeCompleteCallback(EncodedImageCallback* callback) 
{
	_encodedCompleteCallback = callback;
	return WEBRTC_VIDEO_CODEC_OK;
}


I420Decoder_14::I420Decoder_14():
_decodedImage(),
_width(0),
_height(0),
_inited(false),
_decodeCompleteCallback(NULL),
_lastEncidx(-1),
_isH264(false)
{
	memset(_pEncoded_Decoded_Info, 0, sizeof(Encoded_Decoded_Info)*4);
    memset(_pDown_Layer_Info, 0, sizeof(Down_Layer_Info));
    _pDown_Layer_Info[0].downLayer = 0xE;
    _pDown_Layer_Info[0].requestLayer = 0xE;
	tmpo[0] = new char[1920*1080];
	tmpo[1] = new char[1920*1080/4];
	tmpo[2] = new char[1920*1080/4];
	_pDecHandle = NULL;
}

I420Decoder_14::~I420Decoder_14()
{
	delete tmpo[0];
	delete tmpo[1];
	delete tmpo[2];
	//delete _spatialResampler;
	Release();
}

int I420Decoder_14::Reset()
{
	return WEBRTC_VIDEO_CODEC_OK;
}

int  I420Decoder_14::InitSVCDec(/*const*/ webrtc::VideoCodec* inst)
{
	videoInParams DecParams = {0};
	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"dec read the codec.ini: bFrame=%d.", _decCfg.m_bframebum);

	DecParams.encIDX = inst->encIdx;//通过RTP扩展头解析获得
	DecParams.bframenum = _decCfg.m_bframebum;//3;
	DecParams.framerate = inst->maxFramerate;
	DecParams.gop = 33;

	DecParams.width_used = inst->width_used;
	DecParams.height_used = inst->height_used;
	DecParams.widthAlloc = normResolutionTab[inst->allocIdx][0];
	DecParams.heightAlloc = normResolutionTab[inst->allocIdx][1];

	DecParams.layer = 4;
	DecParams.codec = 1;  //bit0:0-enc,1-dec;bit1:0-svc,1-h264
	DecParams.redundancy = 0;

	DecParams.mtu_size = H264MTU;
	DecParams.mult_slice = 1;
	DecParams.ml_err_resilience = 1;
	DecParams.payload_type = inst->plType;
	_isH264 = false;
	if(DecParams.payload_type == PAYLOADTYPE_H264)
	{
		DecParams.codec |= 2; //bit0:0-enc,1-dec;bit1:0-svc,1-h264
		_isH264 = true;
		if(inst->h264SubType <0 )
			return -1;
	}

	//__android_log_print(ANDROID_LOG_ERROR, "kmm","InitSVCDec start");
	_pDecHandle = GVECodecInit(&DecParams);
	//__android_log_print(ANDROID_LOG_ERROR, "kmm","InitSVCDec ok %p", _pDecHandle);
	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"Current decoded resolution: %d x %d, payload type:%d",DecParams.width_used,DecParams.height_used,DecParams.payload_type);


	if (_decCfg.m_wirteDecfile)
	{
		char *filename0 = "/sdcard/x264_dec_0.264";
		char *filename1 = "/sdcard/x264_dec_1.264";
		char *filename2 = "/sdcard/x264_dec_2.264";
		char *filename3 = "/sdcard/x264_dec_3.264";
		FILE *pf0 = fopen(filename0, "wb"); 
		FILE *pf1 = fopen(filename1, "wb"); 
		FILE *pf2 = fopen(filename2, "wb"); 
		FILE *pf3 = fopen(filename3, "wb"); 
		GVESetDebugFile(_pDecHandle,(void*)pf0,filename0,0,1,1);
		GVESetDebugFile(_pDecHandle,(void*)pf1,filename1,1,2,1);
		GVESetDebugFile(_pDecHandle,(void*)pf2,filename2,2,3,1);
		GVESetDebugFile(_pDecHandle,(void*)pf3,filename3,3,4,1);
	}

	if(_pDecHandle == NULL)
		return -1;
	else
		return 0;

}

WebRtc_Word32 I420Decoder_14::SVCDec(const EncodedImage& inputImage)
{
	int ret = 0;
	//__android_log_print(ANDROID_LOG_ERROR, "kmm","SVCDec start");
	if(_pDecHandle && inputImage._length)
	{
		char *obuf[3];
		char *inbuf_ptr = (char *)inputImage._buffer;
		int size = inputImage._length;
		int olen = 0;
		int w = 0;
		int h = 0;

		int DecKeyFrm = 0;
		if (inputImage._packetSize <= 0 || inputImage._count <=0 || size<=0)
		{
			return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
		}

		//__android_log_print(ANDROID_LOG_ERROR, "kmm","SVCDec GVECodecDec");
        ret = GVECodecDec(_pDecHandle,inbuf_ptr, size, tmpo, &olen, &w, &h, (unsigned short *)inputImage._packetSize, inputImage._count,_pEncoded_Decoded_Info, _pDown_Layer_Info, &_lastEncidx);
        //__android_log_print(ANDROID_LOG_ERROR, "kmm","SVCDec GVECodecDecend %d", ret);
        if(_pDown_Layer_Info[0].lostRate)
            WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0,
                         "wishToalCnts is :%d, realTotalCnts is :%d, currPts is %d, downLayer is %d lostRate is %d, requestLayer is %d, ", _pDown_Layer_Info[0].wishTotalCnts, _pDown_Layer_Info[0].realTotalCnts, _pDown_Layer_Info[0].currPts, _pDown_Layer_Info[0].downLayer,  _pDown_Layer_Info[0].lostRate, _pDown_Layer_Info[0].requestLayer);
            
		if(olen && ret >= 0)//kmm add
		{
			//if (_isH264)
			{
				_width = w;
				_height = ((h == 64)? 60 : h);
			}
			int half_width = (_width + 1) / 2;
			_decodedImage.CreateEmptyFrame(_width, _height,
				_width, half_width, half_width);
			obuf[0] = (char *)_decodedImage.buffer(webrtc::kYPlane);
			obuf[1] = (char *)_decodedImage.buffer(webrtc::kUPlane);
			obuf[2] = (char *)_decodedImage.buffer(webrtc::kVPlane);

			//if (olen == _decodedImage.width()*_decodedImage.height()*3/2)
			{
                memcpy_neon(obuf[0], tmpo[0], _decodedImage.allocated_size(kYPlane));
                memcpy_neon(obuf[1], tmpo[1], _decodedImage.allocated_size(kUPlane));
                memcpy_neon(obuf[2], tmpo[2], _decodedImage.allocated_size(kVPlane));

				/*memcpy(obuf[0], tmpo[0], _decodedImage.allocated_size(kYPlane));
				memcpy(obuf[1], tmpo[1], _decodedImage.allocated_size(kUPlane));
				memcpy(obuf[2], tmpo[2], _decodedImage.allocated_size(kVPlane));*/

			}
			/*else//分层补偿
			{
				int decW = _decodedImage.width();
				int decH = _decodedImage.height();
				int currentW = w;
				int currentH = ((h == 64)? 60 : h);
               
                _spatialResampler->SetTargetFrameSize(decW, decH);
                
                I420VideoFrame tmp;
                tmp.CreateFrame(currentW*currentH,(uint8_t*)tmpo[0],
                                currentW*currentH/4,(uint8_t*)tmpo[1],
                                currentW*currentH/4,(uint8_t*)tmpo[2],
                                currentW,currentH,
                                currentW,currentW/2,currentW/2);

                //_spatialResampler->SetInputFrameResampleMode(kBox);
                _spatialResampler->SetInputFrameResampleMode(kBiLinear);
                _spatialResampler->ApplyResample(currentW, currentH);
                _spatialResampler->ResampleFrame(tmp, &_decodedImage);

		
				WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0,
					"Resample resolution: %d x %d --> %d x %d", currentW ,currentH, decW, decH);
			}*/
		}
		else
		{
			return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
		}

	} 
	return WEBRTC_VIDEO_CODEC_OK;
}

int I420Decoder_14::InitDecode(/*const*/ VideoCodec* codecSettings,
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

int I420Decoder_14::Decode(const EncodedImage& inputImage,
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

int I420Decoder_14::RegisterDecodeCompleteCallback(DecodedImageCallback* callback) 
{
	_decodeCompleteCallback = callback;
	return WEBRTC_VIDEO_CODEC_OK;
}

int I420Decoder_14::Release() 
{
	_inited = false;

	if(_pDecHandle)
	{
		GVECodecClose(_pDecHandle);
		_pDecHandle = NULL;
	}

	return WEBRTC_VIDEO_CODEC_OK;
}
}
