#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "../../inc/ffmpegdemuxdec_api.h"

#define  INBUF_SIZE 81920

audio_dst_filename = "audio.pcm";
video_dst_filename = "video.yuv";

FILE *audiofp;
FILE *videofp;

void WriteDecodeDate(int gotfame,FrameInfo *frame,void *handle)
{
	switch(gotfame)
	{
	case 0:
		if (videofp != NULL)
		{
			fwrite(frame->Buffer,1,frame->FrameLen,videofp);
			printf("read size = %4d, displayer number = %4d\n",frame->ReadBytes,frame->TimeStamp);
		}
		break;
	case 1:
		if (audiofp != NULL)
		{
			fwrite(frame->Buffer,1,frame->FrameLen,audiofp);
			//printf("read size = %d\n",frame->ReadBytes);
		}
		break;
	default:
		printf("ERROR\n");
	}
}

int main()
{
	unsigned long	GVE_FFDemux_Handle = 0;
	GVE_FFDemux_OperatePar  OperatePar = {0};
	GVE_FFDemux_OutPutInfo  OutPutInfo = {0}; 
	GVE_FFDemux_ConfigPar   ConfigPar  = {0};

	//塘崔歌方��淫凄猟周兆才指距痕方廣過��
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	ConfigPar.input_filename = "wildlife1.flv";
	ConfigPar.pfun = WriteDecodeDate;
    //！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

	audiofp = fopen(audio_dst_filename, "wb");
	if (!audiofp) {
		printf("Could not open %s\n", audio_dst_filename);
	}

	videofp = fopen(video_dst_filename, "wb");
	if (!videofp) {
		printf( "Could not open %s\n", video_dst_filename);
	}

	//兜兵晒佚連��岻朔誼欺謹箪悶猟周議佚連��
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	GVE_FFMPEG_Demux_Create(&GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

	//補竃猟周式盾鷹篇撞咄撞佚連��
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	printf("  Duration: ");
	printf("%02d:%02d:%02d.%02d", OutPutInfo.fileinfo.hours, OutPutInfo.fileinfo.mins, OutPutInfo.fileinfo.secs,
		OutPutInfo.fileinfo.us);
	printf( ", bitrate: ");
	printf( "%d kb/s\n", OutPutInfo.fileinfo.BitRate);  //鷹楕
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

	//兜兵晒俶勣議篇撞式咄撞buffer
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	OperatePar.audio_dst_data = malloc(INBUF_SIZE *sizeof(char));
	if(OperatePar.audio_dst_data == NULL)
	{
		exit(1);
	}
	OperatePar.video_dst_len = OutPutInfo.videoinfo.Width * OutPutInfo.videoinfo.Height * 3 /2;
	OperatePar.video_dst_data = malloc(OperatePar.video_dst_len  *sizeof(char));
	if(OperatePar.video_dst_data == NULL)
	{
		exit(1);
	}
	//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

	//譜崔seek了崔
	//-------------------------------------------------------------------------------------------------------------
	//ConfigPar.seek_perscent = 0.52;

   //蝕兵盾鷹謹箪悶猟周
	//---------------------------------------------------------------------------------------------------------------
	GVE_FFMPEG_Demux_Decoder(GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

// 	Sleep(1000);
// 
// 	GVE_FFMPEG_Demux_Suspend(GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
// 
// 	Sleep(1000);
// 	GVE_FFMPEG_Demux_Resume(GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

	Sleep(40000);

	GVE_FFMPEG_Demux_Destroy(GVE_FFDemux_Handle);

	if (OperatePar.audio_dst_data != NULL)
	{
		free(OperatePar.audio_dst_data);
	}
	if (OperatePar.video_dst_data != NULL)
	{
		free(OperatePar.video_dst_data);
	}

	fclose(audiofp);
	fclose(videofp);

	getchar();

}
