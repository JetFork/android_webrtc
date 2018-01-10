#include <stdio.h>
#include <stdlib.h>
#include "libffmpegdemuxdec.h"
#if !defined(WIN32) && !defined(WIN64)
#include <pthread.h>
#define WINAPI
#define USE_PTHREAD
#include <stdbool.h>
#else
#include <Windows.h>
#endif
#ifdef ANDROID
#include <android/log.h>
#define LOG(...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg_log_tag", __VA_ARGS__)
#else 
#define LOG(...) 
#endif
/*
volatile int terminatedecode = 0;
#if !defined(WIN32) && !defined(WIN64)
volatile int suspenddecode = 1;
#else
volatile int suspenddecode = 0;
#endif
*/
static int decode_packet(FFmpegDmux_Handle *ffmpegInfo,char*audio_buffer,int audio_len,char *video_buffer,int video_len,pcallbackfun fun,__int64 readBytes)
{
	//SYSTEMTIME sys;
	int audio_dst_linesize;
	size_t unpadded_linesize,dst_nb_samples;
	int ret = 0;
	int decoded = ffmpegInfo->pkt.size;
	FrameInfo cur_frame;
	uint8_t *dst_data[4];
	int dst_linesizes[4];

	ffmpegInfo->got_frame = 0;

	if (ffmpegInfo->pkt.stream_index == ffmpegInfo->video_stream_idx) 
	{
		/* decode video frame */
		ret = avcodec_decode_video2(ffmpegInfo->video_dec_ctx, ffmpegInfo->frame, &ffmpegInfo->got_frame, &ffmpegInfo->pkt);
		if (ret < 0)
		{
			//fprintf(stderr, "Error decoding video frame (%s)\n"/*, av_err2str(ret)*/);
			return ret;
		} 

		if (ffmpegInfo->got_frame) {
			//GetLocalTime( &sys ); 
			//printf("time is %5d:%5d:%5d\n",sys.wMinute, sys.wSecond,sys.wMilliseconds);
			//              printf("video_frame%s n:%d coded_n:%d pts:%s\n",
			//                     cached ? "(cached)" : "",
			//                     video_frame_count++, frame->coded_picture_number,
			//                     av_ts2timestr(frame->pts, &video_dec_ctx->time_base));

			/* copy decoded frame to destination buffer:
			* this is required since rawvideo expects non aligned data */

			dst_linesizes[0] = ffmpegInfo->video_dec_ctx->width;
			dst_linesizes[1] = ffmpegInfo->video_dec_ctx->width/2;
			dst_linesizes[2] = ffmpegInfo->video_dec_ctx->width/2;
			dst_linesizes[3] = ffmpegInfo->video_dec_ctx->width/2;
			dst_data[0] = video_buffer;
			dst_data[1] = video_buffer+ ffmpegInfo->video_dec_ctx->width * ffmpegInfo->video_dec_ctx->height;
			dst_data[2] = dst_data[1] + ffmpegInfo->video_dec_ctx->width * ffmpegInfo->video_dec_ctx->height/4;
			dst_data[3] = dst_data[1] + ffmpegInfo->video_dec_ctx->width * ffmpegInfo->video_dec_ctx->height/4;
			av_image_copy(dst_data, dst_linesizes,
				(const uint8_t **)(ffmpegInfo->frame->data), ffmpegInfo->frame->linesize,
				ffmpegInfo->video_dec_ctx->pix_fmt, ffmpegInfo->video_dec_ctx->width, ffmpegInfo->video_dec_ctx->height);
			//memcpy(video_buffer,ffmpegInfo->frame->data[0],768/*ffmpegInfo->video_dec_ctx->width*/*ffmpegInfo->video_dec_ctx->height);
			//memcpy(video_buffer+/*ffmpegInfo->video_dec_ctx->width*/768*ffmpegInfo->video_dec_ctx->height,ffmpegInfo->frame->data[1],768/*ffmpegInfo->video_dec_ctx->width*/*ffmpegInfo->video_dec_ctx->height/4);
			//memcpy(video_buffer+768/*ffmpegInfo->video_dec_ctx->width*/*ffmpegInfo->video_dec_ctx->height+768/*ffmpegInfo->video_dec_ctx->width*/*ffmpegInfo->video_dec_ctx->height/4,ffmpegInfo->frame->data[2],768/*ffmpegInfo->video_dec_ctx->width*/*ffmpegInfo->video_dec_ctx->height/4);
			/* write to rawvideo file */
			cur_frame.Buffer = video_buffer;
			cur_frame.FrameLen = video_len;
			cur_frame.ReadBytes = readBytes;
			cur_frame.TimeStamp = ffmpegInfo->frame->best_effort_timestamp*av_q2d(ffmpegInfo->video_stream->time_base);
			fun(0,&cur_frame,ffmpegInfo->fun_par);
			//fwrite(video_buffer[0], 1, video_len, ffmpegInfo->video_dst_file);
		}
	} else if (ffmpegInfo->pkt.stream_index == ffmpegInfo->audio_stream_idx) {
		/* decode audio frame */
		ret = avcodec_decode_audio4(ffmpegInfo->audio_dec_ctx, ffmpegInfo->frame, &ffmpegInfo->got_frame, &ffmpegInfo->pkt);
		if (ret < 0) {
			//fprintf(stderr, "Error decoding audio frame (%s)\n"/*, av_err2str(ret)*/);
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		* called again with the remainder of the packet data.
		* Sample: fate-suite/lossless-audio/luckynight-partial.shn
		* Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, ffmpegInfo->pkt.size);

		if (ffmpegInfo->got_frame) {
			//GetLocalTime( &sys ); 
			// 			printf("time is %5d:%5d:%5d\n",sys.wMinute, sys.wSecond,sys.wMilliseconds);
			/*size_t */unpadded_linesize = ffmpegInfo->frame->nb_samples * av_get_bytes_per_sample(ffmpegInfo->frame->format);
			//             printf("audio_frame%s n:%d nb_samples:%d pts:%s\n",
			//                    cached ? "(cached)" : "",
			//                    audio_frame_count++, frame->nb_samples,
			//                    av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

			/* Write the raw audio data samples of the first plane. This works
			* fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			* most audio decoders output planar audio, which uses a separate
			* plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			* In other words, this code will write only the first audio channel
			* in these cases.
			* You should use libswresample or libavfilter to convert the frame
			* to packed data. */
			//resample
			if (ffmpegInfo->resampleflag)
			{

				/* buffer is going to be directly written to a rawaudio file, no alignment */
				dst_nb_samples = av_rescale_rnd(swr_get_delay(ffmpegInfo->swr_ctx, ffmpegInfo->audio_dec_ctx->sample_rate) + ffmpegInfo->frame->nb_samples, 16000, ffmpegInfo->audio_dec_ctx->sample_rate, AV_ROUND_UP);
				if (ffmpegInfo->dst_nb_samples  == 0)
				{
					ffmpegInfo->dst_nb_samples = dst_nb_samples;
					ffmpegInfo->dst_nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO);
					ret = av_samples_alloc_array_and_samples(&ffmpegInfo->resampledata, &ffmpegInfo->resamplelinesize, ffmpegInfo->dst_nb_channels,
						ffmpegInfo->dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
				}
				/* compute destination number of samples */
				//  				dst_nb_samples = av_rescale_rnd(swr_get_delay(ffmpegInfo->swr_ctx, ffmpegInfo->audio_dec_ctx->sample_rate) +
				//  					ffmpegInfo->frame->nb_samples, 16000, ffmpegInfo->audio_dec_ctx->sample_rate, AV_ROUND_UP);
				/* convert to destination format */
				ret = swr_convert(ffmpegInfo->swr_ctx, ffmpegInfo->resampledata, dst_nb_samples, (const uint8_t **)ffmpegInfo->frame->extended_data, ffmpegInfo->frame->nb_samples);
				if (ret < 0) 
				{
					return ret;
				}
				unpadded_linesize = av_samples_get_buffer_size(&audio_dst_linesize, av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO)/*dst_nb_channels*/,
					ret, AV_SAMPLE_FMT_S16, 1);
				memcpy(audio_buffer,ffmpegInfo->resampledata[0],unpadded_linesize);
			}
			else
			{
				memcpy(audio_buffer,ffmpegInfo->frame->extended_data[0],unpadded_linesize);  //add wyh
			}
			cur_frame.Buffer = audio_buffer;
			cur_frame.FrameLen = unpadded_linesize;
			cur_frame.ReadBytes = readBytes;
			cur_frame.TimeStamp = ffmpegInfo->frame->best_effort_timestamp*av_q2d(ffmpegInfo->audio_stream->time_base);
			fun(1,&cur_frame,ffmpegInfo->fun_par);
			//fwrite(audio_buffer, 1, unpadded_linesize, ffmpegInfo->audio_dst_file);
		}
	}

	/* If we use the new API with reference counting, we own the data and need
	* to de-reference it when we don't use it anymore */
	//     if (ffmpegInfo->got_frame && api_mode == API_MODE_NEW_API_REF_COUNT)
	//         av_frame_unref(frame);

	return decoded;
}


static int open_codec_context(int *stream_idx,
							  AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret;
	AVStream *st;
	AVCodecContext *dec_ctx = NULL;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		// 		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
		// 			av_get_media_type_string(type), src_filename);
		return ret;
	} else {
		*stream_idx = ret;
		st = fmt_ctx->streams[*stream_idx];

		/* find decoder for the stream */
		dec_ctx = st->codec;
		dec = avcodec_find_decoder(dec_ctx->codec_id);
		if (!dec) {
			// 			fprintf(stderr, "Failed to find %s codec\n",
			// 				av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		/* Init the decoders, with or without reference counting */
		// 		if (api_mode == API_MODE_NEW_API_REF_COUNT)
		// 			av_dict_set(&opts, "refcounted_frames", "1", 0);
		if ((ret = avcodec_open2(dec_ctx, dec, &opts)) < 0) {
			// 			fprintf(stderr, "Failed to open %s codec\n",
			// 				av_get_media_type_string(type));
			return ret;
		}
	}

	return 0;
}

void WINAPI FFmpegDemux_Thread(void *threadpar)
{
	int ret;
	__int64 ts;
	AVPacket orig_pkt;
	FFmpegDmux_Handle *handle = (FFmpegDmux_Handle *) threadpar;

	while (handle->terminatedecode ==0)
	{
		if ((handle->suspenddecode == 0))
		{
			if (av_read_frame(handle->fmt_ctx, &handle->pkt) <0)
			{
				continue;
			}
			ts = avio_tell(handle->fmt_ctx->pb);
			ts = (ts/32768 + ((ts%32768) ? 1:0)) * 32768;
			orig_pkt = handle->pkt;
			do {
				ret = decode_packet(handle,handle->audio_dst_data,handle->audio_dst_len,handle->video_dst_data,handle->video_dst_len,handle->pfun,ts);
				if (ret < 0)
					break;
				handle->pkt.data += ret;
				handle->pkt.size -= ret;
			} while (handle->pkt.size > 0);
			av_free_packet(&orig_pkt);
		}
	}

	return ;
}

int GVE_FFMPEG_Demux_Create(unsigned long           *GVE_FFDemux_Handle,
							GVE_FFDemux_OperatePar *OperatePar,
							GVE_FFDemux_ConfigPar  *ConfigPar,
							GVE_FFDemux_OutPutInfo *OutPutInfo)
{
	int ret = 0;
	int i;
	int64_t duration;
	FFmpegDmux_Handle *handle = NULL;

	handle = (FFmpegDmux_Handle *)malloc( sizeof(FFmpegDmux_Handle) );
	memset(handle, 0, sizeof(FFmpegDmux_Handle));
	//init handle
	handle->audio_stream_idx = -1;
	handle->video_stream_idx = -1;
	handle->hThread = NULL;


	*GVE_FFDemux_Handle = (unsigned long)handle;

	/* register all formats and codecs */
	av_register_all();

	/* open input file, and allocate format context */
	if (avformat_open_input(&handle->fmt_ctx, ConfigPar->input_filename, NULL, NULL) < 0) {
		LOG("Could not open source file %s\n", ConfigPar->input_filename);
		GVE_FFMPEG_Demux_Destroy(*GVE_FFDemux_Handle);
		*GVE_FFDemux_Handle = 0;
		return -1;
	}

	/* retrieve stream information */
	if (avformat_find_stream_info(handle->fmt_ctx, NULL) < 0) {
		LOG("Could not find stream information\n");
		GVE_FFMPEG_Demux_Destroy(*GVE_FFDemux_Handle);
		*GVE_FFDemux_Handle = 0;
		return -1;
	}

	if (open_codec_context(&handle->video_stream_idx, handle->fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) 
	{
		handle->video_stream = handle->fmt_ctx->streams[handle->video_stream_idx];
		handle->video_dec_ctx = handle->video_stream->codec;

		//         handle->video_dst_file = fopen(ConfigPar->video_dst_filename, "wb");
		//         if (!handle->video_dst_file) {
		//             fprintf(stderr, "Could not open destination file %s\n", ConfigPar->video_dst_filename);
		//             ret = -1;
		//         }

		/* allocate image where the decoded image will be put */
		//         ret = av_image_alloc(OperatePar->video_dst_data, OperatePar->video_dst_linesize,
		//                              handle->video_dec_ctx->width, handle->video_dec_ctx->height,
		//                              handle->video_dec_ctx->pix_fmt, 1);
		//if (ret < 0) {
		//fprintf(stderr, "Could not allocate raw video buffer\n");
		//ret -1;
		// }
		handle->video_dst_bufsize = ret;
	}
	else
		LOG("ffmpeg demux create fail: opencodeccontext video error");

	if (open_codec_context(&handle->audio_stream_idx, handle->fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) 
	{
		handle->audio_stream = handle->fmt_ctx->streams[handle->audio_stream_idx];
		handle->audio_dec_ctx = handle->audio_stream->codec;


		//add audio pcm resample init information
		//src pcm
		if ((handle->audio_dec_ctx->channel_layout != AV_CH_LAYOUT_MONO)||(handle->audio_dec_ctx->sample_rate != 16000)||(handle->audio_dec_ctx->sample_fmt != AV_SAMPLE_FMT_S16))
		{
			handle->resampleflag = 1;
			/* create resampler context */
			handle->swr_ctx = swr_alloc();
			if (!handle->swr_ctx) 
			{
				GVE_FFMPEG_Demux_Destroy(*GVE_FFDemux_Handle);
				*GVE_FFDemux_Handle = 0;
				return -1;
			}
			av_opt_set_int(handle->swr_ctx, "in_channel_layout",    handle->audio_dec_ctx->channel_layout, 0);
			av_opt_set_int(handle->swr_ctx, "in_sample_rate",       handle->audio_dec_ctx->sample_rate, 0);
			av_opt_set_sample_fmt(handle->swr_ctx, "in_sample_fmt", handle->audio_dec_ctx->sample_fmt, 0);
			//dst pcm
			av_opt_set_int(handle->swr_ctx, "out_channel_layout",    AV_CH_LAYOUT_MONO, 0);
			av_opt_set_int(handle->swr_ctx, "out_sample_rate",       16000, 0);
			av_opt_set_sample_fmt(handle->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

			/* initialize the resampling context */
			if ((ret = swr_init(handle->swr_ctx)) < 0)
			{
				GVE_FFMPEG_Demux_Destroy(*GVE_FFDemux_Handle);
				*GVE_FFDemux_Handle = 0;
				return -1;
			}

			handle->dst_nb_samples = 0;

			//  			/* buffer is going to be directly written to a rawaudio file, no alignment */
			//  			dst_nb_samples = av_rescale_rnd(swr_get_delay(handle->swr_ctx, handle->audio_dec_ctx->sample_rate) + 1024/*handle->frame->nb_samples*/, 16000, handle->audio_dec_ctx->sample_rate, AV_ROUND_UP);
			//  			dst_nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_MONO);
			//  			ret = av_samples_alloc_array_and_samples(&handle->resampledata, &handle->resamplelinesize, dst_nb_channels,
			//  				dst_nb_samples, AV_SAMPLE_FMT_S16, 0);
		}
		//end
	}
	else
		LOG("ffmpeg demux create fail: opencodeccontext video error");


	/* dump input information to stderr */
	//av_dump_format(handle->fmt_ctx, 0, ConfigPar->input_filename, 0);
	//add wyh 获取文件信息，包括长度，视频分辨率，音频采样率等
	memset(OutPutInfo, 0, sizeof(GVE_FFDemux_OutPutInfo));
	duration = handle->fmt_ctx->duration + 5000;
	OutPutInfo->fileinfo.secs  = duration / AV_TIME_BASE;
	OutPutInfo->fileinfo.us    = duration % AV_TIME_BASE;
	OutPutInfo->fileinfo.mins  = OutPutInfo->fileinfo.secs / 60;
	OutPutInfo->fileinfo.secs %= 60;
	OutPutInfo->fileinfo.hours = OutPutInfo->fileinfo.mins / 60;
	OutPutInfo->fileinfo.mins %= 60;
	OutPutInfo->fileinfo.us = (OutPutInfo->fileinfo.us * 100) / AV_TIME_BASE;

	if (handle->fmt_ctx->bit_rate)
	{
		OutPutInfo->fileinfo.BitRate = handle->fmt_ctx->bit_rate / 1000;
	}
	for (i = 0;i < handle->fmt_ctx->nb_streams;i++)
	{
		AVStream *st = handle->fmt_ctx->streams[i];
		if (st->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			OutPutInfo->audioinfo.SampleRate = 16000;
			OutPutInfo->audioinfo.BytesPerSample = //av_get_bytes_per_sample(st->codec->sample_fmt);
				OutPutInfo->audioinfo.Channels = st->codec->channels;
			OutPutInfo->audioinfo.BitRate = st->codec->bit_rate / 1000;
		}
		else if (st->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			OutPutInfo->videoinfo.Width = st->codec->width;
			OutPutInfo->videoinfo.Height = st->codec->height;
			OutPutInfo->videoinfo.FPS = (int)(st->avg_frame_rate.num / (double)st->avg_frame_rate.den);
			OutPutInfo->videoinfo.BitRate = st->codec->bit_rate / 1000;
			OutPutInfo->fileinfo.Type = 1;
		}
	}
	//end wyh

	if (!handle->audio_stream && !handle->video_stream) {
		LOG("Could not find audio or video stream in the input, aborting\n");
		GVE_FFMPEG_Demux_Destroy(*GVE_FFDemux_Handle);
		*GVE_FFDemux_Handle = 0;
		return -1;
	}

	/* When using the new API, you need to use the libavutil/frame.h API, while
	* the classic frame management is available in libavcodec */
	// if (api_mode == API_MODE_OLD)
	handle->frame = avcodec_alloc_frame();
	//     else
	//         frame = av_frame_alloc();
	if (!handle->frame) {
		LOG("Could not allocate frame\n");
		GVE_FFMPEG_Demux_Destroy(*GVE_FFDemux_Handle);
		*GVE_FFDemux_Handle = 0;
		return -1;
	}

	/* initialize packet, set data to NULL, let the demuxer fill it */
	av_init_packet(&handle->pkt);
	handle->pkt.data = NULL;
	handle->pkt.size = 0;

	//创建解码线程，挂起不执行
#ifdef USE_PTHREAD
	handle->suspenddecode = 1;
	pthread_create(&handle->hThread, NULL, FFmpegDemux_Thread, (void*)handle);  
#else
	handle->suspenddecode = 0;
	handle->hThread = _beginthreadex(NULL,0,FFmpegDemux_Thread,(void*)handle,CREATE_SUSPENDED, NULL);
#endif

	//     if (handle->video_stream)
	//         printf("Demuxing video from file '%s' into '%s'\n", ConfigPar->input_filename, ConfigPar->video_dst_filename);
	//     if (handle->audio_stream)
	//         printf("Demuxing audio from file '%s' into '%s'\n", ConfigPar->input_filename, ConfigPar->audio_dst_filename);


	return 0;
}

void GVE_FFMPEG_Demux_Destroy(unsigned long           GVE_FFDemux_Handle)
{
	FFmpegDmux_Handle *handle = (FFmpegDmux_Handle *) GVE_FFDemux_Handle;

	handle->terminatedecode = 1;
	if (handle->hThread != NULL)
	{
#ifdef USE_PTHREAD
		//		pthread_cancel(*(int*)handle->hThread);
#else		
		WaitForSingleObject(handle->hThread, INFINITE);
		CloseHandle(handle->hThread);
#endif
		handle->hThread == NULL;
	}

	if (handle->video_dec_ctx)
	{
		avcodec_close(handle->video_dec_ctx);
	}
	if (handle->audio_dec_ctx)
	{
		avcodec_close(handle->audio_dec_ctx);
	}
	if (&handle->fmt_ctx)
	{
		avformat_close_input(&handle->fmt_ctx);
	}
	if (&handle->frame)
	{
		avcodec_free_frame(&handle->frame);
	}
	if (&handle->resampledata[0])
	{
		av_freep(&handle->resampledata[0]);
	}
	if (&handle->resampledata)
	{
		av_freep(&handle->resampledata);
	}
	if (&handle->swr_ctx)
	{
		swr_free(&handle->swr_ctx);
	}
	if (handle)
	{
		free(handle);
		handle = NULL;
	}
}

int GVE_FFMPEG_Demux_Decoder(unsigned long            GVE_FFDemux_Handle,
							 GVE_FFDemux_OperatePar *OperatePar,
							 GVE_FFDemux_ConfigPar  *ConfigPar,
							 GVE_FFDemux_OutPutInfo *OutPutInfo)
{
	int ret;
	__int64 ts;
	FFmpegDmux_Handle *handle = (FFmpegDmux_Handle *)GVE_FFDemux_Handle;

	handle->video_dst_data = OperatePar->video_dst_data;
	handle->video_dst_len = OperatePar->video_dst_len;
	handle->audio_dst_data = OperatePar->audio_dst_data;
	handle->audio_dst_len = OperatePar->audio_dst_len;
	handle->pfun = ConfigPar->pfun;
	handle->fun_par = ConfigPar->pfun_par;

	ts = ConfigPar->seek_perscent * handle->fmt_ctx->duration;

	ret = avformat_seek_file(handle->fmt_ctx, -1,INT64_MIN /*seek_min*/, ts/*seek_target*/, INT64_MAX/*seek_max*/, 0);

	if (handle->hThread != NULL)
	{

#ifdef USE_PTHREAD
		//ResumeThread();
		handle->terminatedecode = 0;
		handle->suspenddecode = 0;
#else		
		handle->terminatedecode = 0;
		handle->suspenddecode = 0;
		ResumeThread(handle->hThread);
#endif		
	}
#if 0
	while (av_read_frame(handle->fmt_ctx, &handle->pkt) >= 0)
	{
		AVPacket orig_pkt = handle->pkt;
		do {
			ret = decode_packet(handle,OperatePar->audio_dst_data,OperatePar->audio_dst_len,OperatePar->video_dst_data,OperatePar->video_dst_len,ConfigPar->pfun);
			if (ret < 0)
				break;
			handle->pkt.data += ret;
			handle->pkt.size -= ret;
		} while (handle->pkt.size > 0);
		av_free_packet(&orig_pkt);
	}
#endif

	return 0;
}

void GVE_FFMPEG_Demux_Suspend(unsigned long GVE_FFDemux_Handle)
{
	FFmpegDmux_Handle *handle = (FFmpegDmux_Handle *) GVE_FFDemux_Handle;
	handle->suspenddecode = 1;
}

void GVE_FFMPEG_Demux_Resume(unsigned long GVE_FFDemux_Handle) 
{
	FFmpegDmux_Handle *handle = (FFmpegDmux_Handle *) GVE_FFDemux_Handle;
	handle->suspenddecode = 0;
}
