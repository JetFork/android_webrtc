/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video_render_android_native_opengl2.h"
#include "critical_section_wrapper.h"
#include "tick_util.h"
#define ANDROID_LOG
#ifdef ANDROID_LOG
#include <stdio.h>
#include "android/log.h"


#undef WEBRTC_TRACE
#define WEBRTC_TRACE(a,b,c,...)  __android_log_print(ANDROID_LOG_DEBUG, "*WEBRTC*", __VA_ARGS__)
#else
#include "trace.h"
#endif

namespace webrtc {
extern void *globalVideoRenderOpenGLScClass;

AndroidNativeOpenGl2Renderer::AndroidNativeOpenGl2Renderer(
    const WebRtc_Word32 id,
    const VideoRenderType videoRenderType,
    void* window,
    const bool fullscreen) :
    VideoRenderAndroid(id, videoRenderType, window, fullscreen),
    _javaRenderObj(NULL),
    _javaRenderClass(NULL) {
}

bool AndroidNativeOpenGl2Renderer::UseOpenGL2(void* window) {
  if (!g_jvm) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, -1,
                 "RendererAndroid():UseOpenGL No JVM set.");
    return false;
  }
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = g_jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
      WEBRTC_TRACE(
          kTraceError,
          kTraceVideoRenderer,
          -1,
          "RendererAndroid(): Could not attach thread to JVM (%d, %p)",
          res, env);
      return false;
    }
    isAttached = true;
  }

  // get the renderer class
 /* jclass javaRenderClassLocal =
      env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
  if (!javaRenderClassLocal) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, -1,
                 "%s: could not find ViEAndroidRenderer class",
                 __FUNCTION__);
    return false;
  }
  */
  jclass javaRenderClassLocal = reinterpret_cast<jclass>(globalVideoRenderOpenGLScClass);

  // get the method ID for UseOpenGL
  jmethodID cidUseOpenGL = env->GetStaticMethodID(javaRenderClassLocal,
                                                  "UseOpenGL2",
                                                  "(Ljava/lang/Object;)Z");
  if (cidUseOpenGL == NULL) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, -1,
                 "%s: could not get UseOpenGL ID", __FUNCTION__);
    return false;
  }
  jboolean res = env->CallStaticBooleanMethod(javaRenderClassLocal,
                                              cidUseOpenGL, (jobject) window);

  // Detach this thread if it was attached
  if (isAttached) {
    if (g_jvm->DetachCurrentThread() < 0) {
      WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, -1,
                   "%s: Could not detach thread from JVM", __FUNCTION__);
    }
  }
  return res;
}

AndroidNativeOpenGl2Renderer::~AndroidNativeOpenGl2Renderer() {
  WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id,
               "AndroidNativeOpenGl2Renderer dtor");
  if (g_jvm) {
    // get the JNI env for this thread
    bool isAttached = false;
    JNIEnv* env = NULL;
    if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
      // try to attach the thread and get the env
      // Attach this thread to JVM
      jint res = g_jvm->AttachCurrentThread(&env, NULL);

      // Get the JNI env for this thread
      if ((res < 0) || !env) {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                     "%s: Could not attach thread to JVM (%d, %p)",
                     __FUNCTION__, res, env);
        env = NULL;
      }
      else {
        isAttached = true;
      }
    }

    env->DeleteGlobalRef(_javaRenderObj);
    env->DeleteGlobalRef(_javaRenderClass);

    if (isAttached) {
      if (g_jvm->DetachCurrentThread() < 0) {
        WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
                     "%s: Could not detach thread from JVM",
                     __FUNCTION__);
      }
    }
  }
}

WebRtc_Word32 AndroidNativeOpenGl2Renderer::Init() {
  WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s", __FUNCTION__);
  if (!g_jvm) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "(%s): Not a valid Java VM pointer.", __FUNCTION__);
    return -1;
  }
  if (!_ptrWindow) {
    WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
                 "(%s): No window have been provided.", __FUNCTION__);
    return -1;
  }

  // get the JNI env for this thread
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = g_jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
      WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                   "%s: Could not attach thread to JVM (%d, %p)",
                   __FUNCTION__, res, env);
      return -1;
    }
    isAttached = true;
  }

  // get the ViEAndroidGLES20 class
 /* jclass javaRenderClassLocal =
      env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
  if (!javaRenderClassLocal) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: could not find ViEAndroidGLES20", __FUNCTION__);
    return -1;
  }*/
	jclass javaRenderClassLocal = reinterpret_cast<jclass>(globalVideoRenderOpenGLScClass);
  // create a global reference to the class (to tell JNI that
  // we are referencing it after this function has returned)
  _javaRenderClass =
      reinterpret_cast<jclass> (env->NewGlobalRef(javaRenderClassLocal));
  if (!_javaRenderClass) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: could not create Java SurfaceHolder class reference",
                 __FUNCTION__);
    return -1;
  }

  // Delete local class ref, we only use the global ref
  //env->DeleteLocalRef(javaRenderClassLocal);

  // create a reference to the object (to tell JNI that we are referencing it
  // after this function has returned)
  _javaRenderObj = env->NewGlobalRef(_ptrWindow);
  if (!_javaRenderObj) {
    WEBRTC_TRACE(
        kTraceError,
        kTraceVideoRenderer,
        _id,
        "%s: could not create Java SurfaceRender object reference",
        __FUNCTION__);
    return -1;
  }

  // Detach this thread if it was attached
  if (isAttached) {
    if (g_jvm->DetachCurrentThread() < 0) {
      WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
                   "%s: Could not detach thread from JVM", __FUNCTION__);
    }
  }

  WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s done",
               __FUNCTION__);
  return 0;

}
AndroidStream*
AndroidNativeOpenGl2Renderer::CreateAndroidRenderChannel(
    WebRtc_Word32 streamId,
    WebRtc_Word32 zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom,
    VideoRenderAndroid& renderer) {
  WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s: Id %d",
               __FUNCTION__, streamId);
  AndroidNativeOpenGl2Channel* stream =
      new AndroidNativeOpenGl2Channel(streamId, g_jvm, renderer,
                                      _javaRenderObj);
  if (stream && stream->Init(zOrder, left, top, right, bottom) == 0)
    return stream;
  else {
    delete stream;
  }
  return NULL;
}

AndroidNativeOpenGl2Channel::AndroidNativeOpenGl2Channel(
    WebRtc_UWord32 streamId,
    JavaVM* jvm,
    VideoRenderAndroid& renderer,jobject javaRenderObj):
    _id(streamId),
    _renderCritSect(*CriticalSectionWrapper::CreateCriticalSection()),
    _renderer(renderer), _jvm(jvm), _javaRenderObj(javaRenderObj),
    _registerNativeCID(NULL), _deRegisterNativeCID(NULL),
    _openGLRenderer(streamId), _width(0), _height(0), scaledmode_(1) {

}
AndroidNativeOpenGl2Channel::~AndroidNativeOpenGl2Channel() {
  WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id,
               "AndroidNativeOpenGl2Channel dtor");
//  delete &_renderCritSect;
  if (_jvm) {
    // get the JNI env for this thread
    bool isAttached = false;
    JNIEnv* env = NULL;
    if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
      // try to attach the thread and get the env
      // Attach this thread to JVM
      jint res = _jvm->AttachCurrentThread(&env, NULL);

      // Get the JNI env for this thread
      if ((res < 0) || !env) {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                     "%s: Could not attach thread to JVM (%d, %p)",
                     __FUNCTION__, res, env);
        env = NULL;
      } else {
        isAttached = true;
      }
    }
    if (env && _deRegisterNativeCID) {
      env->CallVoidMethod(_javaRenderObj, _deRegisterNativeCID);
    }

    if (isAttached) {
      if (_jvm->DetachCurrentThread() < 0) {
        WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
                     "%s: Could not detach thread from JVM",
                     __FUNCTION__);
      }
    }
  }
	_renderCritSect.Enter();
  delete &_renderCritSect;
}

WebRtc_Word32 AndroidNativeOpenGl2Channel::Init(WebRtc_Word32 zOrder,
                                                const float left,
                                                const float top,
                                                const float right,
                                                const float bottom)
{
  WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id,
               "%s: AndroidNativeOpenGl2Channel", __FUNCTION__);
  if (!_jvm) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: Not a valid Java VM pointer", __FUNCTION__);
    return -1;
  }

  // get the JNI env for this thread
  bool isAttached = false;
  JNIEnv* env = NULL;
  if (_jvm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
    // try to attach the thread and get the env
    // Attach this thread to JVM
    jint res = _jvm->AttachCurrentThread(&env, NULL);

    // Get the JNI env for this thread
    if ((res < 0) || !env) {
      WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                   "%s: Could not attach thread to JVM (%d, %p)",
                   __FUNCTION__, res, env);
      return -1;
    }
    isAttached = true;
  }
/*
  jclass javaRenderClass =
      env->FindClass("org/webrtc/videoengine/ViEAndroidGLES20");
  if (!javaRenderClass) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: could not find ViESurfaceRenderer", __FUNCTION__);
    return -1;
  }
*/
  jclass javaRenderClass = reinterpret_cast<jclass>(globalVideoRenderOpenGLScClass);

  // get the method ID for the ReDraw function
  _redrawCid = env->GetMethodID(javaRenderClass, "ReDraw", "()V");
  if (_redrawCid == NULL) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: could not get ReDraw ID", __FUNCTION__);
    return -1;
  }

  _registerNativeCID = env->GetMethodID(javaRenderClass,
                                        "RegisterNativeObject", "(J)V");
  if (_registerNativeCID == NULL) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: could not get RegisterNativeObject ID", __FUNCTION__);
    return -1;
  }

  _deRegisterNativeCID = env->GetMethodID(javaRenderClass,
                                          "DeRegisterNativeObject", "()V");
  if (_deRegisterNativeCID == NULL) {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s: could not get DeRegisterNativeObject ID",
                 __FUNCTION__);
    return -1;
  }

  JNINativeMethod nativeFunctions[2] = {
    { "DrawNative",
      "(J)V",
      (void*) &AndroidNativeOpenGl2Channel::DrawNativeStatic, },
    { "CreateOpenGLNative",
      "(JII)I",
      (void*) &AndroidNativeOpenGl2Channel::CreateOpenGLNativeStatic },
  };
  if (env->RegisterNatives(javaRenderClass, nativeFunctions, 2) == 0) {
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, -1,
                 "%s: Registered native functions", __FUNCTION__);
  }
  else {
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, -1,
                 "%s: Failed to register native functions", __FUNCTION__);
    return -1;
  }

  env->CallVoidMethod(_javaRenderObj, _registerNativeCID, (jlong) this);

  // Detach this thread if it was attached
  if (isAttached) {
    if (_jvm->DetachCurrentThread() < 0) {
      WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
                   "%s: Could not detach thread from JVM", __FUNCTION__);
    }
  }

  if (_openGLRenderer.SetCoordinates(zOrder, left, top, right, bottom) != 0) {
    return -1;
  }
  WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id,
               "%s: AndroidNativeOpenGl2Channel done", __FUNCTION__);
  return 0;
}
//FILE* f=NULL;
WebRtc_Word32 AndroidNativeOpenGl2Channel::RenderFrame(
    const WebRtc_UWord32 /*streamId*/,
    I420VideoFrame& videoFrame) {
  //   WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer,_id, "%s:" ,__FUNCTION__);
  _renderCritSect.Enter();
#if 0
  if(f==NULL)
	f=fopen("/sdcard/rend.yuv", "wb");
  if(0)
	{
		fwrite(videoFrame.buffer(kYPlane), videoFrame.allocated_size(kYPlane), 1, f);
		fwrite(videoFrame.buffer(kUPlane), videoFrame.allocated_size(kUPlane), 1, f);
		fwrite(videoFrame.buffer(kVPlane), videoFrame.allocated_size(kVPlane), 1, f);
		
	}
#endif
  if(_width > 0 && _height > 0){
  switch (scaledmode_)
	{
	case 0:
		_bufferToRender.SwapFrame(&videoFrame);
		videoFrame.set_timestamp(_bufferToRender.timestamp());
		videoFrame.set_render_time_ms(videoFrame.render_time_ms());
		break;
	case 1:
		{
            
			double srcRatio = (double)videoFrame.width()/(double)videoFrame.height();
			double dstRatio = (double)_width/(double)_height;
			if (fabs(srcRatio - dstRatio) <= 1e-6)
			{
				_bufferToRender.SwapFrame(&videoFrame);

			}
			else if (srcRatio < dstRatio)
			{
				int srcWidth = (int)(videoFrame.height() * dstRatio);
				srcWidth = (srcWidth+7) >> 3<<3;
				int half_width = (srcWidth + 1)/2;
				int srcWidth_offset = (srcWidth - videoFrame.width())/2;
				unsigned char *obuf[3],*tmpo[3];
				if (srcWidth_offset % 2)
				{
					srcWidth_offset += 1;
				}
				_bufferToRender.CreateEmptyFrame(srcWidth, videoFrame.height(),
					srcWidth, half_width, half_width);
				obuf[0] = (unsigned char *)_bufferToRender.buffer(webrtc::kYPlane);
				obuf[1] = (unsigned char *)_bufferToRender.buffer(webrtc::kUPlane);
				obuf[2] = (unsigned char *)_bufferToRender.buffer(webrtc::kVPlane);
				memset(obuf[0], 0, _bufferToRender.stride(webrtc::kYPlane)*_bufferToRender.height());
 				memset(obuf[1], 0x80, _bufferToRender.stride(webrtc::kUPlane)*((_bufferToRender.height()+1)/2));
 				memset(obuf[2], 0x80, _bufferToRender.stride(webrtc::kVPlane)*((_bufferToRender.height()+1)/2));

				tmpo[0] = (unsigned char *)videoFrame.buffer(webrtc::kYPlane);
				tmpo[1] = (unsigned char *)videoFrame.buffer(webrtc::kUPlane);
				tmpo[2] = (unsigned char *)videoFrame.buffer(webrtc::kVPlane);

				for (int h = 0; h < (videoFrame.height()+1)/2; h++)
				{
					memcpy(obuf[0]+(h*2)*_bufferToRender.stride(kYPlane) + srcWidth_offset, tmpo[0] + (h*2)*videoFrame.stride(kYPlane), videoFrame.width());
					memcpy(obuf[0]+(h*2+1)*_bufferToRender.stride(kYPlane) + srcWidth_offset, tmpo[0] + (h*2+1)*videoFrame.stride(kYPlane), videoFrame.width());
					memcpy(obuf[1]+ h*_bufferToRender.stride(kUPlane) + (srcWidth_offset >>1), tmpo[1] + h*videoFrame.stride(kUPlane), ((videoFrame.width()+1)/2));
					memcpy(obuf[2]+ h*_bufferToRender.stride(kVPlane) + (srcWidth_offset >>1), tmpo[2] + h*videoFrame.stride(kVPlane), ((videoFrame.width()+1)/2));
				}
			}
			else
			{
				int srcHeight = (int)(videoFrame.width()/dstRatio);
				int srcWidth = (videoFrame.width()+7)>>3<<3;
				int half_width = (srcWidth + 1)/2;
				int srcHeight_offset = (srcHeight - videoFrame.height())/2;
				unsigned char *obuf[3],*tmpo[3];
				if (srcHeight_offset % 2)
				{
					srcHeight_offset += 1;
				}

				_bufferToRender.CreateEmptyFrame(srcWidth, srcHeight, srcWidth, half_width, half_width);
				obuf[0] = (unsigned char *)_bufferToRender.buffer(webrtc::kYPlane);
				obuf[1] = (unsigned char *)_bufferToRender.buffer(webrtc::kUPlane);
				obuf[2] = (unsigned char *)_bufferToRender.buffer(webrtc::kVPlane);
				memset(obuf[0], 0, _bufferToRender.stride(webrtc::kYPlane)*_bufferToRender.height());
 				memset(obuf[1], 0x80, _bufferToRender.stride(webrtc::kUPlane)*((_bufferToRender.height()+1)/2));
 				memset(obuf[2], 0x80, _bufferToRender.stride(webrtc::kVPlane)*((_bufferToRender.height()+1)/2));
				tmpo[0] = (unsigned char *)videoFrame.buffer(webrtc::kYPlane);
				tmpo[1] = (unsigned char *)videoFrame.buffer(webrtc::kUPlane);
				tmpo[2] = (unsigned char *)videoFrame.buffer(webrtc::kVPlane);

//				memcpy(obuf[0] + _bufferToRender.width() * srcHeight_offset, tmpo[0], videoFrame.allocated_size(kYPlane));
//				memcpy(obuf[1] + (half_width * srcHeight_offset / 2), tmpo[1], videoFrame.allocated_size(kUPlane));
//				memcpy(obuf[2] + (half_width * srcHeight_offset / 2), tmpo[2], videoFrame.allocated_size(kVPlane));
				for (int h = 0; h < (videoFrame.height()+1)/2; h++)
				{
					memcpy(obuf[0]+(2*h+srcHeight_offset)*_bufferToRender.stride(kYPlane), tmpo[0]+2*h*videoFrame.stride(kYPlane), _bufferToRender.width());
					memcpy(obuf[0]+(2*h+1+srcHeight_offset)*_bufferToRender.stride(kYPlane), tmpo[0]+(2*h+1)*videoFrame.stride(kYPlane), _bufferToRender.width());
					memcpy(obuf[1]+ (h+srcHeight_offset/2)*_bufferToRender.stride(kUPlane), tmpo[1] + (h)*(videoFrame.stride(webrtc::kUPlane)), (_bufferToRender.width()+1)/2);
					memcpy(obuf[2]+ (h+srcHeight_offset/2)*_bufferToRender.stride(kVPlane), tmpo[2] + (h)*(videoFrame.stride(webrtc::kVPlane)), (_bufferToRender.width()+1)/2);
				}
			}
		}
		_bufferToRender.set_timestamp(videoFrame.timestamp());
		_bufferToRender.set_render_time_ms(videoFrame.render_time_ms());
		break;
	case 2:
		{
			double srcRatio = (double)videoFrame.width()/(double)videoFrame.height();
			double dstRatio = (double)_width/(double)_height;

			if (fabs(srcRatio - dstRatio) <= 1e-6)
			{
				_bufferToRender.SwapFrame(&videoFrame);
				
			}
			else if (srcRatio > dstRatio)
			{
				int srcWidth = (int)(videoFrame.height() * dstRatio);
				srcWidth = (srcWidth >> 3)<<3;
				int half_width = (srcWidth + 1)/2;
				int srcWidth_offset = (videoFrame.width() - srcWidth)/2;
				unsigned char *obuf[3],*tmpo[3];
				if (srcWidth_offset % 2)
				{
					srcWidth_offset += 1;
				}


				_bufferToRender.CreateEmptyFrame(srcWidth, videoFrame.height(),
					srcWidth, half_width, half_width);
				obuf[0] = (unsigned char *)_bufferToRender.buffer(webrtc::kYPlane);
				obuf[1] = (unsigned char *)_bufferToRender.buffer(webrtc::kUPlane);
				obuf[2] = (unsigned char *)_bufferToRender.buffer(webrtc::kVPlane);

				tmpo[0] = (unsigned char *)videoFrame.buffer(webrtc::kYPlane);
				tmpo[1] = (unsigned char *)videoFrame.buffer(webrtc::kUPlane);
				tmpo[2] = (unsigned char *)videoFrame.buffer(webrtc::kVPlane);

				for (int h = 0; h < (videoFrame.height()+1)/2; h++)
				{
					memcpy(obuf[0]+(h*2)*_bufferToRender.stride(kYPlane), tmpo[0] + (h*2)*videoFrame.stride(kYPlane) + srcWidth_offset, _bufferToRender.stride(kYPlane));
					memcpy(obuf[0]+(h*2+1)*_bufferToRender.stride(kYPlane), tmpo[0] + (h*2+1)*videoFrame.stride(kYPlane) + srcWidth_offset, _bufferToRender.stride(kYPlane));
					memcpy(obuf[1]+ h*_bufferToRender.stride(kUPlane), tmpo[1] + h*videoFrame.stride(kUPlane) + (srcWidth_offset >>1), _bufferToRender.stride(kUPlane));
					memcpy(obuf[2]+ h*_bufferToRender.stride(kVPlane), tmpo[2] + h*videoFrame.stride(kVPlane) + (srcWidth_offset >>1), _bufferToRender.stride(kVPlane));
				}

			}
			else
			{
				int srcHeight = (int)(videoFrame.width()/dstRatio);
				int srcWidth = (videoFrame.width()>>3)<<3;
				srcHeight = (srcHeight>>3)<<3;
				int half_width = (srcWidth + 1)/2;
				int srcHeight_offset = (videoFrame.height() - srcHeight)/2;
				unsigned char *obuf[3],*tmpo[3];
				if (srcHeight_offset % 2)
				{
					srcHeight_offset += 1;
				}

				_bufferToRender.CreateEmptyFrame(srcWidth, srcHeight, srcWidth, half_width, half_width);
				obuf[0] = (unsigned char *)_bufferToRender.buffer(webrtc::kYPlane);
				obuf[1] = (unsigned char *)_bufferToRender.buffer(webrtc::kUPlane);
				obuf[2] = (unsigned char *)_bufferToRender.buffer(webrtc::kVPlane);
				memset(obuf[0], 0, _bufferToRender.allocated_size(webrtc::kYPlane));
				memset(obuf[1], 0x80, _bufferToRender.allocated_size(webrtc::kUPlane));
				memset(obuf[2], 0x80, _bufferToRender.allocated_size(webrtc::kVPlane));

				tmpo[0] = (unsigned char *)videoFrame.buffer(webrtc::kYPlane);
				tmpo[1] = (unsigned char *)videoFrame.buffer(webrtc::kUPlane);
				tmpo[2] = (unsigned char *)videoFrame.buffer(webrtc::kVPlane);
//   __android_log_print(ANDROID_LOG_ERROR, "yyf"," render win:w=%d,h=%d,src:w=%d,h=%d,dst:w=%d,h=%d, height_off=%d, Usize = %d", _width, _height, videoFrame.width(), videoFrame.height(), _bufferToRender.width(), _bufferToRender.height(), srcHeight_offset, _bufferToRender.allocated_size(webrtc::kUPlane));
				
				for (int h = 0; h < (_bufferToRender.height()+1)/2; h++)
				{
					memcpy(obuf[0]+(2*h)*_bufferToRender.stride(kYPlane), tmpo[0] + srcHeight_offset*videoFrame.stride(webrtc::kYPlane)+2*h*videoFrame.stride(kYPlane), _bufferToRender.width());
					memcpy(obuf[0]+(2*h+1)*_bufferToRender.stride(kYPlane), tmpo[0] + (srcHeight_offset)*videoFrame.stride(webrtc::kYPlane)+(2*h+1)*videoFrame.stride(kYPlane), _bufferToRender.width());
					memcpy(obuf[1]+ h*_bufferToRender.stride(kUPlane), tmpo[1] + (h+srcHeight_offset/2)*(videoFrame.stride(webrtc::kUPlane)), (_bufferToRender.width()+1)/2);
					memcpy(obuf[2]+ h*_bufferToRender.stride(kVPlane), tmpo[2] + (h+srcHeight_offset/2)*(videoFrame.stride(webrtc::kVPlane)), (_bufferToRender.width()+1)/2);
				}
			}
		}
		_bufferToRender.set_timestamp(videoFrame.timestamp());
		_bufferToRender.set_render_time_ms(videoFrame.render_time_ms());
		break;
	default:
		_bufferToRender.SwapFrame(&videoFrame);
		videoFrame.set_timestamp(_bufferToRender.timestamp());
		videoFrame.set_render_time_ms(videoFrame.render_time_ms());
		break;
	}
  }
  _renderCritSect.Leave();
  _renderer.ReDraw();
  return 0;
}

/*Implements AndroidStream
 * Calls the Java object and render the buffer in _bufferToRender
 */
void AndroidNativeOpenGl2Channel::DeliverFrame(JNIEnv* jniEnv) {
  //TickTime timeNow=TickTime::Now();

  //Draw the Surface
  // WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideo, 0, "AndroidNativeOpenGl2Channel DeliverFrame 111");
   jniEnv->CallVoidMethod(_javaRenderObj, _redrawCid);

  // WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer,_id,
  // "%s: time to deliver %lld" ,__FUNCTION__,
  // (TickTime::Now()-timeNow).Milliseconds());
}

/*
 * JNI callback from Java class. Called when the render
 * want to render a frame. Called from the GLRenderThread
 * Method:    DrawNative
 * Signature: (J)V
 */
void JNICALL AndroidNativeOpenGl2Channel::DrawNativeStatic(
    JNIEnv * env, jobject, jlong context) {
  AndroidNativeOpenGl2Channel* renderChannel =
      reinterpret_cast<AndroidNativeOpenGl2Channel*>(context);
  renderChannel->DrawNative();
}

void AndroidNativeOpenGl2Channel::DrawNative() {
  _renderCritSect.Enter();
#if 0
if(_id > 1000 ){
  if(strlen(testFile) == 0)
	sprintf(testFile, "/sdcard/test%d.yuv", _id+1);
   __android_log_print(ANDROID_LOG_ERROR, "yyf","drawnative testFile:%s, width:%d, height%d", testFile, _bufferToRender.stride((kYPlane)), _bufferToRender.height());
  if(f==NULL)
	f=fopen(testFile, "wb");
  if(f!=NULL )
	{
		fwrite(_bufferToRender.buffer(kYPlane), _bufferToRender.allocated_size(kYPlane), 1, f);
		fwrite(_bufferToRender.buffer(kUPlane), _bufferToRender.allocated_size(kUPlane), 1, f);
		fwrite(_bufferToRender.buffer(kVPlane), _bufferToRender.allocated_size(kVPlane), 1, f);
		
	}
}
#endif
  _openGLRenderer.Render(_bufferToRender);
  _renderCritSect.Leave();
}

/*
 * JNI callback from Java class. Called when the GLSurfaceview
 * have created a surface. Called from the GLRenderThread
 * Method:    CreateOpenGLNativeStatic
 * Signature: (JII)I
 */
jint JNICALL AndroidNativeOpenGl2Channel::CreateOpenGLNativeStatic(
    JNIEnv * env,
    jobject,
    jlong context,
    jint width,
    jint height) {
  AndroidNativeOpenGl2Channel* renderChannel =
      reinterpret_cast<AndroidNativeOpenGl2Channel*> (context);
  WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, -1, "%s: %d, %d", __FUNCTION__, width, height);
  return renderChannel->CreateOpenGLNative(width, height);
}

jint AndroidNativeOpenGl2Channel::CreateOpenGLNative(
    int width, int height) {
    _width = width;
	_height = height;
  return _openGLRenderer.Setup(width, height);
}

WebRtc_Word32 AndroidNativeOpenGl2Channel::GetScreenResolution(int *width, int *height)
{
	if(_width == 0 || _height == 0)
		return -1;
	
	*width = _width;
	*height = _height;

	return 0;
}

void AndroidNativeOpenGl2Channel::ConfigureRenderMode(const int scaledmode)
{
   //__android_log_print(ANDROID_LOG_ERROR, "yyf","opengl config render mode, scaledmode = %d", scaledmode);
	scaledmode_ = scaledmode;
}

}  //namespace webrtc
