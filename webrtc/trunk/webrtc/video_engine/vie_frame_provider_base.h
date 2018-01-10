/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_VIDEO_ENGINE_VIE_FRAME_PROVIDER_BASE_H_
#define WEBRTC_VIDEO_ENGINE_VIE_FRAME_PROVIDER_BASE_H_

#include <vector>

#include "common_types.h"  // NOLINT
#include "system_wrappers/interface/scoped_ptr.h"
#include "typedefs.h"  // NOLINT

namespace webrtc {

class CriticalSectionWrapper;
class VideoEncoder;
class I420VideoFrame;

// ViEFrameCallback shall be implemented by all classes receiving frames from a
// frame provider.
class ViEFrameCallback {
 public:
  virtual void DeliverFrame(int id,
                            I420VideoFrame* video_frame,
                            int num_csrcs = 0,
                            const WebRtc_UWord32 CSRC[kRtpCsrcSize] = NULL) = 0;

  // The capture delay has changed from the provider. |frame_delay| is given in
  // ms.
  virtual void DelayChanged(int id, int frame_delay) = 0;

  // Get the width, height and frame rate preferred by this observer.
  virtual int GetPreferedFrameSettings(int* width,
                                       int* height,
                                       int* frame_rate) = 0;

  // ProviderDestroyed is called when the frame is about to be destroyed. There
  // must not be any more calls to the frame provider after this.
  virtual void ProviderDestroyed(int id) = 0;

  //added by wuzhong 20130716 for local render image processing
  // 0 ViERenderer
  // 1 ViEEncoder
  // 2 ViECaptureSnapshot
  virtual int GetType()=0;

  virtual ~ViEFrameCallback() {}
};

// ViEFrameProviderBase is a base class that will deliver frames to all
// registered ViEFrameCallbacks.
class ViEFrameProviderBase {
 public:
  ViEFrameProviderBase(int Id, int engine_id);
  virtual ~ViEFrameProviderBase();

  // Returns the frame provider id.
  int Id();

  // Register frame callbacks, i.e. a receiver of the captured frame.
  virtual int RegisterFrameCallback(int observer_id,
                                    ViEFrameCallback* callback_object);

  virtual int DeregisterFrameCallback(const ViEFrameCallback* callback_object);

  virtual bool IsFrameCallbackRegistered(
      const ViEFrameCallback* callback_object);

  int NumberOfRegisteredFrameCallbacks();

  // FrameCallbackChanged
  // Inherited classes should check for new frame_settings and reconfigure
  // output if possible.
  virtual int FrameCallbackChanged() = 0;

 protected:
	 /**
	 * @brief 修改WebRTC原有代码，增加图像后处理，图像后处理主函数。
	 * @par 需求：
	 GVE-SRS-148
	 * @param[in] I420VideoFrame* video_frame：输入I420格式的一帧图像数据。
	 * @param[in] int num_csrcs = 0
	 * @param[in] const WebRtc_UWord32 CSRC[kRtpCsrcSize] = NULL
	 * @returns   void
	 * @par 详细描述：
	 此函数基于WebRTC原有函数修改，在图像数据即将渲染之前，对图像数据进行了后处理，其中包含对比度增强、颜色增强、锐化。
	 */
	 void DeliverFrame(I420VideoFrame* video_frame,
		 int num_csrcs = 0,
		 const WebRtc_UWord32 CSRC[kRtpCsrcSize] = NULL);
  void SetFrameDelay(int frame_delay);
  int FrameDelay();
  int GetBestFormat(int* best_width,
                    int* best_height,
                    int* best_frame_rate);

  int id_;
  int engine_id_;

  // Frame callbacks.
  typedef std::vector<ViEFrameCallback*> FrameCallbacks;
  FrameCallbacks frame_callbacks_;
  scoped_ptr<CriticalSectionWrapper> provider_cs_;

 private:
  scoped_ptr<I420VideoFrame> extra_frame_;
  int frame_delay_;

  //kmm2 add
  bool colorEnhance_enable_;//图像增强开关变量
  bool contrastEnhance_enable_;
  bool sharpen_enable_;
  /**
  * @brief    对比度增强初始化
  * @par 需求：
  GVE-SRS-150
  * @param[in]     float deltaC: 取值范围大于-100
  * - deltaC>0，表示对比度提升百分之deltaC。
  * - deltaC<0，表示对比度降低百分之deltaC。
  * - deltaC=0，对比度不变。
  * @returns   int型，成功返回0，参数非法返回-1。
  * @par 详细描述：
  对比度初始化的主要目的是生成对比度增强映射表contrastTable[][]，行下标表示图像整体亮度均值，列下标表示要映射的像素亮度值。\n
  contrastTable[][]按以下公式生成：
  \f[
  contrastTable[mean][i] = \left\{ {\begin{array}{*{20}{c}}
  {\frac{i}{{deltaC}},i < \frac{{deltaC \times mean}}{{1 + deltaC}}}\\
  {mean + (i - mean) \times deltaC,i \in {\rm{other}}}\\
  {\frac{{i - 255}}{{deltaC}} + 255,i > 255 + \frac{{deltaC \times mean}}{{1 + deltaC}}}
  \end{array}} \right.
  \f]
  */
  int InitContrastTable(float deltaC);

  /**
  * @brief 对比度增强映射表。行：均值；列：亮度值。
  */
  WebRtc_UWord8 contrastTable[256][256];

  /**
  * @brief    对比度增强主函数
  * @par 需求：
  GVE-SRS-149
  * @param[in,out] I420VideoFrame* video_frame：输入I420格式的一帧图像数据，输出经过对比度增强的I420格式的一帧图像数据。
  * @returns   int型，成功返回0，参数非法返回-1，未完成对比度初始化返回-1。
  * @par 详细描述：
  1.对比度映射表检测，若未生成，调用InitContrastTable()生成对比度映射表，deltaC取值25；\n
  2.提取原图I420数据的Y平面数据；\n
  3.计算原图的亮度平均值；\n
  4.根据原图的亮度平均值和对比度映射表，将原有亮度值映射出新的亮度值。
  */
  int  I420ContrastEnhance(I420VideoFrame* video_frame);//对比度增强

  /**
  * @brief    锐化主函数
  * @par 需求：
  GVE-SRS-151
  * @param[in,out] I420VideoFrame* video_frame：输入I420格式的一帧图像数据，输出经过锐化的I420格式的一帧图像数据。
  * @returns   int型，成功返回0，参数非法返回-1。
  * @par 详细描述：
  本锐化算法是带有噪声抑制功能的，属于图像邻域模板运算，也是图像加权均值滤波，核心思想是在不同区域运用不同的锐化模板。本算法选取3*3的正方形模板。
  \f[
  g(x,y) = \frac{{\sum\limits_{s =  - a}^a {\sum\limits_{t =  - b}^b {w(s,t)f(x + s,y + t)} } }}{{\sum\limits_{s =  - a}^a {\sum\limits_{s =  - b}^b {w(s,t)} } }}
  \f]
  上式是其运算过程，重点是\f${w(s,t)}\f$的构造。\n
  \f[
  w(s,t) = \left[ {\begin{array}{*{20}{c}}
  { - 1}&{ - 1}&{ - 1}\\
  { - 1}&n&{ - 1}\\
  { - 1}&{ - 1}&{ - 1}
  \end{array}} \right]
  \f]
  本算法根据邻域内亮度值的方差var，n按以下式子取值：
  \f[
  n = \left\{ {\begin{array}{*{20}{c}}
  { - 1,var < 50}\\
  {20,var > 200}\\
  {15,var \in {\rm{other}}}
  \end{array}} \right.
  \f]
  参数n用来控制锐化力度：
  - n>=9时，n越小，锐化力度越大
  - n<0时，模板转化为平滑模板，n越小，越接近原图
  - n=-1，为均值滤波器
  */
  int I420SharpenMask(I420VideoFrame* video_frame);//锐化：带噪声抑制功能
public:
	/**
	* @brief    渲染类开启/关闭色彩增强
	* @par 需求：
	* GVE-SRS-137
	* @param[in] bool enable: 开关标志位，置true为打开，置false为关闭。
	* @returns   WebRtc_Word32类型，返回0
	*/
	WebRtc_Word32 EnableColorEnhancement(bool enable);//开启或关闭图像增强开关

	/**
	* @brief    渲染类获取色彩增强开关是否打开
	* @par 需求：
	* GVE-SRS-145
	* @returns   bool类型，色彩增强打开返回true，关闭返回false。
	*/
	bool GetEnableColorEnhancementState();//得到图像增强开关状态

	/**
	* @brief    渲染类开启/关闭对比度增强
	* @par 需求：
	* GVE-SRS-135
	* @param[in] bool enable: 开关标志位，置true为打开，置false为关闭。
	* @returns   WebRtc_Word32类型，返回0
	*/
	WebRtc_Word32 EnableContrastEnhancement(bool enable);

	/**
	* @brief    渲染类获取对比度增强开关是否打开
	* @par 需求：
	* GVE-SRS-143
	* @returns   bool类型，色对比度增强打开返回true，关闭返回false。
	*/
	bool GetEnableContrastEnhancementState();

	/**
	* @brief    渲染类开启/关闭锐化
	* @par 需求：
	* GVE-SRS-139
	* @param[in] bool enable: 开关标志位，置true为打开，置false为关闭。
	* @returns   WebRtc_Word32类型，返回0
	*/
	WebRtc_Word32 EnableSharpen(bool enable);

	/**
	* @brief    渲染类获取锐化开关是否打开
	* @par 需求：
	* GVE-SRS-147
	* @returns   bool类型，锐化打开返回true，关闭返回false。
	*/
	bool GetEnableSharpenState();
	//kmm2 end add
};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_VIE_FRAME_PROVIDER_BASE_H_
