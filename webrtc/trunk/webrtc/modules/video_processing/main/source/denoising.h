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
*@brief		ͼ��ȥ��
*
*********************************************************************/

#ifndef VPM_DENOISING_H
#define VPM_DENOISING_H

#include "typedefs.h"
#include "video_processing.h"

namespace webrtc {

/**
* @brief	ȥ��
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
	* @brief �����ʼ����
	* @par ����:
	* GVE-SRS-128
	* @param[in] double sigma_color: ��ɫ���ƶȱ�׼����ڵ���0���Ƽ�10.0��30.0��Խ��ͼ��Խģ����
	* @param[in] double sigma_space: �ռ�����׼����ڵ���0���Ƽ�2.0~5.0��Խ��ͼ��Խģ����
	* @returns WebRtc_Word32�ͣ��ɹ�����VPM_OK��
	* @exception �������Ϸ�������VPM_PARAMETER_ERROR��
	* @par ��ϸ������
	�����ʼ������Ҫ��������ǰ��������˫���˲���Ȩ�ز�ѯ�������˫���˲�Ч�ʡ�
	*/
	WebRtc_Word32 InitDenoise(double sigma_color, double sigma_space);

	 /**
  * @brief ˫���˲�������ͼ��ȥ������㷨��
  * @par ����:
  * GVE-SRS-127
  * @param[in] const WebRtc_UWord8* src������ͼ��Yƽ�����ݣ���СӦΪwidth*height�ֽڡ�
  * @param[out] WebRtc_UWord8* dst��������������ͼ��Yƽ�����ݣ���СӦΪwidth*height�ֽڡ�
  * @param[in] int width��ͼ���ȡ�
  * @param[in] int height��ͼ��߶ȡ�
  * @returns WebRtc_Word32�ͣ��ɹ�����VPM_OK��
  * @exception �������Ϸ�������VPM_PARAMETER_ERROR��
  * @par ��ϸ������
  ���㷨��Ҫ����ͼ��˫���˲����ﵽ�����Ŀ�ġ�\n
  ˫���˲�ʵ������ͼ������ģ�����㣬Ҳ��ͼ���Ȩ��ֵ�˲��������ص����ں˵Ĺ��졣���㷨ѡȡ�뾶Ϊ1��Բ�������ںˡ�
  \f[
  g(x,y) = \frac{{\sum\limits_{s =  - a}^a {\sum\limits_{t =  - b}^b {w(s,t)f(x + s,y + t)} } }}{{\sum\limits_{s =  - a}^a {\sum\limits_{s =  - b}^b {w(s,t)} } }}
  \f]
    ��ʽ����������̣��ص���\f${w(s,t)}\f$�Ĺ��졣\n
	���㷨ѡȡ��˹�ںˣ��ۺϿ������ؾ����ɫ�����ƶ��������أ�u��v������ͼ�����أ�
	\f[
	w(u,v) = {e^{ - \frac{1}{2}{{(\frac{{d(u,v)}}{{{\sigma _d}}})}^2}}} \cdot {e^{ - \frac{1}{2}{{(\frac{{\delta (u,v)}}{{{\sigma _r}}})}^2}}}{\rm{ = }}{e^{ - \frac{1}{2}\left[ {\frac{{d{{(u,v)}^2}}}{{{\sigma _r}^2}} + \frac{{\delta {{(u,v)}^2}}}{{{\sigma _r}^2}}} \right]}}
	\f]
	�ռ���룺\f$d(u,v) = \left| {u - v} \right|{\rm{ = }}\left| {{x_u} - {x_v}} \right| + \left| {{y_u} - {y_v}} \right| \f$��x��yΪ��������ֵ��\n
	��ɫ���ƶȣ�\f$\delta (u,v) = \left| {f(u) - f(v)} \right| = \left| {{Y_u} - {Y_v}} \right|\f$��YΪ��������ֵ��\n
	�ռ�����׼�\f${\sigma _d}\f$����InitDenoise()�Ĳ���sigma_space��\n
	��ɫ���ƶȱ�׼�\f${\sigma _r}\f$����InitDenoise()�Ĳ���sigma_color��\n
  */
	WebRtc_Word32 BilateralFilter(const WebRtc_UWord8* src,	WebRtc_UWord8* dst,	int width, int height);//kmm add


  /**
  * @brief ͼ��ȥ����������
  * @par ����:
  * GVE-SRS-127
  * @param[in,out] I420VideoFrame* frame������I420��ʽ��һ֡ͼ�����ݣ��������ȥ�봦���I420��ʽ��һ֡ͼ�����ݡ�
  * @returns WebRtc_Word32�ͣ��ɹ�����VPM_OK��
  * @exception �������Ϸ�������VPM_PARAMETER_ERROR��
  * @par ��ϸ������
  1.�ں�Ȩ�ر��⣬��δ�����ں�Ȩ�ر�����InitDenoise()�����ں�Ȩ�ر�sigma_colorȡֵ20.0��sigma_colorȡֵ3.0��\n
  2.�����ں˲�������deltas[8];\n
  3.��ȡԭͼI420���ݵ�Yƽ�����ݣ�\n
  4.��Yƽ������BilateralFilter()�������Σ����ε������˫���˲���
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
  
