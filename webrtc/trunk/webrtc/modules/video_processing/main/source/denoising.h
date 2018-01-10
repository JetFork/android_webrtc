/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

/********************************************************************
*@file		denoising.h
*	
*@brief		图像去噪
*
*********************************************************************/

#ifndef VPM_DENOISING_H
#define VPM_DENOISING_H

#include "typedefs.h"
#include "video_processing.h"

namespace webrtc {

/**
* @brief	去噪
*/
class VPMDenoising
{

public:
    VPMDenoising();
    ~VPMDenoising();

    WebRtc_Word32 ChangeUniqueId(WebRtc_Word32 id);

    void Reset();
#ifndef GXH_TEST_H264	
	/**
	* @brief 降噪初始化。
	* @par 需求:
	* GVE-SRS-128
	* @param[in] double sigma_color: 颜色相似度标准差，大于等于0，推荐10.0～30.0，越大图像越模糊。
	* @param[in] double sigma_space: 空间距离标准差，大于等于0，推荐2.0~5.0，越大图像越模糊。
	* @returns WebRtc_Word32型，成功返回VPM_OK。
	* @exception 参数不合法，返回VPM_PARAMETER_ERROR。
	* @par 详细描述：
	降噪初始化的主要任务是提前计算生成双边滤波的权重查询表，以提高双边滤波效率。
	*/
	WebRtc_Word32 InitDenoise(double sigma_color, double sigma_space);

	 /**
  * @brief 双边滤波函数，图像去噪核心算法。
  * @par 需求:
  * GVE-SRS-127
  * @param[in] const WebRtc_UWord8* src：输入图像Y平面数据，大小应为width*height字节。
  * @param[out] WebRtc_UWord8* dst：输出经过降噪的图像Y平面数据，大小应为width*height字节。
  * @param[in] int width：图像宽度。
  * @param[in] int height：图像高度。
  * @returns WebRtc_Word32型，成功返回VPM_OK。
  * @exception 参数不合法，返回VPM_PARAMETER_ERROR。
  * @par 详细描述：
  本算法主要利用图像双边滤波来达到降噪的目的。\n
  双边滤波实质属于图像邻域模板运算，也是图像加权均值滤波，所以重点是内核的构造。本算法选取半径为1的圆形邻域内核。
  \f[
  g(x,y) = \frac{{\sum\limits_{s =  - a}^a {\sum\limits_{t =  - b}^b {w(s,t)f(x + s,y + t)} } }}{{\sum\limits_{s =  - a}^a {\sum\limits_{s =  - b}^b {w(s,t)} } }}
  \f]
    上式是其运算过程，重点是\f${w(s,t)}\f$的构造。\n
	本算法选取高斯内核，综合考虑像素距离和色彩相似度两个因素，u、v是两个图像像素：
	\f[
	w(u,v) = {e^{ - \frac{1}{2}{{(\frac{{d(u,v)}}{{{\sigma _d}}})}^2}}} \cdot {e^{ - \frac{1}{2}{{(\frac{{\delta (u,v)}}{{{\sigma _r}}})}^2}}}{\rm{ = }}{e^{ - \frac{1}{2}\left[ {\frac{{d{{(u,v)}^2}}}{{{\sigma _r}^2}} + \frac{{\delta {{(u,v)}^2}}}{{{\sigma _r}^2}}} \right]}}
	\f]
	空间距离：\f$d(u,v) = \left| {u - v} \right|{\rm{ = }}\left| {{x_u} - {x_v}} \right| + \left| {{y_u} - {y_v}} \right| \f$。x，y为像素坐标值。\n
	颜色相似度：\f$\delta (u,v) = \left| {f(u) - f(v)} \right| = \left| {{Y_u} - {Y_v}} \right|\f$。Y为像素亮度值。\n
	空间距离标准差：\f${\sigma _d}\f$，即InitDenoise()的参数sigma_space。\n
	颜色相似度标准差：\f${\sigma _r}\f$，即InitDenoise()的参数sigma_color。\n
  */
	WebRtc_Word32 BilateralFilter(const WebRtc_UWord8* src,	WebRtc_UWord8* dst,	int width, int height);//kmm add


  /**
  * @brief 图像去噪主函数。
  * @par 需求:
  * GVE-SRS-127
  * @param[in,out] I420VideoFrame* frame：输入I420格式的一帧图像数据，输出经过去噪处理的I420格式的一帧图像数据。
  * @returns WebRtc_Word32型，成功返回VPM_OK。
  * @exception 参数不合法，返回VPM_PARAMETER_ERROR。
  * @par 详细描述：
  1.内核权重表检测，若未生成内核权重表，调用InitDenoise()生成内核权重表，sigma_color取值20.0，sigma_color取值3.0；\n
  2.生成内核步长矩阵deltas[8];\n
  3.提取原图I420数据的Y平面数据；\n
  4.将Y平面输入BilateralFilter()处理两次，两次迭代完成双边滤波。
  */
	WebRtc_Word32 UserProcessFrame(I420VideoFrame* frame);



#endif
    WebRtc_Word32 ProcessFrame(I420VideoFrame* frame);

private:
    WebRtc_Word32 _id;

    WebRtc_UWord32*   _moment1;           // (Q8) First order moment (mean)
    WebRtc_UWord32*   _moment2;           // (Q8) Second order moment
    WebRtc_UWord32    _frameSize;         // Size (# of pixels) of frame
    int               _denoiseFrameCnt;   // Counter for subsampling in time
#ifndef GXH_TEST_H264
// 	double color_weight[766];
// 	double space_weight[2];
	double weightEven[256];//kmm add
	double weightMulti[256][256];//kmm add
	WebRtc_Word32 deltas[8];
	bool inited;
#endif
};

} //namespace

#endif // VPM_DENOISING_H
  
