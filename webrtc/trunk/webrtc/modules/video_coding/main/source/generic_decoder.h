/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/********************************************************************
* @file		generic_decoder.h
* file path	e:\work\webrtc_HLD_0516\trunk\webrtc\modules\video_coding\main\source
*	
* @brief	解码器相关类
*
*********************************************************************/

#ifndef WEBRTC_MODULES_VIDEO_CODING_GENERIC_DECODER_H_
#define WEBRTC_MODULES_VIDEO_CODING_GENERIC_DECODER_H_

#include "timing.h"
#include "timestamp_map.h"
#include "video_codec_interface.h"
#include "encoded_frame.h"
#include "module_common_types.h"

namespace webrtc
{

class VCMReceiveCallback;

enum { kDecoderFrameMemoryLength = 10 };

struct VCMFrameInformation
{
    WebRtc_Word64     renderTimeMs;
    WebRtc_Word64     decodeStartTimeMs;
    void*             userData;
};

/** 
* class		class VCMDecodedFrameCallback in generic_decoder.h  
* @brief	decode callback 函数
*/
class VCMDecodedFrameCallback : public DecodedImageCallback
{
public:
    VCMDecodedFrameCallback(VCMTiming& timing, Clock* clock);
    virtual ~VCMDecodedFrameCallback();
    void SetUserReceiveCallback(VCMReceiveCallback* receiveCallback);

    virtual WebRtc_Word32 Decoded(I420VideoFrame& decodedImage);
    virtual WebRtc_Word32 ReceivedDecodedReferenceFrame(const WebRtc_UWord64 pictureId);
    virtual WebRtc_Word32 ReceivedDecodedFrame(const WebRtc_UWord64 pictureId);

    WebRtc_UWord64 LastReceivedPictureID() const;

    WebRtc_Word32 Map(WebRtc_UWord32 timestamp, VCMFrameInformation* frameInfo);
    WebRtc_Word32 Pop(WebRtc_UWord32 timestamp);

private:
    CriticalSectionWrapper* _critSect;
    Clock* _clock;
    I420VideoFrame _frame;
    VCMReceiveCallback* _receiveCallback;
    VCMTiming& _timing;
    VCMTimestampMap _timestampMap;
    WebRtc_UWord64 _lastReceivedPictureID;
};

/** 
* class		class VCMGenericDecoder in generic_decoder.h  
* @brief	解码器函数
*
* 初始化解码器，解码，注销解码器等
*/
class VCMGenericDecoder
{
	friend class VCMCodecDataBase;
public:
    VCMGenericDecoder(VideoDecoder& decoder, WebRtc_Word32 id = 0, bool isExternal = false);
    ~VCMGenericDecoder();

    /**
    *	Initialize the decoder with the information from the VideoCodec
    */
/**
* ingroup	API_Codec
* @brief    初始化解码器（InitDecode）
* 
* Initialize the decoder with the information from the VideoCodec
* @req		GVE-SRS-106
* @param[in]    const VideoCodec* settings: 编解码初始化参数集合  
* @param[in]    WebRtc_Word32 numberOfCores: Number of cores available for the encoder.
* @param[in]    bool requireKeyFrame: 是否需要关键帧    
* @returns   WebRtc_Word32, WEBRTC_VIDEO_CODEC_OK if OK. 0 - Error
*/
    WebRtc_Word32 InitDecode(/*const*/ VideoCodec* settings,
                             WebRtc_Word32 numberOfCores,
                             bool requireKeyFrame);

    /**
    *	Decode to a raw I420 frame,
    *
    *	inputVideoBuffer	reference to encoded video frame
    */
/**
* ingroup	API_Codec
* @brief    解码（Decode）
*
* @req		GVE-SRS-107
* @param[in]     inputImage	: Encoded image to be decoded
* @param[in]     nowMs		: 当前时间
* @returns   WEBRTC_VIDEO_CODEC_OK if OK. 0 - Error
*/
	
    WebRtc_Word32 Decode(const VCMEncodedFrame& inputFrame, int64_t nowMs);

    /**
    *	Free the decoder memory
    */
/**
* ingroup	API_Codec
* @brief    关闭解码器，Free the decoder memory
* 
* @req		GVE-SRS-108
* @returns    WEBRTC_VIDEO_CODEC_OK if OK. 0 - Error
*/
    WebRtc_Word32 Release();

    /**
    *	Reset the decoder state, prepare for a new call
    */
/**
* @brief    Reset the decoder state, prepare for a new call

* @returns   WEBRTC_VIDEO_CODEC_OK if OK. 0 - Error
*/
    WebRtc_Word32 Reset();

    /**
    *	Codec configuration data sent out-of-band, i.e. in SIP call setup
    *
    *	buffer pointer to the configuration data
    *	size the size of the configuration data in bytes
    */
    WebRtc_Word32 SetCodecConfigParameters(const WebRtc_UWord8* /*buffer*/,
                                           WebRtc_Word32 /*size*/);

    WebRtc_Word32 RegisterDecodeCompleteCallback(VCMDecodedFrameCallback* callback);

    bool External() const;

protected:

    WebRtc_Word32               _id;
    VCMDecodedFrameCallback*    _callback;
    VCMFrameInformation         _frameInfos[kDecoderFrameMemoryLength];
    WebRtc_UWord32              _nextFrameInfoIdx;
    VideoDecoder&               _decoder;
    VideoCodecType              _codecType;
    bool                        _isExternal;
    bool                        _requireKeyFrame;
    bool                        _keyFrameDecoded;

};

} // namespace webrtc

#endif // WEBRTC_MODULES_VIDEO_CODING_GENERIC_DECODER_H_
