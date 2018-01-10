
#ifndef _AVINTERFACE_H_
#define _AVINTERFACE_H_

#include "voe_base.h"
#include "voe_codec.h"
#include "voe_file.h"
#include "voe_network.h"
#include "voe_audio_processing.h"
#include "voe_volume_control.h"
#include "voe_hardware.h"
#include "voe_rtp_rtcp.h"
#include "voe_external_media.h"
#include "voe_encryption.h"

#include "vie_base.h"
#include "vie_codec.h"
#include "vie_capture.h"
#include "vie_external_codec.h"
#include "vie_network.h"
#include "vie_render.h"
#include "vie_rtp_rtcp.h"
#include "vie_file.h"
#include "vie_image_process.h"
#include "ffmpegdemuxdec_api.h"

#include "common_types.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace webrtc;

VoiceEngine* VoiceEngine_Create();
bool VoiceEngine_Delete(VoiceEngine*& voiceEngine);
int VoiceEngine_SetTraceFilter(const unsigned int filter);
int VoiceEngine_SetTraceFile(const char* fileNameUTF8);
int VoiceEngine_SetTraceCallback(TraceCallback* callback);
int VoiceEngine_SetAndroidObjects(void* javaVM, void* env, void* context);
VoEBase* VoEBase_GetInterface(VoiceEngine* voiceEngine);
VoECodec* VoECodec_GetInterface(VoiceEngine* voiceEngine);
VoEFile* VoEFile_GetInterface(VoiceEngine* voiceEngine);
VoEHardware* VoEHardware_GetInterface(VoiceEngine* voiceEngine);
VoENetwork* VoENetwork_GetInterface(VoiceEngine* voiceEngine);
VoERTP_RTCP* VoERTP_RTCP_GetInterface(VoiceEngine* voiceEngine);
VoEVolumeControl* VoEVolumeControl_GetInterface(VoiceEngine* voiceEngine);
VoEAudioProcessing* VoEAudioProcessing_GetInterface(VoiceEngine* voiceEngine);
VoEExternalMedia* VoEExternalMedia_GetInterface(VoiceEngine* voiceEngine);
VoEEncryption* VoEEncryption_GetInterface(VoiceEngine* voiceEngine);

VideoEngine* VideoEngine_Create();
bool VideoEngine_Delete(VideoEngine*& video_engine);
int VideoEngine_SetTraceFilter(const unsigned int filter);
int VideoEngine_SetTraceFile(const char* file_nameUTF8);
int VideoEngine_SetTraceCallback(TraceCallback* callback);
int VideoEngine_SetAndroidObjects(void* javaVM, void* context);
int VideoEngine_VideoSetCaptureAndroidVM(void* javaVM, void* context);
int VideoEngine_VideoSetRenderAndroidVM(void* javaVM);
void VideoEngine_GVE_FFMPEG_Demux_Destroy(unsigned long GVE_FFDemux_Handle);
void VideoEngine_GVE_FFMPEG_Demux_Suspend(unsigned long handle);
void VideoEngine_GVE_FFMPEG_Demux_Resume(unsigned long handle);
int VideoEngine_GVE_FFMPEG_Demux_Create(unsigned long           *GVE_FFDemux_Handle,
                            GVE_FFDemux_OperatePar *OperatePar,
                            GVE_FFDemux_ConfigPar  *ConfigPar,
                            GVE_FFDemux_OutPutInfo *OutPutInfo);
int VideoEngine_GVE_FFMPEG_Demux_Decoder(unsigned long            GVE_FFDemux_Handle,
                             GVE_FFDemux_OperatePar *OperatePar,
                             GVE_FFDemux_ConfigPar  *ConfigPar,
                             GVE_FFDemux_OutPutInfo *OutPutInfo);

ViEBase* ViEBase_GetInterface(VideoEngine* video_engine);
ViECapture* ViECapture_GetInterface(VideoEngine* video_engine);
ViECodec* ViECodec_GetInterface(VideoEngine* video_engine);
ViENetwork* ViENetwork_GetInterface(VideoEngine* video_engine);
ViERender* ViERender_GetInterface(VideoEngine* video_engine);
ViERTP_RTCP* ViERTP_RTCP_GetInterface(VideoEngine* video_engine);
ViEImageProcess* ViEImageProcess_GetInterface(VideoEngine* video_engine);
ViEFile* ViEFile_GetInterface(VideoEngine* video_engine);

#ifdef __cplusplus
}
#endif
 
#endif
