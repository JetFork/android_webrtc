#include "svcdecrtp.h"

#define VCM_SPR_PAYLOAD_TYPE    35

void SVCDec_SliceTransform(short *sliceType)
{
	switch(*sliceType)
	{
		case PIC_TYPE_AUTO:
			*sliceType = SVC_PICTURE_TYPE_NONE;
			break;
		case PIC_TYPE_IDR:           
		case PIC_TYPE_I:
		case PIC_TYPE_KEYFRAME:
			*sliceType = SVC_PICTURE_TYPE_I;
			break;
		case PIC_TYPE_P:
			*sliceType = SVC_PICTURE_TYPE_P;
			break;
		case PIC_TYPE_BREF:
		case PIC_TYPE_B:
			*sliceType = SVC_PICTURE_TYPE_B;
			break;
		default:
			break;
	}
}
//注意：
//未经重排序的数据各个层的每个rtp包可能是杂乱的，需要根据层数、片类型、包序号进行甄别，并对满足条件的某一序列包进行主动丢包处理。

int SVCDec_RTPH264GetLayerAndSliceType(char *buf,int len,int redundancyLen,void *outinfo,RTPacket *rtPacket)
{
	RtpNalu *nal = (RtpNalu *)outinfo;
	RTP_FIXED_HEADER  *rtp_hdr;	
	RTPEXTENDHEADER *rtp_ext;
	int offset = 0;
	int i = 0;
	int rptHeadSize = RTPHEADSIZE;
	int redundancy_code = redundancyLen;//rtPacket->redundancy;//VP8REDUNDANCYSIZE;//0;//12;//RTPHEADSIZE;//vp8为了区分连续不同的MTU而插入了12字节的特征码
	short slice_type = 0;
	short last_slice_type = -1;
	short rtplen = 0;
	unsigned short video_seqnum;
	short curr_video_seqnum;
	short last_video_seqnum = -1;
	int rtp_count_low = 0;//num & 0xF;
	int rtp_count_high = 0;//(num >> 4) & 0xF;

	memset(nal,0,sizeof(RtpNalu));
	while(offset < len)
	{
		rtp_hdr =(RTP_FIXED_HEADER*)&buf[offset];
		nal->rtpSeqnum[i] = rtp_hdr->seq_no;
		
		if(rtp_hdr->extension)
		{
			rtp_ext = (RTPEXTENDHEADER *)&buf[offset + rptHeadSize];
			nal->pts[i]=rtp_ext->rtp_extend_profile0;
			nal->size[i] = rtplen = (unsigned short)rtp_ext->rtp_extend_rtplen & 0x7FF;
			rtp_count_high = (unsigned short)rtp_ext->rtp_extend_rtplen >> 12;
			nal->layer[i] = rtp_ext->rtp_extend_profile & 7;
			
			slice_type = (rtp_ext->rtp_extend_profile >> 3) & 7;
			SVCDec_SliceTransform(&slice_type);
			nal->type[i] = slice_type;
			nal->maxLayerNum = (rtp_ext->rtp_extend_profile >> 6) & 7;
			nal->bnum = ((rtp_ext->rtp_extend_profile >> 9) & 7) + 1;
			rtp_count_low = (rtp_ext->rtp_extend_profile >> 12) & 0xF;
			
			video_seqnum = rtp_ext->rtp_extend_svcheader >> 2;
			nal->layerPacketCount[i] = video_seqnum & 0x7F;
			nal->videoSeqnum[i] = curr_video_seqnum = (video_seqnum >> 7) & 0x7F;

			if(last_slice_type == -1 && slice_type)
				last_slice_type = slice_type;
			if(last_video_seqnum == -1)
				last_video_seqnum = curr_video_seqnum;	
 
			offset += rtplen;
			offset += redundancy_code;
			
		}
		else if(rtp_hdr->payload == VCM_SPR_PAYLOAD_TYPE)
		{
			return 1;
		}
		else
		{
			return -1;
		}
		i++;
	}
	if(i >= MAXRTPNUM)
		return -1;
	nal->count = i;
 
   i = 0;

	while(i < nal->count)
	{	
		rtplen = nal->size[i];
		rtPacket->size[i] = rtplen;
		i++;
	}
	return 0;
}