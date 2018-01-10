#include "interface.h"
#include <android/log.h>
#include<jni.h> 

#define WEBRTC_LOG_TAG "AVEngineCore"

// Global variables
JavaVM* webrtcGlobalVM;
jclass globalAudioDevClass = NULL;
jclass globalVideoCmClass = NULL;
jclass globalVideoCmDevInfoClass = NULL;
jclass globalVideoCmCapabilityClass = NULL;
jclass globalVideoRenderClass = NULL;
jclass globalVideoRenderOpenGLClass = NULL;

// JNI_OnLoad
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  webrtcGlobalVM = vm;

  if (!webrtcGlobalVM)
  {
    __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                        "JNI_OnLoad did not receive a valid VM pointer");
    return -1;
  }

  // Get JNI
  JNIEnv* env;
  bool isAttached = false;
  if (JNI_OK != webrtcGlobalVM->GetEnv(reinterpret_cast<void**> (&env),
                           JNI_VERSION_1_4)) {
    jint res = webrtcGlobalVM->AttachCurrentThread(&env, NULL);
    if (res<0 || !env)
    {
      __android_log_print(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG,
                        "could not attach thread to JVM(%d, %p)", res, env);
      return -1;
    }
    isAttached = true;
  }
  
  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, webrtcGlobleVM:%p evn:%p", __FUNCTION__, webrtcGlobalVM, env);
 
  // Audio
  jclass audioClazz = env->FindClass("org/webrtc/voiceengine/WebRTCAudioDevice");
  if (!audioClazz)
  {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s find class faild", __FUNCTION__);
  }

  globalAudioDevClass = reinterpret_cast<jclass> (env->NewGlobalRef(audioClazz));
  if (!globalAudioDevClass) {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s class create reference faild", __FUNCTION__);
  }

  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, audioClazz:%p globalAudioDevClass:%p", __FUNCTION__, audioClazz, globalAudioDevClass);

  // Video
  jclass javaCmClazz = env->FindClass("org/webrtc/videoengine/VideoCaptureAndroid");
  if (!javaCmClazz)
  {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s find class faild", __FUNCTION__);
  }

  globalVideoCmClass = reinterpret_cast<jclass> (env->NewGlobalRef(javaCmClazz));
  if (!globalVideoCmClass) {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s class create reference faild", __FUNCTION__);
  }

  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, javaCmClazz:%p globalVideoCmClass:%p", __FUNCTION__, javaCmClazz, globalVideoCmClass);

  jclass javaCmDevInfoClazz = env->FindClass("org/webrtc/videoengine/VideoCaptureDeviceInfoAndroid");
  if (!javaCmDevInfoClazz)
  {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s find class faild", __FUNCTION__);
  }

  globalVideoCmDevInfoClass = reinterpret_cast<jclass> (env->NewGlobalRef(javaCmDevInfoClazz));
  if (!globalVideoCmDevInfoClass) {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s class create reference faild", __FUNCTION__);
  }

  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, javaCmDevInfoClazz:%p globalVideoCmDevInfoClass:%p", __FUNCTION__, javaCmDevInfoClazz, globalVideoCmDevInfoClass);

  jclass javaCmCapabilityClazz = env->FindClass("org/webrtc/videoengine/CaptureCapabilityAndroid");
  if (!javaCmCapabilityClazz)
  {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s find class faild", __FUNCTION__);
  }

  globalVideoCmCapabilityClass = reinterpret_cast<jclass> (env->NewGlobalRef(javaCmCapabilityClazz));
  if (!globalVideoCmCapabilityClass) {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s class create reference faild", __FUNCTION__);
  }

  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, javaCmCapabilityClazz:%p globalVideoCmCapabilityClass:%p", __FUNCTION__, javaCmCapabilityClazz, globalVideoCmCapabilityClass);

  jclass javaRenderClazz = env->FindClass("org/webrtc/videoengine/ViESurfaceRenderer");
  if (!javaRenderClazz)
  {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s find class faild", __FUNCTION__);
  }

  globalVideoRenderClass = reinterpret_cast<jclass> (env->NewGlobalRef(javaRenderClazz));
  if (!globalVideoRenderClass) {
    __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s class create reference faild", __FUNCTION__);
  }
  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, javaRenderClazz:%p globalVideoRenderClass:%p", __FUNCTION__, javaRenderClazz, globalVideoRenderClass);
  jclass javaRenderOpenGLClazz = env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
    if (!javaRenderOpenGLClazz)
    {
      __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s find ViEAndroidGLES20 class faild", __FUNCTION__);
    }

    globalVideoRenderOpenGLClass = reinterpret_cast<jclass> (env->NewGlobalRef(javaRenderOpenGLClazz));
    if (!globalVideoRenderOpenGLClass) {
      __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s class create reference faild", __FUNCTION__);
    }

  __android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s, javaRenderClazz:%p globalVideoRenderClass:%p", __FUNCTION__, javaRenderOpenGLClazz, globalVideoRenderOpenGLClass);

  env->DeleteLocalRef(audioClazz);
  env->DeleteLocalRef(javaCmClazz);
  env->DeleteLocalRef(javaCmDevInfoClazz);
  env->DeleteLocalRef(javaCmCapabilityClazz);
  env->DeleteLocalRef(javaRenderClazz);
  env->DeleteLocalRef(javaRenderOpenGLClazz);

  if (isAttached)
  {
    if (webrtcGlobalVM->DetachCurrentThread() < 0)
    {
      __android_log_write(ANDROID_LOG_ERROR, WEBRTC_LOG_TAG, "could not detach thread from JVM");
    }
  }

  return JNI_VERSION_1_4;
}

VoiceEngine* VoiceEngine_Create()
{
	return VoiceEngine::Create();
}
bool VoiceEngine_Delete(VoiceEngine*& voiceEngine)
{
	return VoiceEngine::Delete(voiceEngine);
}
int VoiceEngine_SetTraceFilter(const unsigned int filter)
{
	return VoiceEngine::SetTraceFilter(filter);
}
int VoiceEngine_SetTraceFile(const char* fileNameUTF8)
{
	return VoiceEngine::SetTraceFile(fileNameUTF8);
}
int VoiceEngine_SetTraceCallback(TraceCallback* callback)
{
	return VoiceEngine::SetTraceCallback(callback);
}	
int VoiceEngine_SetAndroidObjects(void* javaVM, void* env, void* context)
{
        //__android_log_print(ANDROID_LOG_DEBUG, WEBRTC_LOG_TAG, "JNI:%s javaVM:%p env:%p context:%p", __FUNCTION__, javaVM, env, context);
	VoiceEngine::SetAudioJavaScClass(globalAudioDevClass);
	return VoiceEngine::SetAndroidObjects(javaVM, env, context);
}
VoEBase* VoEBase_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEBase::GetInterface(voiceEngine);
}
VoECodec* VoECodec_GetInterface(VoiceEngine* voiceEngine)
{
	return VoECodec::GetInterface(voiceEngine);
}
VoEFile* VoEFile_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEFile::GetInterface(voiceEngine);
}
VoEHardware* VoEHardware_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEHardware::GetInterface(voiceEngine);
}
VoENetwork* VoENetwork_GetInterface(VoiceEngine* voiceEngine)
{
	return VoENetwork::GetInterface(voiceEngine);
}
VoERTP_RTCP* VoERTP_RTCP_GetInterface(VoiceEngine* voiceEngine)
{
	return VoERTP_RTCP::GetInterface(voiceEngine);
}
VoEVolumeControl* VoEVolumeControl_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEVolumeControl::GetInterface(voiceEngine);
}
VoEAudioProcessing* VoEAudioProcessing_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEAudioProcessing::GetInterface(voiceEngine);
}
VoEExternalMedia* VoEExternalMedia_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEExternalMedia::GetInterface(voiceEngine);
}
VoEEncryption* VoEEncryption_GetInterface(VoiceEngine* voiceEngine)
{
	return VoEEncryption::GetInterface(voiceEngine);
}

VideoEngine* VideoEngine_Create()
{
	return VideoEngine::Create();
}
bool VideoEngine_Delete(VideoEngine*& video_engine)
{
	return VideoEngine::Delete(video_engine);
}
int VideoEngine_SetTraceFilter(const unsigned int filter)
{
	return VideoEngine::SetTraceFilter(filter);
}
int VideoEngine_SetTraceFile(const char* file_nameUTF8)
{
	return VideoEngine::SetTraceFile(file_nameUTF8);
}
int VideoEngine_SetTraceCallback(TraceCallback* callback)
{
	return VideoEngine::SetTraceCallback(callback);
}
int VideoEngine_SetAndroidObjects(void* javaVM, void* context)
{
	VideoEngine::SetVideoJavaScClass(globalVideoCmClass, globalVideoCmDevInfoClass, globalVideoCmCapabilityClass, globalVideoRenderClass,  globalVideoRenderOpenGLClass);
	//return VideoEngine::SetAndroidObjects(javaVM, context);
	return VideoEngine::SetAndroidObjects(webrtcGlobalVM, context);
}
int VideoEngine_VideoSetCaptureAndroidVM(void* javaVM, void* context)
{
	VideoEngine::SetVideoJavaScClass(globalVideoCmClass, globalVideoCmDevInfoClass, globalVideoCmCapabilityClass, globalVideoRenderClass, globalVideoRenderOpenGLClass);
        return VideoEngine::VideoSetCaptureAndroidVM(webrtcGlobalVM, context);
}
int VideoEngine_VideoSetRenderAndroidVM(void* javaVM)
{
        return VideoEngine::VideoSetRenderAndroidVM(webrtcGlobalVM);
}

void VideoEngine_GVE_FFMPEG_Demux_Destroy(unsigned long GVE_FFDemux_Handle)
{
    GVE_FFMPEG_Demux_Destroy(GVE_FFDemux_Handle);
}

void VideoEngine_GVE_FFMPEG_Demux_Suspend(unsigned long            GVE_FFDemux_Handle)
{
    GVE_FFMPEG_Demux_Suspend(GVE_FFDemux_Handle);
}

void VideoEngine_GVE_FFMPEG_Demux_Resume(unsigned long            GVE_FFDemux_Handle)
{
    GVE_FFMPEG_Demux_Resume(GVE_FFDemux_Handle);
}


int VideoEngine_GVE_FFMPEG_Demux_Create(unsigned long           *GVE_FFDemux_Handle,
                            GVE_FFDemux_OperatePar *OperatePar,
                            GVE_FFDemux_ConfigPar  *ConfigPar,
                            GVE_FFDemux_OutPutInfo *OutPutInfo)
{
    return GVE_FFMPEG_Demux_Create(GVE_FFDemux_Handle,OperatePar,ConfigPar,OutPutInfo);
}

int VideoEngine_GVE_FFMPEG_Demux_Decoder(unsigned long            GVE_FFDemux_Handle,
                             GVE_FFDemux_OperatePar *OperatePar,
                             GVE_FFDemux_ConfigPar  *ConfigPar,
                             GVE_FFDemux_OutPutInfo *OutPutInfo)
{
    return GVE_FFMPEG_Demux_Decoder(GVE_FFDemux_Handle,OperatePar,ConfigPar,OutPutInfo);
}


ViEBase* ViEBase_GetInterface(VideoEngine* video_engine)
{
	return ViEBase::GetInterface(video_engine);
}
ViECapture* ViECapture_GetInterface(VideoEngine* video_engine)
{
	return ViECapture::GetInterface(video_engine);
}
ViECodec* ViECodec_GetInterface(VideoEngine* video_engine)
{
	return ViECodec::GetInterface(video_engine);
}
ViENetwork* ViENetwork_GetInterface(VideoEngine* video_engine)
{
	return ViENetwork::GetInterface(video_engine);
}
ViERender* ViERender_GetInterface(VideoEngine* video_engine)
{
	return ViERender::GetInterface(video_engine);
}
ViERTP_RTCP* ViERTP_RTCP_GetInterface(VideoEngine* video_engine)
{
	return ViERTP_RTCP::GetInterface(video_engine);
}
ViEImageProcess* ViEImageProcess_GetInterface(VideoEngine* video_engine)
{
	return ViEImageProcess::GetInterface(video_engine);
}
ViEFile* ViEFile_GetInterface(VideoEngine* video_engine)
{
	return ViEFile::GetInterface(video_engine);
}

