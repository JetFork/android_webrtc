/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_WINDOWS_VIDEO_CAPTURE_DS_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_WINDOWS_VIDEO_CAPTURE_DS_H_

#include "../video_capture_impl.h"
#include <tchar.h>
#include <Qedit.h>
#include "device_info_ds.h"
#include <atlbase.h>
#include <windows.h>
#include <dshow.h>
#include <Qedit.h>
#include "grabsample.h"
#define CAPTURE_FILTER_NAME L"VideoCaptureFilter"
#define SINK_FILTER_NAME L"SinkFilter"
#include <vector>
using namespace std;
namespace webrtc
{
namespace videocapturemodule
{
// Forward declaraion
class CaptureSinkFilter;

typedef struct CameraInfo
{
	char mainType[30];
	char subType[30];
	long width;
	long height;
	double frameRate;
}CAMERAINFO;

class VideoCaptureDS: public VideoCaptureImpl
{
public:
    VideoCaptureDS(const WebRtc_Word32 id);

    virtual WebRtc_Word32 Init(const WebRtc_Word32 id,
                               const char* deviceUniqueIdUTF8);

    /*************************************************************************
     *
     *   Start/Stop
     *
     *************************************************************************/
    virtual WebRtc_Word32
        StartCapture(const VideoCaptureCapability& capability);
    virtual WebRtc_Word32 StopCapture();

    /**************************************************************************
     *
     *   Properties of the set device
     *
     **************************************************************************/

    virtual bool CaptureStarted();
    virtual WebRtc_Word32 CaptureSettings(VideoCaptureCapability& settings);
	 void convertMediaType(AM_MEDIA_TYPE *pmt, RawVideoType type);
   void UnconvertMediaType(AM_MEDIA_TYPE *pmt, RawVideoType &type);
protected:
    virtual ~VideoCaptureDS();

    // Help functions
	BOOL  VideoCaptureDS::GetCamParams(ICaptureGraphBuilder2* pBuilder,IBaseFilter* pCap);
    WebRtc_Word32 SetCameraOutput(const VideoCaptureCapability& requestedCapability);

	WebRtc_Word32 CreateCapabilityMap();
	WebRtc_Word32 GetBestMatchedCapability(const VideoCaptureCapability& requested,VideoCaptureCapability& resulting);
    WebRtc_Word32 DisconnectGraph();
    DeviceInfoDS _dsInfo;


	ISampleGrabber			*m_pSampleGrabber;
	IBaseFilter				*m_pGrabberFilter;
	//ISampleGrabber*   m_pGrabber;
	//ICaptureGraphBuilder2* m_pCaptureBuild2;
    IBaseFilter* _captureFilter;
   // IGraphBuilder* _graphBuilder;
    IMediaControl* _mediaControl;
	CSampleGrabberCB *m_pmCB;
	vector<CAMERAINFO> m_vecCamAllInfo;
	CAMERAINFO m_currentCamInfo;
	 IAMStreamConfig *iconfig;      // for video cap
	MapWrapper _captureCapabilities;

	IAMVideoProcAmp *m_pProcAmp;
	long m_Sharpness;

};
} // namespace videocapturemodule
} //namespace webrtc
#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_WINDOWS_VIDEO_CAPTURE_DS_H_
