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
*@brief		图像处理
*
*********************************************************************/

/** 
* @defgroup	API_ImgPreProcess
* @brief	API和webRTC前处理之间的函数接口。\n
前处理是采集端的功能，包含：分辨率自适应和降噪。在设备采集一帧图像之后即刻进行。
*/

/** 
* @defgroup	API_ImgPostProcess
* @brief	API和webRTC后处理之间的函数接口。\n
后处理是渲染端的功能，包含：渲染分辨率处理，对比对增强，色彩增强，锐化。在渲染之前进行。
*/
#ifndef WEBRTC_VIDEO_ENGINE_VIE_IMAGE_PROCESS_IMPL_H_
#define WEBRTC_VIDEO_ENGINE_VIE_IMAGE_PROCESS_IMPL_H_

#include "typedefs.h"  // NOLINT
#include "video_engine/include/vie_image_process.h"
#include "video_engine/vie_ref_count.h"

namespace webrtc {

	class ViESharedData;

	/** 
	* @brief	图像处理相关函数
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
		* @brief    WebRTC原有接口：打开/关闭图像降噪。此接口是WebRTC原有接口，但改变了底层降噪算法。
		* @param[in] const int capture_id: WebRTC采集对象id。
		* @param[in] const bool enable: 降噪开关标志位，置true为打开，置false为关闭。
		* @returns   int型，成功返回0，失败返回-1。
		* @exception 1.capture_id无效，返回-1；2.调用采集类降噪开关失败，返回-1。
		*/
		virtual int EnableDenoising(const int capture_id, const bool enable);

		/**
		* @ingroup	API_ImgPreProcess
		* @brief    新增接口：获取图像降噪开关是否打开。
		* @par 需求:
		* GVE-SRS-129
		* @param[in] const int capture_id: WebRTC采集对象id。
		* @returns   int型，降噪打开返回1，关闭返回0，获取失败返回-1。
		* @exception capture_id无效，返回-1。
		*/
		virtual  int GetEnableDenoisingState(const int capture_id);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    新增接口：打开/关闭色彩增强。
		* @par 需求:
		* GVE-SRS-136
		* @param[in] const int render_id: WebRTC渲染对象id，可以是capture id或者channel id。
		* @param[in] const bool enable: 色彩增强开关标志位，置true为打开，置false为关闭。
		* @returns   int型，成功返回0，失败返回-1。。
		* @exception capture_id无效，返回-1。
		*/
		virtual int EnableColorEnhancement(const int render_id, const bool enable);

		/**
		* @ingroup	API_ImgPostProcess
		* @brief    新增接口：获取图像色彩增强开关是否打开。
		* @par 需求:
		* GVE-SRS-144
		* @param[in] const int render_id: WebRTC渲染对象id，可以是capture id或者channel id。
		* @returns   int型，色彩增强打开返回1，关闭返回0，获取失败返回-1。
		* @exception render_id无效，返回-1。
		*/
		virtual  int GetEnableColorEnhancementState(const int render_id);//kmm add

		/**
		* @ingroup	API_ImgPostProcess
		* @brief    新增接口：打开/关闭对比度增强。
		* @par 需求:
		* GVE-SRS-134
		* @param[in] const int render_id: WebRTC渲染对象id，可以是capture id或者channel id。
		* @param[in] const bool enable: 对比度增强开关标志位，置true为打开，置false为关闭。
		* @returns  int型，成功返回0，失败返回-1。
		* @exception capture_id无效，返回-1。
		*/
		virtual int EnableContrastEnhancement(const int render_id, const bool enable);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    新增接口：获取图像对比度增强开关是否打开。
		* @par 需求:
		* GVE-SRS-142
		* @param[in] const int render_id: WebRTC渲染对象id，可以是capture id或者channel id。
		* @returns   int型，对比度增强打开返回1，关闭返回0，获取失败返回-1。
		* @exception render_id无效，返回-1。
		*/
		virtual  int GetEnableContrastEnhancementState(const int render_id);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    新增接口：打开/关闭锐化。
		* @par 需求:
		* GVE-SRS-138
		* @param[in] const int render_id: WebRTC渲染对象id，可以是capture id或者channel id。
		* @param[in] const bool enable: 锐化开关标志位，置true为打开，置false为关闭。
		* @returns   int型，成功返回0，失败返回-1。
		* @exception capture_id无效，返回-1。
		*/
		virtual int EnableSharpen(const int render_id, const bool enable);//kmm add


		/**
		* @ingroup	API_ImgPostProcess
		* @brief    新增接口：获取图像锐化开关是否打开。
		* @par 需求:
		* GVE-SRS-146
		* @param[in] const int render_id: WebRTC渲染对象id，可以是capture id或者channel id。
		* @returns   int型，锐化打开返回1，关闭返回0，获取失败返回-1。
		* @exception render_id无效，返回-1。
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
