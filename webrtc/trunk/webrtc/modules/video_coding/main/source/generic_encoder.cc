/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "encoded_frame.h"
#include "generic_encoder.h"
#include "media_optimization.h"
#include "../../../../engine_configurations.h"

#include "android/log.h"
#include <sys/stat.h>
#include <sys/time.h> 
#include <unistd.h>


#define  LOG_TAG    "encoder"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)



namespace webrtc {

//#define DEBUG_ENCODER_BIT_STREAM

VCMGenericEncoder::VCMGenericEncoder(VideoEncoder& encoder, bool internalSource /*= false*/)
:
_encoder(encoder),
_codecType(kVideoCodecUnknown),
_VCMencodedFrameCallback(NULL),
_bitRate(0),
_frameRate(0),
_internalSource(internalSource)
{
}


VCMGenericEncoder::~VCMGenericEncoder()
{
}

WebRtc_Word32 VCMGenericEncoder::Release()
{
    _bitRate = 0;
    _frameRate = 0;
    _VCMencodedFrameCallback = NULL;
    return _encoder.Release();
}

WebRtc_Word32
VCMGenericEncoder::InitEncode(const VideoCodec* settings,
                              WebRtc_Word32 numberOfCores,
                              WebRtc_UWord32 maxPayloadSize)
{
    _bitRate = settings->startBitrate;
    _frameRate = settings->maxFramerate;
    _codecType = settings->codecType;
    if (_VCMencodedFrameCallback != NULL)
    {
        _VCMencodedFrameCallback->SetCodecType(_codecType);
    }
    return _encoder.InitEncode(settings, numberOfCores, maxPayloadSize);
}
#ifndef GXH_TEST
WebRtc_Word32
VCMGenericEncoder::Encode(const I420VideoFrame& inputFrame,
                          const CodecSpecificInfo* codecSpecificInfo,
                          const std::vector<FrameType>& frameTypes) {
  std::vector<VideoFrameType> video_frame_types(frameTypes.size(),
                                                kDeltaFrame);
  VCMEncodedFrame::ConvertFrameTypes(frameTypes, &video_frame_types);
    //printf("encoder_________ \n");
    //printf("_encoder = %x \n",_encoder.Encode);
    //return 0;
#ifdef  GXH_TEST
    printf("_encoder = %x \n",&_encoder);
    //printf("_encoder = %x \n",&_encoder.Encode);    
#endif
  WebRtc_Word32 ret = _encoder.Encode(inputFrame, codecSpecificInfo, &video_frame_types);
    //printf("ret = %x \n",ret);
    return ret;
}
#else
    WebRtc_Word32
    VCMGenericEncoder::Encode(const I420VideoFrame& inputFrame,
                              const CodecSpecificInfo* codecSpecificInfo,
                              const std::vector<FrameType>& frameTypes) {
        std::vector<VideoFrameType> video_frame_types(frameTypes.size(),
                                                      kDeltaFrame);
        VCMEncodedFrame::ConvertFrameTypes(frameTypes, &video_frame_types);

        return _encoder.Encode(inputFrame, codecSpecificInfo, &video_frame_types);
    }
#endif

WebRtc_Word32
VCMGenericEncoder::SetChannelParameters(WebRtc_Word32 packetLoss, int rtt)
{
    return _encoder.SetChannelParameters(packetLoss, rtt);
}

WebRtc_Word32
VCMGenericEncoder::SetRates(WebRtc_UWord32 newBitRate, WebRtc_UWord32 frameRate)
{
    WebRtc_Word32 ret = _encoder.SetRates(newBitRate, frameRate);
    if (ret < 0)
    {
        return ret;
    }
    _bitRate = newBitRate;
    _frameRate = frameRate;
    return VCM_OK;
}

WebRtc_Word32
VCMGenericEncoder::CodecConfigParameters(WebRtc_UWord8* buffer, WebRtc_Word32 size)
{
    WebRtc_Word32 ret = _encoder.CodecConfigParameters(buffer, size);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

WebRtc_UWord32 VCMGenericEncoder::BitRate() const
{
    return _bitRate;
}

WebRtc_UWord32 VCMGenericEncoder::FrameRate() const
{
    return _frameRate;
}

WebRtc_Word32
VCMGenericEncoder::SetPeriodicKeyFrames(bool enable)
{
    return _encoder.SetPeriodicKeyFrames(enable);
}

WebRtc_Word32 VCMGenericEncoder::RequestFrame(
    const std::vector<FrameType>& frame_types) {
  I420VideoFrame image;
  std::vector<VideoFrameType> video_frame_types(frame_types.size(),
                                                kDeltaFrame);
  VCMEncodedFrame::ConvertFrameTypes(frame_types, &video_frame_types);
  return _encoder.Encode(image, NULL, &video_frame_types);
}

WebRtc_Word32
VCMGenericEncoder::RegisterEncodeCallback(VCMEncodedFrameCallback* VCMencodedFrameCallback)
{
   _VCMencodedFrameCallback = VCMencodedFrameCallback;

   _VCMencodedFrameCallback->SetCodecType(_codecType);
   _VCMencodedFrameCallback->SetInternalSource(_internalSource);
   return _encoder.RegisterEncodeCompleteCallback(_VCMencodedFrameCallback);
}

bool
VCMGenericEncoder::InternalSource() const
{
    return _internalSource;
}


 /***************************
  * Callback Implementation
  ***************************/
VCMEncodedFrameCallback::VCMEncodedFrameCallback():
_sendCallback(),
_mediaOpt(NULL),
_encodedBytes(0),
_payloadType(0),
_codecType(kVideoCodecUnknown),
_internalSource(false)
#ifdef DEBUG_ENCODER_BIT_STREAM
, _bitStreamAfterEncoder(NULL)
#endif
{
#ifdef DEBUG_ENCODER_BIT_STREAM
    _bitStreamAfterEncoder = fopen("encoderBitStream.bit", "wb");
#endif
 lastTime = 0;
	frameCount = 0;

}

VCMEncodedFrameCallback::~VCMEncodedFrameCallback()
{
#ifdef DEBUG_ENCODER_BIT_STREAM
    fclose(_bitStreamAfterEncoder);
#endif
}

WebRtc_Word32
VCMEncodedFrameCallback::SetTransportCallback(VCMPacketizationCallback* transport)
{
    _sendCallback = transport;
    return VCM_OK;
}

WebRtc_Word32
VCMEncodedFrameCallback::Encoded(
    EncodedImage &encodedImage,
    const CodecSpecificInfo* codecSpecificInfo,
    const RTPFragmentationHeader* fragmentationHeader)
{
    FrameType frameType = VCMEncodedFrame::ConvertFrameType(encodedImage._frameType);
#if 0
	struct timeval tv; 
	struct timezone tz; 
	long spentTime;
	gettimeofday (&tv , &tz); 
	if(lastTime != 0)
	{
		spentTime= (tv.tv_sec * 1000 + tv.tv_usec / 1000) - lastTime;
		
		if(spentTime >= 1000)
			{
			LOGE("capture frameRate %d", frameCount);
			frameCount = 0;
			lastTime = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
			}
		else
			frameCount++;
	}
	else
		lastTime = (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
    WebRtc_UWord32 encodedBytes = 0;
    if (_sendCallback != NULL)
    {
        encodedBytes = encodedImage._length;

#ifdef DEBUG_ENCODER_BIT_STREAM
        if (_bitStreamAfterEncoder != NULL)
        {
            fwrite(encodedImage._buffer, 1, encodedImage._length, _bitStreamAfterEncoder);
        }
#endif

        RTPVideoHeader rtpVideoHeader;
        RTPVideoHeader* rtpVideoHeaderPtr = &rtpVideoHeader;
        if (codecSpecificInfo)
        {
            CopyCodecSpecific(*codecSpecificInfo, &rtpVideoHeaderPtr);
        }
        else
        {
            rtpVideoHeaderPtr = NULL;
        }

        WebRtc_Word32 callbackReturn = _sendCallback->SendData(
            frameType,
            _payloadType,
            encodedImage._timeStamp,
            encodedImage.capture_time_ms_,
            encodedImage._buffer,
            encodedBytes,
            *fragmentationHeader,
            rtpVideoHeaderPtr);
       if (callbackReturn < 0)
       {
           return callbackReturn;
       }
    }
    else
    {
        return VCM_UNINITIALIZED;
    }
    _encodedBytes = encodedBytes;
    if (_mediaOpt != NULL) {
      _mediaOpt->UpdateWithEncodedData(_encodedBytes, frameType);
      if (_internalSource)
      {
          return _mediaOpt->DropFrame(); // Signal to encoder to drop next frame
      }
    }
    return VCM_OK;
}

WebRtc_UWord32
VCMEncodedFrameCallback::EncodedBytes()
{
    return _encodedBytes;
}

void
VCMEncodedFrameCallback::SetMediaOpt(VCMMediaOptimization *mediaOpt)
{
    _mediaOpt = mediaOpt;
}

void VCMEncodedFrameCallback::CopyCodecSpecific(const CodecSpecificInfo& info,
                                                RTPVideoHeader** rtp) {
    switch (info.codecType) {
        case kVideoCodecVP8: {
            (*rtp)->codecHeader.VP8.InitRTPVideoHeaderVP8();
            (*rtp)->codecHeader.VP8.pictureId =
                info.codecSpecific.VP8.pictureId;
            (*rtp)->codecHeader.VP8.nonReference =
                info.codecSpecific.VP8.nonReference;
            (*rtp)->codecHeader.VP8.temporalIdx =
                info.codecSpecific.VP8.temporalIdx;
            (*rtp)->codecHeader.VP8.layerSync =
                info.codecSpecific.VP8.layerSync;
            (*rtp)->codecHeader.VP8.tl0PicIdx =
                info.codecSpecific.VP8.tl0PicIdx;
            (*rtp)->codecHeader.VP8.keyIdx =
                info.codecSpecific.VP8.keyIdx;
            (*rtp)->simulcastIdx = info.codecSpecific.VP8.simulcastIdx;
            return;
        }
        default: {
            // No codec specific info. Change RTP header pointer to NULL.
            *rtp = NULL;
            return;
        }
    }
}
} // namespace webrtc
