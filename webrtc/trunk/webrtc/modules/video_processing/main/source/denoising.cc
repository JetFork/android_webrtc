/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "denoising.h"
#ifndef GXH_TEST_H264
#include "math.h"//gxh
#endif
#include "trace.h"

#include <cstring>

namespace webrtc {

enum { kSubsamplingTime = 0 };       // Down-sampling in time (unit: number of frames)
enum { kSubsamplingWidth = 0 };      // Sub-sampling in width (unit: power of 2)
enum { kSubsamplingHeight = 0 };     // Sub-sampling in height (unit: power of 2)
enum { kDenoiseFiltParam = 179 };    // (Q8) De-noising filter parameter
enum { kDenoiseFiltParamRec = 77 };  // (Q8) 1 - filter parameter
enum { kDenoiseThreshold = 19200 };  // (Q8) De-noising threshold level

VPMDenoising::VPMDenoising() :
    _id(0),
    _moment1(NULL),
    _moment2(NULL)
{
	memset(weightEven, 0.0, sizeof(weightEven));//kmm add @2013.7.19
	memset(weightMulti, 0.0, sizeof(weightMulti));//kmm add @2013.7.19
	inited = false;
    Reset();
}

VPMDenoising::~VPMDenoising()
{
	/*kmm del
    if (_moment1)
    {
        delete [] _moment1;
        _moment1 = NULL;
    }
    
    if (_moment2)
    {
        delete [] _moment2;
        _moment2 = NULL;
    }
	*///kmm end del
}

WebRtc_Word32
VPMDenoising::ChangeUniqueId(const WebRtc_Word32 id)
{
    _id = id;
    return VPM_OK;
}

void
VPMDenoising::Reset()
{
    _frameSize = 0;
    _denoiseFrameCnt = 0;

	/*kmm del
     if (_moment1)
     {
         delete [] _moment1;
         _moment1 = NULL;
     }
     
     if (_moment2)
     {
         delete [] _moment2;
         _moment2 = NULL;
     }
	 *///kmm end del
}

#ifndef GXH_TEST_H264
inline  int  Round( double value )
{
	int t = 0;
#ifdef _WIN32
	__asm
	{
		fld value;
		fistp t;
	}
#endif
	return t;
}

WebRtc_Word32 VPMDenoising::InitDenoise(double sigma_color, double sigma_space)
{
	if (sigma_color <=0 || sigma_space <=0)
	{
		return VPM_PARAMETER_ERROR;
	}
	double gauss_color_coeff = -0.5/(sigma_color*sigma_color);
	double gauss_space_coeff = -0.5/(sigma_space*sigma_space);

	for( int i = 0; i < 256; i++ )
	{
		//color_weight[i] = (double)exp(i*i*gauss_color_coeff);

		weightEven[i] = (double)exp(gauss_space_coeff+i*i*gauss_color_coeff);

		for (int j=0; j<256; j++)
		{
			weightMulti[i][j] = (double)(weightEven[i]*j);
		}
	}
	
	return VPM_OK;
}

//kmm add:双边滤波
WebRtc_Word32 VPMDenoising::BilateralFilter(const WebRtc_UWord8* src, WebRtc_UWord8* dst,	WebRtc_Word32 width, WebRtc_Word32 height)
//不支持in place操作
{
	if (src == NULL || dst == NULL || width <=0 || height <=0)
	{
		return VPM_PARAMETER_ERROR;
	}
	double mean1;
	double mean0;
	double w;

	WebRtc_Word32 i, j; //Shelley modified WebRtc_UWord32 to WebRtc_Word32 20130425
	//if(deltas[0] == 0)
	//{
	//	deltas[0] =  1; deltas[1] = -width + 1;
	//	deltas[2] = -width; deltas[3] = -width - 1;
	//	deltas[4] = -1; deltas[5] =  width - 1;
	//	deltas[6] =  width; deltas[7] =  width + 1;
	//}

#define INIT_C1\
	color = src[0]; mean0 = 1; mean1 = color

#define UPDATE_OUTPUT_C1\
		*dst = (WebRtc_UWord8)Round(mean1/mean0)

#define KERNEL_ELEMENT_EVEN(k)\
	temp_color = src[deltas[k]];\
	diff = abs(color - temp_color);\
	w = weightEven[diff];\
	mean0 += w;\
	mean1 += weightMulti[diff][temp_color]


	WebRtc_UWord8 color, temp_color;
	WebRtc_UWord32 diff;

	for (i=0; i<height; i++)
	{
		for (j=0; j<width; j++)
		{
			if (i ==0 || j==0 || i==height-1 || j==width-1)
			{
				*dst = *src;
			}
			else
			{
 				//color = *src;
 				//mean0 = 1; 
 				//mean1 = color;

				INIT_C1;
				KERNEL_ELEMENT_EVEN(0);
				KERNEL_ELEMENT_EVEN(2);
				KERNEL_ELEMENT_EVEN(4);
				KERNEL_ELEMENT_EVEN(6);


   			//	for (int k=0; k<8; k+=2)
   			//	{
 					//temp_color = src[deltas[k]];
 					//WebRtc_UWord32 diff=abs(color - temp_color);
 					//w = weightEven[diff];
 					//mean0 += w;
 					//mean1 += weightMulti[diff][temp_color];
   			//	}
				//*dst = (WebRtc_UWord8)Round(mean1/mean0);
				
				UPDATE_OUTPUT_C1;
			}
			src++;
			dst++;
		}
	}
	return VPM_OK;

#undef INIT_C1
#undef UPDATE_OUTPUT_C1
#undef KERNEL_ELEMENT_EVEN
}
//kmm end add

WebRtc_Word32
VPMDenoising::ProcessFrame(I420VideoFrame* frame0)
{
	if (frame0->IsZeroSize())
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoPreocessing, _id,
			"zero size frame");
		return VPM_PARAMETER_ERROR;
	}

	const WebRtc_UWord32 width = frame0->width();
	const WebRtc_UWord32 height = frame0->height();

	if (width<= 0 || height <= 0) {
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoPreocessing, _id,
			"Invalid frame size");
		return VPM_PARAMETER_ERROR;
	}

	if (!inited)
	{
		WebRtc_Word32 result = InitDenoise(20, 3);
		if (result != VPM_OK)
		{
			WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoPreocessing, _id,
				"Denoise uninitialized");
			return VPM_UNINITIALIZED;
		}
		inited = true;
	}

//	if(deltas[0] == 0)
	{//Shelley add 强制类型转换WebRtc_Word32 20130425
		deltas[0] =  1; deltas[1] = -(WebRtc_Word32)width + 1;
		deltas[2] = -(WebRtc_Word32)width; deltas[3] = -(WebRtc_Word32)width - 1;
		deltas[4] = -1; deltas[5] =  (WebRtc_Word32)width - 1;
		deltas[6] =  (WebRtc_Word32)width; deltas[7] =  (WebRtc_Word32)width + 1;
	}

	WebRtc_UWord8* frame = (WebRtc_UWord8*)frame0->buffer(kYPlane);//frame0->buffer;

    const WebRtc_UWord32 ysize  = height * width;

    /* Initialization */
    /*if (ysize != _frameSize)
    {
        delete [] _moment1;
        _moment1 = NULL;

        delete [] _moment2;
        _moment2 = NULL;
    }*/
    _frameSize = ysize;

    /*if (!_moment1)
    {
        _moment1 = new WebRtc_UWord32[ysize];
        memset(_moment1, 0, sizeof(WebRtc_UWord32)*ysize);
    }
    
    if (!_moment2)
    {
        _moment2 = new WebRtc_UWord32[ysize];
        memset(_moment2, 0, sizeof(WebRtc_UWord32)*ysize);
    }*/
	//================added 
	WebRtc_UWord8 *temp = new WebRtc_UWord8[ysize];


	//memcpy(temp, src, ysize);
	WebRtc_Word32 result;
   	result = BilateralFilter(frame, temp, width, height);
	if (result != VPM_OK)
	{
		delete temp;
		return VPM_GENERAL_ERROR;
	}
	result = BilateralFilter(temp,  frame,width, height);
	if (result != VPM_OK)
	{
		delete temp;
		return VPM_GENERAL_ERROR;
	}
	else
	{
		delete temp;
		return VPM_OK;
	}
}
#endif

//WebRtc_Word32
//VPMDenoising::ProcessFrame(I420VideoFrame* frame)
//{
//    assert(frame);
//    WebRtc_Word32     thevar;
//    int               k;
//    int               jsub, ksub;
//    WebRtc_Word32     diff0;
//    WebRtc_UWord32    tmpMoment1;
//    WebRtc_UWord32    tmpMoment2;
//    WebRtc_UWord32    tmp;
//    WebRtc_Word32     numPixelsChanged = 0;
//
//    if (frame->IsZeroSize())
//    {
//        WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoPreocessing, _id,
//                     "zero size frame");
//        return VPM_GENERAL_ERROR;
//    }
//
//    int width = frame->width();
//    int height = frame->height();
//
//    /* Size of luminance component */
//    const WebRtc_UWord32 ysize  = height * width;
//
//    /* Initialization */
//    if (ysize != _frameSize)
//    {
//        delete [] _moment1;
//        _moment1 = NULL;
//
//        delete [] _moment2;
//        _moment2 = NULL;
//    }
//    _frameSize = ysize;
//
//    if (!_moment1)
//    {
//        _moment1 = new WebRtc_UWord32[ysize];
//        memset(_moment1, 0, sizeof(WebRtc_UWord32)*ysize);
//    }
//    
//    if (!_moment2)
//    {
//        _moment2 = new WebRtc_UWord32[ysize];
//        memset(_moment2, 0, sizeof(WebRtc_UWord32)*ysize);
//    }
//
//    /* Apply de-noising on each pixel, but update variance sub-sampled */
//    uint8_t* buffer = frame->buffer(kYPlane);
//    for (int i = 0; i < height; i++)
//    { // Collect over height
//        k = i * width;
//        ksub = ((i >> kSubsamplingHeight) << kSubsamplingHeight) * width;
//        for (int j = 0; j < width; j++)
//        { // Collect over width
//            jsub = ((j >> kSubsamplingWidth) << kSubsamplingWidth);
//            /* Update mean value for every pixel and every frame */
//            tmpMoment1 = _moment1[k + j];
//            tmpMoment1 *= kDenoiseFiltParam; // Q16
//            tmpMoment1 += ((kDenoiseFiltParamRec *
//                          ((WebRtc_UWord32)buffer[k + j])) << 8);
//            tmpMoment1 >>= 8; // Q8
//            _moment1[k + j] = tmpMoment1;
//
//            tmpMoment2 = _moment2[ksub + jsub];
//            if ((ksub == k) && (jsub == j) && (_denoiseFrameCnt == 0))
//            {
//                tmp = ((WebRtc_UWord32)buffer[k + j] *
//                      (WebRtc_UWord32)buffer[k + j]);
//                tmpMoment2 *= kDenoiseFiltParam; // Q16
//                tmpMoment2 += ((kDenoiseFiltParamRec * tmp)<<8);
//                tmpMoment2 >>= 8; // Q8
//            }
//            _moment2[k + j] = tmpMoment2;
//            /* Current event = deviation from mean value */
//            diff0 = ((WebRtc_Word32)buffer[k + j] << 8) - _moment1[k + j];
//            /* Recent events = variance (variations over time) */
//            thevar = _moment2[k + j];
//            thevar -= ((_moment1[k + j] * _moment1[k + j]) >> 8);
//            /***************************************************************************
//             * De-noising criteria, i.e., when should we replace a pixel by its mean
//             *
//             * 1) recent events are minor
//             * 2) current events are minor
//             ***************************************************************************/
//            if ((thevar < kDenoiseThreshold)
//                && ((diff0 * diff0 >> 8) < kDenoiseThreshold))
//            { // Replace with mean
//                buffer[k + j] = (WebRtc_UWord8)(_moment1[k + j] >> 8);
//                numPixelsChanged++;
//            }
//        }
//    }
//
//    /* Update frame counter */
//    _denoiseFrameCnt++;
//    if (_denoiseFrameCnt > kSubsamplingTime)
//    {
//        _denoiseFrameCnt = 0;
//    }
//
//    return numPixelsChanged;
//}

} //namespace
