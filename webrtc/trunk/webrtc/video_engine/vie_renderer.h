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
*@file		vie_renderer.h
*file path	e:\work\webrtc_HLD_0516\trunk\webrtc\video_engine
*	
*@brief		图像渲染相关参数和函数
*
*********************************************************************/

#ifndef WEBRTC_VIDEO_ENGINE_VIE_RENDERER_H_
#define WEBRTC_VIDEO_ENGINE_VIE_RENDERER_H_

#include "webrtc/modules/video_render/include/video_render_defines.h"
#include "system_wrappers/interface/map_wrapper.h"
#include "system_wrappers/interface/scoped_ptr.h"
#include "video_engine/include/vie_render.h"
#include "video_engine/vie_frame_provider_base.h"
#include "webrtc/common_video/jpeg/include/jpeg.h"

namespace webrtc {

	class VideoRender;
	class VideoRenderCallback;
	class ViERenderManager;

	class ViEExternalRendererImpl : public VideoRenderCallback {
	public:
		ViEExternalRendererImpl();
		virtual ~ViEExternalRendererImpl() {}

		int SetViEExternalRenderer(ExternalRenderer* external_renderer,
			RawVideoType video_input_format);

		// Implements VideoRenderCallback.
		virtual WebRtc_Word32 RenderFrame(const WebRtc_UWord32 stream_id,
			I420VideoFrame& video_frame);
		void ConfigureRenderMode(const int scaledmode){}

	private:
		ExternalRenderer* external_renderer_;
		RawVideoType external_renderer_format_;
		int external_renderer_width_;
		int external_renderer_height_;
		// Converted_frame_ in color format specified by render_format_.
		scoped_ptr<VideoFrame> converted_frame_;
	};

	/** 
	* @brief		渲染回调函数
	*/
	class ViERenderer: public ViEFrameCallback {
	public:
		static ViERenderer* CreateViERenderer(const WebRtc_Word32 render_id,
			const WebRtc_Word32 engine_id,
			VideoRender& render_module,
			ViERenderManager& render_manager,
			const WebRtc_UWord32 z_order,
			const float left,
			const float top,
			const float right,
			const float bottom);
		~ViERenderer(void);

		 int GetType(){return m_type;}
		WebRtc_Word32 StartRender();
		WebRtc_Word32 StopRender();
		void Snapshot(const char *filename);
		void SetRenderFrameDeliver(RenderFrameDeliver* frameDeliver);
		void ConfigureRenderMode(const int scaledmode);

		WebRtc_Word32 GetLastRenderedFrame(const WebRtc_Word32 renderID,
			I420VideoFrame& video_frame);

		int SetExpectedRenderDelay(int render_delay);

		WebRtc_Word32 ConfigureRenderer(const unsigned int z_order,
			const float left,
			const float top,
			const float right,
			const float bottom);

		VideoRender& RenderModule();

		WebRtc_Word32 EnableMirroring(const WebRtc_Word32 render_id,
			const bool enable,
			const bool mirror_xaxis,
			const bool mirror_yaxis);

		WebRtc_Word32 SetTimeoutImage(const I420VideoFrame& timeout_image,
			const WebRtc_Word32 timeout_value);
		WebRtc_Word32 SetRenderStartImage(const I420VideoFrame& start_image);
		WebRtc_Word32 SetExternalRenderer(const WebRtc_Word32 render_id,
			RawVideoType video_input_format,
			ExternalRenderer* external_renderer);

	private:
		int m_type;		
		ViERenderer(const WebRtc_Word32 render_id, const WebRtc_Word32 engine_id,
			VideoRender& render_module,
			ViERenderManager& render_manager);

		WebRtc_Word32 Init(const WebRtc_UWord32 z_order,
			const float left,
			const float top,
			const float right,
			const float bottom);

		// Implement ViEFrameCallback
		virtual void DeliverFrame(int id,
			I420VideoFrame* video_frame,
			int num_csrcs = 0,
			const WebRtc_UWord32 CSRC[kRtpCsrcSize] = NULL);


		virtual void DelayChanged(int id, int frame_delay);
		virtual int GetPreferedFrameSettings(int* width,
			int* height,
			int* frame_rate);
		virtual void ProviderDestroyed(int id);

		WebRtc_UWord32 render_id_;
		VideoRender& render_module_;
		ViERenderManager& render_manager_;
		VideoRenderCallback* render_callback_;
		ViEExternalRendererImpl* incoming_external_callback_;			
		JpegEncoder *photo_;
		bool is_snapshot_;
		int  scaledmode_; 
		I420VideoFrame render_frame_;		
		RenderFrameDeliver* frame_Deliver_;
		//kmm add
		/**
		* @brief    渲染分辨率处理，目的是为了渲染图像
		* @par 需求：
		GVE-SRS-152
		* @param[in,out] I420VideoFrame* video_frame：输入I420格式的一帧图像数据，输出经过锐化的I420格式的一帧图像数据。
		* @param[in] int type：策略类型，置0表示本地渲染策略，置1表示远程渲染策略。
		* @returns int型, 成功返回0，失败返回-1。
		* @par 详细描述：
		本地渲染策略如下 (type = 0)：\n
		1.若采集分辨率(Wi, Hi)= 编码设定分辨率(W, H)，则实际渲染分辨率(w，h)=(W, H)；\n
		2.若采集分辨率(Wi, Hi)比编码设定分辨率(W, H)的宽和高都大，则实际渲染分辨率(w，h)=(W, H)，图像数据以图像几何中心为中心按(w，h)裁剪；\n
		3.若采集分辨率(Wi, Hi)比编码设定分辨率(W, H)的宽和高其中一个大，则实际渲染分辨率(w，h)需满足以下条件，图像数据以图像几何中心为中心按(w，h)裁剪；\n
		- (w，h)从设备支持的采集分辨率中选取
		- (w，h)比(W, H)宽和高都小
		- 面积最接近W*H
		.
		4.上述条件均不成立的，则取设备支持的最大采集分辨率。\n
		远程渲染策略如下 (type = 1)：\n
		1.若采集分辨率(Wi, Hi)= 编码设定分辨率(W, H)，则实际渲染分辨率(w，h)=(Wi, Hi)；\n
		2.若采集分辨率(Wi, Hi)比编码设定分辨率(W, H)小，则实际渲染分辨率(w，h)=(W, H)，图像数据经过上采样放大至(W, H)。\n
		注：采集分辨率即是经过CaptureResolutionAdapt()采集分辨率自适应输出的分辨率。
		*/
		int RenderResolutionProcess(I420VideoFrame* video_frame, int type);
		//kmm end add
		
		
	};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_VIE_RENDERER_H_
