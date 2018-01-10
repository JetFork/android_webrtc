/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_VIDEO_ENGINE_VIE_INPUT_MANAGER_H_
#define WEBRTC_VIDEO_ENGINE_VIE_INPUT_MANAGER_H_

#include "webrtc/modules/video_capture/include/video_capture.h"
#include "system_wrappers/interface/map_wrapper.h"
#include "system_wrappers/interface/scoped_ptr.h"
#include "typedefs.h"  // NOLINT
#include "video_engine/include/vie_capture.h"
#include "video_engine/vie_defines.h"
#include "video_engine/vie_frame_provider_base.h"
#include "video_engine/vie_manager_base.h"

namespace webrtc {

class CriticalSectionWrapper;
class ProcessThread;
class RWLockWrapper;
class ViECapturer;
class ViEExternalCapture;
class ViEFilePlayer;
class ViEVirtualFilePlayer;
class VoiceEngine;

class ViEInputManager : private ViEManagerBase {
  friend class ViEInputManagerScoped;
 public:
  explicit ViEInputManager(int engine_id);
  ~ViEInputManager();

  void SetModuleProcessThread(ProcessThread* module_process_thread);

  // Returns number of capture devices.
  int NumberOfCaptureDevices();

  // Gets name and id for a capture device.
  int GetDeviceName(WebRtc_UWord32 device_number,
                    char* device_nameUTF8,
                    WebRtc_UWord32 device_name_length,
                    char* device_unique_idUTF8,
                    WebRtc_UWord32 device_unique_idUTF8Length);

  // Returns the number of capture capabilities for a specified device.
  int NumberOfCaptureCapabilities(const char* device_unique_idUTF8);

  // Gets a specific capability for a capture device.
  int GetCaptureCapability(const char* device_unique_idUTF8,
                           const WebRtc_UWord32 device_capability_number,
                           CaptureCapability& capability);

  // Show OS specific Capture settings.
  int DisplayCaptureSettingsDialogBox(const char* device_unique_idUTF8,
                                      const char* dialog_titleUTF8,
                                      void* parent_window,
                                      WebRtc_UWord32 positionX,
                                      WebRtc_UWord32 positionY);
  int GetOrientation(const char* device_unique_idUTF8,
                     RotateCapturedFrame& orientation);

  // Creates a capture module for the specified capture device and assigns
  // a capture device id for the device.
  // Return zero on success, ViEError on failure.
  int CreateCaptureDevice(const char* device_unique_idUTF8,
                          const WebRtc_UWord32 device_unique_idUTF8Length,
                          int& capture_id);
  int CreateCaptureDevice(VideoCaptureModule* capture_module,
                          int& capture_id);
  int CreateExternalCaptureDevice(ViEExternalCapture*& external_capture,
                                  int& capture_id);
  int DestroyCaptureDevice(int capture_id);

  int CreateFilePlayer(const char* file_nameUTF8, const bool loop,
                       const FileFormats file_format,
                       VoiceEngine* voe_ptr,
                       int& file_id);

  int CreateVirtualDataProvide(const char* file_nameUTF8, const bool loop,
	  const FileFormats file_format,/*
	  VoiceEngine* voe_ptr,*/
	  int& file_id);

  
  int DestoryVirtualDataProvide( int& file_id);
 /**
* @ingroup API_DataInput
* @brief    启动参数传递，将数据传入内部处理函数
* @req		GVE-SRS-165 
* @param[in] char* buffer: 输入的数据buffer
* @param[in] unsigned int length:输入的数据长度
* @param[in] const VideoCaptureCapability& frameinfo:图像的基本信息
* VideoCaptureCapability结构如下:
*	WebRtc_Word32 width:宽度
*	WebRtc_Word32 height:高度
*	WebRtc_Word32 maxFPS:最大帧率
*	WebRtc_Word32 expectedCaptureDelay
*	RawVideoType rawType;
*	VideoCodecType codecType:编码类型
*	bool interlaced;
* @returns   int类型，成功，返回0，失败，返回-1. 
*/

    int CreateVirtualDataPlayer(char* buffer,unsigned int length,const VideoCaptureCapability& frameinfo,int fileID);
  int CreateDataPlayer(char* buffer,unsigned int length,const VideoCaptureCapability& frameinfo,int fileID);
  int DestroyFilePlayer(int file_id);

 private:
  // Gets and allocates a free capture device id. Assumed protected by caller.
  bool GetFreeCaptureId(int* freecapture_id);

  // Frees a capture id assigned in GetFreeCaptureId.
  void ReturnCaptureId(int capture_id);

  // Gets and allocates a free file id. Assumed protected by caller.
  bool GetFreeFileId(int* free_file_id);

  // Frees a file id assigned in GetFreeFileId.
  void ReturnFileId(int file_id);

  // Gets the ViEFrameProvider for this capture observer.
  ViEFrameProviderBase* ViEFrameProvider(
      const ViEFrameCallback* capture_observer) const;

  // Gets the ViEFrameProvider for this capture observer.
  ViEFrameProviderBase* ViEFrameProvider(int provider_id) const;

  // Gets the ViECapturer for the capture device id.
  ViECapturer* ViECapturePtr(int capture_id) const;

  // Gets the ViEFilePlayer for this file_id.
  ViEFilePlayer* ViEFilePlayerPtr(int file_id) const;

  int engine_id_;
  scoped_ptr<CriticalSectionWrapper> map_cs_;
public:
  MapWrapper vie_frame_provider_map_;

  // Capture devices.
  VideoCaptureModule::DeviceInfo* capture_device_info_;
  int free_capture_device_id_[kViEMaxCaptureDevices];

  // File Players.
  int free_file_id_[kViEMaxFilePlayers];

  ProcessThread* module_process_thread_;  // Weak.
};

// Provides protected access to ViEInputManater.
class ViEInputManagerScoped: private ViEManagerScopedBase {
 public:
  explicit ViEInputManagerScoped(const ViEInputManager& vie_input_manager);

  ViECapturer* Capture(int capture_id) const;
  ViEFilePlayer* FilePlayer(int file_id) const;
  ViEFrameProviderBase* FrameProvider(int provider_id) const;
  ViEFrameProviderBase* FrameProvider(const ViEFrameCallback*
                                      capture_observer) const;
};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_VIE_INPUT_MANAGER_H_
