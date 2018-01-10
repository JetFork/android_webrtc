/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video_capture_ds.h"

#include "../video_capture_config.h"
#include "critical_section_wrapper.h"
#include "help_functions_ds.h"
#include "sink_filter_ds.h"
#include "trace.h"

#include <Dvdmedia.h> // VIDEOINFOHEADER2
#define  DEFAULT_CAPTURE_WIDHT 640
#define  DEFAULT_CAPTURE_HEIGHT 480
namespace webrtc
{
namespace videocapturemodule
{
 

VideoCaptureDS::VideoCaptureDS(const WebRtc_Word32 id)
    : VideoCaptureImpl(id), _dsInfo(id), m_pSampleGrabber(NULL),m_pGrabberFilter(NULL),_captureFilter(NULL),
       _mediaControl(NULL), m_pProcAmp(NULL), m_Sharpness(0)/*, _sinkFilter(NULL),*/
      /*_inputSendPin(NULL), _outputCapturePin(NULL), _dvFilter(NULL),
      _inputDvPin(NULL), _outputDvPin(NULL)*/
{
	m_pmCB= new CSampleGrabberCB(*this);

}

VideoCaptureDS::~VideoCaptureDS()
{
	RELEASE_AND_CLEAR(m_pProcAmp); 
	if(m_pmCB)
	m_pmCB->SetGrabData(false);
    if (_mediaControl)
    {
        _mediaControl->StopWhenReady();
		_mediaControl->Stop();
    }
    if (_dsInfo.GetGraphBuilderPtr())
    {
        if (_captureFilter)
            _dsInfo.GetGraphBuilderPtr()->RemoveFilter(_captureFilter);
    }
 
	if(m_pGrabberFilter)		 
			_dsInfo.GetGraphBuilderPtr()->RemoveFilter(m_pGrabberFilter);

	RELEASE_AND_CLEAR(m_pSampleGrabber); 
	RELEASE_AND_CLEAR(m_pGrabberFilter); 
	RELEASE_AND_CLEAR(_mediaControl);
	RELEASE_AND_CLEAR(iconfig);
	if(m_pmCB!=NULL)
	{

		delete m_pmCB;
		m_pmCB =NULL;
	}
	RELEASE_AND_CLEAR(_captureFilter); // release the capture device
}


#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
//得到摄像头支持的相关格式，包括支持的所有媒体格式及当前媒体格式
BOOL  VideoCaptureDS::GetCamParams(ICaptureGraphBuilder2* pBuilder,IBaseFilter* pCap) 
{

	IAMStreamConfig* iconfig = NULL; 

	HRESULT hr = S_OK;
	hr = pBuilder->FindInterface( &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Interleaved, pCap,IID_IAMStreamConfig,(void**)&iconfig); 
	if(hr!=NOERROR)
	{ 
		hr=pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,pCap,IID_IAMStreamConfig,(void**)&iconfig); 
	} 

	if(FAILED(hr)) 
		return FALSE; 

	int nCount = 0;
	int nSize = 0;
	hr = iconfig->GetNumberOfCapabilities(&nCount, &nSize);

	// 判断是否为视频信息
	if (sizeof(VIDEO_STREAM_CONFIG_CAPS) == nSize)
	{

		//清空_captureCapabilities
		MapItem* item = NULL;
		while (item = _captureCapabilities.Last())
		{
			VideoCaptureCapabilityWindows* cap =static_cast<VideoCaptureCapabilityWindows*> (item->GetItem());
			delete cap;
			_captureCapabilities.Erase(item);
		}

		 
		for (int i=0; i<nCount; i++)
		{
			VIDEO_STREAM_CONFIG_CAPS scc;
			AM_MEDIA_TYPE* pmmt;

			hr = iconfig->GetStreamCaps(i, &pmmt, (BYTE *)&scc);//得到摄像头支持的所有媒体格式
			CAMERAINFO info;

			if (pmmt->formattype == FORMAT_VideoInfo || pmmt->formattype == FORMAT_VideoInfo2)
			{

				VideoCaptureCapabilityWindows* capability = new VideoCaptureCapabilityWindows();
				VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER *)pmmt->pbFormat;
				capability->height =  pvih->bmiHeader.biHeight;
			   capability->width =pvih->bmiHeader.biWidth;
				capability->expectedCaptureDelay =0;
				capability->maxFPS =10000000/pvih->AvgTimePerFrame;
				UnconvertMediaType(pmmt,capability->rawType);
				 _captureCapabilities.Insert(i, capability);
				 
			}

		}
	}
	SAFE_RELEASE(iconfig); 
	return TRUE; 
}
WebRtc_Word32 VideoCaptureDS::Init(const WebRtc_Word32 id,
                                          const char* deviceUniqueIdUTF8)
{
    const WebRtc_Word32 nameLength =
        (WebRtc_Word32) strlen((char*) deviceUniqueIdUTF8);
    if (nameLength > kVideoCaptureUniqueNameLength)
        return -1;

    // Store the device name
    _deviceUniqueId = new (std::nothrow) char[nameLength + 1];
    memcpy(_deviceUniqueId, deviceUniqueIdUTF8, nameLength + 1);

    if (_dsInfo.Init() != 0)
        return -1;
    return 0;
}

WebRtc_Word32 VideoCaptureDS::StartCapture(  const VideoCaptureCapability& capability)
{
	CriticalSectionScoped cs(&_apiCs);
	_captureFilter = _dsInfo.GetDeviceFilter(_deviceUniqueId);
	if (!_captureFilter)
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to create capture filter.");
		return -1;
	}

	HRESULT hr;
	//CComPtr <IAMVideoProcAmp> pProcAmp;
	hr = _captureFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&m_pProcAmp);
	if(SUCCEEDED(hr))
	{
		long Min, Max, Step, Default, Flags, Val;
		if( m_Sharpness == 0 )
		{
			hr = m_pProcAmp->GetRange(VideoProcAmp_Sharpness, &Min, &Max, &Step,
				&Default, &Flags);
			if(SUCCEEDED(hr))
			{
				hr = m_pProcAmp->Get(VideoProcAmp_Sharpness, &Val, &Flags);
				m_Sharpness = Val;
				Val = Default + (((Max-Min)*0.2)/Step)*Step;
				hr = m_pProcAmp->Set(VideoProcAmp_Sharpness, Val, Flags);
				if(SUCCEEDED(hr))
				{
					WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCapture, _id,
						"Set the camera sharpness, %d -> %d", m_Sharpness, Val);
				}
			}
		}
	}

	hr= _dsInfo.GetGraphBuilderPtr()->QueryInterface(IID_IMediaControl,
		(void **) &_mediaControl);
	if (FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to create media control builder.");
		return -1;
	}
	hr = _dsInfo.GetGraphBuilderPtr()->AddFilter(_captureFilter, CAPTURE_FILTER_NAME);
	if (FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to add the capture device to the graph.");
		return -1;
	}
	hr = _dsInfo.GetCpatureGraphBuilder2Ptr()->SetFiltergraph(_dsInfo.GetGraphBuilderPtr());
	if(FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to SetFiltergraph.");
		return hr;
	}
	//init grab ISampleGrabber;

	// 创建ISampeGrabber过滤器
	hr   =   CoCreateInstance(CLSID_SampleGrabber,   NULL,   CLSCTX_INPROC_SERVER, IID_IBaseFilter,   (void**)&m_pGrabberFilter); 
	if(FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to CoCreateInstance.");
		return hr;
	}

	hr = _dsInfo.GetGraphBuilderPtr()->AddFilter(m_pGrabberFilter, _T("Grabber Filter"));
	if(FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to AddFilter.");
		return hr;
	}

	hr =	m_pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void **)&m_pSampleGrabber);
	if(FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to QueryInterface.");
		return hr;
	}  

	hr = _dsInfo.GetCpatureGraphBuilder2Ptr()->RenderStream (&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,_captureFilter, NULL,m_pGrabberFilter);
	if(FAILED(hr))
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to RenderStream.");
		return hr;
	}  
	// Temporary connect here.
	// This is done so that no one else can use the capture device.
	if (SetCameraOutput(_requestedCapability) != 0)
	{
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to SetCameraOutput.");
		return -1;
	}

 
	m_pmCB->SetGrabData(true);
	if(_mediaControl)
	{
		hr = _mediaControl->Run();
	}
	if (FAILED(hr))
	{
		StopCapture();
		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
			"Failed to start the Capture device.");
		return -1;
	}
	return 0;
}

WebRtc_Word32 VideoCaptureDS::StopCapture()
{
    CriticalSectionScoped cs(&_apiCs);
	if (m_pProcAmp != NULL)
	{
		HRESULT hr;
		long Val, Flags;
		hr = m_pProcAmp->Get(VideoProcAmp_Sharpness, &Val, &Flags);
		hr = m_pProcAmp->Set(VideoProcAmp_Sharpness, m_Sharpness, Flags);
		if(SUCCEEDED(hr))
		{
			WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCapture, _id,
				"Reset the camera default sharpness, %d", m_Sharpness);
		}
	}
	RELEASE_AND_CLEAR(m_pProcAmp); 
	
	m_pmCB->SetGrabData(false);
	if (_mediaControl)
	{
		_mediaControl->StopWhenReady();
		_mediaControl->Stop();
	}
	if (_dsInfo.GetGraphBuilderPtr())
	{
		if (_captureFilter)
			_dsInfo.GetGraphBuilderPtr()->RemoveFilter(_captureFilter);
	}

	if(m_pGrabberFilter)		 
		_dsInfo.GetGraphBuilderPtr()->RemoveFilter(m_pGrabberFilter);

	RELEASE_AND_CLEAR(m_pSampleGrabber); 
	RELEASE_AND_CLEAR(m_pGrabberFilter); 
	RELEASE_AND_CLEAR(_mediaControl);
	RELEASE_AND_CLEAR(iconfig);
	if(m_pmCB!=NULL)
	{

		delete m_pmCB;
		m_pmCB =NULL;
	}
	RELEASE_AND_CLEAR(_captureFilter); // release the capture device

    return 0;
}
bool VideoCaptureDS::CaptureStarted()
{
    OAFilterState state = 0;

	if(_mediaControl==NULL)
		return false;
    HRESULT hr = _mediaControl->GetState(1000, &state);
    if (hr != S_OK && hr != VFW_S_CANT_CUE)
    {
        WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                     "Failed to get the CaptureStarted status");
    }
    WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
                 "CaptureStarted %d", state);
    return state == State_Running;

}
WebRtc_Word32 VideoCaptureDS::CaptureSettings(
                                             VideoCaptureCapability& settings)
{
    settings = _requestedCapability;
    return 0;
}
void VideoCaptureDS::UnconvertMediaType(AM_MEDIA_TYPE *pmt, RawVideoType &type)
{
	char * subname =GuidNames[pmt->subtype];

	if(strcmp(subname,"MEDIASUBTYPE_I420")==0)
		type = kVideoI420;
	else if(strcmp(subname,"MEDIASUBTYPE_IYUV")==0)
		type= kVideoIYUV;
	else if(strcmp(subname,"MEDIASUBTYPE_RGB24")==0)
		type =kVideoRGB24;
	else if(strcmp(subname,"MEDIASUBTYPE_ARGB4444")==0)
		type = kVideoARGB4444;
	else if(strcmp(subname,"MEDIASUBTYPE_RGB565")==0)
		type = kVideoRGB565;
	else if(strcmp(subname,"MEDIASUBTYPE_ARGB1555")==0)
		type = kVideoARGB1555;
	else if(strcmp(subname,"MEDIASUBTYPE_YUY2")==0)
		type = kVideoYUY2;
	else if(strcmp(subname,"MEDIASUBTYPE_YV12")==0)
		type = kVideoYV12;
	else if(strcmp(subname,"MEDIASUBTYPE_UYVY")==0)
		type= kVideoUYVY;
	else if(strcmp(subname,"MEDIASUBTYPE_NV12")==0)
		type = kVideoNV12;
	else if(strcmp(subname,"MEDIASUBTYPE_MJPG")==0)
		type = kVideoMJPEG;
	else
		type =kVideoYUY2;
	 

}
void VideoCaptureDS::convertMediaType(AM_MEDIA_TYPE *pmt, RawVideoType type)
{
		switch (type)
		{
	case kVideoI420:
		  pmt->subtype =MEDIASUBTYPE_RGB24;
		  break;
	case kVideoIYUV:
		 pmt->subtype =MEDIASUBTYPE_IYUV;
		break;
	case kVideoRGB24:
		pmt->subtype =MEDIASUBTYPE_RGB24;
		break;;
	/*case kVideoARGB:
		return pmt->subtype =MEDIASUBTYPE_ARGB32;;*/
	case kVideoARGB4444:
		 pmt->subtype =MEDIASUBTYPE_ARGB4444;
		 break;
	case kVideoRGB565:
		 pmt->subtype =MEDIASUBTYPE_RGB565;
		 break;
	case kVideoARGB1555:
		 pmt->subtype =MEDIASUBTYPE_ARGB1555;
		 break;
	case kVideoYUY2:
		 pmt->subtype =MEDIASUBTYPE_YUY2;
		 break;
	case kVideoYV12:
		 pmt->subtype =MEDIASUBTYPE_YV12;
		 break;
	case kVideoUYVY:
		  pmt->subtype =MEDIASUBTYPE_UYVY;
		  break;
	/*case kVideoNV21:
		return pmt->subtype =MEDIASUBTYPE_NV21;;*/
	case kVideoNV12:
		  pmt->subtype =MEDIASUBTYPE_NV12;
		  break;
	/*case kVideoBGRA:
		return pmt->subtype =MEDIASUBTYPE_B;;*/
	case kVideoMJPEG:
		  pmt->subtype =MEDIASUBTYPE_MJPG;
		  break;
	default:
		  pmt->subtype =MEDIASUBTYPE_YUY2;
	}
}


WebRtc_Word32 VideoCaptureDS::CreateCapabilityMap()
{
		GetCamParams(_dsInfo.GetCpatureGraphBuilder2Ptr(),_captureFilter);
return 0;
}

WebRtc_Word32 VideoCaptureDS::GetBestMatchedCapability(
													   const VideoCaptureCapability& requested,
													   VideoCaptureCapability& resulting)
{

	CreateCapabilityMap();

	const WebRtc_Word32 numberOfCapabilies = _captureCapabilities.Size();
	//默认选择640*480 
	for (WebRtc_Word32 tmp = 0; tmp < numberOfCapabilies; ++tmp) // Loop through all capabilities
	{
		MapItem* item = _captureCapabilities.Find(tmp);
		if (!item)
			return -1;

		VideoCaptureCapability *capability = static_cast<VideoCaptureCapability*>(item->GetItem());

		if(capability->width == DEFAULT_CAPTURE_WIDHT&& capability->height==DEFAULT_CAPTURE_HEIGHT)
		{
			if(capability->rawType==kVideoRGB24|| capability->rawType==kVideoYUY2)
			{

				resulting = *capability;
				WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCapture, _id,"Selected Capture default resolution is %dx%d",resulting.width,resulting.height);
				return 0;
			}
		}
	}


	//没有640*480 匹配的选择第一个
	if(numberOfCapabilies>0)
	{
		MapItem* item = _captureCapabilities.Find(0);
		if (!item)
			return -1;
		VideoCaptureCapability *capability = static_cast<VideoCaptureCapability*>(item->GetItem());

		resulting = *capability;
		WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCapture, _id,"Selected Capture the 1st resolution is %dx%d",resulting.width,resulting.height);
		return 0;
	}

	WEBRTC_TRACE(webrtc::kTraceStateInfo, webrtc::kTraceVideoCapture, _id,"Selected Capture resolution is %dx%d",resulting.width,resulting.height);
	return 0;
}
WebRtc_Word32 VideoCaptureDS::SetCameraOutput(
                             const VideoCaptureCapability& requestedCapability)
{

    // Get the best matching capability
    VideoCaptureCapability capability;
    WebRtc_Word32 capabilityIndex;

    // Store the new requested size
    _requestedCapability = requestedCapability;
  
   if ((capabilityIndex = GetBestMatchedCapability( _requestedCapability,capability)) < 0)
    {
        return -1;
    }
    //Reduce the frame rate if possible.
    if (capability.maxFPS > requestedCapability.maxFPS)
    {
        capability.maxFPS = requestedCapability.maxFPS;
    } else if (capability.maxFPS <= 0)
    {
        capability.maxFPS = 30;
    }
	 if(capability.width <=0)
		 capability.width =640;
	 if(capability.height<=0)
		 capability.height =480;
    // Store the new expected capture delay
    _captureDelay = capability.expectedCaptureDelay;

	HRESULT hr = _dsInfo.GetCpatureGraphBuilder2Ptr()->FindInterface(&PIN_CATEGORY_CAPTURE,&MEDIATYPE_Video, _captureFilter,IID_IAMStreamConfig, (void **)&iconfig);
	if (FAILED(hr)) 
		return hr;
	if(iconfig)
	{
		AM_MEDIA_TYPE *pmt;
		//得到当前默认采集格式
		hr = iconfig->GetFormat(&pmt);
	/*	VIDEOINFOHEADER* phead=(VIDEOINFOHEADER*)(pmt->pbFormat);
		int width = phead->bmiHeader.biWidth ; 
		int height=phead->bmiHeader.biHeight; 
		int averageTime =phead->AvgTimePerFrame;*/

		//capability.width =640;
		//capability.height =480;
		//设置新的分辨率
		//设置视频采集格式
		VIDEOINFOHEADER *phead = (VIDEOINFOHEADER *)pmt->pbFormat;
		phead->bmiHeader.biWidth= capability.width;
		phead->bmiHeader.biHeight= capability.height;
		phead->AvgTimePerFrame = 1000/15;
		convertMediaType(pmt,capability.rawType);// 
		pmt->majortype =MEDIATYPE_Video;
		hr = iconfig->SetFormat(pmt);
		if (FAILED(hr)) 
		{

			WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,"Failed to SetFormat.");
			return hr;
		}
			
		WEBRTC_TRACE(webrtc::kTraceStateInfo, kTraceVideoCapture, _id,"Capture setting: width= %d,height=%d,rawType=%d",capability.width,capability.height,capability.rawType);

		hr = m_pSampleGrabber->SetMediaType(pmt);
		if (FAILED(hr)) 
		{
			WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,"Failed to SetMediaType.");
			return hr;
		}

		DeleteMediaType(pmt);
	}
	if(m_pmCB==NULL)
		return -1;

	m_pmCB->lWidth = capability.width;
	m_pmCB->lHeight = capability.height;
	m_pmCB->iRawdatatype =capability.rawType;
	m_pmCB->bGrabVideo = FALSE ; 
	
	m_pmCB->SetGrabData(true);
	hr = m_pSampleGrabber->SetBufferSamples(FALSE);
	hr = m_pSampleGrabber->SetOneShot(FALSE);
	hr = m_pSampleGrabber->SetCallback(m_pmCB, 1);

    return 0;
}

WebRtc_Word32 VideoCaptureDS::DisconnectGraph()
{
    HRESULT hr = _mediaControl->Stop();
	if (FAILED(hr)) 
	{

		WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,"Failed to DisconnectGraph.");
	   return hr;
	}
    return 0;
}

} // namespace videocapturemodule
} //namespace webrtc
