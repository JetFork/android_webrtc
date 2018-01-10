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

	//ÅäÖÃ²ÎÊý£¬°üÀ¨ÎÄ¼þÃûºÍ»Øµ÷º¯Êý×¢²á£»
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª
	ConfigPar.input_filename = "wildlife1.flv";
	ConfigPar.pfun = WriteDecodeDate;
    //¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª

	audiofp = fopen(audio_dst_filename, "wb");
	if (!audiofp) {
		printf("Could not open %s\n", audio_dst_filename);
	}

	videofp = fopen(video_dst_filename, "wb");
	if (!videofp) {
		printf( "Could not open %s\n", video_dst_filename);
	}

	//³õÊ¼»¯ÐÅÏ¢£¬Ö®ºóµÃµ½¶àÃ½ÌåÎÄ¼þµÄÐÅÏ¢£»
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª
	GVE_FFMPEG_Demux_Create(&GVE_FFDemux_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª

	//Êä³öÎÄ¼þ¼°½âÂëÊÓÆµÒôÆµÐÅÏ¢£»
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª
	printf("  Duration: ");
	printf("%02d:%02d:%02d.%02d", OutPutInfo.fileinfo.hours, OutPutInfo.fileinfo.mins, OutPutInfo.fileinfo.secs,
		OutPutInfo.fileinfo.us);
	printf( ", bitrate: ");
	printf( "%d kb/s\n", OutPutInfo.fileinfo.BitRate);  //ÂëÂÊ
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª

	//³õÊ¼»¯ÐèÒªµÄÊÓÆµ¼°ÒôÆµbuffer
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª
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
	//¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª¡ª

	//ÉèÖÃseekÎ»ÖÃ
	//-------------------------------------------------------------------------------------------------------------
	//ConfigPar.seek_perscent = 0.52;

   //¿ªÊ¼½âÂë¶àÃ½ÌåÎÄ¼þ
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
