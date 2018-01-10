#ifndef __LIBFFMPEG_DEMUX_API_H__
#define __LIBFFMPEG_DEMUX_API_H__

#define inline  __inline
#define __inline__  __inline

//#define snprintf _snprintf

#include "../inc/ffmpegdemuxdec_api.h"

//#include <process.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>

typedef struct
{
	//
	AVFormatContext *fmt_ctx;
	AVCodecContext *video_dec_ctx; 
	AVCodecContext *audio_dec_ctx;
	AVStream *video_stream;
	AVStream *audio_stream;

	int dst_nb_samples;
	int dst_nb_channels;
	struct SwrContext *swr_ctx;
	uint8_t  **resampledata;
	int resamplelinesize;
	int video_dst_bufsize;

	int video_stream_idx; 
	int audio_stream_idx;
	AVFrame *frame;
	AVPacket pkt;
	int got_frame;
	int resampleflag;

	void	*hThread;
	char *video_dst_data;
	char *audio_dst_data;
	int  video_dst_len;
	int audio_dst_len;
	void *fun_par;
	pcallbackfun pfun;

	volatile int suspenddecode;
	volatile int terminatedecode;

}FFmpegDmux_Handle;

#endif
