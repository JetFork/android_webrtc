/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video_engine/vie_frame_provider_base.h"

#include <algorithm>

#include "common_video/interface/i420_video_frame.h"
#include "system_wrappers/interface/critical_section_wrapper.h"
#include "system_wrappers/interface/tick_util.h"
#include "system_wrappers/interface/trace.h"
#include "video_engine/vie_defines.h"
#include "video_processing.h"//kmm2 add

namespace webrtc {

ViEFrameProviderBase::ViEFrameProviderBase(int Id, int engine_id)
    : id_(Id),
      engine_id_(engine_id),
      provider_cs_(CriticalSectionWrapper::CreateCriticalSection()),
      frame_delay_(0),
	  colorEnhance_enable_(false),//kmm2 add
	  contrastEnhance_enable_(false),//kmm2 add
	  sharpen_enable_(false)//kmm2 add
{
	memset(contrastTable, 255, sizeof(contrastTable));//kmm2 add
}

ViEFrameProviderBase::~ViEFrameProviderBase() {
  if (frame_callbacks_.size() > 0) {
    WEBRTC_TRACE(kTraceWarning, kTraceVideo, ViEId(engine_id_, id_),
                 "FrameCallbacks still exist when Provider deleted %d",
                 frame_callbacks_.size());
  }

  for (FrameCallbacks::iterator it = frame_callbacks_.begin();
       it != frame_callbacks_.end(); ++it) {
    (*it)->ProviderDestroyed(id_);
  }
  frame_callbacks_.clear();
}

int ViEFrameProviderBase::Id() {
  return id_;
}

void ViEFrameProviderBase::DeliverFrame(
    I420VideoFrame* video_frame,
    int num_csrcs,
    const WebRtc_UWord32 CSRC[kRtpCsrcSize]) {
#ifdef DEBUG_
  const TickTime start_process_time = TickTime::Now();
#endif
  CriticalSectionScoped cs(provider_cs_.get());


 // WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "DeliverFrame 000");
  // Deliver the frame to all registered callbacks.
  if (frame_callbacks_.size() > 0) {
       // WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "DeliverFrame 111");
    if (frame_callbacks_.size() == 1) {
		//kmm2 add

       // WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "DeliverFrame 222");
		if (frame_callbacks_.front()->GetType() == 0)//帧流向render
		{
			if (contrastEnhance_enable_)
			{
				I420ContrastEnhance(video_frame);//对比度增强
			}
			if (colorEnhance_enable_) 
			{
				VideoProcessingModule::ColorEnhancement(video_frame);//色彩增强
			}
			if (sharpen_enable_)
			{
				I420SharpenMask(video_frame);//锐化（带噪声模糊)
			}
		}
		//kmm2 end add
		// We don't have to copy the frame.
        //WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "DeliverFrame 333");
	   	frame_callbacks_.front()->DeliverFrame(id_, video_frame, num_csrcs, CSRC);
    } else {
      // Make a copy of the frame for all callbacks.callback
      //for (FrameCallbacks::iterator it = frame_callbacks_.begin();
      //     it != frame_callbacks_.end(); ++it) {
      //  if (!extra_frame_.get()) {
      //    extra_frame_.reset(new I420VideoFrame());
      //  }
      //  extra_frame_->CopyFrame(*video_frame);
      //  (*it)->DeliverFrame(id_, extra_frame_.get(), num_csrcs, CSRC);
      //}
	//kmm2 modify
       // WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "DeliverFrame 444");
		scoped_ptr<I420VideoFrame> prcess_frame;
		if (!prcess_frame.get()) {
			prcess_frame.reset(new I420VideoFrame());
		}
		prcess_frame->CopyFrame(*video_frame);

		for (FrameCallbacks::iterator it = frame_callbacks_.begin();
			it != frame_callbacks_.end(); ++it) {
				if ( (*it)->GetType() == 0)//帧流向rendeer且没有经过后处理
				{
					if (contrastEnhance_enable_)
					{
						I420ContrastEnhance(prcess_frame.get());//对比度增强
					}
					if (colorEnhance_enable_) 
					{
						VideoProcessingModule::ColorEnhancement(prcess_frame.get());//色彩增强
					}
					if (sharpen_enable_)
					{
						I420SharpenMask(prcess_frame.get());//锐化（带噪声模糊)
					}
				}

				if (!extra_frame_.get()) {
					extra_frame_.reset(new I420VideoFrame());
				}
				if ((*it)->GetType() == 0)//帧流向render且已经经过后处理
				{
					extra_frame_->CopyFrame(*prcess_frame);
				}
				else
				{
					extra_frame_->CopyFrame(*video_frame);
				}
        //WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "DeliverFrame 555");
				(*it)->DeliverFrame(id_, extra_frame_.get(), num_csrcs, CSRC);
		}
    }
  }
#ifdef DEBUG_
  const int process_time =
      static_cast<int>((TickTime::Now() - start_process_time).Milliseconds());
  if (process_time > 25) {
    // Warn if the delivery time is too long.
    WEBRTC_TRACE(kTraceWarning, kTraceVideo, ViEId(engine_id_, id_),
                 "%s Too long time: %ums", __FUNCTION__, process_time);
  }
#endif
}

void ViEFrameProviderBase::SetFrameDelay(int frame_delay) {
  CriticalSectionScoped cs(provider_cs_.get());
  frame_delay_ = frame_delay;

  for (FrameCallbacks::iterator it = frame_callbacks_.begin();
       it != frame_callbacks_.end(); ++it) {
    (*it)->DelayChanged(id_, frame_delay);
  }
}

int ViEFrameProviderBase::FrameDelay() {
  return frame_delay_;
}

int ViEFrameProviderBase::GetBestFormat(int* best_width,
                                        int* best_height,
                                        int* best_frame_rate) {
  int largest_width = 0;
  int largest_height = 0;
  int highest_frame_rate = 0;

  CriticalSectionScoped cs(provider_cs_.get());
  for (FrameCallbacks::iterator it = frame_callbacks_.begin();
       it != frame_callbacks_.end(); ++it) {
    int prefered_width = 0;
    int prefered_height = 0;
    int prefered_frame_rate = 0;
    if ((*it)->GetPreferedFrameSettings(&prefered_width, &prefered_height,
                                        &prefered_frame_rate) == 0) {
      if (prefered_width > largest_width) {
        largest_width = prefered_width;
      }
      if (prefered_height > largest_height) {
        largest_height = prefered_height;
      }
      if (prefered_frame_rate > highest_frame_rate) {
        highest_frame_rate = prefered_frame_rate;
      }
    }
  }
  *best_width = largest_width;
  *best_height = largest_height;
  *best_frame_rate = highest_frame_rate;
  return 0;
}

int ViEFrameProviderBase::RegisterFrameCallback(
    int observer_id, ViEFrameCallback* callback_object) {
  assert(callback_object);
  WEBRTC_TRACE(kTraceInfo, kTraceVideo, ViEId(engine_id_, id_), "%s(0x%p)",
               __FUNCTION__, callback_object);
  {
    CriticalSectionScoped cs(provider_cs_.get());
    if (std::find(frame_callbacks_.begin(), frame_callbacks_.end(),
                  callback_object) != frame_callbacks_.end()) {
      // This object is already registered.
      WEBRTC_TRACE(kTraceWarning, kTraceVideo, ViEId(engine_id_, id_),
                   "%s 0x%p already registered", __FUNCTION__,
                   callback_object);
      assert(false && "frameObserver already registered");
      return -1;
    }
    WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "RegisterFrameCallBack push_back 111");
   	frame_callbacks_.push_back(callback_object);
  }
  // Report current capture delay.
  callback_object->DelayChanged(id_, frame_delay_);

  // Notify implementer of this class that the callback list have changed.
  FrameCallbackChanged();
  return 0;
}

int ViEFrameProviderBase::DeregisterFrameCallback(
    const ViEFrameCallback* callback_object) {
  assert(callback_object);
  WEBRTC_TRACE(kTraceInfo, kTraceVideo, ViEId(engine_id_, id_), "%s(0x%p)",
               __FUNCTION__, callback_object);
  CriticalSectionScoped cs(provider_cs_.get());

  FrameCallbacks::iterator it = std::find(frame_callbacks_.begin(),
                                          frame_callbacks_.end(),
                                          callback_object);
  if (it == frame_callbacks_.end()) {
    WEBRTC_TRACE(kTraceWarning, kTraceVideo, ViEId(engine_id_, id_),
                 "%s 0x%p not found", __FUNCTION__, callback_object);
    return -1;
  }
  frame_callbacks_.erase(it);
  WEBRTC_TRACE(kTraceInfo, kTraceVideo, ViEId(engine_id_, id_),
               "%s 0x%p deregistered", __FUNCTION__, callback_object);

  // Notify implementer of this class that the callback list have changed.
  FrameCallbackChanged();
  return 0;
}

bool ViEFrameProviderBase::IsFrameCallbackRegistered(
    const ViEFrameCallback* callback_object) {
  assert(callback_object);
  WEBRTC_TRACE(kTraceInfo, kTraceVideo, ViEId(engine_id_, id_),
               "%s(0x%p)", __FUNCTION__, callback_object);

  CriticalSectionScoped cs(provider_cs_.get());
  return std::find(frame_callbacks_.begin(), frame_callbacks_.end(),
                   callback_object) != frame_callbacks_.end();
}

int ViEFrameProviderBase::NumberOfRegisteredFrameCallbacks() {
  CriticalSectionScoped cs(provider_cs_.get());
  return frame_callbacks_.size();
}

//kmm2 add
WebRtc_Word32 ViEFrameProviderBase::EnableColorEnhancement(bool enable)
{
	colorEnhance_enable_ = enable;
	return 0;
}

bool ViEFrameProviderBase::GetEnableColorEnhancementState()
{
	return colorEnhance_enable_;
}

WebRtc_Word32 ViEFrameProviderBase::EnableContrastEnhancement(bool enable)
{
	contrastEnhance_enable_ = enable;
	return 0;
}

bool ViEFrameProviderBase::GetEnableContrastEnhancementState()
{
	return contrastEnhance_enable_;
}

WebRtc_Word32 ViEFrameProviderBase::EnableSharpen(bool enable)
{
	sharpen_enable_ = enable;
	return 0;
}

bool ViEFrameProviderBase::GetEnableSharpenState()
{
	return sharpen_enable_;
}


int ViEFrameProviderBase::InitContrastTable(float deltaContrast)
{
	if (deltaContrast <= -100)
	{
		return -1;
	}
	float c = (100+deltaContrast)/100.0f;
	for (int mean=0; mean<256; mean++)
	{
		int x1=(c*mean)/(1+c);
		int x2= (255+c*mean)/(1+c);
		for (int i=0; i<256; i++)
		{
			if (i<x1)
			{
				contrastTable[mean][i] = WebRtc_UWord8(i/c);
			}
			else if (i>x2)
			{
				contrastTable[mean][i] = WebRtc_UWord8((i-255)/c+255);
			}
			else
			{
				contrastTable[mean][i] = WebRtc_UWord8(mean + (i-mean)*c);
			}
		}
	}
	return 0;
}

int  ViEFrameProviderBase::I420ContrastEnhance(I420VideoFrame* video_frame)
{
	if (video_frame->IsZeroSize() ||
		video_frame->width() ==0 || video_frame->height() ==0)
	{
		return -1;
	}

	if (contrastTable[0][0] != 0)
	{
		int result = InitContrastTable(25);//建立对比度映射表
		if (result !=0)
		{
			return -1;
		}
	}

	WebRtc_UWord8 *pY = video_frame->buffer(kYPlane);
	int yStride = video_frame->width()*video_frame->height();

	int Ymean =0;

	for (int i=0; i<yStride; i++)
	{
		Ymean += pY[i];
	} 
	Ymean /= yStride; //计算平均亮度

	for (int i=0; i<yStride; i++)
	{
		*pY= contrastTable[Ymean][*pY];
		pY++;
	}

	return 0;
}

WebRtc_Word32 ViEFrameProviderBase::I420SharpenMask(I420VideoFrame* video_frame)
//△3*3分块，模板锐化，Y通道
{
	WebRtc_UWord32 w = video_frame->width();
	WebRtc_UWord32 h = video_frame->height();

	if (video_frame->IsZeroSize() || w==0 || h==0)
	{
		return -1;
	}

	WebRtc_UWord32 ySize = w*h;
	WebRtc_Word32 yStride = w; //Shelley add 强制类型转换WebRtc_UWord32为WebRtc_Word32 20130425

	WebRtc_UWord8 *pYdst = video_frame->buffer(kYPlane);
	//WebRtc_UWord8 *pTmp = new WebRtc_UWord8[yStride];
	WebRtc_UWord8 *pYdstS=pYdst;

	WebRtc_UWord8 *pYsrcS = (WebRtc_UWord8 *)malloc(ySize);
	memcpy(pYsrcS, pYdstS, ySize);
	WebRtc_UWord8 *pYsrc = pYsrcS;

	//	int deltas0[9];
	WebRtc_Word32 *deltas0 = (WebRtc_Word32 *)malloc(9*(sizeof(WebRtc_Word32))); 
	deltas0[5] =  1;  
	deltas0[2] = -yStride + 1;
	deltas0[1] = -yStride; 
	deltas0[0] = -yStride - 1;
	deltas0[3] = -1;  
	deltas0[6] =  yStride - 1;
	deltas0[7] =  yStride; 
	deltas0[8] =  yStride + 1;
	deltas0[4] = 0;

	/*
	模板：
	(-1  -1  -1)
	1/(n-8)*(-1   N   -1)
	(-1  -1  -1)	 
	int tem[9];
	*/			  
	//参数N用来控制锐化力度，n>=9，N越小，锐化力度越大。n为负数时，模板转化为平滑模板，n=-1，为均值滤波器，N越小，越接近原图。

	WebRtc_UWord32 T1=4050;//50*81;
	WebRtc_UWord32 T2=16200;//200*81;

	WebRtc_UWord32 i, j, k;
	WebRtc_UWord32 stepH, step;
	WebRtc_Word32 tmp;

	WebRtc_UWord32 sum, var81, qSum;

	for (j=1; j< h-1; j++)
	{
		stepH = yStride*j;
		for (i=1; i<w-1; i++)
		{
			step = stepH+i;
			pYsrc = pYsrcS+step;
			pYdst = pYdstS+step;
			sum = 0;
			qSum=0;

			for (k=0; k<9; k++)
			{
				tmp =  pYsrc[deltas0[k]];
				sum +=tmp;
				qSum += tmp*tmp;
			}

			//var81 = 9*qSum-sum*sum;
			var81 = qSum + (qSum<<3) - sum*sum;

			if (var81 < T1)
			{
				//n = -1;//模糊
				tmp =sum/9;
			}
			else if (var81 > T2)
			{
				//n = 24;//20;//锐化
				//tmp =(25*pYsrc[0] -sum)/16;
				tmp = pYsrc[0] + (pYsrc[0]>>1) +(pYsrc[0]>>4) - (sum>>4);
			}
			else
			{
				//n= 16//15;//锐化：力度最大
				//tmp =(17*pYsrc[0] -sum)/8;
				tmp =(pYsrc[0]<<1) + (pYsrc[0]>>3) - (sum>>3);
			}
			//tmp =((n+1)*pYsrc[0] -sum)/(n-8);

			*pYdst =  tmp<= 0? 0 : tmp >= 255? 255 : tmp;
		}
	}
	//delete pTmp;
	free(pYsrcS);
	free(deltas0);	
	return 0;
}
//kmm2 end add
}  // namespac webrtc
