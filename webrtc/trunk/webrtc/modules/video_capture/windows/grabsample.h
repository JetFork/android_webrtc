#ifndef GRABSAMPLE_H_
#define GRABSAMPLE_H_
#include <atlbase.h>
#include <windows.h>
#include <dshow.h>
#include <Qedit.h>
#include "video_capture_defines.h"
namespace webrtc
{
	namespace videocapturemodule
	{
class CSampleGrabberCB : public ISampleGrabberCB 
{
public:
	
	FILE * fptest;
	long	lWidth ; 
	long	lHeight ;
	int     iRawdatatype;
	//CVdoFrameHandler *  frame_handler ; 
	BOOL	bGrabVideo ; 
public:
	void  SetGrabData(bool grab) { bGrabVideo = grab;}
	CSampleGrabberCB(VideoCaptureExternal& captureObserver):_captureObserver(captureObserver)
	{ 
		lWidth = 0 ; 
		lHeight = 0 ;
		iRawdatatype =-1;
		bGrabVideo = FALSE ; 
		//frame_handler = NULL ; 
		fptest =NULL;
	} 
	STDMETHODIMP_(ULONG) AddRef() 
	{ 
		return 2; 
	}
	STDMETHODIMP_(ULONG) Release() 
	{ 
		return 1; 
	}
	STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
	{
		if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown )
		{ 
			*ppv = (void *) static_cast<ISampleGrabberCB*> (this);
			return NOERROR;
		} 
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample )
	{
		int x=100;
		return 0;
	}
	STDMETHODIMP BufferCB( double dblSampleTime, BYTE * pBuffer, long lBufferSize )
	{
#if 0
		if(fptest ==NULL)
			fptest = fopen("c:\\rgb.yuv","wb");
		if (!pBuffer) 
			return E_POINTER;
		//if(bGrabVideo/* && frame_handler*/) 
		{
			//frame_handler->VdoFrameData(lWidth,lHeight,pBuffer,lBufferSize);
			fwrite(pBuffer,1,lBufferSize,fptest);
			fflush(fptest);
		}
#endif
		VideoCaptureCapability frameInfo;
		if(lWidth ==0 ||lHeight ==0)
		{

			lWidth =640;
			lHeight =480;
		}
		frameInfo.width = lWidth;
		frameInfo.height =lHeight;
		frameInfo.rawType= (RawVideoType)iRawdatatype;
		if(bGrabVideo)
		_captureObserver.IncomingFrame((unsigned char *)pBuffer,lBufferSize,frameInfo);

		return 0;
	}
	VideoCaptureExternal& _captureObserver;

};
}
}
#endif 