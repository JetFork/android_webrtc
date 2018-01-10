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
//#include "modules/video_coding/codecs/CxImage/header/ximage.h"

#include <string.h>
#include "math.h"

#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "../../system_wrappers/interface/trace.h"

#define WEBRTC_TEST
#define LAYERNUM		4//3
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
#ifndef GXH_TEST_H264

//#define VCM_I420_PAYLOAD_TYPE      124
//#define VCM_H264_PAYLOAD_TYPE		96
//#define VCM_SPR_PAYLOAD_TYPE		35
//#define VCM_SVC_PAYLOAD_TYPE		36//124//36

#ifdef _WIN32
#include < windows.h >
#include "Timer.h"
CMyTimer ustm = {0};
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif//_WIN32

static	WebRtc_Word32			_com_width = 320;
static	WebRtc_Word32			_com_height = 240;

#endif

/*
__int64 CompareFileTime ( FILETIME time1, FILETIME time2 )//转换为整数时间
{
	__int64 a = time1.dwHighDateTime << 32 | time1.dwLowDateTime ;
	__int64 b = time2.dwHighDateTime << 32 | time2.dwLowDateTime ;

	return   (b - a);
}
*/
namespace webrtc
{

I420Encoder::I420Encoder():
_inited(false),
_encodedImage(),
_encodedCompleteCallback(NULL),
//add by zj
_network()
//add end
{
#ifndef GXH_TEST_H264
	int i;
    _pEncHandle = NULL;
    _pRtPacket = NULL;
	_maxImgBufCnt = 0;
	_inImgBufIdx = 0;
	_outImgBufIdx = 0;
	_lastOutImgBufIdx = -1;
	_lastTime = 0;
	_frameRate = 30;
	for (i = 0; i < MAX_IMG_BUF; i++)
	{
		_encImg[i]._buffer = NULL;
		_encImg[i]._timeStamp = 0;
		_encImg[i].capture_time_ms_ = 0;
		_encImg[i]._encodedHeight = 0;//_com_height;
		_encImg[i]._encodedWidth = 0;//_com_width;
		//_inImageBuf[i] = NULL;
	}
	_inEncBuf = NULL;
#endif
}

I420Encoder::~I420Encoder() {
  _inited = false;
  if (_encodedImage._buffer != NULL) {
    delete [] _encodedImage._buffer;
    _encodedImage._buffer = NULL;
  }
#ifndef GXH_TEST
 
  delete _pSvc_event;
  delete _pSvc_critsect;
  delete _pSvc_thread;
  _pSvc_event = NULL;
  _pSvc_critsect = NULL;
  _pSvc_thread = NULL;
#endif
}

int I420Encoder::Release() {
  // Should allocate an encoded frame and then release it here, for that we
  // actually need an init flag.
 SVCStopThread();
	//_pSvc_thread->Stop();
  if (_encodedImage._buffer != NULL) {
    delete [] _encodedImage._buffer;
    _encodedImage._buffer = NULL;
  }
#ifndef	GXH_TEST_H264
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
#endif
  _inited = false;
  return WEBRTC_VIDEO_CODEC_OK;
}
    
#ifndef GXH_TEST_H264
WebRtc_Word32 I420Encoder::InitSVCEnc(const webrtc::VideoCodec* inst)
{
	int i = 0;
	videoInParams EncParams ;
	memset(&EncParams,0,sizeof(videoInParams));
	EncParams.bframenum = 3;
#ifdef WEBRTC_TESTk
	//EncParams.bitsrate = inst->maxBitrate ? inst->maxBitrate >> 2 : 384;//for webrtc
	EncParams.bitsrate = inst->maxBitrate ? inst->maxBitrate: 384;//需要增加异常值保护
	EncParams.bitsrate = EncParams.bitsrate > 384 ? 384 : EncParams.bitsrate;//!!!
	EncParams.redundancy = 0;//12;
	EncParams.bitsrate = inst->startBitrate;
#else
	EncParams.redundancy = 0;
	EncParams.bitsrate = inst->Bitrates[0]+inst->Bitrates[1]+inst->Bitrates[2]+MINLAYERBITRATE;
	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"Current encoded bitrate is : %d\n",EncParams.bitsrate);

#endif
	EncParams.codec = 0;
	EncParams.depth = 12;
	_frameRate = EncParams.framerate = inst->maxFramerate;
	EncParams.gop = 32;//inst->maxFramerate;// << 1;//32;
	EncParams.width = inst->widthAlloc;
    EncParams.height = inst->heightAlloc;
	EncParams.width_used = inst->width;
	EncParams.height_used = inst->height;
	if(EncParams.width_used  > 320)
	{
        _network.encodeLayer = 0xF;
	}
	else if(EncParams.width_used  > 160)
	{
        _network.encodeLayer = 0xE;
	}
	else 
	{
        _network.encodeLayer = 0xC;
	}
	//add by zj@2013-06-25
	_network.nInf.minBandWidth = inst->minBitrate;//200//kmm mod
	_network.nInf.maxBandWidth = inst->maxBitrate;//512

	EncParams.bitrateLayer[0] = inst->Bitrates[0];
	EncParams.bitrateLayer[1] = inst->Bitrates[1];
	EncParams.bitrateLayer[2] = inst->Bitrates[2];
	EncParams.bitrateLayer[3] = MINLAYERBITRATE;
	for (i=0; i<4; i++)
	{
		EncParams.qp[i][0] = inst->qp[i][0];
		EncParams.qp[i][1] = inst->qp[i][1];
	}
	
	
	_network.bitrateLayer[0] = EncParams.bitrateLayer[0];
	_network.bitrateLayer[1] = EncParams.bitrateLayer[1];
	_network.bitrateLayer[2] = EncParams.bitrateLayer[2];
	_network.bitrateLayer[3] = EncParams.bitrateLayer[3];
	_network.lostPktPolicy1 = inst->LostPktPolicy[0];
	_network.lostPktPolicy2 = inst->LostPktPolicy[1];
	
	//_network.frameRate = inst->maxFramerate;
	_network.gopSize = EncParams.gop + 1;
	//add end
	//
	_com_width = inst->width;
	_com_height = inst->height;
	//
	if(inst->enclayer!=LAYERNUM)
	EncParams.layer =LAYERNUM;//4;//1;
	else
	EncParams.layer=inst->enclayer;
	EncParams.psnr = 0;
	EncParams.rf_constant = 28;
	EncParams.qpmax = 40;//50;//40;//inst->qpMax;
	EncParams.qpmin = 6;
	//EncParams.redundancy = 12;
	//
	EncParams.mtu_size = H264MTU;
	EncParams.mult_slice = 1;
	EncParams.ml_err_resilience = 1;
	EncParams.payload_type = inst->plType;//VCM_H264_PAYLOAD_TYPE;//VCM_I420_PAYLOAD_TYPE;//VCM_SVC_PAYLOAD_TYPE;//96;//124;
	EncParams.me_range = 32;
	EncParams.sliced_threads = 0;//1;
	EncParams.gen_threads = 1;//0
	EncParams.rc_method = 2;//0;//2
	EncParams.subpel_refine = 5;
	EncParams.me_method = 1;//
	EncParams.disable_cabac = 0;
	if(EncParams.payload_type == 96)
	{
		EncParams.codec |= 2;
		EncParams.width = inst->width;
		EncParams.height = inst->height;
		EncParams.width_used = inst->width;
		EncParams.height_used = inst->height;
		if(inst->h264SubType <0 ||inst->h264SubType>=2)
			return -1;
		
		if(inst->h264SubType ==0)
		{
		EncParams.disable_cabac =1;
		EncParams.bframenum =0;
		}
		else if(inst->h264SubType ==1)
		{
			EncParams.disable_cabac =0;
			EncParams.bframenum =3;
		}
	}

	if(EncParams.bitsrate <= 384)
	{
		EncParams.qpmin = 32 - 2;
	}
	else if(EncParams.bitsrate <= 512)
	{
		EncParams.qpmin = 30 - 2;
	}
	else if(EncParams.bitsrate <= 768)
	{
		EncParams.qpmin = 28 - 2;
	}
	else
	{
		EncParams.qpmin = 26 - 2;
	}

	_network.minQP = EncParams.qpmin;
	_network.maxQP = EncParams.qpmax;
	_network.bitrate = EncParams.bitsrate;

	_pEncHandle = GVECodecInit(&EncParams);
	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"Current encoded resolution: %d x %d, payload type:%d,framerate:%d,bitrate:%d",EncParams.width_used,EncParams.height_used,EncParams.payload_type,EncParams.framerate,EncParams.bitsrate);

#if 0

	char *filename0 = "d:\\x264_enc_0.264";
	char *filename1 = "d:\\x264_enc_1.264";
	char *filename2 = "d:\\x264_enc_2.264";
	char *filename3 = "d:\\x264_enc_3.264";
	FILE *pf0 = fopen(filename0, "wb"); 
	FILE *pf1 = fopen(filename1, "wb"); 
	FILE *pf2 = fopen(filename2, "wb"); 
	FILE *pf3 = fopen(filename3, "wb"); 
	GVESetDebugFile(_pEncHandle,(void*)pf0,filename0,0,1,0);
	GVESetDebugFile(_pEncHandle,(void*)pf1,filename1,1,2,0);
	GVESetDebugFile(_pEncHandle,(void*)pf2,filename2,2,3,0);
	GVESetDebugFile(_pEncHandle,(void*)pf3,filename3,3,4,0);



#endif
	if(_pEncHandle == NULL)
		return WEBRTC_VIDEO_CODEC_ERROR;
	if(_pRtPacket == NULL)
		_pRtPacket = (RTPacket *)GVECodecGetRtpPoint(_pEncHandle);
	//if(_pBitsBuf == NULL)
	//	_pBitsBuf = new char [MAXRTPSIZE]  
	_width_used = EncParams.width_used;
	_height_used = EncParams.height_used;
	_maxImgBufCnt =4;// EncParams.bframenum + 1;
	_width = EncParams.width;
	_height = EncParams.height;
	int size = (_width * _height * 3) >> 1;
	for (i = 0; i < _maxImgBufCnt; i++)
	{
		if(_encImg[i]._buffer == NULL)
			_encImg[i]._buffer = new uint8_t[size];
	}
	if(_inEncBuf == NULL)
		_inEncBuf = new uint8_t[size];
#if 1
	//trace_impl.cc
	//_screenUpdateThread = ThreadWrapper::CreateThread(ScreenUpdateThreadProc,this, kRealtimePriority);                                                 
	//_screenUpdateEvent = EventWrapper::Create();
	//
	
	_pSvc_thread = ThreadWrapper::CreateThread(SVCThreadProc, this,kNormalPriority, "SVC");//kNormalPriority//kHighestPriority
    _pSvc_event = EventWrapper::Create();
    _pSvc_critsect = CriticalSectionWrapper::CreateCriticalSection();

	

	  //
	unsigned int tid = 0;
	_pSvc_thread->Start(tid);
	 

  /*for (int m = 0; m < WEBRTC_TRACE_NUM_ARRAY; ++m) {
    for (int n = 0; n < WEBRTC_TRACE_MAX_QUEUE; ++n) {
      message_queue_[m][n] = new
      char[WEBRTC_TRACE_MAX_MESSAGE_SIZE];
    }
  }*/
#endif
    return WEBRTC_VIDEO_CODEC_OK;
}

    WebRtc_Word32
    I420Encoder::GetEncodedPartitions() 
    {
        int part_idx = 0;
        _encodedImage._length = 0;
        webrtc::RTPFragmentationHeader frag_info;
        //webrtc::CodecSpecificInfo codecSpecific;
        RTPacket *rtPacket = _pRtPacket;
        int i = 0;
        int count = rtPacket->count;
        
        if(rtPacket->rtpLen > 0)
        {
            frag_info.VerifyAndAllocateFragmentationHeader(count);
            
        }
//add by zj@2013-06-21
		if(96 == rtPacket->payLoadType)
		{
			for (i = 0; i < count; i++)
			{
				if(rtPacket->size[i] > 0)
				{				  
					memcpy(&_encodedImage._buffer[_encodedImage._length],
						   &rtPacket->buf[_encodedImage._length],
						   rtPacket->size[i]);
					frag_info.fragmentationOffset[part_idx] = _encodedImage._length;
					frag_info.fragmentationLength[part_idx] =  rtPacket->size[i];//packet[i]->size;
					frag_info.fragmentationPlType[part_idx] = 0;  // not known here
					frag_info.fragmentationTimeDiff[part_idx] = 0;
					_encodedImage._length += rtPacket->size[i];//packet[i]->size;
	                
					if(_encodedImage._length > _encodedImage._size)
					{
						break;
					}
					++part_idx;
					rtPacket->size[i] = 0;//test
				}
			}	
		}
		else
		{
			int loseFilter = 0;
			int layer;
			//int adjustType;
			networkInf *nInf;
			int send_length = 0;
			layer = _network.layer;
			nInf = &_network.nInf;
			//adjustType = nInf->adjustType;
			
			for (i = 0; i < count; i++)
			{
				if(rtPacket->size[i] > 0)
				{		
					//int losePacket = 0;
					loseFilter = _network.UpThrowLayerFilter(layer, &rtPacket->buf[_encodedImage._length]);
					if (loseFilter == 0)
					{
						
						memcpy(&_encodedImage._buffer[send_length],
							&rtPacket->buf[_encodedImage._length],
							rtPacket->size[i]);
						
						frag_info.fragmentationOffset[part_idx] = send_length;
						frag_info.fragmentationLength[part_idx] =  rtPacket->size[i];//packet[i]->size;
						frag_info.fragmentationPlType[part_idx] = 0;  // not known here
						frag_info.fragmentationTimeDiff[part_idx] = 0;
						send_length += rtPacket->size[i];//packet[i]->size;
						++part_idx;
					}
					 _encodedImage._length += rtPacket->size[i];//packet[i]->size;
					if(_encodedImage._length > _encodedImage._size)
					{
						break;
					}
					rtPacket->size[i] = 0;//test
				}
			}
	/*		_network.gopIndex++;
			_network.gopIndex = _network.gopIndex > _network.gopSize ? 1 : _network.gopIndex;*/
			frag_info.fragmentationVectorSize = part_idx;
		}
//add end
        if (_encodedImage._length > 0) 
        { 
#ifdef GXH_TEST_H264
			encoded_image_._timeStamp = input_image.timestamp();
			encoded_image_.capture_time_ms_ = input_image.render_time_ms();
			encoded_image_._encodedHeight = raw_->h;
			encoded_image_._encodedWidth = raw_->w;
    
#endif
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
	int time_limit = 1000 / _frameRate;//40;///1;///33;//40;///1;//400;
	int flag = 0;
	int size = _width_used * _height_used;
	WebRtc_Word64 curTime = TickTime::MillisecondTimestamp();
	WebRtc_Word64 curTime1 = 0;//TickTime::MillisecondTimestamp();
	WebRtc_Word64 diffTime = (curTime - _lastTime);// / 1000;
	//GS
	//if(_lastTime && diffTime > 100)
	//	printf("diffTime0 = %d ms\n",(int)diffTime);
	_lastTime = curTime;
    _pSvc_event->Wait(time_limit);
	//
    if (!_pSvc_thread)
    {
        //stop the thread
        return false;
    }
	if(_outImgBufIdx != _lastOutImgBufIdx)
	{
		//CriticalSectionScoped cs(_pSvc_critsect);
		_pSvc_critsect->Enter();
		flag = abs(_outImgBufIdx - _inImgBufIdx) >= 1;
		if(flag)// && (_outImgBufIdx != _lastOutImgBufIdx))
		{
			memcpy(_inEncBuf,_encImg[_outImgBufIdx]._buffer,(size * 3) >> 1);
			_encodedImage._frameType = kKeyFrame; // No coding.
		    _encodedImage._timeStamp = _encImg[_outImgBufIdx]._timeStamp;
		    _encodedImage.capture_time_ms_ = _encImg[_outImgBufIdx].capture_time_ms_;
		    _encodedImage._encodedHeight = _encImg[_outImgBufIdx]._encodedHeight;
		    _encodedImage._encodedWidth = _encImg[_outImgBufIdx]._encodedWidth;
			_lastOutImgBufIdx = _outImgBufIdx;
			//printf("svc_thread_cnt = %d \n",svc_thread_cnt);
			//svc_thread_cnt++;
		}
		_pSvc_critsect->Leave();
		//CriticalSectionScoped cs(_pSvc_critsect);
		if(flag)
		{
			char *outImgBuf[3];
			outImgBuf[0] = (char *)_inEncBuf;//_inImageBuf[_outImgBufIdx];
			outImgBuf[1] = &outImgBuf[0][size];
			outImgBuf[2] = &outImgBuf[1][size >> 2];
			//GS
			SVCEnc(outImgBuf);
			_outImgBufIdx++;
			_outImgBufIdx = _outImgBufIdx >= _maxImgBufCnt ? 0 : _outImgBufIdx;
			//E
			//DisplayInfo0
			//printf("svc_thread_cnt = %d \n",svc_thread_cnt);
			svc_thread_cnt++;
			//_pSvc_event->Set();
			curTime1 = TickTime::MillisecondTimestamp();
			diffTime = (curTime1 - curTime);
			//if(diffTime > 100)
			//	printf("diffTime1 = %d ms*******************\n",(int)diffTime);
		}
	}
	//E
	//DisplayInfo0
	
    return true;
}

//added by zj@2013-06-25
void I420Encoder::SetNetworkInf(int pushBW, int sendBW, int totalSendBW, int uAvailableBW)
{
	_network.nInf.pushBW = pushBW;
	_network.nInf.sendBW = sendBW;
	_network.nInf.totalPushBW = uAvailableBW;
	_network.nInf.totalSendBW = totalSendBW;
	_network.nInf.isChangeBW = TRUE;
	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"Current SetNetworkInf pushBW: %d, sendBW: %d, uAvailableBW:%d, totalSendBW:%d",pushBW, sendBW,uAvailableBW, totalSendBW);
	
}
void I420Encoder::SetGenNetworkInf(int LossRate, int sendBW, int delay, int jitter, int recvAvgDelay, int recvMaxDelay, int recvAvgJitter, int recvMaxJitter, int adjustBW, int recvAvgLostCount)
{
	_network.nInf.sendDelay = delay;
	_network.nInf.recvAvgDelay = recvAvgDelay;
	_network.nInf.recvMaxDelay = recvMaxDelay;
	_network._rttFilterSendDelay.Update(delay);
	_network._rttFilterRecvAvgDelay.Update(recvAvgDelay);
	_network._rttFilterRecvMaxDelay.Update(recvMaxDelay);
	_network.nInf.lastSendDelay = _network._rttFilterSendDelay.GetAvgRtt();
	_network.nInf.lastRecvAvgDelay = _network._rttFilterRecvAvgDelay.GetAvgRtt();
	_network.nInf.genBandWidth = 512;// _network.GenNetworkBandwidth(&_network.nInf);
	/*WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
		"Current SetGenNetworkInf delay: %d, avgDelay: %d, LossRate:%d, recvAvgDelay:%d, lastrecvAvgDelay:%d, genBandWidth: %d",delay, _network.nInf.lastSendDelay, LossRate, recvAvgDelay,_network.nInf.lastRecvAvgDelay,_network.nInf.genBandWidth);*/

	//_network.nInf.lastRecvMaxDelay = _network._rttFilterRecvMaxDelay.GetAvgRtt();
	//_network.Update(&_network.nInf);
}
void I420Encoder::SetPeopleNumber(int number)
{
    _network.nInf.peopleNumber = number;
}
void I420Encoder::SetVideoMode(int mode)
{
	_network.lastMode = _network.mode;
    _network.mode = mode;
}

bool I420Encoder::SVCStopThread() {
  // Release the worker thread so that it can flush any lingering messages.
  ///_pSvc_event->Set();

  // Allow 10 ms for pending messages to be flushed out.
  // TODO(hellner): why not use condition variables to do this? Or let the
  //                worker thread die and let this thread flush remaining
  //                messages?
  ///SleepMs(10);

  _pSvc_thread->SetNotAlive();
  // Make sure the thread finishes as quickly as possible (instead of having
  // to wait for the timeout).
 // _pSvc_event->Set();
  bool stopped = _pSvc_thread->Stop();

  return stopped;
}
WebRtc_Word32 
I420Encoder::ImgCopy(const webrtc::I420VideoFrame& inputImage)
{
  int size =  inputImage.width() * inputImage.height();//_encodedImage._encodedWidth * _encodedImage._encodedHeight;
  char *inY = (char *)inputImage.buffer(webrtc::kYPlane);
  char *inU = (char *)inputImage.buffer(webrtc::kUPlane);
  char *inV = (char *)inputImage.buffer(webrtc::kVPlane);
  char *outImgBuf[3];
  outImgBuf[0] = (char *)_encImg[_inImgBufIdx]._buffer;
  outImgBuf[1] = &outImgBuf[0][size];
  outImgBuf[2] = &outImgBuf[1][size >> 2];
  memcpy(outImgBuf[0],inY,size);
  memcpy(outImgBuf[1],inU,size >> 2);
  memcpy(outImgBuf[2],inV,size >> 2);

  _encImg[_inImgBufIdx]._timeStamp = inputImage.timestamp();
  _encImg[_inImgBufIdx].capture_time_ms_ = inputImage.render_time_ms();
  _encImg[_inImgBufIdx]._encodedHeight = inputImage.height();
  _encImg[_inImgBufIdx]._encodedWidth = inputImage.width();
  
  //_inImgBufIdx++;
  //_inImgBufIdx = _inImgBufIdx > _maxImgBufCnt ? 0 : _inImgBufIdx;
	return  WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}

//FILE *f1[3]={NULL, NULL, NULL};
WebRtc_Word32
I420Encoder::SVCEnc(char *apSrcData[3])//(const webrtc::I420VideoFrame& inputImage)
{
#if 0
	BOOL res ;
	float cpu = 0.0;
	//FILETIME idleTime;
	//FILETIME kernelTime;
	//FILETIME userTime;
	int idle = 0;
	int kernel = 0; 
	int user = 0;


	res = GetSystemTimes( &idleTime, &kernelTime, &userTime );
	idle = CompareFileTime( _network.idleTime, idleTime);
	kernel = CompareFileTime( _network.kernelTime, kernelTime);
	user = CompareFileTime(_network.userTime, userTime);

	if(((kernel+user) != 0) && ((kernel + user) > idle))
	{
        cpu = (kernel + user - idle) *100.0/(kernel+user);
	}

	if (cpu >= 80.0)
	{
		WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
			"Current CPU: %f%", cpu);
	}

	_network.idleTime = idleTime;
	_network.kernelTime = kernelTime;
	_network.userTime = userTime ;

	_network.cpuCount++;
	_network.cpuAmount += cpu;
	if(_network.cpuCount%15 == 0)
	{
		cpu = _network.cpuAmount / 15;
		if(cpu > 80)
		{
			_network.encodeLayer = _network.encodeLayer & (_network.encodeLayer  << 1);
		}
		else if (cpu < 50)
		{
		    _network.encodeLayer = _network.encodeLayer | (_network.encodeLayer >> 1);
		}
        _network.cpuCount = 0;
		_network.cpuAmount = 0;
		if (cpu > 80.0)
		{
			WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
				"Average CPU: %f%", cpu);
		}

	}

#endif
	static int svc_enc_cnt = 0;
	//printf("svc_enc_cnt = %d \n",svc_enc_cnt);
	svc_enc_cnt++;
    //if(_pEncHandle && (inputImage.width()) && (inputImage.height()))
	if(_pEncHandle)
    {
		int size = _width_used * _height_used;
        //char *apSrcData[3];///(char *)&inputImage.buffer[0];//m_pImgBuf;//


        char *apDstBuff = (char *)_encodedImage._buffer;//m_pBitsBuf;//?
        int len = size * 3 >> 1;//(inputImage.width() * inputImage.height() * 3) >> 1;
        int aiDstBuffLen = len;
        //memcpy(m_pImgBuf,inputImage._buffer,len);
        int EncKeyFrm = 0;
		//EncKeyFrm = PIC_TYPE_IDR;//PIC_TYPE_KEYFRAME;//PIC_TYPE_I;//
		if(apSrcData[0] == NULL || apSrcData[1] == NULL || apSrcData[2] == NULL)
		{
			apSrcData[0] = (char *)_encImg[_inImgBufIdx]._buffer;
			apSrcData[1] = &apSrcData[0][size];
			apSrcData[2] = &apSrcData[1][size >> 2];
		}
        /*apSrcData[0] = (char *)inputImage.buffer(webrtc::kYPlane);
        apSrcData[1] = (char *)inputImage.buffer(webrtc::kUPlane);
        apSrcData[2] = (char *)inputImage.buffer(webrtc::kVPlane);*/

//add by zj
#if 1
		if (_network.nInf.resetBitrate)
		{
			int layer;
			int i;
			float encBandWidth;
			int bitrateAll = 0;
			int bitrateLayer[4] = {0};
			layer =  4;
			encBandWidth = _network.nInf.encBandWidth;
			for(i = 0; i< layer; i++)
			{
				bitrateAll += _network.bitrateLayer[i];
			}
			for(i = 0; i< layer; i++)
			{
				bitrateLayer[i] = encBandWidth * _network.bitrateLayer[i]/bitrateAll;
			}
			//GVEStreamControl(_pEncHandle, encBandWidth, layer);
			GVEResetBitrate(_pEncHandle, bitrateLayer, layer);
			_network.nInf.resetBitrate = FALSE;
		}
		if(_network.lastMode != _network.mode)
		{
			float k = 0.0;
			int layer = 4;
			int QP, minQP, maxQP;
		    _network.bitrate = _network.nInf.lastAvailableBandWidth;
			if (2 == _network.mode)
			{
				k = 1 / sqrt((640 * 480.0) / (_width * _height));
				QP = 30 + 6 * log(k * (512 * _network.frameRate) / (_network.bitrate * 15.0)) * 1.44269504088896340736;
				minQP = CLIP3((QP - 2), 0, 51);
				maxQP = CLIP3((QP + 2), 0, 51);
			}
			else
			{
                minQP = _network.minQP;
				maxQP = _network.maxQP; 
			}
			GVEResetQP(_pEncHandle, minQP, maxQP, layer);
			_network.lastMode = _network.mode;
		}
#endif
//add end

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

		_encodedImage._length = GVECodecEnc(_pEncHandle,apSrcData,len,apDstBuff,&aiDstBuffLen,&EncKeyFrm);

	  	if (EncKeyFrm == PIC_TYPE_IDR || EncKeyFrm == PIC_TYPE_I || EncKeyFrm == PIC_TYPE_KEYFRAME)
			_encodedImage._frameType = kKeyFrame;
		else
			_encodedImage._frameType = kDeltaFrame;

//add by zj@2013-05-23
#if 1//开启关闭网络反馈
		if(_pRtPacket->payLoadType != 96) 
		{
			if (_network.nInf.isChangeBW)
			{   
				networkInf *nInf;

				int mode = _network.mode;
				nInf = &_network.nInf;
	    	
				//nInf->genBandWidth = _network.GenNetworkBandwidth(nInf);
				nInf->videoBandWidth = _network.UpBandWidth(*nInf);
				if (nInf->videoBandWidth > nInf->lastAvailableBandWidth)
				{
                    nInf->upBandWidth = TRUE;
				}
				else
				{
                    nInf->upBandWidth = FALSE;
				}
				nInf->lastAvailableBandWidth = nInf->videoBandWidth;
				if(nInf->videoBandWidth  < _network.lostPktPolicy2)
				{
	                _network.layer = 12;
				}
				else if(nInf->videoBandWidth  < _network.lostPktPolicy1)
				{
					_network.layer = 14;
				}
				else
				{
                    _network.layer = 15;
				}

				WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
					"Current prepare encode bitrate: %d, layer: %d",nInf->videoBandWidth, _network.layer);

				_network.AdjustBitrate(nInf, mode);
				nInf->lastEncBandWidth = nInf->encBandWidth;
				_network.nInf.isChangeBW = FALSE;
			}
			else
			{
				networkInf *nInf;

				int mode = _network.mode;
				nInf = &_network.nInf;
				if ((1 == EncKeyFrm) && (nInf->upBandWidth))
				{
					nInf->videoBandWidth = _network.HeightenBandWidth(*nInf);
					nInf->lastAvailableBandWidth = nInf->videoBandWidth;
					if(nInf->videoBandWidth  < _network.lostPktPolicy2)
					{
						_network.layer = 12;
					}
					else if(nInf->videoBandWidth  < _network.lostPktPolicy1)
					{
						_network.layer = 14;
					}
					else
					{
						_network.layer = 15;
					}

					WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
						"Current prepare encode bitrate: %d, layer: %d",nInf->videoBandWidth, _network.layer);

					_network.AdjustBitrate(nInf, mode);
					nInf->lastEncBandWidth = nInf->encBandWidth;
				}
			}
		}
#endif
//add end
        if(_encodedImage._length > 0)
            GetEncodedPartitions();
        return WEBRTC_VIDEO_CODEC_OK; 
    }
	
    return  WEBRTC_VIDEO_CODEC_NO_OUTPUT;
}
    
#endif
int I420Encoder::InitEncode(const VideoCodec* codecSettings,
                            int /*numberOfCores*/,
                            uint32_t /*maxPayloadSize */) {
  if (codecSettings == NULL) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (codecSettings->width < 1 || codecSettings->height < 1) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  // Allocating encoded memory.
  if (_encodedImage._buffer != NULL) {
    delete [] _encodedImage._buffer;
    _encodedImage._buffer = NULL;
    _encodedImage._size = 0;
  }
  const uint32_t newSize = CalcBufferSize(kI420,
                                          codecSettings->width,
                                          codecSettings->height);
  uint8_t* newBuffer = new uint8_t[newSize];
  if (newBuffer == NULL) {
    return WEBRTC_VIDEO_CODEC_MEMORY;
  }
  _encodedImage._size = newSize;
  _encodedImage._buffer = newBuffer;
#ifndef GXH_TEST_H264
    InitSVCEnc(codecSettings);
#endif

  // If no memory allocation, no point to init.
  _inited = true;
  return WEBRTC_VIDEO_CODEC_OK;
}



int I420Encoder::Encode(const I420VideoFrame& inputImage,
                        const CodecSpecificInfo* /*codecSpecificInfo*/,
                        const std::vector<VideoFrameType>* /*frame_types*/) {
  if (!_inited) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (_encodedCompleteCallback == NULL) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
#if 0 //gxh_test
  _encodedImage._frameType = kKeyFrame; // No coding.
  _encodedImage._timeStamp = inputImage.timestamp();
  _encodedImage._encodedHeight = inputImage.height();
  _encodedImage._encodedWidth = inputImage.width();
#endif
  int req_length = CalcBufferSize(kI420, inputImage.width(),
                                  inputImage.height());
#if 0//gxh_test
  if (_encodedImage._size > static_cast<unsigned int>(req_length)) {
    // Allocating encoded memory.
    if (_encodedImage._buffer != NULL) {
      delete [] _encodedImage._buffer;
      _encodedImage._buffer = NULL;
      _encodedImage._size = 0;
    }
    uint8_t* newBuffer = new uint8_t[req_length];
    if (newBuffer == NULL) {
      return WEBRTC_VIDEO_CODEC_MEMORY;
    }
    _encodedImage._size = req_length;
    _encodedImage._buffer = newBuffer;
  }
#endif

#ifndef GXH_TEST_H264
  WebRtc_Word32 runflag = WEBRTC_VIDEO_CODEC_OK;//WEBRTC_VIDEO_CODEC_NO_OUTPUT;
  if(_pEncHandle)
  ///if(_pEncHandle && (inputImage.width()) && (inputImage.height()))
  {
	CriticalSectionScoped cs(_pSvc_critsect);
    //_pSvc_critsect->Enter();
	//GS
	ImgCopy(inputImage);
#if 0
	_encodedImage._frameType = kKeyFrame; // No coding.
    _encodedImage._timeStamp = inputImage.timestamp();
	///_encodedImage.capture_time_ms_ = inputImage.render_time_ms();
    _encodedImage._encodedHeight = inputImage.height();
    _encodedImage._encodedWidth = inputImage.width();
	char *apSrcData[3] = {NULL};
	runflag = SVCEnc(apSrcData);//(inputImage);
#endif
	//E
	//DisplayInfo0
	_inImgBufIdx++;
    _inImgBufIdx = _inImgBufIdx >= _maxImgBufCnt ? 0 : _inImgBufIdx;
	//_pSvc_critsect->Leave();
	_pSvc_event->Set();
  }
  else
	  runflag = WEBRTC_VIDEO_CODEC_NO_OUTPUT;

    return runflag;//SVCEnc(inputImage);
#endif
  int ret_length = ExtractBuffer(inputImage, req_length, _encodedImage._buffer);
  if (ret_length < 0)
    return WEBRTC_VIDEO_CODEC_MEMORY;
  _encodedImage._length = ret_length;

  _encodedCompleteCallback->Encoded(_encodedImage);
  return WEBRTC_VIDEO_CODEC_OK;
}


int
I420Encoder::RegisterEncodeCompleteCallback(EncodedImageCallback* callback) {
  _encodedCompleteCallback = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}


I420Decoder::I420Decoder():
_decodedImage(),
_width(0),
_height(0),
_inited(false),
_decodeCompleteCallback(NULL),
_lastEncidx(-1)
{
	memset(_pEncoded_Decoded_Info, 0, sizeof(Encoded_Decoded_Info)*4);
	tmpo[0] = new char[640*480];
	tmpo[1] = new char[640*480/4];
	tmpo[2] = new char[640*480/4];
	_spatialResampler	 = new VPMSimpleSpatialResampler();
#ifndef GXH_TEST_H264
    _pDecHandle = NULL;
#endif
}

I420Decoder::~I420Decoder() {
	delete tmpo[0];
	delete tmpo[1];
	delete tmpo[2];
	delete _spatialResampler;
  Release();
}

int
I420Decoder::Reset() {

// 	for(int i =0;i<4;i++)
// 	if(m_pEncoded_Decoded_Info[i]!=NULL)
// 		free(m_pEncoded_Decoded_Info[i]);
  return WEBRTC_VIDEO_CODEC_OK;
}

//Encoded_Decoded_Info	m_pEncoded_Decoded_Info[4]={0};
//int lastEncidx=-1;

#ifndef GXH_TEST_H264
    int 
    I420Decoder::InitSVCDec(const webrtc::VideoCodec* inst)
    {
        videoInParams DecParams = {0};
		DecParams.encIDX = inst->encIdx;//通过RTP扩展头解析获得
        DecParams.bframenum = 3;
        DecParams.framerate = inst->maxFramerate;
        DecParams.gop = 32;//inst->maxFramerate;// << 1;//32;
#ifdef WEBRTC_TEST	//for webrtc
		DecParams.width_used = inst->width;
        DecParams.height_used = inst->height;
        DecParams.width = normResolutionTab[inst->allocIdx][0];//WIDTH;//_com_width;//inst->width;//
        DecParams.height = normResolutionTab[inst->allocIdx][1];//HEIGHT;//_com_height;//inst->height;//
		//DecParams.width = normResolutionTab[0][inst->allocIdx];//WIDTH;//_com_width;//inst->width;//
        //DecParams.height = normResolutionTab[1][inst->allocIdx];//HEIGHT;//_com_height;//inst->height;//
#else
        DecParams.width = inst->width;//640;//
        DecParams.height = inst->height;//480;//
#endif
        DecParams.layer = LAYERNUM;//4;//5;//1;
        DecParams.codec = 1;
        DecParams.redundancy = 0;//12;
        //
        DecParams.mtu_size = H264MTU;
        DecParams.mult_slice = 1;
        DecParams.ml_err_resilience = 1;
		//DecParams.displayLayer = 0;
		DecParams.payload_type = inst->plType;
		
		if(DecParams.payload_type == 96)
		{
			DecParams.codec |= 2;
			DecParams.width_used = inst->width;
			DecParams.height_used = inst->height;
			DecParams.width = DecParams.width_used;
			DecParams.height = DecParams.height_used;
			if(inst->h264SubType <0 ||inst->h264SubType >=2)
			return -1;
			if(inst->h264SubType ==0||inst->h264SubType==1)
				   DecParams.bframenum = 0;
		}
// 		for(int i=0;i<4;i++)
// 		{
// 			//m_pEncoded_Decoded_Info[i] = (Encoded_Decoded_Info*)malloc(sizeof(Encoded_Decoded_Info));
// 			memset(&m_pEncoded_Decoded_Info[i],0,sizeof(Encoded_Decoded_Info));
//             m_pEncoded_Decoded_Info[i].lostFlag = 0;
//             m_pEncoded_Decoded_Info[i].lastPts = 0;
// 			m_pEncoded_Decoded_Info[i].lostType = 0;
// 			m_pEncoded_Decoded_Info[i].waitIDR = 0;
// 			m_pEncoded_Decoded_Info[i].repeatCnt =0;
// 			m_pEncoded_Decoded_Info[i].startDecode = 0;
// 		}
       // if(_pDecHandle == NULL)
            _pDecHandle = GVECodecInit(&DecParams);
			WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCoding, 0,
				"Current decoded resolution: %d x %d, payload type:%d",DecParams.width_used,DecParams.height_used,DecParams.payload_type);

#if 0
			char *filename0 = "d:\\x264_dec_0.264";
			char *filename1 = "d:\\x264_dec_1.264";
			char *filename2 = "d:\\x264_dec_2.264";
			char *filename3 = "d:\\x264_dec_3.264";
		FILE *pf0 = fopen(filename0, "wb"); 
		FILE *pf1 = fopen(filename1, "wb"); 
		FILE *pf2 = fopen(filename2, "wb"); 
		FILE *pf3 = fopen(filename3, "wb"); 
		GVESetDebugFile(_pDecHandle,(void*)pf0,filename0,0,1,1);
		GVESetDebugFile(_pDecHandle,(void*)pf1,filename1,1,2,1);
		GVESetDebugFile(_pDecHandle,(void*)pf2,filename2,2,3,1);
		GVESetDebugFile(_pDecHandle,(void*)pf3,filename3,3,4,1);
#endif
		if(_pDecHandle == NULL)
			return -1;
		else
			return 0;
    
    }

WebRtc_Word32
    I420Decoder::SVCDec(const EncodedImage& inputImage)
    {
        int ret = 0;
        if(_pDecHandle && inputImage._length)
        {
            char *obuf[3];// = (char *)_decodedImage.buffer(webrtc::kYPlane);//NULL;//
            char *inbuf_ptr = (char *)inputImage._buffer;//buffer;
            int size = inputImage._length;
            int olen = 0;//1
            //int len = 0;
            //static int testdecodenum = 0;
            int DecKeyFrm = 0;
            obuf[0] = (char *)_decodedImage.buffer(webrtc::kYPlane);
            obuf[1] = (char *)_decodedImage.buffer(webrtc::kUPlane);
            obuf[2] = (char *)_decodedImage.buffer(webrtc::kVPlane);

			if (inputImage._packetSize <= 0 || inputImage._count <=0 || size<=0)
			{
				return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
			}      
            ret = GVECodecDec(_pDecHandle,inbuf_ptr, size, tmpo, &olen, (unsigned short *)inputImage._packetSize, inputImage._count,_pEncoded_Decoded_Info, &_lastEncidx);
            //ret = GVECodecDec(_pDecHandle,inbuf_ptr, size, obuf, &olen, NULL, 0, &DecKeyFrm);
            if(olen && ret >= 0)//kmm add
            {
				if (olen == _decodedImage.width()*_decodedImage.height()*3/2 && ret >= 0)
				{
					memcpy(obuf[0], tmpo[0], _decodedImage.allocated_size(kYPlane));
					memcpy(obuf[1], tmpo[1], _decodedImage.allocated_size(kUPlane));
					memcpy(obuf[2], tmpo[2], _decodedImage.allocated_size(kVPlane));
				}
				else//分层补偿
				{
					int currentW = 0;
					int currentH = 0;
					int decW = _decodedImage.width();
					int decH = _decodedImage.height();
					if (olen == 320*240*3/2)
					{
						currentW = 320;
						currentH = 240;
					}
					else if (olen == 160*120*3/2)
					{
						currentW = 160;
						currentH = 120;
					}
					else if (olen == 80*64*3/2)
					{
						currentW = 80;
						currentH = 60;
					}
					else
					{
						return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
					}

					_spatialResampler->SetTargetFrameSize(decW, decH);	

					I420VideoFrame tmp;
					tmp.CreateFrame(currentW*currentH,(uint8_t*)tmpo[0],
						currentW*currentH/4,(uint8_t*)tmpo[1],
						currentW*currentH/4,(uint8_t*)tmpo[2],
						currentW,currentH,
						currentW,currentW/2,currentW/2);
#if 1
					_spatialResampler->SetInputFrameResampleMode(kBox);
					_spatialResampler->ApplyResample(currentW, currentH);
					_spatialResampler->ResampleFrame(tmp, &_decodedImage);
#else

					unsigned char *dst_frame=new unsigned char[currentW*currentH*3];
					ConvertFromI420(tmp, kRGB24, 0, dst_frame);
 	
 					CxImage image;
 					image.CreateFromArray(dst_frame, currentW, currentH, 24, currentW*3, false);
					delete dst_frame;
 
 					CxImage bigImg;
 					//image.Resample2(decW, decH, CxImage::IM_BILINEAR, CxImage::OM_REPEAT, &bigImg);
					image.Resample2(decW, decH, CxImage::IM_BICUBIC2, CxImage::OM_REPEAT, &bigImg);
					//image.Resample2(decW, decH, CxImage::IM_BSPLINE, CxImage::OM_REPEAT, &bigImg);
 
 					ConvertToI420(kRGB24, bigImg.GetBits(0), 0, 0, decW,decH, 0, kRotateNone, &_decodedImage);
#endif
					WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0,
						"Resample resolution: %d x %d --> %d x %d", currentW ,currentH, decW, decH);

				}
            }
            else
			{
				return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
			}

        } 
        return WEBRTC_VIDEO_CODEC_OK;//WEBRTC_VIDEO_CODEC_NO_OUTPUT;
    }
#endif

int
I420Decoder::InitDecode(const VideoCodec* codecSettings,
                        int /*numberOfCores */) {
  if (codecSettings == NULL) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  } else if (codecSettings->width < 1 || codecSettings->height < 1) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  _width = codecSettings->width;
  _height = codecSettings->height;

///#ifndef GXH_TEST_H264
   if(-1== InitSVCDec(codecSettings))
   {
	   WEBRTC_TRACE(
		   kTraceError,
		   kTraceVideoCoding,
		   -1,
		   "InitSVCDec is failed!!!"); 
	   return -1;
   }
///#endi
  _inited = true;
  WEBRTC_TRACE(
	  kTraceInfo,
	  kTraceVideoCoding,
	  -1,
	  "InitSVCDec is successful"); 
  return WEBRTC_VIDEO_CODEC_OK;
}

int
I420Decoder::Decode(const EncodedImage& inputImage,
                    bool /*missingFrames*/,
                    const RTPFragmentationHeader* /*fragmentation*/,
                    const CodecSpecificInfo* /*codecSpecificInfo*/,
                    int64_t /*renderTimeMs*/) {
  if (inputImage._buffer == NULL) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
  if (_decodeCompleteCallback == NULL) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
  if (inputImage._length <= 0) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
#ifdef GXH_TEST_H264
  if (inputImage._completeFrame == false) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }
#endif
  if (!_inited) {
   return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }
#ifdef GXH_TEST_H264
  // Verify that the available length is sufficient:
  int req_length = CalcBufferSize(kI420, _width, _height);
  if (req_length > static_cast<int>(inputImage._length)) {
    return WEBRTC_VIDEO_CODEC_ERROR;
  }
#endif
  // Set decoded image parameters.//分辨率切换时是否需要进行处理？
  int half_width = (_width + 1) / 2;
  _decodedImage.CreateEmptyFrame(_width, _height,
                                 _width, half_width, half_width);
#ifndef GXH_TEST_H264
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

  #endif
}

int
I420Decoder::RegisterDecodeCompleteCallback(DecodedImageCallback* callback) {
  _decodeCompleteCallback = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int
I420Decoder::Release() {
  _inited = false;
#ifndef GXH_TEST_H264
	if(_pDecHandle)
	{
		GVECodecClose(_pDecHandle);
		_pDecHandle = NULL;
	}


	//free:_decodedImage;
#endif
  return WEBRTC_VIDEO_CODEC_OK;
}

}
