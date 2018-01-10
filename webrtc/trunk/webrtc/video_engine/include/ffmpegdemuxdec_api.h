#ifndef __FFMPEG_DEMUX_API_H__
#define __FFMPEG_DEMUX_API_H__

#ifdef __cplusplus
extern "C" {
#endif
#if !defined (WIN32) && !defined (WIN64)
#ifdef __LP64__
    typedef long int __int64;
#else
    typedef long long int __int64;
#endif /* __LP64__ */
#endif

typedef struct
{
	char *Buffer;
	int FrameLen;
	__int64 ReadBytes;
	double TimeStamp;

}FrameInfo;

typedef void (*pcallbackfun)(int gotfame,FrameInfo *frame,void *handle);

typedef struct
{
	int hours; 
	int mins;
	int secs;
	int us;
	int BitRate;
	int Type;
}FileInfo;

typedef struct
{
	int SampleRate;
	int BytesPerSample;
	int BitRate;
	int Channels;

}AudioFileInfo;

typedef struct
{
	int Width;
	int Height;
	int FPS;
	int BitRate;

}VideoFileInfo;

typedef struct 
{
	char *video_dst_data;
	char *audio_dst_data;
	int  video_dst_len;
	int audio_dst_len;

}GVE_FFDemux_OperatePar;

typedef struct
{
	const char *input_filename;
	pcallbackfun pfun;
	double seek_perscent;
	void *pfun_par;

}GVE_FFDemux_ConfigPar;

typedef struct 
{
	FileInfo fileinfo;
	AudioFileInfo audioinfo;
	VideoFileInfo videoinfo;

}GVE_FFDemux_OutPutInfo;


int GVE_FFMPEG_Demux_Create(unsigned long           *GVE_FFDemux_Handle,
							GVE_FFDemux_OperatePar *OperatePar,
							GVE_FFDemux_ConfigPar  *ConfigPar,
							GVE_FFDemux_OutPutInfo *OutPutInfo);

int GVE_FFMPEG_Demux_Decoder(unsigned long            GVE_FFDemux_Handle,
							 GVE_FFDemux_OperatePar *OperatePar,
							 GVE_FFDemux_ConfigPar  *ConfigPar,
							 GVE_FFDemux_OutPutInfo *OutPutInfo);


//用于解码线程暂停
void GVE_FFMPEG_Demux_Suspend(unsigned long            GVE_FFDemux_Handle);

//用以恢复暂停的解码线程
void GVE_FFMPEG_Demux_Resume(unsigned long            GVE_FFDemux_Handle);

void GVE_FFMPEG_Demux_Destroy(unsigned long           GVE_FFDemux_Handle);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif
