/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifdef ARCH_ARM//gxh_test
#include "video_capture_avfoundation_info.h"
#else
#include "video_capture_impl.h"
#endif
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "critical_section_wrapper.h"
#include "module_common_types.h"
#include "ref_count.h"
#include "tick_util.h"
#include "trace.h"
#include "video_capture_config.h"

#include <stdlib.h>
#include "android/log.h"

namespace webrtc
{
namespace videocapturemodule
{
VideoCaptureModule* VideoCaptureImpl::Create(
    const WebRtc_Word32 id,
    VideoCaptureExternal*& externalCapture)
{
#ifdef ARCH_ARM
    return NULL;//gxh_test//wuzhong
#else
    RefCountImpl<VideoCaptureImpl>* implementation =
        new RefCountImpl<VideoCaptureImpl>(id);
    externalCapture = implementation;
    return implementation;
#endif
}

const char* VideoCaptureImpl::CurrentDeviceName() const
{
    return _deviceUniqueId;
}

WebRtc_Word32 VideoCaptureImpl::ChangeUniqueId(const WebRtc_Word32 id)
{
    _id = id;
    return 0;
}

// returns the number of milliseconds until the module want a worker thread to call Process
WebRtc_Word32 VideoCaptureImpl::TimeUntilNextProcess()
{
    CriticalSectionScoped cs(&_callBackCs);

    WebRtc_Word32 timeToNormalProcess = kProcessInterval
        - (WebRtc_Word32)((TickTime::Now() - _lastProcessTime).Milliseconds());

    return timeToNormalProcess;
}

// Process any pending tasks such as timeouts
WebRtc_Word32 VideoCaptureImpl::Process()
{
    CriticalSectionScoped cs(&_callBackCs);

    const TickTime now = TickTime::Now();
    _lastProcessTime = TickTime::Now();

    // Handle No picture alarm

    if (_lastProcessFrameCount.Ticks() == _incomingFrameTimes[0].Ticks() &&
        _captureAlarm != Raised)
    {
        if (_noPictureAlarmCallBack && _captureCallBack)
        {
            _captureAlarm = Raised;
            _captureCallBack->OnNoPictureAlarm(_id, _captureAlarm);
        }
    }
    else if (_lastProcessFrameCount.Ticks() != _incomingFrameTimes[0].Ticks() &&
             _captureAlarm != Cleared)
    {
        if (_noPictureAlarmCallBack && _captureCallBack)
        {
            _captureAlarm = Cleared;
            _captureCallBack->OnNoPictureAlarm(_id, _captureAlarm);

        }
    }

    // Handle frame rate callback
    if ((now - _lastFrameRateCallbackTime).Milliseconds()
        > kFrameRateCallbackInterval)
    {
        if (_frameRateCallBack && _captureCallBack)
        {
            const WebRtc_UWord32 frameRate = CalculateFrameRate(now);
            _captureCallBack->OnCaptureFrameRate(_id, frameRate);
        }
        _lastFrameRateCallbackTime = now; // Can be set by EnableFrameRateCallback

    }

    _lastProcessFrameCount = _incomingFrameTimes[0];

    return 0;
}

VideoCaptureImpl::VideoCaptureImpl(const WebRtc_Word32 id)
    : _id(id), _deviceUniqueId(NULL), _apiCs(*CriticalSectionWrapper::CreateCriticalSection()),
      _captureDelay(0), _requestedCapability(),
      _callBackCs(*CriticalSectionWrapper::CreateCriticalSection()),
      _lastProcessTime(TickTime::Now()),
      _lastFrameRateCallbackTime(TickTime::Now()), _frameRateCallBack(false),
      _noPictureAlarmCallBack(false), _captureAlarm(Cleared), _setCaptureDelay(0),
      _dataCallBack(NULL), _captureCallBack(NULL),
      _lastProcessFrameCount(TickTime::Now()), _rotateFrame(kRotateNone),
      last_capture_time_(TickTime::MillisecondTimestamp())
{
    _requestedCapability.width = kDefaultWidth;
    _requestedCapability.height = kDefaultHeight;
    _requestedCapability.maxFPS = 30;
    _requestedCapability.rawType = kVideoI420;
    _requestedCapability.codecType = kVideoCodecUnknown;
    memset(_incomingFrameTimes, 0, sizeof(_incomingFrameTimes));
	m_bCapture4_3 = 0;
    //added by wuzhong
//#ifdef WEBRTC_IOS
    _spatialResampler = new VPMSimpleSpatialResampler();
//#endif
}

VideoCaptureImpl::~VideoCaptureImpl()
{
    DeRegisterCaptureDataCallback();
    DeRegisterCaptureCallback();
    delete &_callBackCs;
    delete &_apiCs;

    if (_deviceUniqueId)
        delete[] _deviceUniqueId;
}
//#ifdef WEBRTC_IOS
void VideoCaptureImpl::SetResampleResolution(int width,int height)
{

    _spatialResampler->SetTargetFrameSize(width, height);
  
}
//#endif
WebRtc_Word32 VideoCaptureImpl::RegisterCaptureDataCallback(
                                        VideoCaptureDataCallback& dataCallBack)
{
    CriticalSectionScoped cs(&_apiCs);
    CriticalSectionScoped cs2(&_callBackCs);
    _dataCallBack = &dataCallBack;

    return 0;
}

WebRtc_Word32 VideoCaptureImpl::DeRegisterCaptureDataCallback()
{
    CriticalSectionScoped cs(&_apiCs);
    CriticalSectionScoped cs2(&_callBackCs);
    _dataCallBack = NULL;
    return 0;
}
WebRtc_Word32 VideoCaptureImpl::RegisterCaptureCallback(VideoCaptureFeedBack& callBack)
{

    CriticalSectionScoped cs(&_apiCs);
    CriticalSectionScoped cs2(&_callBackCs);
    _captureCallBack = &callBack;
    return 0;
}
WebRtc_Word32 VideoCaptureImpl::DeRegisterCaptureCallback()
{

    CriticalSectionScoped cs(&_apiCs);
    CriticalSectionScoped cs2(&_callBackCs);
    _captureCallBack = NULL;
    return 0;

}
WebRtc_Word32 VideoCaptureImpl::SetCaptureDelay(WebRtc_Word32 delayMS)
{
    CriticalSectionScoped cs(&_apiCs);
    _captureDelay = delayMS;
    return 0;
}
WebRtc_Word32 VideoCaptureImpl::CaptureDelay()
{
    CriticalSectionScoped cs(&_apiCs);
    return _setCaptureDelay;
}

WebRtc_Word32 VideoCaptureImpl::DeliverCapturedFrame(I420VideoFrame&
  captureFrame, WebRtc_Word64 capture_time) {
  UpdateFrameCount();  // frame count used for local frame rate callback.

  const bool callOnCaptureDelayChanged = _setCaptureDelay != _captureDelay;
  // Capture delay changed
  if (_setCaptureDelay != _captureDelay) {
      _setCaptureDelay = _captureDelay;
  }

  // Set the capture time
  if (capture_time != 0) {
      captureFrame.set_render_time_ms(capture_time);
  }
  else {
      captureFrame.set_render_time_ms(TickTime::MillisecondTimestamp());
  }

  if (captureFrame.render_time_ms() == last_capture_time_) {
    // We don't allow the same capture time for two frames, drop this one.
    return -1;
  }
  last_capture_time_ = captureFrame.render_time_ms();

  if (_dataCallBack) {
    if (callOnCaptureDelayChanged) {
      _dataCallBack->OnCaptureDelayChanged(_id, _captureDelay);
    }
    _dataCallBack->OnIncomingCapturedFrame(_id, captureFrame);
  }

  return 0;
}

WebRtc_Word32 VideoCaptureImpl::DeliverEncodedCapturedFrame(
    VideoFrame& captureFrame, WebRtc_Word64 capture_time,
    VideoCodecType codecType) {
  UpdateFrameCount();  // frame count used for local frame rate callback.

  const bool callOnCaptureDelayChanged = _setCaptureDelay != _captureDelay;
  // Capture delay changed
  if (_setCaptureDelay != _captureDelay) {
      _setCaptureDelay = _captureDelay;
  }

  // Set the capture time
  if (capture_time != 0) {
     captureFrame.SetRenderTime(capture_time);
  }
  else {
      captureFrame.SetRenderTime(TickTime::MillisecondTimestamp());
  }

  if (captureFrame.RenderTimeMs() == last_capture_time_) {
    // We don't allow the same capture time for two frames, drop this one.
    return -1;
  }
  last_capture_time_ = captureFrame.RenderTimeMs();

  if (_dataCallBack) {
    if (callOnCaptureDelayChanged) {
      _dataCallBack->OnCaptureDelayChanged(_id, _captureDelay);
    }
    _dataCallBack->OnIncomingCapturedEncodedFrame(_id, captureFrame, codecType);
  }

  return 0;
}

WebRtc_Word32 VideoCaptureImpl::IncomingFrame(
    WebRtc_UWord8* videoFrame,
    WebRtc_Word32 videoFrameLength,
    const VideoCaptureCapability& frameInfo,
    WebRtc_Word64 captureTime/*=0*/)
{
     int conversionResult =-1;
 /*   WEBRTC_TRACE(webrtc::kTraceStream, webrtc::kTraceVideoCapture, _id,
               "IncomingFrame width %d, height %d", (int) frameInfo.width,
               (int) frameInfo.height);*/

    TickTime startProcessTime = TickTime::Now();

    CriticalSectionScoped cs(&_callBackCs);

    const WebRtc_Word32 width = frameInfo.width;
    const WebRtc_Word32 height = frameInfo.height;

		__android_log_print(ANDROID_LOG_ERROR, "yyf"," _deviceUniqueId = %s, codectype=%d\n", _deviceUniqueId, frameInfo.codecType);

    VideoCaptureCapability frameInfo1 = frameInfo;
   frameInfo1.codecType = kVideoCodecUnknown;
   frameInfo1.rawType = kVideoNV21;
    if (frameInfo1.codecType == kVideoCodecUnknown)
    {
		__android_log_print(ANDROID_LOG_ERROR, "kmm"," 2933333333333 incoming ");
    	if (frameInfo1.rawType == kVideoMediaFileI420)
		{
		__android_log_print(ANDROID_LOG_ERROR, "kmm"," 2955555555555 incoming ");
			int half_width = (width + 1) >> 1;
			int half_height = (height + 1) >> 1;
			int y_size = width*height;
			int uv_size = half_width * half_height;

			uint8_t *y = videoFrame;
			uint8_t *u = y+y_size;
		//	uint8_t *v = u+uv_size;
			
			int ret = _captureFrame.CreateFrame(y_size, videoFrame, 
				uv_size, u,
				uv_size, y,
				width, height, width, half_width, half_width);
				if (ret < 0)
				{
					WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
						"Failed to allocate kVideoMediaFileI420 frame.");
					return -1;
				}
			DeliverCapturedFrame(_captureFrame, captureTime);
		}
		else
		{
		__android_log_print(ANDROID_LOG_ERROR, "kmm"," 318888888888888888888 ");
        // Not encoded, convert to I420.
        const VideoType commonVideoType =
                  RawVideoTypeToCommonVideoVideoType(frameInfo1.rawType);

        if (frameInfo1.rawType != kVideoMJPEG &&
            CalcBufferSize(commonVideoType, width,
                           abs(height)) != videoFrameLength)
        {
            WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                         "Wrong incoming frame length.");
            return -1;
        }

        int stride_y = width;
        int stride_uv = (width + 1) / 2;
		int target_width = width;
        int target_height = height;


        //WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCapture, 0,
        //               "rotateFrame is %d", _rotateFrame);
        // Rotating resolution when for 90/270 degree rotations.
        if (_rotateFrame == kRotate90 || _rotateFrame == kRotate270)  {
          target_width = abs(height);
          target_height = width;
		if(m_bCapture4_3 == 1)
			 target_height = target_width * 3/4;//_spatialResampler->TargetHeight() / _spatialResampler->TargetWidth();
		else
			target_width = target_height * 9/16;
        }		
		else
		{
			if(m_bCapture4_3 == 1)
			 	target_height  = target_width * 3/4;//_spatialResampler->TargetHeight() / _spatialResampler->TargetWidth();
			else
				target_height = target_width * 9/16;
		}
        // TODO(mikhal): Update correct aligned stride values.
        //Calc16ByteAlignedStride(target_width, &stride_y, &stride_uv);
        // Setting absolute height (in case it was negative).
        // In Windows, the image starts bottom left, instead of top left.
        // Setting a negative source height, inverts the image (within LibYuv).
        //int dstHeight  = target_width * _spatialResampler->TargetHeight() / _spatialResampler->TargetWidth();
        stride_y = target_width;
        stride_uv = (target_width + 1) / 2;
		__android_log_print(ANDROID_LOG_ERROR, "kmm"," incoming width = %d, width = %d\n", target_width, target_height);
        int ret = _captureFrame.CreateEmptyFrame(target_width,
                                                 abs(target_height),
                                                 stride_y,
                                                 stride_uv, stride_uv);
        if (ret < 0)
        {
            WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                       "Failed to allocate I420 frame.");
            return -1;
        }	

		int crop_x, crop_y;	
        if (_rotateFrame == kRotate90 || _rotateFrame == kRotate270)  {
			crop_x = 0;
			crop_y = ( height - target_width + 1) / 2;
		}
		else
		{		
			crop_x = 0;
			crop_y = ( height - target_height + 1) / 2;
		}

		//	__android_log_print(ANDROID_LOG_ERROR, "yyf"," incoming x = %d, y = %d\n", crop_x, crop_y);

        //gxh_img_format
             conversionResult = ConvertToI420(commonVideoType,
                                                   videoFrame,
                                                   crop_x, crop_y,
                                                   width, height,
                                                   videoFrameLength,
                                                   _rotateFrame,
                                                   &_captureFrame);
        

        if (conversionResult < 0)
        {
            WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                       "Failed to convert capture frame from type %d to I420",
                       frameInfo.rawType);
            return -1;
        }
        if(CurrentDeviceName() && strncmp(CurrentDeviceName(), "Camera 1", 8) == 0)
        {
            
            transformed_video_frame_.CreateEmptyFrame(_captureFrame.width(),
                                                      _captureFrame.height(),
                                                      _captureFrame.stride(kYPlane),
                                                      _captureFrame.stride(kUPlane),
                                                      _captureFrame.stride(kVPlane));

                MirrorI420LeftRight(&_captureFrame,
                                    &transformed_video_frame_);
                _captureFrame.SwapFrame(&transformed_video_frame_);
            
        }

		DeliverCapturedFrame(_captureFrame, captureTime);
#if 0
		static FILE * fpConvertedYUV =NULL;
		if(fpConvertedYUV ==NULL)
			//fpConvertedYUV = fopen("c:\\converted.yuv","wb");
			fpConvertedYUV = fopen("/sdcard/converted.yuv","wb");
		 
		 if(fpConvertedYUV && (width == 640)) 
		{
			fwrite(_captureFrame.buffer(kYPlane),1,width*height,fpConvertedYUV);
			fwrite(_captureFrame.buffer(kUPlane),1,width*height/4,fpConvertedYUV);
			fwrite(_captureFrame.buffer(kVPlane),1,width*height/4,fpConvertedYUV);
			fflush(fpConvertedYUV);
		}
#endif    
    }
	}
    else // Encoded format
    {
        if (_capture_encoded_frame.CopyFrame(videoFrameLength, videoFrame) != 0)
        {
            WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                       "Failed to copy captured frame of length %d",
                       static_cast<int>(videoFrameLength));
        }
        DeliverEncodedCapturedFrame(_capture_encoded_frame, captureTime,
                                    frameInfo1.codecType);
    }

    const WebRtc_UWord32 processTime =
        (WebRtc_UWord32)(TickTime::Now() - startProcessTime).Milliseconds();
    if (processTime > 10) // If the process time is too long MJPG will not work well.
    {
        WEBRTC_TRACE(webrtc::kTraceWarning, webrtc::kTraceVideoCapture, _id,
                   "Too long processing time of Incoming frame: %ums",
                   (unsigned int) processTime);
    }

    return 0;
}

WebRtc_Word32 VideoCaptureImpl::IncomingFrameI420(
    const VideoFrameI420& video_frame, WebRtc_Word64 captureTime) {

  CriticalSectionScoped cs(&_callBackCs);
  int size_y = video_frame.height * video_frame.y_pitch;
  int size_u = video_frame.u_pitch * ((video_frame.height + 1) / 2);
  int size_v =  video_frame.v_pitch * ((video_frame.height + 1) / 2);
  // TODO(mikhal): Can we use Swap here? This will do a memcpy.
  int ret = _captureFrame.CreateFrame(size_y, video_frame.y_plane,
                                      size_u, video_frame.u_plane,
                                      size_v, video_frame.v_plane,
                                      video_frame.width, video_frame.height,
                                      video_frame.y_pitch, video_frame.u_pitch,
                                      video_frame.v_pitch);
  if (ret < 0) {
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                 "Failed to create I420VideoFrame");
    return -1;
  }

  DeliverCapturedFrame(_captureFrame, captureTime);

  return 0;
}

WebRtc_Word32 VideoCaptureImpl::SetCaptureRotation(VideoCaptureRotation
                                                   rotation) {
  CriticalSectionScoped cs(&_apiCs);
  CriticalSectionScoped cs2(&_callBackCs);
  switch (rotation){
    case kCameraRotate0:
      _rotateFrame = kRotateNone;
      break;
    case kCameraRotate90:
      _rotateFrame = kRotate90;
      break;
    case kCameraRotate180:
      _rotateFrame = kRotate180;
      break;
    case kCameraRotate270:
      _rotateFrame = kRotate270;
      break;
  }
  //__android_log_print(ANDROID_LOG_ERROR, "yyf","setcaptureRotation _rotateFrame = %d\n", _rotateFrame);

  return 0;
}

WebRtc_Word32 VideoCaptureImpl::EnableFrameRateCallback(const bool enable)
{
    CriticalSectionScoped cs(&_apiCs);
    CriticalSectionScoped cs2(&_callBackCs);
    _frameRateCallBack = enable;
    if (enable)
    {
        _lastFrameRateCallbackTime = TickTime::Now();
    }
    return 0;
}

WebRtc_Word32 VideoCaptureImpl::EnableNoPictureAlarm(const bool enable)
{
    CriticalSectionScoped cs(&_apiCs);
    CriticalSectionScoped cs2(&_callBackCs);
    _noPictureAlarmCallBack = enable;
    return 0;
}

void VideoCaptureImpl::UpdateFrameCount()
{
    if (_incomingFrameTimes[0].MicrosecondTimestamp() == 0)
    {
        // first no shift
    }
    else
    {
        // shift
        for (int i = (kFrameRateCountHistorySize - 2); i >= 0; i--)
        {
            _incomingFrameTimes[i + 1] = _incomingFrameTimes[i];
        }
    }
    _incomingFrameTimes[0] = TickTime::Now();
}

WebRtc_UWord32 VideoCaptureImpl::CalculateFrameRate(const TickTime& now)
{
    WebRtc_Word32 num = 0;
    WebRtc_Word32 nrOfFrames = 0;
    for (num = 1; num < (kFrameRateCountHistorySize - 1); num++)
    {
        if (_incomingFrameTimes[num].Ticks() <= 0
            || (now - _incomingFrameTimes[num]).Milliseconds() > kFrameRateHistoryWindowMs) // don't use data older than 2sec
        {
            break;
        }
        else
        {
            nrOfFrames++;
        }
    }
    if (num > 1)
    {
        WebRtc_Word64 diff = (now - _incomingFrameTimes[num - 1]).Milliseconds();
        if (diff > 0)
        {
            return WebRtc_UWord32((nrOfFrames * 1000.0f / diff) + 0.5f);
        }
    }

    return nrOfFrames;
}

} // namespace videocapturemodule
} // namespace webrtc
