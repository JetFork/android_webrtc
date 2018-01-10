#include "svcdecrtp.h"

#ifdef LOSTLOG
FILE *fdebuglog=NULL;
#ifndef _WIN32
#define _WIN32
#endif
#endif


//add wyh
//int *DEC_SVCGop = NULL;
//int64_t *DEC_SVCGopPTS = NULL;
//end wyh
#if  0  //wyh
const int gop[33]={
	PIC_TYPE_I, PIC_TYPE_P, PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B,  //1.2.3.4.5.
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B,  //6.7.8.9
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B,  //10.11.12.13
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B, //14.15.16.17
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B, //18.19.20.21
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B, //22.23.24.25
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B, //26.27.28.29
	PIC_TYPE_P,PIC_TYPE_B, PIC_TYPE_B, PIC_TYPE_B, //30.31.32.33
};

const int64_t gopPTS[33]={
	1, 5, 2, 3, 4, //1.2.3.4.5.
	9, 6, 7, 8, //6.7.8.9
	13, 10, 11, 12, //10.11.12.13
	17, 14, 15, 16, //14.15.16.17
	21, 18, 19, 20, //18.19.20.21
	25, 22, 23, 24, //22.23.24.25
	29, 26, 27, 28, //26.27.28.29
	33, 30, 31, 32, //30.31.32.33
};
#endif  //wyh
const unsigned char GVESpsPpsTable[4][500] =
{
	//==============================704x576===================================================
	{
		//sps 	layer 0
		0x1a,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x81,	0x60,	0x24,	0xd0,	0x80,	0x0,	0x0,	
			0x3,	0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
			//pps 	layer 0
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
			//sps 	layer 1
			0x1b,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x82,	0xc1,	0x2d,	0x8,	0x0,	0x0,	0x3,	
			0x0,	0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
			//pps 	layer 1
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
			//sps 	layer 2
			0x19,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x85,	0x89,	0xd0,	0x80,	0x0,	0x0,	0x3,	
			0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
			//pps 	layer 2
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
			//sps 	layer 3
			0x1b,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x8c,	0x5f,	0xcb,	0x8,	0x0,	0x0,	0x3,	
			0x0,	0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
			//pps 	layer 3
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
	},
	//===========================640x480==================================================================================
	{
		//sps 	layer 0
		0x1b,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x81,	0x40,	0x7b,	0x42,	0x0,	0x0,	0x3,	
			0x0,	0x2,	0x0,	0x0,	0x3,	0x0,	0x65,	0x1e,	0x28,	0x52,	0x70	,
			//pps 	layer 0
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
			//sps 	layer 1
			0x19,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x82,	0x83,	0xf4,	0x20,	0x0,	0x0,	0x3,	
			0x0,	0x20,	0x0,	0x0,	0x6,	0x51,	0xe2,	0x85,	0x27	,
			//pps 	layer 1
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
			//sps 	layer 2
			0x1a,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x85,	0x8,	0xfc,	0xb0,	0x80,	0x0,	0x0,	
			0x3,	0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
			//pps 	layer 2
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
			//sps 	layer 3
			0x1a,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x8a,	0x4d,	0x8,	0x0,	0x0,	0x3,	0x0,	
			0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
			//pps 	layer 3
			0x8,	0x0,	
			0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
		},
		//===========================704x576_MULT_SLICE===============================================================================================
		{
			//sps 	layer 0
			0x1a,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x81,	0x60,	0x24,	0xd0,	0x80,	0x0,	0x0,	
				0x3,	0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
				//pps 	layer 0
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
				//sps 	layer 1
				0x1b,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x82,	0xc1,	0x2d,	0x8,	0x0,	0x0,	0x3,	
				0x0,	0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
				//pps 	layer 1
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
				//sps 	layer 2
				0x19,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x85,	0x89,	0xd0,	0x80,	0x0,	0x0,	0x3,	
				0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
				//pps 	layer 2
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,
				//sps 	layer 3
				0x1b,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x8c,	0x5f,	0xcb,	0x8,	0x0,	0x0,	0x3,	
				0x0,	0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
				//pps 	layer 3
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xef,	0xbc,	0x80	,	
		},
		//===========================640x480_MULT_SLICE===============================================================================================
		{
			//sps 	layer 0
			0x1a,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xba,	0x60,	0x50,	0x1e,	0xd0,	0x80,	0x0,	0x0,	
				0x3,	0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
				//pps 	layer 0
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xee,	0x3c,	0x80	,
				//sps 	layer 1
				0x1b,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xaa,	0x60,	0xa0,	0xfd,	0x8,	0x0,	0x0,	0x3,	
				0x0,	0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
				//pps 	layer 1
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xee,	0x3c,	0x80	,
				//sps 	layer 2
				0x1a,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x85,	0x8,	0xfc,	0xb0,	0x80,	0x0,	0x0,	
				0x3,	0x0,	0x80,	0x0,	0x0,	0x19,	0x47,	0x8a,	0x14,	0x9c	,
				//pps 	layer 2
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xee,	0x3c,	0x80	,
				//sps 	layer 3
				0x1a,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x67,	0x4d,	0x40,	0x1e,	0xe9,	0x8a,	0x4d,	0x8,	0x0,	0x0,	0x3,	0x0,	
				0x8,	0x0,	0x0,	0x3,	0x1,	0x94,	0x78,	0xa1,	0x49,	0xc0	,
				//pps 	layer 3
				0x8,	0x0,	
				0x0,	0x0,	0x0,	0x1,	0x68,	0xee,	0x3c,	0x80,	
			}

};

extern int SVCDec_RTPH264GetLayerAndSliceType(char *buf,int len,int redundancyLen,void *outinfo,RTPacket *rtPacket);

static int SVCDec_RTPH264UnPacket(char *buf,NALU_t nal[MAXRTPNUM],int *num,RTPacket *rtPacket)
{
	char *receivebuf = &rtPacket->buf[rtPacket->pos];
	RTP_FIXED_HEADER  *rtp_hdr;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	RTPEXTENDHEADER *rtp_ext;

	char* nalu_payload;
	short *rtpSize = &rtPacket->size[0];//[rtPacket->count];	

	int offset = 0;
	int idx = 0;
	int i = 0;

	int starthead = 4;
	unsigned int itype = 0;
	int pos = 0;
	short extprofile = -1;
	short extlen = 0;
	int seqnum = 0;
	int ssrc = 0;
	int marker = 0;
	short slice_type = 0;
	int runflag = 0;
	int rtp_extend_length = 0;
	int redundancy_code = rtPacket->redundancy;//VP8REDUNDANCYSIZE;//0;//12;//RTPHEADSIZE;//vp8为了区分连续不同的MTU而插入了12字节的特征码
	int cn = 0;
	int discard = 0;
	int sps_pps_flag = 0;
	int codecType = rtPacket->codecType;
// 	if(codecType)
// 	{
// 		goto unpacket_run;
// 	}

#ifdef LOSTLOG
	fdebuglog = fopen("c:\\lost.log", "w");
#endif
	runflag = SVCDec_RTPH264GetLayerAndSliceType( rtPacket->buf ,rtPacket->rtpLen,rtPacket->redundancy,rtPacket->packetInfo,rtPacket);
	if(runflag < 0)
	{
		return runflag;
	}
//unpacket_run:
	offset = 0;
	while( offset < rtPacket->rtpLen)
	{
		NALU_t *n = &nal[i++];

		if(!codecType && (i > (rtPacket->count + cn)))
			break;

		n->buf = &buf[pos];
		rtp_hdr =(RTP_FIXED_HEADER*)&receivebuf[offset];
		nalu_hdr =(NALU_HEADER*)&receivebuf[offset + RTPHEADSIZE];
		itype = 
			n->nal_unit_type = nalu_hdr->TYPE;
		n->nal_reference_idc = nalu_hdr->NRI << 5;
		n->forbidden_bit = nalu_hdr->F;
		ssrc = rtp_hdr->ssrc;
		seqnum = rtp_hdr->seq_no;
		marker = rtp_hdr->marker;

		extprofile = -1;
		extlen = 0;
		if(rtp_hdr->extension /*&& (!codecType)*/)
		{	
			rtp_ext = (RTPEXTENDHEADER *)&receivebuf[offset + RTPHEADSIZE];

			extprofile =
				n->layer = rtp_ext->rtp_extend_profile & 7;
			n->pts= rtp_ext->rtp_extend_profile0;
			rtp_extend_length = rtp_ext->rtp_extend_length;
			rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
			extlen = (rtp_extend_length + 1) << 2;	
			rtPacket->encIDX = rtp_ext->rtp_extend_globInfo & 3;//解析出扩展头中的编码分辨率索引

			fu_ind =(FU_INDICATOR*)&receivebuf[offset + RTPHEADSIZE + extlen];
			fu_hdr =(FU_HEADER*)&receivebuf[offset + RTPHEADSIZE + extlen + 1];

			itype = fu_ind->TYPE;

			n->nal_unit_type = fu_ind->TYPE == 28 ? fu_hdr->TYPE : itype;
			n->nal_reference_idc = fu_ind->NRI << 5;
			n->forbidden_bit = fu_ind->F;

		}
		else if(codecType)
		{
			if(rtp_hdr->extension)
			{
				rtp_ext = (RTPEXTENDHEADER *)&receivebuf[offset + RTPHEADSIZE];
				rtp_extend_length = rtp_ext->rtp_extend_length;
				rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				extlen = (rtp_extend_length + 1) << 2;
			}
			fu_ind =(FU_INDICATOR*)&receivebuf[offset + RTPHEADSIZE + extlen];
			fu_hdr =(FU_HEADER*)&receivebuf[offset + RTPHEADSIZE + extlen + 1];

			itype = fu_ind->TYPE;
			n->layer = 0;
			n->nal_unit_type = fu_ind->TYPE == 28 ? fu_hdr->TYPE : itype;
			n->nal_reference_idc = fu_ind->NRI << 5;
			n->forbidden_bit = fu_ind->F;
		}

		switch (itype)
		{
		case 1://非IDR图像的片
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);				
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
            memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			slice_type = SVC_PICTURE_TYPE_I;
			n->len += starthead;
			offset += rtpSize[idx++];

			break;
		case 5://IDR图像的片				
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);				
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
            memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			slice_type = SVC_PICTURE_TYPE_I;
			sps_pps_flag |= 2;

			n->len += starthead;
			offset += rtpSize[idx++];

			break;
		case 6://SEI
			starthead =
				n->startcodeprefix_len = VIDIORASHEADSIZE - 1;
			n->len = rtpSize[idx] - (RTPHEADSIZE + extlen);					
			nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 1];
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
            memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
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
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
            memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
			n->buf[0] = 0;
			n->buf[1] = 0;
			n->buf[2] = 0;
			n->buf[3] = 1;
			n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
			sps_pps_flag |= 1;
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
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
            memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
			memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
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
			slice_type == 3 ? SVC_PICTURE_TYPE_I : slice_type == 2 ? SVC_PICTURE_TYPE_P : slice_type == 0 ? SVC_PICTURE_TYPE_B : -1;
			//设置FU HEADER,并将这个HEADER填入sendbuf[13]			
			if(fu_hdr->S)
			{
				starthead =n->startcodeprefix_len = VIDIORASHEADSIZE;
				n->len = rtpSize[idx] - (RTPHEADSIZE + extlen + 1);				
				if(n->len < 0 || n->len > rtPacket->rtpLen)
					return -1;
				nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 2];
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
                memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
				memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
				n->buf[0] = 0;
				n->buf[1] = 0;
				n->buf[2] = 0;
				n->buf[3] = 1;

				n->buf[4] = n->forbidden_bit | n->nal_reference_idc |  n->nal_unit_type;
				n->len += starthead;
				offset += rtpSize[idx++];
			}
			else
			{
				if(!n->forbidden_bit)
				{
					starthead =
						n->startcodeprefix_len = VIDIORASHEADSIZE - 1;
					n->len = rtpSize[idx] - (RTPHEADSIZE + extlen + 1);				
					nalu_payload=&receivebuf[offset + RTPHEADSIZE + extlen + 2];
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
                    memcpy_neon(&n->buf[starthead + 1],nalu_payload,n->len-1);
#else
					memcpy(&n->buf[starthead + 1],nalu_payload,n->len-1);
#endif
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
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
                    memcpy_neon(&n->buf[starthead],nalu_payload,n->len);
#else
					memcpy(&n->buf[starthead],nalu_payload,n->len);
#endif
				}

				n->len += starthead;
				offset += rtpSize[idx++];
			}			

			break;
		default:
			offset += rtpSize[idx];
			discard++;
			break;
		}

		pos += n->len;
		offset += redundancy_code;
	}
	*num = i - discard;

	return runflag;
}

static int SVCDec_NalAntiSort(NALU_t nal[MAXRTPNUM],
					   SVCDecPacket *pMultPacket[MAXLAYER],
					   int count, int layer)  //wyh
// 					   int start,
// 					   int end)
{	
	int pos[MAXLAYER] = {0};
	int i = 0, j = 0;
	int min = MAXLAYER;
	int packetSize = 0;
	SVCDecPacket *packet;
	while(i < count)
	{
		NALU_t *n = &nal[i++];
		j = n->layer;
		if(j < min) min = j;

		if(j >= 0 && j < MAXLAYER)
		{
			packet = pMultPacket[j];
#if (defined SVC_DECODER_IOS) || (defined SVC_DECODER_ANDROID)
            memcpy_neon(&packet->data[pos[j]],n->buf,n->len);
#else
			memcpy(&packet->data[pos[j]],n->buf,n->len);
#endif
			packet->pts =n->pts;
			packet->size = n->len;
			pos[j] += n->len;
		}
	}

// 	if(count)
// 		start = min;  //delete wyh

	//以下为统计丢包信息
	for (j = 0; j < layer; j++)  //wyh
	//for (j = end; j >= start; j--)
	{
		SVCDecPacket *packet = pMultPacket[j];
		packetSize += packet->size = pos[j];		
	}	

	return packetSize;
}
int SVCDec_avInitPacket(SVCDecPacket *pMultPacket[MAXLAYER],int m)
{
	int i = 0;
	for (i = 0; i < m; i++) 
	{   
		int size = MAXRTPSIZE >> i;
		if( (pMultPacket[i]->data =(char *)malloc(size *sizeof(char)))== NULL )
			return -1;
		pMultPacket[i]->size = 0;//???
	} 
	return 0;
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
void SVCDec_avResetRtPacket(RTPacket *rtPacket, char *extBuf,unsigned short *pktSize, int count)
{
	int i;
	rtPacket->buf = extBuf;
	rtPacket->flags = 0;
	rtPacket->count = 0;
	rtPacket->pos = 0;
	rtPacket->NalNum = 0;
	rtPacket->rtpLen = 0;
	if(pktSize && count) /// 判断pktSize不为NULL并且count不为0
	{
		for (i = 0; i < count; i++) /// 为rtPacket->size和rtPacket->count赋值
		{
			rtPacket->size[i] = pktSize[i];

		}
		rtPacket->count = count;
	}
	else /// 如果pktSize为NULL并且count为0
	{
		memset(rtPacket->size,0,MAXRTPNUM * sizeof(short)); /// 将rtPacket->size全部置0
	}
}
static int SVCDec_avGetVGAMode(int width, int height,int mult_slice)
{
	int result = -1;
	if(width == 704 && height == 576)
	{
		result = 0;
	}
	else if(width == 640 && height == 480)
	{
		result = 1;
	}
	else if(width == 352 && height == 288)
	{
		result = 0;
	}
	else if(width == 320 && height == 240)
	{
		result = 1;
	}
	else if(width == 176 && height == 144)
	{
		result = 0;
	}
	else if(width == 160 && height == 120)
	{
		result = 1;
	}
	result += (mult_slice << 1);
	return result;
}


int SVCDec_avInitRtPacket(RTPacket *rtPacket,GVE_SVCDec_ConfigPar *params)
{
	int i;
	int idx = 0;

	SVCDec_avResetRtPacket(rtPacket, NULL,0,0);

	if(rtPacket->tmpBuf == NULL)
		rtPacket->tmpBuf = (char *)malloc(MAXRTPSIZE * sizeof(char));
	if(rtPacket->tmpBuf == NULL)
		return -1;

	rtPacket->videoSeqnum = 0;
	//rtPacket->mtu_size = params->mtu_size;

	if(rtPacket->packetInfo == NULL)
	{
		rtPacket->packetInfo = (void *)malloc(sizeof(RtpNalu));
		if(rtPacket->packetInfo == NULL)
			return -1;
		//((RtpNalu *)rtPacket->packetInfo)->layerNum = params->layer;
		//((RtpNalu *)rtPacket->packetInfo)->bnum = params->bframenum;
	}

 	if(rtPacket->rsvData == NULL)
 	{
		idx = 1;
 		//idx = avGetVGAMode(params->widthAlloc,params->heightAlloc,params->mult_slice);
 		if(idx != -1)
 			rtPacket->rsvData = (void *)GVESpsPpsTable[idx];
 	}
	return 0;
}
void SVCDec_avFreePacket(RTPacket *rtPacket)
{
	if(rtPacket->rsvData)
	{
		rtPacket->rsvData = NULL;
	}
   	if(rtPacket->packetInfo)
   	{
   		free(rtPacket->packetInfo);
   		rtPacket->packetInfo = NULL;
   	}
	if(rtPacket->tmpBuf)
	{
		free(rtPacket->tmpBuf);
		rtPacket->tmpBuf = NULL;
	}
}

//丢包类型检测
//[in]priorPts：前一帧的PTS
//[in]currentPTS：当前帧的PTS
//[in]layer：当前帧的层号
//[out]*lostI, *lostP, *lostB：I、P、B帧分别是否丢失，1为丢失，0为没丢
//[in]flag：当前帧是否丢失，1为丢失，0为没丢
//[in]repeat：当priorPts==currentPTS时，此参数有用，表示已经相同了几次
static int SVCDec_LostInfoAnalyze3(int64_t priorPts, int64_t currentPTS, int layer, int *lostI, int *lostP, int *lostB, int flag,int repeat, int encIdx,int gopsize,int startI[],int *goptype,int64_t *goppts)
{
	int64_t priorPtsTMP;
	int64_t currentPTSTMP;
	int i,j,x,m;
	int delta;

	*lostI = *lostP = *lostB = 0;

	if (layer ==1)
	{
		if (priorPts !=0)
		{
			priorPts -= startI[1]/*16*/;
		}
		currentPTS -= startI[1]/*16*/;
	}
	else 	if (layer ==2)
	{
		if (priorPts !=0)
		{
			priorPts -= startI[2]/*8*/;
		}
		currentPTS -= startI[2]/*8*/;
	}


	if (priorPts<0 || currentPTS <0)
	{
		return -1;
	}

	//若当前帧是I帧，当前帧若丢失，标记IPB全丢；若没丢，标记IPB没丢
	if ((currentPTS-1)%/*GOPSIZE*/gopsize==0)
	{
		*lostI = *lostP = *lostB = (flag ? 1 : 0);
		return 0;
	}

	//将PTS归一到第0层第一个GOP内
	priorPtsTMP = priorPts%/*GOPSIZE*/gopsize ?  priorPts%gopsize/*GOPSIZE*/ : gopsize/*GOPSIZE*/;
	currentPTSTMP = currentPTS%gopsize/*GOPSIZE*/ ? currentPTS%gopsize/*GOPSIZE*/ : gopsize/*GOPSIZE*/;

	delta = (currentPTS-1)/gopsize/*GOPSIZE*/ - (priorPts-1)/gopsize/*GOPSIZE*/;
	if (delta <0)
	{
		//在PTS溢出临界点的处理
		if (delta == -1983 || delta == -1984)
		{
			priorPts = priorPtsTMP;
		}
		else//PTS在溢出临界点相差超过一个GOP，标记全丢
		{
			*lostI = *lostP =*lostB =1;
			return 0;
		}
	}
	else if (delta >1)//前后PTS相差超过一个GOP，标记全丢
	{
		*lostI = *lostP =*lostB =1;
		return 0;
	}

	if (priorPts ==0)//适应层号，使全部归一到第0层
	{
			if (layer ==1 && currentPTS== (startI[1] + 1)/*17*/)
			{
				return 0;
			}
			else if (layer ==2 && currentPTS == (startI[2] + 1)/*9*/)
			{
				return 0;
			}
			else if (layer ==3  && currentPTS == (startI[3] + 1)/*25*/)
			{
				return 0;
			}
			else
			{
				*lostI = 1;
				return 0;
			}
	}


	//更新前后PTS差
	delta = (currentPTS-1)/gopsize/*33*/ - (priorPts-1)/gopsize/*33*/;

	if ( delta== 0)//前后PTS在一个GOP内
	{
		i=j=x=0;
		while (/*gopPTS[i]*/goppts[i] != priorPtsTMP)
		{
			i++;
			if (i >(gopsize - 1)/*32*/)
			{
				return -1;
			}		
		}
		if (priorPts ==currentPTS && repeat >0)
		{
			j = i+repeat;
			flag =1;
		}
		else
		{
			while (/*gopPTS[j]*/goppts[j] != currentPTSTMP)
			{
				j++;
				if (j>(gopsize -1)/*32*/)
				{
					return -1;
				}
			}
		}
		if (i>j)
		{
			*lostI = *lostP = *lostB = 1;
		}
		m= flag? j+1 : j;
		if(m>(gopsize -1)/*32*/)
		{
			*lostI = *lostP = *lostB = 1;
			return 0;
		}
		for (x=i+1; x<m; x++)
		{
			if (/*gop[x]*/goptype[x] == PIC_TYPE_B)
			{
				*lostB=1;
			}
			else if (/*gop[x] */goptype[x]== PIC_TYPE_P)
			{
				*lostP=1;
			}
			else if (/*gop[x]*/goptype[x] == PIC_TYPE_I)
			{
				*lostI=1;
			}
		}
	}
	else if (delta== 1)//前后PTS在相邻的两个GOP内
	{
		i=j=x=0;
		while (/*gopPTS[i]*/goppts[i] != priorPtsTMP)
		{
			i++;
			if (i>(gopsize -1)/*32*/)
			{
				return -1;
			}		
		}
		while (/*gopPTS[j]*/goppts[j] != currentPTSTMP)
		{
			j++;
			if (j>(gopsize -1)/*32*/)
			{
				return -1;
			}		
		}
		m= flag? j+1 : j;
		if(m>(gopsize -1)/*32*/)
		{
			*lostI = *lostP = *lostB = 1;
			return 0;
		}
		for (x=i+1; x<gopsize/*GOPSIZE*/; x++)
		{
			if (/*gop[x]*/goptype[x] == PIC_TYPE_B)
			{
				*lostB=1;
			}
			else if (/*gop[x]*/goptype[x] == PIC_TYPE_P)
			{
				*lostP=1;
			}
			else if (/*gop[x]*/goptype[x] == PIC_TYPE_I)
			{
				*lostI=1;
			}
		}
		for (x=0; x<m; x++)
		{
			if (/*gop[x]*/goptype[x] == PIC_TYPE_B)
			{
				*lostB=1;
			}
			else if (/*gop[x]*/goptype[x] == PIC_TYPE_P)
			{
				*lostP=1;
			}
			else if (/*gop[x]*/goptype[x] == PIC_TYPE_I)
			{
				*lostI=1;
			}
		}
	}
	else
	{
		*lostI = *lostP =*lostB =1;
	}

	return 0;
}

static int SVCDec_LostInfoAnalyze0(int64_t priorPts, int64_t currentPTS, int layer, int *lostI, int *lostP, int flag,int repeat, int encIdx,int gopsize,int startI[])
{
	int i;
	int startPts[4]={0,0,0,0};

	*lostI = *lostP = 0;
	if (flag || repeat >0)
	{
		*lostI = *lostP = 1;
		return 0;
	}

	if (encIdx ==0)
	{
		startPts[0] =/*0*/startI[0];
		startPts[1] = startI[1]/*16*/;
		startPts[2] = startI[2]/*8*/;
		startPts[3] = startI[3]/*24*/;
	}
	else if (encIdx ==1)
	{
		startPts[0] = startI[0]/*0*/;
		startPts[1] = startI[1]/*0*/;
		startPts[2] = startI[2]/*11*/;
		startPts[3] = startI[3]/*22*/;
	}
	else if (encIdx ==2)
	{
		startPts[0] = startI[0]/*0*/;
		startPts[1] = startI[1]/*0*/;
		startPts[2] = startI[2]/*0*/;
		startPts[3] = startI[3]/*16*/;
	}

	if (1 == (currentPTS - priorPts))
	{
		*lostI = *lostP = 0;
	}
	else	if (priorPts == 65535/gopsize*gopsize && currentPTS == gopsize + 1/*33*/)
	{
		*lostI = *lostP = 0;
	}
	else if (priorPts ==0 && currentPTS == startPts[layer])
	{
		*lostI = *lostP = 0;
	}
	else
	{
		*lostI = *lostP = 1;
	}

	return 0;
}

static void SVCDec_updateLostMark(RTPacket *rtPacket, SVCDecPacket *pMultPacket[MAXLAYER], Encoded_Decoded_Info info[3],int maxlayer,/*int start,int end, */int bframenum,int gopsize,int startI[],int *goptype,int64_t *goppts)
{


	RtpNalu *nal = (RtpNalu *)rtPacket->packetInfo;
	int i = 0;
	int layer = 0;
	int slice_type = 0;
	int slice_type_real[MAXLAYER]= {0};
	int islog=0;

	int layerRealCnts[MAXLAYER] = {0};
	int layerWishCnts[MAXLAYER] = {0};

	while(i < nal->count)
	{	
		slice_type = nal->type[i];
		layer = nal->layer[i];
		if(slice_type != SVC_PICTURE_TYPE_NONE)
		{
			layerWishCnts[layer] = nal->layerPacketCount[i];
			layerRealCnts[layer]++;
			slice_type_real[layer] = slice_type;
		}
		i++;
	}

	//检测某层数据有数据包丢失
	for ( i = 0; i < maxlayer; i++)  //wyh
	//for ( i = start; i <= end; i++)
	{
		short diff;
		diff = layerWishCnts[i] - layerRealCnts[i];
		if(diff > 0 || layerRealCnts[i] == 0)
		{
			info[i].lostFlag = 1;
			islog = 1;
		}
		else
		{
			info[i].lostFlag = 0;
		}
//		if(diff == 0 && (slice_type_real[i]== SVC_PICTURE_TYPE_I ))
//		{
//			info[i].lostType= 0;
//			info[i].startDecode = 1;
//		}
	}


#ifdef LOSTLOG
	if (fdebuglog != NULL)
	{
		int start =0;
		int end = maxlayer-1;
		fprintf(fdebuglog,"start:%d\n",start);
		fprintf(fdebuglog,"nal count is:%d\n", nal->count);

		fprintf(fdebuglog,"slice_type: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",slice_type_real[i]);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"layerWishCnts: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",layerWishCnts[i]);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"layerRealCnts: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",layerRealCnts[i]);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"lostFlag: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",info[i].lostFlag);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"lastPts: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",info[i].lastPts);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"currPts: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"**%d,\t",pMultPacket[i]->pts);
		}
		fprintf(fdebuglog,"\n");

		fflush(fdebuglog);
	}
#endif


	//检测某层数据丢失及丢包类型
	for(i = 0/*start*/; i < maxlayer/*= end*/; i++)
	{
		int lostI = 0;
		int lostP = 0;
		int lostB = 0;

		if ( info[i].lastPts== pMultPacket[i]->pts)
		{
			info[i].repeatCnt++;
		}
		else
		{
			info[i].repeatCnt=0;
		}
		if (3 == bframenum)
		{
			SVCDec_LostInfoAnalyze3( info[i].lastPts, pMultPacket[i]->pts, i, &lostI, &lostP, &lostB, info[i].lostFlag, info[i].repeatCnt, rtPacket->encIDX,gopsize,startI,goptype,goppts);//检测丢包类型
		}
		else if (0 == bframenum)
		{
			SVCDec_LostInfoAnalyze0( info[i].lastPts, pMultPacket[i]->pts, i, &lostI, &lostP, info[i].lostFlag, info[i].repeatCnt, rtPacket->encIDX,gopsize,startI);//检测丢包类型
            if (info[i].lostFlag) {
                if (slice_type_real[i] == SVC_PICTURE_TYPE_I ) {
                    lostI = 1;
                }
                else {
                    lostP = 1;
                }
            }
		}

		info[i].lostType = (lostI << 2) | (lostP << 1) | lostB;
	}

	//更新PTS
	for ( i = 0/*start*/; i < maxlayer/*= end*/; i++)
	{
		if(info[i].lostType&6)
		{
			info[i].waitIDR = 1;
			//info[i].startDecode = 0;
		}
		info[i].lastPts = pMultPacket[i]->pts;
	}

#ifdef LOSTLOG
	if (fdebuglog != NULL)
	{
		int start =0;
		int end = maxlayer-1;
		fprintf(fdebuglog,"repeatCnt: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",info[i].repeatCnt);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"lostType: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",info[i].lostType);
		}
		fprintf(fdebuglog,"\n");

		fprintf(fdebuglog,"waitIDR: \t");
		for ( i = start; i <= end; i++)
		{
			fprintf(fdebuglog,"%d,\t",info[i].waitIDR);
		}
		fprintf(fdebuglog,"\n");

		fflush(fdebuglog);
	}
#endif
}

int SVCDec_avRtp2RawStream(RTPacket *rtPacket,SVCDecPacket *pMultPacket[MAXLAYER],/*int start,int end*/int layer,Encoded_Decoded_Info info[4], int *lastEnc, int bframenum,int gopsize,int startI[],int *goptype,int64_t *goppts,int *lastEncRatio)//int n)
{
	int i = 0;
	int j;
	int runflag = 0;
	char *buf = (char *)rtPacket->tmpBuf;//malloc(MAXRTPSIZE * sizeof(char));
	NALU_t nal[MAXRTPNUM];

#ifdef _WIN32
	extern FILE *fdebuglog;
	SYSTEMTIME tm;
#endif

	if(buf == NULL)
		return -1;

	memset(nal,0,sizeof(NALU_t)*MAXRTPNUM);

	runflag = SVCDec_RTPH264UnPacket( buf ,nal, &i,rtPacket);//RTP解包
#ifdef LOSTLOG
	{
		GetLocalTime(&tm);
		if (fdebuglog != NULL)
		{
			fprintf(fdebuglog,"=====%02d:%02d:%02d:%03d, start decode:\n",tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds);
			fprintf(fdebuglog,"lastEnc:\t%d\t", *lastEnc);
			fprintf(fdebuglog,"currentEnc:\t%d\n",rtPacket->encIDX );
		}

	}
#endif


	if ((rtPacket->encIDX != *lastEnc)||(rtPacket->encRatio != *lastEncRatio))//此时标记发来的码流编码器发生初始化，解码丢包信息初始化
	{
		memset(info, 0, sizeof(Encoded_Decoded_Info)*3);
		*lastEnc=-1;
		*lastEncRatio = -1;
		for (j = 0;j < layer;j++)
		{
			pMultPacket[j]->pts = 0;
		}
		
		//pMultPacket[0]->pts = pMultPacket[1]->pts= pMultPacket[2]->pts /*= pMultPacket[3]->pts*/ =0;
	}

	if(!runflag)
		runflag = SVCDec_NalAntiSort(nal,pMultPacket, i, layer/*start,end*/);//n);


	SVCDec_updateLostMark(rtPacket, pMultPacket, info, layer,/*start, end,*/ bframenum,gopsize,startI,goptype,goppts);//丢包判断：输出info

	return runflag;
}