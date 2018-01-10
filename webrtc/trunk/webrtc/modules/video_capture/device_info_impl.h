/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_DEVICE_INFO_IMPL_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_DEVICE_INFO_IMPL_H_

#include "video_capture.h"

#include "map_wrapper.h"
#include "rw_lock_wrapper.h"
#include "video_capture_delay.h"

namespace webrtc
{
namespace videocapturemodule
{
class DeviceInfoImpl: public VideoCaptureModule::DeviceInfo
{
public:
    DeviceInfoImpl(const WebRtc_Word32 id);
    virtual ~DeviceInfoImpl(void);
    virtual WebRtc_Word32 NumberOfCapabilities(const char* deviceUniqueIdUTF8);
    virtual WebRtc_Word32 GetCapability(
        const char* deviceUniqueIdUTF8,
        const WebRtc_UWord32 deviceCapabilityNumber,
        VideoCaptureCapability& capability);

    virtual WebRtc_Word32 GetBestMatchedCapability(
        const char* deviceUniqueIdUTF8,
        const VideoCaptureCapability& requested,
        VideoCaptureCapability& resulting);
	virtual WebRtc_Word32 GetOrientation(
		const char* deviceUniqueIdUTF8,
		VideoCaptureRotation& orientation);

	//kmm add
	/**
	* @brief    采集分辨率自适应，目的是为了采集图像。根据编码设定分辨率在设备支持的采集分辨率中选取。\n
	改写webrtc::DeviceInfoImpl::GetBestMatchedCapability()。
	* @par 需求：
	GVE-SRS-133
	* @param[in,out] int &width：输入设定编码分辨率宽，输出经过自适应的采集分辨率宽。
	* @param[in,out] int &height：输入设定编码分辨率高，输出经过自适应的采集分辨率高。
	* @returns int型, 成功返回0，失败返回-1。
	* @par 详细描述：
	采集分辨率选择策略如下：\n
	1.若编码设定分辨率(W, H)是设备支持的采集分辨率，则实际采集分辨率(Wi, Hi) = (W, H)；\n
	2.若条件1不成立，遍历设备支持的采集分辨率，选取宽和高都大于编码设定分辨率(W, H)的宽和高，且面积最小；\n
	3.若条件1和2都不成立，如果编码设定分辨率(W, H)比设备支持的最大采集分辨率的宽或高其中一个大，在设备支持的采集分辨率中选取宽和高都小于(W, H)，且面积最接近W*H；\n
	4.上述条件均不成立的，则取设备支持的最大采集分辨率。\n
	*/
	int CaptureResolutionAdapt(int &width, int &height);

protected:
    /* Initialize this object*/

    virtual WebRtc_Word32 Init()=0;
    /*
     * Fills the member variable _captureCapabilities with capabilities for the given device name.
     */
    virtual WebRtc_Word32 CreateCapabilityMap(const char* deviceUniqueIdUTF8)=0;

    /* Returns the expected Capture delay*/
    WebRtc_Word32 GetExpectedCaptureDelay(const DelayValues delayValues[],
                                          const WebRtc_UWord32 sizeOfDelayValues,
                                          const char* productId,
                                          const WebRtc_UWord32 width,
                                          const WebRtc_UWord32 height);
protected:
    // Data members
    WebRtc_Word32 _id;
    MapWrapper _captureCapabilities;
    RWLockWrapper& _apiLock;
    char* _lastUsedDeviceName;
    WebRtc_UWord32 _lastUsedDeviceNameLength;
};
} //namespace videocapturemodule
} // namespace webrtc
#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_DEVICE_INFO_IMPL_H_
