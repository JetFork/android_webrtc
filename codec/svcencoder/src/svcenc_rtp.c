#include "svcencrtp.h"

#ifdef LOSTLOG
#ifndef _WIN32
#define _WIN32
#endif
#endif

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

enum SVCPictureType {
	SVC_PICTURE_TYPE_NONE = 0, ///< Undefined
	SVC_PICTURE_TYPE_I,     ///< Intra
	SVC_PICTURE_TYPE_P,     ///< Predicted
	SVC_PICTURE_TYPE_B,     ///< Bi-dir predicted
	SVC_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG4
	SVC_PICTURE_TYPE_SI,    ///< Switching Intra
	SVC_PICTURE_TYPE_SP,    ///< Switching Predicted
	SVC_PICTURE_TYPE_BI,    ///< BI type
};

const unsigned char SVCEnc_SpsPpsTable[4][500] =
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

static int SVCEnc_FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1
	else return 1;
}

static int SVCEnc_FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1
	else return 1;
}

static int SVCEnc_GetAnnexbNALU (NALU_t *nalu,void *inbuf,int len)
{
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf = (unsigned char *)inbuf;
	static int info2=0, info3=0;
	nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节
	info2 = SVCEnc_FindStartCode2 (Buf);//判断是否为0x000001 
	if(info2 != 1) 
	{
		info3 = SVCEnc_FindStartCode3 (Buf);//判断是否为0x00000001
		if (info3 != 1)//如果不是，返回-1
		{ 
			return -1;
		}
		else 
		{
			//如果是0x00000001,得到开始前缀为4个字节
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}

	else
	{
		//如果是0x000001,得到开始前缀为3个字节
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}
	//查找下一个开始字符的标志位
	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;

	while (!StartCodeFound)// && (pos < len))
	{
		if(pos >= len)
		{
			nalu->len = (pos)-nalu->startcodeprefix_len;//gxh_201025
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
            memcpy_neon(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len );
#else
			memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
#endif
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			return pos;
		}
		pos++;
		info3 = SVCEnc_FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001
		if(info3 != 1)
			info2 = SVCEnc_FindStartCode2(&Buf[pos-3]);//判断是否为0x000001
		StartCodeFound = (info2 == 1 || info3 == 1);
	} 
	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;
	// Here the Start code, the complete NALU, and the next start code is in the Buf.  
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
    memcpy_neon(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
#else
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001
#endif
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
}

static int SVCEnc_NalSort(char *buf,NALU_t nal[MAXRTPNUM],
						  SVCPacket *pMultPacket[MAXLAYER],
						  int layerPacketCount[MAXLAYER],
						  int layer)
						  // 				   int start,
						  // 				   int end)
{	
	int pos = 0;
	int offset = 0;
	int i = 0, j = 0;
	int totalSize = 0;
	//for (j = end; j >= start; j--)
	for (j = 0; j < layer; j++)
	{
		SVCPacket *packet = pMultPacket[j];
		offset = 0;
		while(offset < packet->size)
		{
			int len = packet->size;
			nal[i].buf = &buf[pos];
			nal[i].layer = j;
			nal[i].pts = (SVCPacket *)pMultPacket[j]->pts;
			layerPacketCount[j]++;
			offset += SVCEnc_GetAnnexbNALU(&nal[i],&packet->data[offset],len - offset);
			pos += nal[i].len;
			i++;
		}
		if(i >= MAXRTPNUM)
		{
			return -1;
		}
		totalSize += offset;		
	}
	return i;
}
static int SVCEnc_RTPH264Packet(NALU_t nal[MAXRTPNUM],int num, int layerPacketCnt[MAXLAYER],RTPacket *rtPacket)//char *sendbuf)//[1500])
{	
	char *sendbuf = &rtPacket->buf[rtPacket->pos];
	char *sendptr;
	RTP_FIXED_HEADER  *rtp_hdr;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	RTPEXTENDHEADER *rtp_ext;
	char* nalu_payload;
	short *rtpSize = &rtPacket->size[rtPacket->count];	
	int	bytes=0;
	int size = 0;
	int offset = 0;
	int idx = 0;
	int i = 0;
	int firstSlice = 0;
	int nextFirstSlice = 0;
	short extlen = 0;
	int rtp_extend_length = RTPEXTENDSIZE;//2;//1;
	int slice_type = 0;//!!!
	unsigned short videoSeqnum = rtPacket->videoSeqnum;
	unsigned short frameGop = rtPacket->gop;
	unsigned short layerPacketCount = 0;
	static unsigned short seq_no = 0;
	short layer = ((RtpNalu *)rtPacket->packetInfo)->layerNum;
	short bnum = ((RtpNalu *)rtPacket->packetInfo)->bnum+1;
	//short gopsize = GOPSIZE;  //wyh
	short max_video_seq_num =	VIDEOSEQNUM;// (VIDEOSEQNUM / gopsize) * gopsize; //wyh
	int mtu_size = rtPacket->mtu_size;
	int rtp_count_low = num & 0xF;
	int rtp_count_high = (num >> 4) & 0xF;
	//int codecType = rtPacket->codecType;  //delete wyh
	while(i < num)
	{
		NALU_t *n = &nal[i];
		sendptr = &sendbuf[RTPHEADSIZE + offset];
		if ( (n->nal_unit_type == 7) || (n->nal_unit_type == 8) || (n->nal_unit_type == 6))
		{
			slice_type = 0;
			rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
			rtp_hdr->payload     = PAYLOADTYPE_SVC;//rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //负载类型号，wyh									PT
			rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
			rtp_hdr->padding	 = 0;//														P
			rtp_hdr->csrc_len	 = 0;//														CC
			rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
			rtp_hdr->ssrc        = 0;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
			rtp_hdr->extension	 = 1;/*(!codecType);*///1;//  //wyh														X
			rtp_hdr->seq_no		 = seq_no;//htons(seq_no);//0;///htons(seq_num ++); //序列号，每发送一个RTP包增1	SQUENCE NUMBER
			rtp_hdr->timestamp = 0;//htonl(ts_current);
// 			if(!codecType)  //delete wyh
// 			{
				//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
				rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
				rtp_ext->rtp_extend_profile0 = n->pts;
				rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
				extlen = (rtp_extend_length + 1) << 2;
				extlen = rtp_hdr->extension ? extlen : 0;
				rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
				rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
				rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
//			}
			nalu_hdr =(NALU_HEADER*)&sendptr[extlen];//sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
			nalu_hdr->F=n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendptr[extlen + 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
            memcpy_neon(nalu_payload,n->buf+1,n->len-1);
#else
			memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
#endif
			bytes=n->len + RTPHEADSIZE + extlen;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

			offset += bytes;

			rtpSize[idx++] = bytes;	
			//if(!codecType)  delete wyh
				rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
			seq_no++;
			seq_no = seq_no > MAXVIDEOSEQNUM ? 0 : seq_no;
			layerPacketCnt[n->layer]--;
		}
		else
		{
			slice_type = n->nal_reference_idc >> 5;
			slice_type = slice_type == 3 ? SVC_PICTURE_TYPE_I : slice_type == 2 ? SVC_PICTURE_TYPE_P : SVC_PICTURE_TYPE_B;
		}
		i++;		
	}

	rtPacket->NalNum = idx;

	//slice
	firstSlice = 0;
	nextFirstSlice = 0;
	i = 0;
	while(i < num)
	{
		NALU_t *n = &nal[i];

		slice_type = rtPacket->sliceType[n->layer];

		//注意兼顾与sps/pps在一起和不在一起的情况；
		if(n->startcodeprefix_len == 4)
		{
			firstSlice = 1;
		}
		if(nal[i + 1].startcodeprefix_len == 4 || ((i + 1) == num))
		{
			nextFirstSlice = 1;
		}
		if ( (n->nal_unit_type != 7) && (n->nal_unit_type != 8) && (n->nal_unit_type != 6))
		{

			//得到该nalu需要用多少长度为1400字节的RTP包来发送
			int k=0,l=0;
			int t=0;//用于指示当前发送的是第几个分片RTP包
			k=n->len / mtu_size;//需要k个1400字节的RTP包
			l=n->len % mtu_size;//最后一个RTP包的需要装载的字节数
			layerPacketCount = k + (l > 0);
			while(t<=k)
			{
				rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
				rtp_hdr->payload     = PAYLOADTYPE_SVC;//rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //负载类型号，									PT  wyh
				rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
				rtp_hdr->padding	 = 0;//														P
				rtp_hdr->csrc_len	 = 0;//														CC
				rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
				rtp_hdr->ssrc        = 0;//htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
				rtp_hdr->extension	 = 1;/*(!codecType);*///1;//		 wyh												X			
				rtp_hdr->timestamp=0;///htonl(ts_current);
				sendptr = &sendbuf[RTPHEADSIZE + offset];
				rtp_hdr->seq_no = seq_no;//htons(seq_no);//0;///htons(seq_num ++); //序列号，每发送一个RTP包增1

				if(!k)
				{
					layerPacketCount = layerPacketCnt[n->layer];
					if(firstSlice && nextFirstSlice)
					{
						rtp_hdr->seq_no     = seq_no;//htons(seq_no);//0;///htons(seq_num ++); //序列号，每发送一个RTP包增1	SQUENCE NUMBER
// 						if(!codecType)   //delete wyh
// 						{
							//设置NALU HEADER,并将这个HEADER填入sendbuf[12]
							rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
							rtp_ext->rtp_extend_profile0 =n->pts;// rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
							rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
							extlen = (rtp_extend_length + 1) << 2;
							extlen = rtp_hdr->extension ? extlen : 0;
							rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
							rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
							rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
//						}
						nalu_hdr =(NALU_HEADER*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
						nalu_hdr->F=n->forbidden_bit;
						nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
						nalu_hdr->TYPE=n->nal_unit_type;

						nalu_payload=&sendptr[extlen + 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                        memcpy_neon(nalu_payload,n->buf+1,n->len-1);
#else
						memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。
#endif
						rtp_hdr->timestamp = 0;//htonl(ts_current);										TIMESTAMP
						bytes=n->len + RTPHEADSIZE + extlen;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

						offset += bytes;
	//					if(!codecType)  //delete wyh
							rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
						t++;
						firstSlice = 0;
						nextFirstSlice = 0;
					}
					else if(firstSlice && !nextFirstSlice)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
					{
						rtp_hdr->extension	 = 1;/*(!codecType);*/  //wyh
// 						if(!codecType)  //delete wyh
// 						{
							//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
							rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
							rtp_ext->rtp_extend_profile0 =n->pts; //rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
							rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
							extlen = (rtp_extend_length + 1) << 2;
							extlen = rtp_hdr->extension ? extlen : 0;
							rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
							rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
							rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
	//					}
						fu_ind =(FU_INDICATOR*)&sendptr[extlen];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
						fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=1;
						fu_hdr->TYPE=n->nal_unit_type;						

						nalu_payload=&sendptr[extlen + 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//同理将sendbuf[14]赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                        memcpy_neon(nalu_payload,n->buf+1,n->len-1);
#else
						memcpy(nalu_payload,n->buf+1,n->len-1);//去掉NALU头
#endif
						bytes=n->len + RTPHEADSIZE + extlen + 1;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

						offset += bytes;
					//	if(!codecType)  //delete wyh
							rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
						t++;
						firstSlice = 0;

					}					
					else if(nextFirstSlice && !firstSlice)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
					{
// 						if(!codecType)  //delete wyh
// 						{
							rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
							rtp_ext->rtp_extend_profile0 =n->pts; //rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
							rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
							extlen = (rtp_extend_length + 1) << 2;
							extlen = rtp_hdr->extension ? extlen : 0;
							rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
							rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
							rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
//						}
						fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13 + offset];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->TYPE=n->nal_unit_type;
						fu_hdr->E=1;

						nalu_payload=&sendptr[2 + extlen];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                        memcpy_neon(nalu_payload,n->buf+1,n->len-1);
#else
						memcpy(nalu_payload,n->buf+1,n->len-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
#endif
						bytes=n->len + RTPHEADSIZE + extlen + 1;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节

						offset += bytes;
				//		if(!codecType)   //delete wyh
							rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
						t++;
					}
					else
					{
// 						if(!codecType)   //delete wyh
// 						{
							rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
							rtp_ext->rtp_extend_profile0 = n->pts;//rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
							rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
							extlen = (rtp_extend_length + 1) << 2;
							extlen = rtp_hdr->extension ? extlen : 0;
							rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
							rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
							rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
//						}
						fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F=n->forbidden_bit;
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13 + offset];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->E=0;
						fu_hdr->TYPE=n->nal_unit_type;

						nalu_payload=&sendptr[2 + extlen];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                        memcpy_neon(nalu_payload,n->buf+1,n->len-1);
#else
						memcpy(nalu_payload,n->buf+1,n->len-1);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
#endif
						bytes=n->len + RTPHEADSIZE + extlen + 1;			//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

						offset += bytes;
	//					if(!codecType)  //delete wyh
							rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
						t++;
					}
				}
				else if(!t)
				{
					rtp_hdr->extension	 = 1/*(!codecType)*/; //wyh
// 					if(!codecType)  //delete wyh
// 					{
						//设置FU INDICATOR,并将这个HEADER填入sendbuf[12]
						rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
						rtp_ext->rtp_extend_profile0 = n->pts;//rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
						rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
						extlen = (rtp_extend_length + 1) << 2;
						extlen = rtp_hdr->extension ? extlen : 0;
						rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
						rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
						rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
	//				}
					fu_ind =(FU_INDICATOR*)&sendptr[extlen];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
					fu_hdr->E=0;
					fu_hdr->R=0;
					fu_hdr->S=1;
					fu_hdr->TYPE=n->nal_unit_type;						

					nalu_payload=&sendptr[extlen + 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//同理将sendbuf[14]赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                    memcpy_neon(nalu_payload,n->buf+t*mtu_size+1,mtu_size-1);
#else
					memcpy(nalu_payload,n->buf+t*mtu_size+1,mtu_size-1);//去掉NALU头
#endif
					bytes=mtu_size + RTPHEADSIZE + extlen + 1;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

					offset += bytes;
//					if(!codecType)  //delete wyh
						rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
					t++;					
				}
				//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
				else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
				{
// 					if(!codecType)  //delete wyh
// 					{
						rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
						rtp_ext->rtp_extend_profile0 = n->pts;//rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
						rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
						extlen = (rtp_extend_length + 1) << 2;
						extlen = rtp_hdr->extension ? extlen : 0;
						rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
						rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
						rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
//					}
					fu_ind =(FU_INDICATOR*)&sendptr[extlen];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13 + offset];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->TYPE=n->nal_unit_type;
					fu_hdr->E=1;

					nalu_payload=&sendptr[extlen + 2];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                    memcpy_neon(nalu_payload,n->buf+t*mtu_size,l);
#else
					memcpy(nalu_payload,n->buf+t*mtu_size,l);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
#endif
					bytes=l + RTPHEADSIZE + extlen + 2;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节

					offset += bytes;
//					if(!codecType)  //delete wyh
						rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
					t++;
				}
				else if(t<k&&0!=t)
				{
// 					if(!codecType)  //delete wyh
// 					{
						rtp_ext = (RTPEXTENDHEADER *)&sendptr[0];
						rtp_ext->rtp_extend_profile0 = n->pts; //rtPacket->gop;//((RTP_SVC_HEADER_EXTENSION & 0xFF) << 8) | ((RTP_SVC_HEADER_EXTENSION >> 8));
						rtp_ext->rtp_extend_profile = n->layer | (slice_type << 3) | (layer << 6) | ((bnum - 1) << 9) | (rtp_count_low << 12);
						extlen = (rtp_extend_length + 1) << 2;
						extlen = rtp_hdr->extension ? extlen : 0;
						rtp_ext->rtp_extend_length = ((rtp_extend_length & 0xFF) << 8) | ((rtp_extend_length >> 8));
						rtp_ext->rtp_extend_globInfo = rtPacket->encIDX | (rtPacket->allocIDX << 4) | (rtPacket->shareIDX << 8);
						rtp_ext->rtp_extend_svcheader = (!idx) | (((/*videoSeqnum*/frameGop << 7) | layerPacketCount) << 2);
//					}
					fu_ind =(FU_INDICATOR*)&sendptr[extlen];// //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
					fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!!!!
					fu_ind->NRI=n->nal_reference_idc>>5;
					fu_ind->TYPE=28;

					//设置FU HEADER,并将这个HEADER填入sendbuf[13]
					fu_hdr =(FU_HEADER*)&sendptr[extlen + 1];//sendbuf[13 + offset];
					fu_hdr->R=0;
					fu_hdr->S=0;
					fu_hdr->E=0;
					fu_hdr->TYPE=n->nal_unit_type;

					nalu_payload=&sendptr[extlen + 2];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
#if (defined SVC_ENCODER_IOS) || (defined SVC_ENCODER_ANDROID)
                    memcpy_neon(nalu_payload,n->buf+t*mtu_size,mtu_size);
#else
					memcpy(nalu_payload,n->buf+t*mtu_size,mtu_size);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
#endif
					bytes=mtu_size + RTPHEADSIZE + extlen + 2;						//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

					offset += bytes;
//					if(!codecType)  //delete wyh
						rtp_ext->rtp_extend_rtplen = bytes | (rtp_count_high << 12);
					t++;
				}


				rtpSize[idx++] = bytes;//oSize;
				if((rtPacket->count + idx) >= MAXRTPNUM)
					return -1;
				seq_no++;
				seq_no = seq_no > MAXVIDEOSEQNUM ? 0 : seq_no;
			}
		}
		nextFirstSlice = 0;
		i++;		
	}
	if(idx) rtp_hdr->marker = 1;
	size = offset;

	rtPacket->pos += offset;
	rtPacket->count += idx;
	rtPacket->rtpLen = offset;
	videoSeqnum++;
	if(videoSeqnum > max_video_seq_num)//VIDEOSEQNUM)
		videoSeqnum = 0;
	rtPacket->videoSeqnum = videoSeqnum;
	return size;
}

int SVCEnc_avInitPacket(SVCPacket *pMultPacket[MAXLAYER],int m)
{
	int i = 0;
	for (i = 0; i < m; i++) 
	{   
		int size = MAXRTPSIZE >> i;
		// 		if( av_new_packet(pMultPacket[i], size) < 0 )
		// 			return -1;
		if ((pMultPacket[i]->data = malloc(size *sizeof(char))) == NULL)
		{
			return -1;
		}	
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
void SVCEnc_avResetRtPacket(RTPacket *rtPacket, char *extBuf,unsigned short *pktSize, int count)
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
static int SVCEnc_avGetVGAMode(int width, int height,int mult_slice)
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

int SVCEnc_avInitRtPacket(RTPacket *rtPacket,/*void *inParams*/int mtu_size,int layer,int bframenum,int width,int height,int mult_slice)
{
	int i;
	int idx = 0;

	SVCEnc_avResetRtPacket(rtPacket, NULL,0,0);

	if(rtPacket->tmpBuf == NULL)
		rtPacket->tmpBuf = (char *)malloc(MAXRTPSIZE * sizeof(char));
	if(rtPacket->tmpBuf == NULL)
		return -1;

	rtPacket->videoSeqnum = 0;
	rtPacket->mtu_size = mtu_size;

	if(rtPacket->packetInfo == NULL)
	{
		rtPacket->packetInfo = (void *)malloc(sizeof(RtpNalu));
		if(rtPacket->packetInfo == NULL)
			return -1;
		((RtpNalu *)rtPacket->packetInfo)->layerNum = layer;
		((RtpNalu *)rtPacket->packetInfo)->bnum = bframenum;
	}

	if(rtPacket->rsvData == NULL)
	{
		idx = SVCEnc_avGetVGAMode(width,height,mult_slice);
		if(idx != -1)
			rtPacket->rsvData = (void *)SVCEnc_SpsPpsTable[idx];
	}
	return 0;
}
void SVCEnc_avFreePacket(RTPacket *rtPacket)
{
	if(rtPacket)
		rtPacket->rsvData = NULL;
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

int SVCEnc_avRaw2RtpStream(RTPacket *rtPacket,SVCPacket *pMultPacket[MAXLAYER],int layer/*int start,int end*/)//int n)
{
	int i = 0,x=0;
	int runflag = 0;
	char *buf = (char *)rtPacket->tmpBuf;//malloc(MAXRTPSIZE * sizeof(char));
	NALU_t nal[MAXRTPNUM];

	int layerPacketCount[MAXLAYER] = {0};
	memset(nal,0,sizeof(NALU_t)*MAXRTPNUM);
	if(buf == NULL) ///<如果参数有误，返回-1
		return -1;
	i = SVCEnc_NalSort(buf,nal, pMultPacket, layerPacketCount,layer/*start,end*/);//n); ///<为RTP packet的包头赋值
	if(i < 0)
		return -1;

	for( x=0;x<i;x++)
	{
		if(nal[x].len>H264MTU)
			break;
	}

	runflag += SVCEnc_RTPH264Packet( nal, i, layerPacketCount, rtPacket ); ///<RTP打包

	rtPacket->pos = 0;

	if(runflag < 0) 
		return -1;

	//free(buf);

	return runflag;
}