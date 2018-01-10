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
*@brief		ͼ����Ⱦ��ز����ͺ���
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
	* @brief		��Ⱦ�ص�����
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
		* @brief    ��Ⱦ�ֱ��ʴ���Ŀ����Ϊ����Ⱦͼ��
		* @par ����
		GVE-SRS-152
		* @param[in,out] I420VideoFrame* video_frame������I420��ʽ��һ֡ͼ�����ݣ���������񻯵�I420��ʽ��һ֡ͼ�����ݡ�
		* @param[in] int type���������ͣ���0��ʾ������Ⱦ���ԣ���1��ʾԶ����Ⱦ���ԡ�
		* @returns int��, �ɹ�����0��ʧ�ܷ���-1��
		* @par ��ϸ������
		������Ⱦ�������� (type = 0)��\n
		1.���ɼ��ֱ���(Wi, Hi)= �����趨�ֱ���(W, H)����ʵ����Ⱦ�ֱ���(w��h)=(W, H)��\n
		2.���ɼ��ֱ���(Wi, Hi)�ȱ����趨�ֱ���(W, H)�Ŀ�͸߶�����ʵ����Ⱦ�ֱ���(w��h)=(W, H)��ͼ��������ͼ�񼸺�����Ϊ���İ�(w��h)�ü���\n
		3.���ɼ��ֱ���(Wi, Hi)�ȱ����趨�ֱ���(W, H)�Ŀ�͸�����һ������ʵ����Ⱦ�ֱ���(w��h)����������������ͼ��������ͼ�񼸺�����Ϊ���İ�(w��h)�ü���\n
		- (w��h)���豸֧�ֵĲɼ��ֱ�����ѡȡ
		- (w��h)��(W, H)��͸߶�С
		- �����ӽ�W*H
		.
		4.�����������������ģ���ȡ�豸֧�ֵ����ɼ��ֱ��ʡ�\n
		Զ����Ⱦ�������� (type = 1)��\n
		1.���ɼ��ֱ���(Wi, Hi)= �����趨�ֱ���(W, H)����ʵ����Ⱦ�ֱ���(w��h)=(Wi, Hi)��\n
		2.���ɼ��ֱ���(Wi, Hi)�ȱ����趨�ֱ���(W, H)С����ʵ����Ⱦ�ֱ���(w��h)=(W, H)��ͼ�����ݾ����ϲ����Ŵ���(W, H)��\n
		ע���ɼ��ֱ��ʼ��Ǿ���CaptureResolutionAdapt()�ɼ��ֱ�������Ӧ����ķֱ��ʡ�
		*/
		int RenderResolutionProcess(I420VideoFrame* video_frame, int type);
		//kmm end add
		
		
	};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_VIE_RENDERER_H_
