/*
*  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_MODULES_VIDEO_CODING_CODECS_I420_H_
#define WEBRTC_MODULES_VIDEO_CODING_CODECS_I420_H_

#include "video_codec_interface.h"
#include "typedefs.h"
#include "h264_enc_api.h"  //add wyh
#include "h264_dec_api.h"  //add wyh
#include "svc_enc_api.h"  //add wyh
#include "svc_dec_api.h"  //add wyh
#include "svcodec.h"
#include "thread_wrapper.h"
#include "event_wrapper.h"
#include "critical_section_wrapper.h"
#include "system_wrappers/interface/tick_util.h"
#include "../../system_wrappers/interface/trace.h"
#define MAX_IMG_BUF		16

//add wyh
#define MAXRTPNUM 256//64
#define PAYLOADTYPE_H264 96
#define PAYLOADTYPE_SVC_19 37
#define H264MTU				1260
//end wyh

typedef struct 
{
	WebRtc_UWord32              _encodedWidth;
	WebRtc_UWord32              _encodedHeight;
	WebRtc_UWord32              _timeStamp;
	int64_t                     capture_time_ms_;
	WebRtc_UWord8 *				_buffer;
}EncImg;


//add wyh
enum PicType
{
	PIC_TYPE_AUTO = 0,        
	PIC_TYPE_IDR,           
	PIC_TYPE_I,             
	PIC_TYPE_P,             
	PIC_TYPE_BREF,            
	PIC_TYPE_B,             
	PIC_TYPE_KEYFRAME,   
};

typedef struct
{
	char *buf;
	int payLoadType;
	int count;
	int rtpLen;
	short size[MAXRTPNUM];

}WebRtc_RTPacket;
//end wyh

class CodecCfg
{
public:
    int m_wirteEncfile;
    int m_wirteDecfile;
    int m_netFeedback;
    int m_bframebum;
    int m_qp[4][2];
    int m_lostPolicy[2];
    int m_cpuPolicy[2];
    CodecCfg()
    {
        m_wirteEncfile = 0;
        m_wirteDecfile =0;
        m_netFeedback = 0;
        m_bframebum = 3;
        m_qp[0][0] = 23;
        m_qp[0][1]  = 40;
        m_qp[1][0]  = 23;
        m_qp[1][1]  = 40;
        m_qp[2][0]  = 23;
        m_qp[2][1]  = 40;
        m_qp[3][0]  = 23;
        m_qp[3][1]  = 40;
        m_lostPolicy[0]  = 375;
        m_lostPolicy[1]  = 200;
        m_cpuPolicy[0]  = 80;
        m_cpuPolicy[1]  = 50;
    }
};


namespace webrtc 
{
	class I420Encoder : public VideoEncoder {
	public:

		I420Encoder();

		virtual ~I420Encoder();

		// Initialize the encoder with the information from the VideoCodec.
		//
		// Input:
		//          - codecSettings     : Codec settings.
		//          - numberOfCores     : Number of cores available for the encoder.
		//          - maxPayloadSize    : The maximum size each payload is allowed
		//                                to have. Usually MTU - overhead.
		//
		// Return value                 : WEBRTC_VIDEO_CODEC_OK if OK.
		//                                <0 - Error
		virtual int InitEncode(const VideoCodec* codecSettings,
			int /*numberOfCores*/,
			uint32_t /*maxPayloadSize*/);

		// "Encode" an I420 image (as a part of a video stream). The encoded image
		// will be returned to the user via the encode complete callback.
		//
		// Input:
		//          - inputImage        : Image to be encoded.
		//          - codecSpecificInfo : Pointer to codec specific data.
		//          - frameType         : Frame type to be sent (Key /Delta).
		//
		// Return value                 : WEBRTC_VIDEO_CODEC_OK if OK.
		//                                <0 - Error
		virtual int Encode(const I420VideoFrame& inputImage,
			const CodecSpecificInfo* /*codecSpecificInfo*/,
			const std::vector<VideoFrameType>* /*frame_types*/);

		// Register an encode complete callback object.
		//
		// Input:
		//          - callback         : Callback object which handles encoded images.
		//
		// Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
		virtual int RegisterEncodeCompleteCallback(EncodedImageCallback* callback);

		// Free encoder memory.
		//
		// Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
		virtual int Release();

		virtual int SetRates(uint32_t /*newBitRate*/, uint32_t /*frameRate*/)
		{return WEBRTC_VIDEO_CODEC_OK;}

		virtual int SetChannelParameters(uint32_t /*packetLoss*/, int /*rtt*/)
		{return WEBRTC_VIDEO_CODEC_OK;}

		virtual int CodecConfigParameters(uint8_t* /*buffer*/, int /*size*/)
		{return WEBRTC_VIDEO_CODEC_OK;}

	private:
		bool                     _inited;
		EncodedImage             _encodedImage;
		EncodedImageCallback*    _encodedCompleteCallback;
		CodecCfg _EncCfg;

		WebRtc_Word32 InitSVCEnc(const webrtc::VideoCodec* inst);
		WebRtc_Word32 GetEncodedPartitions();
		WebRtc_Word32 ImgCopy(const webrtc::I420VideoFrame& inputImage);
		WebRtc_Word32 SVCEnc(char *apSrcData[3]);
		WebRtc_RTPacket *_pRtPacket;
		void *_pEncDebugFile;
		ThreadWrapper* _pSvc_thread;
		EventWrapper *_pSvc_event;
		CriticalSectionWrapper* _pSvc_critsect;
		static bool SVCThreadProc(void* obj);
		bool SVCStopThread();
		bool SVCProcess();
		EncImg _encImg[MAX_IMG_BUF];
		unsigned char * _inEncBuf;
		int _maxImgBufCnt;
		int _inImgBufIdx;
		int _outImgBufIdx;
		int _lastOutImgBufIdx;
		int _width;
		int _height;
		int _width_used;
		int _height_used;
		//add wyh
		unsigned long GVE_CodecEnc_Handle;
		GVE_H264Enc_OperatePar OperatePar;
		GVE_H264Enc_ConfigPar ConfigPar;
		GVE_H264Enc_OutPutInfo OutPutInfo;
		GVE_SVCEnc_OperatePar SVCOperatePar;
		GVE_SVCEnc_ConfigPar SVCConfigPar;
		GVE_SVCEnc_OutPutInfo SVCOutPutInfo;
		bool _isRunning;


	}; // end of WebRtcI420DEncoder class

	class I420Decoder : public VideoDecoder {
	public:

		I420Decoder();

		virtual ~I420Decoder();

		// Initialize the decoder.
		// The user must notify the codec of width and height values.
		//
		// Return value         :  WEBRTC_VIDEO_CODEC_OK.
		//                        <0 - Errors
		virtual int InitDecode(/*const*/ VideoCodec* codecSettings,
			int /*numberOfCores*/);

		virtual int SetCodecConfigParameters(const uint8_t* /*buffer*/, int /*size*/)
		{return WEBRTC_VIDEO_CODEC_OK;};

		// Decode encoded image (as a part of a video stream). The decoded image
		// will be returned to the user through the decode complete callback.
		//
		// Input:
		//          - inputImage        : Encoded image to be decoded
		//          - missingFrames     : True if one or more frames have been lost
		//                                since the previous decode call.
		//          - codecSpecificInfo : pointer to specific codec data
		//          - renderTimeMs      : Render time in Ms
		//
		// Return value                 : WEBRTC_VIDEO_CODEC_OK if OK
		//                                 <0 - Error
		virtual int Decode(const EncodedImage& inputImage,
			bool missingFrames,
			const RTPFragmentationHeader* /*fragmentation*/,
			const CodecSpecificInfo* /*codecSpecificInfo*/,
			int64_t /*renderTimeMs*/);

		// Register a decode complete callback object.
		//
		// Input:
		//          - callback         : Callback object which handles decoded images.
		//
		// Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
		virtual int RegisterDecodeCompleteCallback(DecodedImageCallback* callback);

		// Free decoder memory.
		//
		// Return value                : WEBRTC_VIDEO_CODEC_OK if OK.
		//                                  <0 - Error
		virtual int Release();

		// Reset decoder state and prepare for a new call.
		//
		// Return value         :  WEBRTC_VIDEO_CODEC_OK.
		//                          <0 - Error
		virtual int Reset();

	private:

		I420VideoFrame              _decodedImage;
		int                         _width;
		int                         _height;
		bool                        _inited;
		DecodedImageCallback*       _decodeCompleteCallback;

		int InitSVCDec(/*const*/ webrtc::VideoCodec* inst);
		WebRtc_Word32 SVCDec(const EncodedImage& inputImage);
		unsigned char *tmpo[3];//kmm add
		CodecCfg _decCfg;
		bool _isH264;

		//add wyh
		unsigned long GVE_CodecDec_Handle;
		GVE_H264Dec_OperatePar OperatePar;
		GVE_H264Dec_ConfigPar ConfigPar;
		GVE_H264Dec_OutPutInfo OutPutInfo;
		GVE_SVCDec_OperatePar SVCOperatePar;
		GVE_SVCDec_ConfigPar SVCConfigPar;
		GVE_SVCDec_OutPutInfo SVCOutPutInfo;

	}; // End of WebRtcI420Decoder class.
    
    class I420Encoder_14 : public VideoEncoder {
    public:
        
        I420Encoder_14();
        
        virtual ~I420Encoder_14();
        
        // Initialize the encoder with the information from the VideoCodec.
        //
        // Input:
        //          - codecSettings     : Codec settings.
        //          - numberOfCores     : Number of cores available for the encoder.
        //          - maxPayloadSize    : The maximum size each payload is allowed
        //                                to have. Usually MTU - overhead.
        //
        // Return value                 : WEBRTC_VIDEO_CODEC_OK if OK.
        //                                <0 - Error
        virtual int InitEncode(const VideoCodec* codecSettings,
                               int /*numberOfCores*/,
                               uint32_t /*maxPayloadSize*/);
        
        // "Encode" an I420 image (as a part of a video stream). The encoded image
        // will be returned to the user via the encode complete callback.
        //
        // Input:
        //          - inputImage        : Image to be encoded.
        //          - codecSpecificInfo : Pointer to codec specific data.
        //          - frameType         : Frame type to be sent (Key /Delta).
        //
        // Return value                 : WEBRTC_VIDEO_CODEC_OK if OK.
        //                                <0 - Error
        virtual int Encode(const I420VideoFrame& inputImage,
                           const CodecSpecificInfo* /*codecSpecificInfo*/,
                           const std::vector<VideoFrameType>* /*frame_types*/);
        
        // Register an encode complete callback object.
        //
        // Input:
        //          - callback         : Callback object which handles encoded images.
        //
        // Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
        virtual int RegisterEncodeCompleteCallback(EncodedImageCallback* callback);
        
        // Free encoder memory.
        //
        // Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
        virtual int Release();
        
        virtual int SetRates(uint32_t /*newBitRate*/, uint32_t /*frameRate*/)
        {return WEBRTC_VIDEO_CODEC_OK;}
        
        virtual int SetChannelParameters(uint32_t /*packetLoss*/, int /*rtt*/)
        {return WEBRTC_VIDEO_CODEC_OK;}
        
        virtual int CodecConfigParameters(uint8_t* /*buffer*/, int /*size*/)
        {return WEBRTC_VIDEO_CODEC_OK;}
        
    private:
        bool                     _inited;
        EncodedImage             _encodedImage;
        EncodedImageCallback*    _encodedCompleteCallback;
        int _bitsrate;
        CodecCfg _EncCfg;
#ifndef GXH_TEST_H264
        WebRtc_Word32 InitSVCEnc(const webrtc::VideoCodec* inst);
        WebRtc_Word32 GetEncodedPartitions();
        
        WebRtc_Word32 ImgCopy(const webrtc::I420VideoFrame& inputImage);
        WebRtc_Word32 SVCEnc(char *apSrcData[3]);
        
        void *_pEncHandle;
        RTPacket *_pRtPacket;
        void *_pEncDebugFile;
        ThreadWrapper* _pSvc_thread;
        EventWrapper *_pSvc_event;
        CriticalSectionWrapper* _pSvc_critsect;
        static bool SVCThreadProc(void* obj);
        bool SVCStopThread();
        bool SVCProcess();
        EncImg _encImg[MAX_IMG_BUF];
        unsigned char * _inEncBuf;
        int _maxImgBufCnt;
        int _inImgBufIdx;
        int _outImgBufIdx;
        int _lastOutImgBufIdx;
        WebRtc_Word64 _lastTime;
        int _frameRate;
        int _width;
        int _height;
        int _width_used;
        int _height_used;
#endif
        
    }; // end of WebRtcI420DEncoder class
    
    class I420Decoder_14 : public VideoDecoder {
    public:
        
        I420Decoder_14();
        
        virtual ~I420Decoder_14();
        
        // Initialize the decoder.
        // The user must notify the codec of width and height values.
        //
        // Return value         :  WEBRTC_VIDEO_CODEC_OK.
        //                        <0 - Errors
		virtual int InitDecode(/*const*/ VideoCodec* codecSettings,
                               int /*numberOfCores*/);
        
        virtual int SetCodecConfigParameters(const uint8_t* /*buffer*/, int /*size*/)
        {return WEBRTC_VIDEO_CODEC_OK;};
        
        // Decode encoded image (as a part of a video stream). The decoded image
        // will be returned to the user through the decode complete callback.
        //
        // Input:
        //          - inputImage        : Encoded image to be decoded
        //          - missingFrames     : True if one or more frames have been lost
        //                                since the previous decode call.
        //          - codecSpecificInfo : pointer to specific codec data
        //          - renderTimeMs      : Render time in Ms
        //
        // Return value                 : WEBRTC_VIDEO_CODEC_OK if OK
        //                                 <0 - Error
        virtual int Decode(const EncodedImage& inputImage,
                           bool missingFrames,
                           const RTPFragmentationHeader* /*fragmentation*/,
                           const CodecSpecificInfo* /*codecSpecificInfo*/,
                           int64_t /*renderTimeMs*/);
        
        // Register a decode complete callback object.
        //
        // Input:
        //          - callback         : Callback object which handles decoded images.
        //
        // Return value                : WEBRTC_VIDEO_CODEC_OK if OK, < 0 otherwise.
        virtual int RegisterDecodeCompleteCallback(DecodedImageCallback* callback);
        
        // Free decoder memory.
        //
        // Return value                : WEBRTC_VIDEO_CODEC_OK if OK.
        //                                  <0 - Error
        virtual int Release();
        
        // Reset decoder state and prepare for a new call.
        //
        // Return value         :  WEBRTC_VIDEO_CODEC_OK.
        //                          <0 - Error
        virtual int Reset();
        
    private:
        
        I420VideoFrame              _decodedImage;
        int                         _width;
        int                         _height;
        bool                        _inited;
        DecodedImageCallback*       _decodeCompleteCallback;
#ifndef GXH_TEST_H265
		int InitSVCDec(/*const*/ webrtc::VideoCodec* inst);
        WebRtc_Word32 SVCDec(const EncodedImage& inputImage);
        void *_pDecHandle;
        void *_pDecDebugFile;
#endif
        char *tmpo[3];//kmm add
        //VPMSpatialResampler* _spatialResampler	;
        
        Encoded_Decoded_Info	_pEncoded_Decoded_Info[4];
        Down_Layer_Info _pDown_Layer_Info[1];
        int _lastEncidx;//¡­?£¤?¦Ì?¡À???¡Æ¡Â¡À¨º??¨¤?¡°?¡ê¡§¡±¡Ì¡°¡®¡Ö¨C??¡À???????¡Æ¨°¡Æ¡é¡­¨B¡Ý??oa?
        
        CodecCfg _decCfg;
        bool _isH264;
        
        
    }; // End of WebRtcI420Decoder class.

} // namespace webrtc

#endif // WEBRTC_MODULES_VIDEO_CODING_CODECS_I420_H_
