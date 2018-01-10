#include "h264decrtp.h"

#ifdef LOSTLOG
#ifndef _WIN32
#define _WIN32
#endif
#endif

static int H264dec_RTPH264UnPacket(char *buf,NALU_t nal[MAXRTPNUM],int *num,H264dec_RTPacket *rtPacket)
{
	char *receivebuf = &rtPacket->buf[rtPacket->pos];
	RTP_FIXED_HEADER  *rtp_hdr;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;

	char* nalu_payload;
	short *rtpSize = &rtPacket->size[0];//[rtPacket->count];	

	int offset = 0;
	int idx = 0;
	int i = 0;

	int starthead = 4;
	unsigned int itype = 0;
	int pos = 0;
	//short extprofile = -1;
	short extlen = 0;
	//int seqnum = 0;
	//int ssrc = 0;
	//int marker = 0;
	short slice_type = 0;
	//int runflag = 0;
	//int rtp_extend_length = 0;
	//int redundancy_code = rtPacket->redundancy;//VP8REDUNDANCYSIZE;//0;//12;//RTPHEADSIZE;//vp8为了区分连续不同的MTU而插入了12字节的特征码
	//int cn = 0;
	int discard = 0;
	//int sps_pps_flag = 0;
	//int codecType = rtPacket->codecType;

	offset = 0;
	while( offset < rtPacket->rtpLen)
	{
		NALU_t *n = &nal[i++];

// 		if(!codecType && (i > (rtPacket->count + cn)))
// 			break;

		n->buf = &buf[pos];
		rtp_hdr =(RTP_FIXED_HEADER*)&receivebuf[offset];
		nalu_hdr =(NALU_HEADER*)&receivebuf[offset + RTPHEADSIZE];
		itype = 
			n->nal_unit_type = nalu_hdr->TYPE;
		n->nal_reference_idc = nalu_hdr->NRI << 5;
		n->forbidden_bit = nalu_hdr->F;
		//ssrc = rtp_hdr->ssrc;
		//seqnum = rtp_hdr->seq_no;
		//marker = rtp_hdr->marker;

		//extprofile = -1;
		extlen = 0;


		fu_ind =(FU_INDICATOR*)&receivebuf[offset + RTPHEADSIZE + extlen];
		fu_hdr =(FU_HEADER*)&receivebuf[offset + RTPHEADSIZE + extlen + 1];

		itype = fu_ind->TYPE;
		n->layer = 0;
		n->nal_unit_type = fu_ind->TYPE == 28 ? fu_hdr->TYPE : itype;
		n->nal_reference_idc = fu_ind->NRI << 5;
		n->forbidden_bit = fu_ind->F;

		switch (itype)
		{
		case 1://非IDR图像的片
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);				
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];			
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);			
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			slice_type = AV_PICTURE_TYPE_I;
			n->len += starthead;
			offset += rtpSize[idx++];

			break;
		case 5://IDR图像的片				
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);				
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];			
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);			
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			slice_type = AV_PICTURE_TYPE_I;
			//sps_pps_flag |= 2;

			n->len += starthead;
			offset += rtpSize[idx++];

			break;
		case 6://SEI
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE - 1;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);					
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];			
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);			
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 1;
			n->buf[3] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;

			n->len += starthead;
			offset += rtpSize[idx++];

			break;

		case 7:	//序列参数集		
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);					
			if(n->len < 0 || n->len > rtPacket->rtpLen)
				return -1;
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];			
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);			
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			//sps_pps_flag |= 1;
			n->len += starthead;
			offset += rtpSize[idx++];
			break;
		case 8://图像参数集
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);					
			if(n->len < 0 || n->len > rtPacket->rtpLen)
				return -1;
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];			
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);			
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;

			n->len += starthead;
			offset += rtpSize[idx++];
			break;
		case 28://片分组方式FU-A	
			slice_type = n->nal_reference_idc >> 5;
			slice_type = slice_type == 3 ? AV_PICTURE_TYPE_I : slice_type == 2 ? AV_PICTURE_TYPE_P : slice_type == 0 ? AV_PICTURE_TYPE_B : -1;
			//设置FU HEADER,并将这个HEADER填入sendbuf[13]			
			if(fu_hdr->S)
			{
				starthead =n->startcodeprefix_len = VIDIORASHEADSIZE;
				n->len = rtpSize[idx] - (RTPHEADSIZE + extlen + 1);				
				if(n->len < 0 || n->len > rtPacket->rtpLen)
					return -1;
				nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 2];			
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);	

				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 0;
				n->buf[3] = 1;

				n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				n->len += starthead;
				offset += rtpSize[idx++];
				while (fu_hdr->E==0  && offset < rtPacket->rtpLen)
				{
					fu_ind = (FU_INDICATOR*)&receivebuf[offset + RTPHEADSIZE + extlen];
					fu_hdr = (FU_HEADER*)&receivebuf[offset + RTPHEADSIZE + extlen + 1];
					itype = fu_ind->TYPE;
					int len = rtpSize[idx] - (RTPHEADSIZE + extlen + 1);
					nalu_payload = &receivebuf[offset + RTPHEADSIZE + extlen + 2];
					memcpy(&n->buf[n->len], nalu_payload, len - 1);
					offset += rtpSize[idx];
					n->len += rtpSize[idx++] - (RTPHEADSIZE + extlen + 2);
				}
			}
			else
			{
		/*		if(!n->forbidden_bit)
				{
					starthead =
						n->startcodeprefix_len = VIDIORASHEADSIZE - 1;
					n->len = rtpSize[idx] - (RTPHEADSIZE + extlen + 1);				
					nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 2];			
					memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);

					n->buf[0] = 0;
					n->buf[1] = 0;
					n->buf[2] = 1;
					n->buf[3] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				}
				else
				{
					starthead =n->startcodeprefix_len = 0;//VIDIORASHEADSIZE - 1;
					n->len = rtpSize[idx] - (RTPHEADSIZE + extlen + 2);				
					if(n->len < 0 || n->len > rtPacket->rtpLen)
						return -1;
					nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 2];			
					memcpy(&n->buf[starthead],nalu_payload,n->len);
				}
*/
			//	n->len += starthead;
				offset += rtpSize[idx++];
			}			

			break;
		default:
			offset += rtpSize[idx++];
			discard++;
			break;
		}

		pos += n->len;
		//offset += redundancy_code;
	}
	*num = i - discard;

	return /*runflag*/0;
}

static int H264dec_NalAntiSort(NALU_t nal[MAXRTPNUM],
					   AVPacket *packet,
					   int count)
{	
	int pos = {0};
	int i = 0, j = 0;
	int packetSize = 0;
	while(i < count)
	{
		NALU_t *n = &nal[i++];


		//AVPacket *packet = pMultPacket;
		memcpy(&packet->data[pos],n->buf,n->len);
		packet->pts =n->pts;
		packet->size = n->len;
		pos += n->len;

	}


	//以下为统计丢包信息
// 	for (j = end; j >= start; j--)
// 	{
		//AVPacket *packet = pMultPacket;
		packetSize += packet->size = pos;		
//	}	

	return packetSize;
}


/**
* @details	处理流程：
*
* @verbatim
STEP1: rtPacket->buf = extBuf;
STEP2: 如果pktSize不为NULL并且count不为0，即有RTP包，将其赋值给rtPacket->size和rtPacket->count。否则，将其置0.
STEP3: 其他值赋0.
@endverbatim
*/
void H264dec_avResetRtPacket(H264dec_RTPacket *rtPacket, char *extBuf,unsigned short *pktSize, int count)
{
	int i;
	rtPacket->buf = extBuf;
	//rtPacket->flags = 0;
	rtPacket->count = 0;
	rtPacket->pos = 0;
	//rtPacket->NalNum = 0;
	rtPacket->rtpLen = 0;
	if(pktSize && count) /// 判断pktSize不为NULL并且count不为0
	{
		for (i = 0; i < count; i++) /// 为rtPacket->size和rtPacket->count赋值
		{
			rtPacket->size[i] = pktSize[i];

		}
		rtPacket->count = count;
		if (count > 1 && pktSize[count - 1] > pktSize[count - 2])//靠靠
		{
			rtPacket->size[count-1] = pktSize[count-2];
			rtPacket->size[count - 2] = pktSize[count - 1];
		}
	}
	else /// 如果pktSize为NULL并且count为0
	{
		memset(rtPacket->size,0,MAXRTPNUM * sizeof(short)); /// 将rtPacket->size全部置0
	}
}

int H264dec_avInitRtPacket(H264dec_RTPacket *rtPacket)
{
	//int i;
	//int idx = 0;
	//videoInParams *params = (videoInParams *)inParams;

	H264dec_avResetRtPacket(rtPacket, NULL,0,0);

	if(rtPacket->tmpBuf == NULL)
		rtPacket->tmpBuf = (char *)malloc(MAXRTPSIZE * sizeof(char));
	if(rtPacket->tmpBuf == NULL)
		return -1;

// 	rtPacket->videoSeqnum = 0;
// 	rtPacket->mtu_size = params->mtu_size;

// 	if(rtPacket->packetInfo == NULL)
// 	{
// 		rtPacket->packetInfo = (void *)malloc(sizeof(RtpNalu));
// 		if(rtPacket->packetInfo == NULL)
// 			return -1;
// 		((RtpNalu *)rtPacket->packetInfo)->layerNum = 1;
// 		((RtpNalu *)rtPacket->packetInfo)->bnum = params->bframenum;
// 	}

// 	if(rtPacket->rsvData == NULL)
// 	{
// 		idx = avGetVGAMode(params->widthAlloc,params->heightAlloc,params->mult_slice);
// 		if(idx != -1)
// 			rtPacket->rsvData = (void *)SpsPpsTable[idx];
// 	}
	return 0;
}
void H264dec_avFreePacket(H264dec_RTPacket *rtPacket)
{
// 	if(rtPacket)
// 		rtPacket->rsvData = NULL;
// 	if(rtPacket->packetInfo)
// 	{
// 		av_free(rtPacket->packetInfo);
// 		rtPacket->packetInfo = NULL;
// 	}
	if(rtPacket->tmpBuf)
	{
		free(rtPacket->tmpBuf);
		rtPacket->tmpBuf = NULL;
	}
}

int H264dec_avRtp2RawStream(H264dec_RTPacket *rtPacket,AVPacket *pMultPacket)
{
	int i = 0;
	int runflag = 0;
	char *buf = (char *)rtPacket->tmpBuf;//malloc(MAXRTPSIZE * sizeof(char));
	NALU_t nal[MAXRTPNUM];


	if(buf == NULL)
		return -1;

	memset(nal,0,sizeof(NALU_t)*MAXRTPNUM);

	/*runflag = */H264dec_RTPH264UnPacket( buf ,nal, &i,rtPacket);//RTP解包



/*	if(!runflag)*/
		runflag = H264dec_NalAntiSort(nal,pMultPacket, i);//n);

	return runflag;
}
