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
	 * @brief �޸�WebRTCԭ�д��룬����ͼ�����ͼ�������������
	 * @par ����
	 GVE-SRS-148
	 * @param[in] I420VideoFrame* video_frame������I420��ʽ��һ֡ͼ�����ݡ�
	 * @param[in] int num_csrcs = 0
	 * @param[in] const WebRtc_UWord32 CSRC[kRtpCsrcSize] = NULL
	 * @returns   void
	 * @par ��ϸ������
	 �˺�������WebRTCԭ�к����޸ģ���ͼ�����ݼ�����Ⱦ֮ǰ����ͼ�����ݽ����˺������а����Աȶ���ǿ����ɫ��ǿ���񻯡�
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
  bool colorEnhance_enable_;//ͼ����ǿ���ر���
  bool contrastEnhance_enable_;
  bool sharpen_enable_;
  /**
  * @brief    �Աȶ���ǿ��ʼ��
  * @par ����
  GVE-SRS-150
  * @param[in]     float deltaC: ȡֵ��Χ����-100
  * - deltaC>0����ʾ�Աȶ������ٷ�֮deltaC��
  * - deltaC<0����ʾ�ԱȶȽ��Ͱٷ�֮deltaC��
  * - deltaC=0���ԱȶȲ��䡣
  * @returns   int�ͣ��ɹ�����0�������Ƿ�����-1��
  * @par ��ϸ������
  �Աȶȳ�ʼ������ҪĿ�������ɶԱȶ���ǿӳ���contrastTable[][]�����±��ʾͼ���������Ⱦ�ֵ�����±��ʾҪӳ�����������ֵ��\n
  contrastTable[][]�����¹�ʽ���ɣ�
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
  * @brief �Աȶ���ǿӳ����У���ֵ���У�����ֵ��
  */
  WebRtc_UWord8 contrastTable[256][256];

  /**
  * @brief    �Աȶ���ǿ������
  * @par ����
  GVE-SRS-149
  * @param[in,out] I420VideoFrame* video_frame������I420��ʽ��һ֡ͼ�����ݣ���������Աȶ���ǿ��I420��ʽ��һ֡ͼ�����ݡ�
  * @returns   int�ͣ��ɹ�����0�������Ƿ�����-1��δ��ɶԱȶȳ�ʼ������-1��
  * @par ��ϸ������
  1.�Աȶ�ӳ����⣬��δ���ɣ�����InitContrastTable()���ɶԱȶ�ӳ���deltaCȡֵ25��\n
  2.��ȡԭͼI420���ݵ�Yƽ�����ݣ�\n
  3.����ԭͼ������ƽ��ֵ��\n
  4.����ԭͼ������ƽ��ֵ�ͶԱȶ�ӳ�����ԭ������ֵӳ����µ�����ֵ��
  */
  int  I420ContrastEnhance(I420VideoFrame* video_frame);//�Աȶ���ǿ

  /**
  * @brief    ��������
  * @par ����
  GVE-SRS-151
  * @param[in,out] I420VideoFrame* video_frame������I420��ʽ��һ֡ͼ�����ݣ���������񻯵�I420��ʽ��һ֡ͼ�����ݡ�
  * @returns   int�ͣ��ɹ�����0�������Ƿ�����-1��
  * @par ��ϸ������
  �����㷨�Ǵ����������ƹ��ܵģ�����ͼ������ģ�����㣬Ҳ��ͼ���Ȩ��ֵ�˲�������˼�����ڲ�ͬ�������ò�ͬ����ģ�塣���㷨ѡȡ3*3��������ģ�塣
  \f[
  g(x,y) = \frac{{\sum\limits_{s =  - a}^a {\sum\limits_{t =  - b}^b {w(s,t)f(x + s,y + t)} } }}{{\sum\limits_{s =  - a}^a {\sum\limits_{s =  - b}^b {w(s,t)} } }}
  \f]
  ��ʽ����������̣��ص���\f${w(s,t)}\f$�Ĺ��졣\n
  \f[
  w(s,t) = \left[ {\begin{array}{*{20}{c}}
  { - 1}&{ - 1}&{ - 1}\\
  { - 1}&n&{ - 1}\\
  { - 1}&{ - 1}&{ - 1}
  \end{array}} \right]
  \f]
  ���㷨��������������ֵ�ķ���var��n������ʽ��ȡֵ��
  \f[
  n = \left\{ {\begin{array}{*{20}{c}}
  { - 1,var < 50}\\
  {20,var > 200}\\
  {15,var \in {\rm{other}}}
  \end{array}} \right.
  \f]
  ����n�������������ȣ�
  - n>=9ʱ��nԽС��������Խ��
  - n<0ʱ��ģ��ת��Ϊƽ��ģ�壬nԽС��Խ�ӽ�ԭͼ
  - n=-1��Ϊ��ֵ�˲���
  */
  int I420SharpenMask(I420VideoFrame* video_frame);//�񻯣����������ƹ���
public:
	/**
	* @brief    ��Ⱦ�࿪��/�ر�ɫ����ǿ
	* @par ����
	* GVE-SRS-137
	* @param[in] bool enable: ���ر�־λ����trueΪ�򿪣���falseΪ�رա�
	* @returns   WebRtc_Word32���ͣ�����0
	*/
	WebRtc_Word32 EnableColorEnhancement(bool enable);//������ر�ͼ����ǿ����

	/**
	* @brief    ��Ⱦ���ȡɫ����ǿ�����Ƿ��
	* @par ����
	* GVE-SRS-145
	* @returns   bool���ͣ�ɫ����ǿ�򿪷���true���رշ���false��
	*/
	bool GetEnableColorEnhancementState();//�õ�ͼ����ǿ����״̬

	/**
	* @brief    ��Ⱦ�࿪��/�رնԱȶ���ǿ
	* @par ����
	* GVE-SRS-135
	* @param[in] bool enable: ���ر�־λ����trueΪ�򿪣���falseΪ�رա�
	* @returns   WebRtc_Word32���ͣ�����0
	*/
	WebRtc_Word32 EnableContrastEnhancement(bool enable);

	/**
	* @brief    ��Ⱦ���ȡ�Աȶ���ǿ�����Ƿ��
	* @par ����
	* GVE-SRS-143
	* @returns   bool���ͣ�ɫ�Աȶ���ǿ�򿪷���true���رշ���false��
	*/
	bool GetEnableContrastEnhancementState();

	/**
	* @brief    ��Ⱦ�࿪��/�ر���
	* @par ����
	* GVE-SRS-139
	* @param[in] bool enable: ���ر�־λ����trueΪ�򿪣���falseΪ�رա�
	* @returns   WebRtc_Word32���ͣ�����0
	*/
	WebRtc_Word32 EnableSharpen(bool enable);

	/**
	* @brief    ��Ⱦ���ȡ�񻯿����Ƿ��
	* @par ����
	* GVE-SRS-147
	* @returns   bool���ͣ��񻯴򿪷���true���رշ���false��
	*/
	bool GetEnableSharpenState();
	//kmm2 end add
};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_VIE_FRAME_PROVIDER_BASE_H_
