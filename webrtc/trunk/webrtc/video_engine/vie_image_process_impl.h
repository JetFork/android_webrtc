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
*@file		vie_image_process_impl.h
*	
*@brief		ͼ����
*
*********************************************************************/

/** 
* @defgroup	API_ImgPreProcess
* @brief	API��webRTCǰ����֮��ĺ����ӿڡ�\n
ǰ�����ǲɼ��˵Ĺ��ܣ��������ֱ�������Ӧ�ͽ��롣���豸�ɼ�һ֡ͼ��֮�󼴿̽��С�
*/

/** 
* @defgroup	API_ImgPostProcess
* @brief	API��webRTC����֮��ĺ����ӿڡ�\n
��������Ⱦ�˵Ĺ��ܣ���������Ⱦ�ֱ��ʴ����Աȶ���ǿ��ɫ����ǿ���񻯡�����Ⱦ֮ǰ���С�
*/
#ifndef WEBRTC_VIDEO_ENGINE_VIE_IMAGE_PROCESS_IMPL_H_
#define WEBRTC_VIDEO_ENGINE_VIE_IMAGE_PROCESS_IMPL_H_

#include "typedefs.h"  // NOLINT
#include "video_engine/include/vie_image_process.h"
#include "video_engine/vie_ref_count.h"

namespace webrtc {

	class ViESharedData;

	/** 
	* @brief	ͼ������غ���
	*/
	class ViEImageProcessImpl
		: public ViEImageProcess,
		public ViERefCount {
	public:
		// Implements ViEImageProcess.
		virtual int Release();
		virtual int RegisterCaptureEffectFilter(const int capture_id,
			ViEEffectFilter& capture_filter);
		virtual int DeregisterCaptureEffectFilter(const int capture_id);
		virtual int RegisterSendEffectFilter(const int video_channel,
			ViEEffectFilter& send_filter);
		virtual int DeregisterSendEffectFilter(const int video_channel);
		virtual int RegisterRenderEffectFilter(const int video_channel,
			ViEEffectFilter& render_filter);
		virtual int DeregisterRenderEffectFilter(const int video_channel);
		virtual int EnableDeflickering(const int capture_id, const bool enable);
		/**
		* @ingroup	API_ImgPreProcess
		* @brief    WebRTCԭ�нӿڣ���/�ر�ͼ���롣�˽ӿ���WebRTCԭ�нӿڣ����ı��˵ײ㽵���㷨��
		* @param[in] const int capture_id: WebRTC�ɼ�����id��
		* @param[in] const bool enable: ���뿪�ر�־λ����trueΪ�򿪣���falseΪ�رա�
		* @returns   int�ͣ��ɹ�����0��ʧ�ܷ���-1��
		* @exception 1.capture_id��Ч������-1��2.���òɼ��ཱུ�뿪��ʧ�ܣ�����-1��
		*/
		virtual int EnableDenoising(const int capture_id, const bool enable);

		/**
		* @ingroup	API_ImgPreProcess
		* @brief    �����ӿڣ���ȡͼ���뿪���Ƿ�򿪡�
		* @par ����:
		* GVE-SRS-129
		* @param[in] const int capture_id: WebRTC�ɼ�����id��
		* @returns   int�ͣ�����򿪷���1���رշ���0����ȡʧ�ܷ���-1��
		* @exception capture_id��Ч������-1��
		*/
		virtual  int GetEnableDenoisingState(const int capture_id);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    �����ӿڣ���/�ر�ɫ����ǿ��
		* @par ����:
		* GVE-SRS-136
		* @param[in] const int render_id: WebRTC��Ⱦ����id��������capture id����channel id��
		* @param[in] const bool enable: ɫ����ǿ���ر�־λ����trueΪ�򿪣���falseΪ�رա�
		* @returns   int�ͣ��ɹ�����0��ʧ�ܷ���-1����
		* @exception capture_id��Ч������-1��
		*/
		virtual int EnableColorEnhancement(const int render_id, const bool enable);

		/**
		* @ingroup	API_ImgPostProcess
		* @brief    �����ӿڣ���ȡͼ��ɫ����ǿ�����Ƿ�򿪡�
		* @par ����:
		* GVE-SRS-144
		* @param[in] const int render_id: WebRTC��Ⱦ����id��������capture id����channel id��
		* @returns   int�ͣ�ɫ����ǿ�򿪷���1���رշ���0����ȡʧ�ܷ���-1��
		* @exception render_id��Ч������-1��
		*/
		virtual  int GetEnableColorEnhancementState(const int render_id);//kmm add

		/**
		* @ingroup	API_ImgPostProcess
		* @brief    �����ӿڣ���/�رնԱȶ���ǿ��
		* @par ����:
		* GVE-SRS-134
		* @param[in] const int render_id: WebRTC��Ⱦ����id��������capture id����channel id��
		* @param[in] const bool enable: �Աȶ���ǿ���ر�־λ����trueΪ�򿪣���falseΪ�رա�
		* @returns  int�ͣ��ɹ�����0��ʧ�ܷ���-1��
		* @exception capture_id��Ч������-1��
		*/
		virtual int EnableContrastEnhancement(const int render_id, const bool enable);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    �����ӿڣ���ȡͼ��Աȶ���ǿ�����Ƿ�򿪡�
		* @par ����:
		* GVE-SRS-142
		* @param[in] const int render_id: WebRTC��Ⱦ����id��������capture id����channel id��
		* @returns   int�ͣ��Աȶ���ǿ�򿪷���1���رշ���0����ȡʧ�ܷ���-1��
		* @exception render_id��Ч������-1��
		*/
		virtual  int GetEnableContrastEnhancementState(const int render_id);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    �����ӿڣ���/�ر��񻯡�
		* @par ����:
		* GVE-SRS-138
		* @param[in] const int render_id: WebRTC��Ⱦ����id��������capture id����channel id��
		* @param[in] const bool enable: �񻯿��ر�־λ����trueΪ�򿪣���falseΪ�رա�
		* @returns   int�ͣ��ɹ�����0��ʧ�ܷ���-1��
		* @exception capture_id��Ч������-1��
		*/
		virtual int EnableSharpen(const int render_id, const bool enable);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    �����ӿڣ���ȡͼ���񻯿����Ƿ�򿪡�
		* @par ����:
		* GVE-SRS-146
		* @param[in] const int render_id: WebRTC��Ⱦ����id��������capture id����channel id��
		* @returns   int�ͣ��񻯴򿪷���1���رշ���0����ȡʧ�ܷ���-1��
		* @exception render_id��Ч������-1��
		*/
		virtual  int GetEnableSharpenState(const int render_id);//kmm add



	protected:
		explicit ViEImageProcessImpl(ViESharedData* shared_data);
		virtual ~ViEImageProcessImpl();

	private:
		ViESharedData* shared_data_;
	};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_VIE_IMAGE_PROCESS_IMPL_H_
