/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// This sub-API supports the following functionalities:
//  - File recording and playing.
//  - Snapshots.
//  - Background images.

/** 
* @defgroup	API_DataInput
* @brief	获取视频输入数据的接口
*
* 增加第三方获取数据接口，其他接口未动
*/

#ifndef WEBRTC_VIDEO_ENGINE_INCLUDE_VIE_FILE_H_
#define WEBRTC_VIDEO_ENGINE_INCLUDE_VIE_FILE_H_

#include "common_types.h"
//#include <windows.h>

namespace webrtc {

class VideoEngine;
struct VideoCodec;

// This structure contains picture data and describes the picture type.
struct ViEPicture {
  unsigned char* data;
  unsigned int size;
  unsigned int width;
  unsigned int height;
  RawVideoType type;

  ViEPicture() {
    data = NULL;
    size = 0;
    width = 0;
    height = 0;
    type = kVideoI420;
  }

  // Call FreePicture to free data.
  ~ViEPicture() {
    data = NULL;
    size = 0;
    width = 0;
    height = 0;
    type = kVideoUnknown;
  }
};
struct Capability {
	unsigned int width;
	unsigned int height;
	unsigned int maxFPS;
	unsigned int expectedCaptureDelay;
	RawVideoType rawType;
	VideoCodecType codecType;
	bool interlaced;
	Capability() {
		width = 0;
		height = 0;
		maxFPS = 0;
		rawType = kVideoI420;
		codecType = kVideoCodecUnknown;
		expectedCaptureDelay = 0;
		interlaced = false;
	}
};

// This enumerator tells which audio source to use for media files.
enum AudioSource {
  NO_AUDIO,
  MICROPHONE,
  PLAYOUT,
  VOICECALL
};

// This class declares an abstract interface for a user defined observer. It is
// up to the VideoEngine user to implement a derived class which implements the
// observer class. The observer is registered using RegisterObserver() and
// deregistered using DeregisterObserver().
class  ViEFileObserver {
 public:
  // This method is called when the end is reached of a played file.
  virtual void PlayFileEnded(const WebRtc_Word32 file_id) = 0;

 protected:
  virtual ~ViEFileObserver() {}
};

class  ViEFile {
 public:
  // Factory for the ViEFile API and increases an internal reference
  // counter if successful. Returns NULL if the API is not supported or if
  // construction fails.
  static ViEFile* GetInterface(VideoEngine* video_engine);

  // Releases the ViEFile sub-API and decreases an internal reference counter.
  // Returns the new reference count. This value should be zero
  // for all sub-API:s before the VideoEngine object can be safely deleted.
  virtual int Release() = 0;

  // Starts playing a video file.
  virtual int StartPlayFile(
	  const char* file_name_utf8,
	  int& file_id,
	  const bool loop = false,
	  const FileFormats file_format = kFileFormatAviFile) = 0;

//////////////////////////////////////////////////////
//以下两个函数主要提供第三方数据接口，其他调用不应调用下面两个方法
/**
* @ingroup API_DataInput
* @brief    初始化并开启相关服务（在调用StartVirtualPlayData之前调用，其他不应调用此接口）
* @req		GVE-SRS-164 
* @param[in] const char* file_name_utf8 :存在的avi文件名
* @param[out] int& file_id :输出的分配的可用文件ID
* @param[in] const bool loop:循环读取
* @param[in] const FileFormats file_format:文件格式，必须是kFileFormatAviFile
* @returns   int类型，成功，返回0，失败，返回-1. 
*/
  virtual int CreateVirtualPlayFile(
	   int& file_id) = 0;
  virtual int DestoryVirtualPlayFile(
	  int& file_id) = 0;
  /**
* @ingroup API_DataInput
* @brief    获取输入的YUV数据，并把数据送给编码器
* @req		GVE-SRS-165 
* @param[in] char* buffer: 输入的数据buffer
* @param[in] unsigned int length:输入的数据长度
* @param[in] const Capability& frameInfo:图像的基本信息
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
  virtual int SetVirtualPlayData(char* buffer,unsigned int length,const Capability& frameInfo,int fileID) = 0;


///////////////////////////////////////////////////////////////////////

  // Stops a file from being played.
  virtual int StopPlayFile(const int file_id) = 0;

  // Registers an instance of a user implementation of the ViEFileObserver.
  virtual int RegisterObserver(int file_id, ViEFileObserver& observer) = 0;

  // Removes an already registered instance of ViEFileObserver.
  virtual int DeregisterObserver(int file_id, ViEFileObserver& observer) = 0;

  // This function tells which channel, if any, the file should be sent on.
  virtual int SendFileOnChannel(const int file_id, const int video_channel) = 0;

  // Stops a file from being sent on a a channel.
  virtual int StopSendFileOnChannel(const int video_channel) = 0;

  // Starts playing the file audio as microphone input for the specified voice
  // channel.
  virtual int StartPlayFileAsMicrophone(const int file_id,
                                        const int audio_channel,
                                        bool mix_microphone = false,
                                        float volume_scaling = 1) = 0;

  // The function stop the audio from being played on a VoiceEngine channel.
  virtual int StopPlayFileAsMicrophone(const int file_id,
                                       const int audio_channel) = 0;

  // The function plays and mixes the file audio with the local speaker signal
  // for playout.
  virtual int StartPlayAudioLocally(const int file_id, const int audio_channel,
                                    float volume_scaling = 1) = 0;

  // Stops the audio from a file from being played locally.
  virtual int StopPlayAudioLocally(const int file_id,
                                   const int audio_channel) = 0;

  // This function starts recording the video transmitted to another endpoint.
  virtual int StartRecordOutgoingVideo(
      const int video_channel,
      const char* file_name_utf8,
      AudioSource audio_source,
      const CodecInst& audio_codec,
      const VideoCodec& video_codec,
      const FileFormats file_format = kFileFormatAviFile) = 0;

  // This function starts recording the incoming video stream on a channel.
  virtual int StartRecordIncomingVideo(
      const int video_channel,
      const char* file_name_utf8,
      AudioSource audio_source,
      const CodecInst& audio_codec,
      const VideoCodec& video_codec,
      const FileFormats file_format = kFileFormatAviFile) = 0;

  // Stops the file recording of the outgoing stream.
  virtual int StopRecordOutgoingVideo(const int video_channel) = 0;

  // Stops the file recording of the incoming stream.
  virtual int StopRecordIncomingVideo(const int video_channel) = 0;

  // Gets the audio codec, video codec and file format of a recorded file.
  virtual int GetFileInformation(
      const char* file_name,
      VideoCodec& video_codec,
      CodecInst& audio_codec,
      const FileFormats file_format = kFileFormatAviFile) = 0;

  // The function takes a snapshot of the last rendered image for a video
  // channel.
  virtual int GetRenderSnapshot(const int video_channel,
                                const char* file_name_utf8) = 0;

  // The function takes a snapshot of the last rendered image for a video
  // channel
  virtual int GetRenderSnapshot(const int video_channel,
                                ViEPicture& picture) = 0;

  // The function takes a snapshot of the last captured image by a specified
  // capture device.
  virtual int GetCaptureDeviceSnapshot(const int capture_id,
                                       const char* file_name_utf8) = 0;

  // The function takes a snapshot of the last captured image by a specified
  // capture device.
  virtual int GetCaptureDeviceSnapshot(const int capture_id,
                                       ViEPicture& picture) = 0;

  virtual int FreePicture(ViEPicture& picture) = 0;

  // This function sets a jpg image to render before the first received video
  // frame is decoded for a specified channel.
  virtual int SetRenderStartImage(const int video_channel,
                                  const char* file_name_utf8) = 0;

  // This function sets an image to render before the first received video
  // frame is decoded for a specified channel.
  virtual int SetRenderStartImage(const int video_channel,
                                  const ViEPicture& picture) = 0;

  // This function sets a jpg image to render if no frame is decoded for a
  // specified time interval.
  virtual int SetRenderTimeoutImage(const int video_channel,
                                    const char* file_name_utf8,
                                    const unsigned int timeout_ms = 1000) = 0;

  // This function sets an image to render if no frame is decoded for a
  // specified time interval.
  virtual int SetRenderTimeoutImage(const int video_channel,
                                    const ViEPicture& picture,
                                    const unsigned int timeout_ms) = 0;

  // Enables recording of debugging information.
  virtual int StartDebugRecording(int video_channel,
                                  const char* file_name_utf8) = 0;
  // Disables recording of debugging information.
  virtual int StopDebugRecording(int video_channel) = 0;


 protected:
  ViEFile() {}
  virtual ~ViEFile() {}
 public:

  //HANDLE hPipoServer;
};

}  // namespace webrtc

#endif  // WEBRTC_VIDEO_ENGINE_INCLUDE_VIE_FILE_H_
