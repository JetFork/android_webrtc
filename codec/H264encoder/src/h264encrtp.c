#include "h264encrtp.h"

#ifdef LOSTLOG
#ifndef _WIN32
#define _WIN32
#endif
#endif

enum H264PictureType {
	H264_PICTURE_TYPE_NONE = 0, ///< Undefined
	H264_PICTURE_TYPE_I,     ///< Intra
	H264_PICTURE_TYPE_P,     ///< Predicted
	H264_PICTURE_TYPE_B,     ///< Bi-dir predicted
	H264_PICTURE_TYPE_S,     ///< S(GMC)-VOP MPEG4
	H264_PICTURE_TYPE_SI,    ///< Switching Intra
	H264_PICTURE_TYPE_SP,    ///< Switching Predicted
	H264_PICTURE_TYPE_BI,    ///< BI type
};

static int H264enc_FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //判断是否为0x000001,如果是返回1
	else return 1;
}

static int H264enc_FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//判断是否为0x00000001,如果是返回1
	else return 1;
}

static int H264enc_GetAnnexbNALU (NALU_t *nalu,void *inbuf,int len)
{
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf = (unsigned char *)inbuf;
	static int info2=0, info3=0;
	nalu->startcodeprefix_len=3;//初始化码流序列的开始字符为3个字节
	info2 = H264enc_FindStartCode2 (Buf);//判断是否为0x000001 
	if(info2 != 1) 
	{
		info3 = H264enc_FindStartCode3 (Buf);//判断是否为0x00000001
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
			memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);     
			nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
			nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
			return pos;
		}
		pos++;
		info3 = H264enc_FindStartCode3(&Buf[pos-4]);//判断是否为0x00000001
		if(info3 != 1)
			info2 = H264enc_FindStartCode2(&Buf[pos-3]);//判断是否为0x000001
		StartCodeFound = (info2 == 1 || info3 == 1);
	} 
	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;
	// Here the Start code, the complete NALU, and the next start code is in the Buf.  
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//拷贝一个完整NALU，不拷贝起始前缀0x000001或0x00000001
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	return (pos+rewind);//返回两个开始字符之间间隔的字节数，即包含有前缀的NALU的长度
}

static int H264enc_NalSort(char *buf,NALU_t nal[MAXRTPNUM],char *data,int size,int pts,
				   int *layerPacketCount)
{	
	int pos = 0;
	int offset = 0;
	int i = 0;
	int totalSize = 0;
// 	for (j = end; j >= start; j--)
// 	{
		offset = 0;
		while(offset < size)
		{
			int len = size;
			nal[i].buf = &buf[pos];
			nal[i].layer = 1;
			nal[i].pts = pts;
			(*layerPacketCount)++;
			offset += H264enc_GetAnnexbNALU(&nal[i],&data[offset],len - offset);
			pos += nal[i].len;
			i++;
		}
		if(i >= MAXRTPNUM)
		{
			return -1;
		}
		totalSize += offset;		
//	}
	return i;
}

static int H264enc_RTPH264BaselinePacket(NALU_t nal[MAXRTPNUM],int num,H264enc_RTPacket *rtPacket)//char *sendbuf)//[1500])
{
	int i =0;
	char *sendbuf;
	RTP_FIXED_HEADER  *rtp_hdr;
	NALU_HEADER		*nalu_hdr;
	char* nalu_payload; 
	int size =0;
	static unsigned short seq_num=0;
	unsigned int timestamp_increse=0,ts_current=0;  
	timestamp_increse=(unsigned int)(90000.0 / 15);
	for(i=0;i<num;i++)
	{
		int bytes=0; 
		NALU_t *n = &nal[i];
		sendbuf= &rtPacket->buf[rtPacket->pos];
		rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[0];   
		memset(rtp_hdr,0,sizeof(rtp_hdr));
		//设置RTP HEADER，   
		rtp_hdr->version = 2;   //版本号，此版本固定为2   
		rtp_hdr->marker  = 0;   //标志位，由具体协议规定其值。   
		rtp_hdr->payload = PAYLOADTYPE_H264;//负载类型号，   
		rtp_hdr->ssrc    = htonl(10);//随机指定为10，并且在本RTP会话中全局唯一   
		//设置rtp M 位；   
		rtp_hdr->marker= 0;//1;  //add wyh
		rtp_hdr->seq_no = htons(seq_num ++); //序列号，每发送一个RTP包增1  
		//设置NALU HEADER,并将这个HEADER填入sendbuf[12]   
		nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；   
		nalu_hdr->F=n->forbidden_bit;  
		nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。   
		nalu_hdr->TYPE=n->nal_unit_type;  
		nalu_payload=&sendbuf[13];//同理将sendbuf[13]赋给nalu_payload   
		memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。   
		ts_current=ts_current+timestamp_increse;  
		rtp_hdr->timestamp=htonl(ts_current);  
		bytes=n->len + 12 ;  //获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节   
		rtPacket->size[i]=bytes;
		rtPacket->pos+=bytes;
		rtPacket->count=num;
		rtPacket->rtpLen +=bytes;
		size+=bytes;
	}

	rtp_hdr->marker = 1;

	return size;
}


static int H264enc_RTPH264Packet(NALU_t nal[MAXRTPNUM],int num,H264enc_RTPacket *rtPacket)//char *sendbuf)//[1500])
{	
	char *sendbuf = &rtPacket->buf[rtPacket->pos];
	char *sendptr;
	RTP_FIXED_HEADER  *rtp_hdr;
	NALU_HEADER		*nalu_hdr;
	FU_INDICATOR	*fu_ind;
	FU_HEADER		*fu_hdr;
	char* nalu_payload;
	short *rtpSize = &rtPacket->size[rtPacket->count];	
	int	bytes=0;
	int size = 0;
	int offset = 0;
	int idx = 0;
	int i = 0;
	int firstSlice = 0;
	int nextFirstSlice = 0;
	int slice_type = 0;//!!!
	static unsigned short seq_no = 0;
	int mtu_size = rtPacket->mtu_size;
	unsigned int timestamp_increse=0,ts_current=0;  
	timestamp_increse=(unsigned int)(90000.0 / 15);
	while(i < num)
	{
		NALU_t *n = &nal[i];
		sendptr = &sendbuf[RTPHEADSIZE + offset];
		if ( (n->nal_unit_type == 7) || (n->nal_unit_type == 8) || (n->nal_unit_type == 6))
		{
			slice_type = 0;
			rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
			rtp_hdr->payload     = PAYLOADTYPE_H264; //rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //负载类型号，									PT
			rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
			rtp_hdr->padding	 = 0;//														P
			rtp_hdr->csrc_len	 = 0;//														CC
			rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
			rtp_hdr->ssrc        = /*0;*/htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
			rtp_hdr->extension	 = 0;//(!codecType);//1;//														X
			rtp_hdr->seq_no		 = /*seq_no;*/htons(seq_no); //序列号，每发送一个RTP包增1	SQUENCE NUMBER
			ts_current=ts_current+timestamp_increse;   //add wyh
			rtp_hdr->timestamp = /*0;*/htonl(ts_current);
			nalu_hdr =(NALU_HEADER*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
			nalu_hdr->F=n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendptr[1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
			memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

			bytes=n->len + RTPHEADSIZE + 0/*extlen*/;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

			offset += bytes;

			rtpSize[idx++] = bytes;	
			seq_no++;
			//seq_no = seq_no > MAXVIDEOSEQNUM ? 0 : seq_no;
			//layerPacketCnt[n->layer]--;
		}
		else
		{
			slice_type = n->nal_reference_idc >> 5;
			slice_type = slice_type == 3 ? H264_PICTURE_TYPE_I : slice_type == 2 ? H264_PICTURE_TYPE_P : H264_PICTURE_TYPE_B;
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

		slice_type = rtPacket->sliceType;

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
			//layerPacketCount = k + (l > 0);
			if (t == k)		//add wyh
			{
				rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
				rtp_hdr->payload     = PAYLOADTYPE_H264;//rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //负载类型号，									PT
				rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
				rtp_hdr->padding	 = 0;//														P
				rtp_hdr->csrc_len	 = 0;//														CC
				rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
				rtp_hdr->ssrc        = /*0;*/htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
				rtp_hdr->extension	 = 0;//(!codecType);//1;//														X	
				ts_current=ts_current+timestamp_increse;   //add wyh
				rtp_hdr->timestamp=/*0;*/htonl(ts_current);
				sendptr = &sendbuf[RTPHEADSIZE + offset];
				rtp_hdr->seq_no =/* seq_no;*/htons(seq_no); //序列号，每发送一个RTP包增1

				nalu_hdr =(NALU_HEADER*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
				nalu_hdr->F=n->forbidden_bit;
				nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
				nalu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendptr[1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

				bytes=n->len + RTPHEADSIZE + 0/*extlen*/;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

				offset += bytes;
				rtpSize[idx++] = bytes;
				seq_no++;
			}
			else
			{
				while(t<=k)
				{
					rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
					rtp_hdr->payload     = PAYLOADTYPE_H264;//rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //负载类型号，									PT
					rtp_hdr->version     = 2;  //版本号，此版本固定为2								V
					rtp_hdr->padding	 = 0;//														P
					rtp_hdr->csrc_len	 = 0;//														CC
					rtp_hdr->marker		 = 0;//(size >= len);   //标志位，由具体协议规定其值。		M
					rtp_hdr->ssrc        = /*0;*/htonl(10);    //随机指定为10，并且在本RTP会话中全局唯一	SSRC
					rtp_hdr->extension	 = 0;//(!codecType);//1;//														X	
					ts_current=ts_current+timestamp_increse;   //add wyh
					rtp_hdr->timestamp=/*0;*/htonl(ts_current);
					sendptr = &sendbuf[RTPHEADSIZE + offset];
					rtp_hdr->seq_no =/* seq_no;*/htons(seq_no); //序列号，每发送一个RTP包增1

					if(!k)
					{
						//layerPacketCount = layerPacketCnt[n->layer];
						if(firstSlice && nextFirstSlice)
						{
							rtp_hdr->seq_no     = /*seq_no;*/htons(seq_no); //序列号，每发送一个RTP包增1	SQUENCE NUMBER
							nalu_hdr =(NALU_HEADER*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给nalu_hdr，之后对nalu_hdr的写入就将写入sendbuf中；
							nalu_hdr->F=n->forbidden_bit;
							nalu_hdr->NRI=n->nal_reference_idc>>5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
							nalu_hdr->TYPE=n->nal_unit_type;

							nalu_payload=&sendptr[ 1];//sendbuf[13 + offset];//同理将sendbuf[13]赋给nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//去掉nalu头的nalu剩余内容写入sendbuf[13]开始的字符串。

							ts_current=ts_current+timestamp_increse;  //add wyh
							rtp_hdr->timestamp = /*0;*/htonl(ts_current);										//TIMESTAMP
							bytes=n->len + RTPHEADSIZE /*+ extlen*/;						//获得sendbuf的长度,为nalu的长度（包含NALU头但除去起始前缀）加上rtp_header的固定长度12字节

							offset += bytes;
							t++;
							firstSlice = 0;
							nextFirstSlice = 0;
						}
						else if(firstSlice && !nextFirstSlice)//发送一个需要分片的NALU的第一个分片，置FU HEADER的S位
						{
							rtp_hdr->extension	 = 0;//(!codecType);
							fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
							fu_ind->F=n->forbidden_bit;
							fu_ind->NRI=n->nal_reference_idc>>5;
							fu_ind->TYPE=28;

							//设置FU HEADER,并将这个HEADER填入sendbuf[13]
							fu_hdr =(FU_HEADER*)&sendptr[1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
							fu_hdr->E=0;
							fu_hdr->R=0;
							fu_hdr->S=1;
							fu_hdr->TYPE=n->nal_unit_type;						

							nalu_payload=&sendptr[ 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//同理将sendbuf[14]赋给nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//去掉NALU头

							bytes=n->len + RTPHEADSIZE /*+ extlen*/ + 1;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

							offset += bytes;
							t++;
							firstSlice = 0;

						}					
						else if(nextFirstSlice && !firstSlice)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
						{
							fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
							fu_ind->F=n->forbidden_bit;
							fu_ind->NRI=n->nal_reference_idc>>5;
							fu_ind->TYPE=28;

							//设置FU HEADER,并将这个HEADER填入sendbuf[13]
							fu_hdr =(FU_HEADER*)&sendptr[ 1];//sendbuf[13 + offset];
							fu_hdr->R=0;
							fu_hdr->S=0;
							fu_hdr->TYPE=n->nal_unit_type;
							fu_hdr->E=1;

							nalu_payload=&sendptr[2 /*+ extlen*/];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
							bytes=n->len + RTPHEADSIZE /*+ extlen*/ + 1;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节

							offset += bytes;
							t++;
						}
						else
						{
							fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
							fu_ind->F=n->forbidden_bit;
							fu_ind->NRI=n->nal_reference_idc>>5;
							fu_ind->TYPE=28;

							//设置FU HEADER,并将这个HEADER填入sendbuf[13]
							fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13 + offset];
							fu_hdr->R=0;
							fu_hdr->S=0;
							fu_hdr->E=0;
							fu_hdr->TYPE=n->nal_unit_type;

							nalu_payload=&sendptr[2 /*+ extlen*/];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
							bytes=n->len + RTPHEADSIZE /*+ extlen*/ + 1;			//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

							offset += bytes;
							t++;
						}
					}
					else if(!t)
					{
						rtp_hdr->extension	 = 0;//(!codecType);
						fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
						fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=1;
						fu_hdr->TYPE=n->nal_unit_type;						

						nalu_payload=&sendptr[/*extlen +*/ 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//同理将sendbuf[14]赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*mtu_size+1,mtu_size-1);//去掉NALU头

						bytes=mtu_size + RTPHEADSIZE /*+ extlen*/ + 1;						//获得sendbuf的长度,为nalu的长度（除去起始前缀和NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

						offset += bytes;
						t++;					
					}
					//发送一个需要分片的NALU的非第一个分片，清零FU HEADER的S位，如果该分片是该NALU的最后一个分片，置FU HEADER的E位
					else if(k==t)//发送的是最后一个分片，注意最后一个分片的长度可能超过1400字节（当l>1386时）。
					{
						fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13 + offset];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->TYPE=n->nal_unit_type;
						fu_hdr->E=1;

						nalu_payload=&sendptr[/*extlen +*/ 2];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*mtu_size,l);//将nalu最后剩余的l-1(去掉了一个字节的NALU头)字节内容写入sendbuf[14]开始的字符串。
						bytes=l + RTPHEADSIZE /*+ extlen */+ 2;		//获得sendbuf的长度,为剩余nalu的长度l-1加上rtp_header，FU_INDICATOR,FU_HEADER三个包头共14字节

						offset += bytes;
						t++;
					}
					else if(t<k&&0!=t)
					{
						fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];// //将sendbuf[12]的地址赋给fu_ind，之后对fu_ind的写入就将写入sendbuf中；
						fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!!!!
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//设置FU HEADER,并将这个HEADER填入sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13 + offset];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->E=0;
						fu_hdr->TYPE=n->nal_unit_type;

						nalu_payload=&sendptr[/*extlen +*/ 2];//sendbuf[14 + offset];//同理将sendbuf[14]的地址赋给nalu_payload
						memcpy(nalu_payload,n->buf+t*mtu_size,mtu_size);//去掉起始前缀的nalu剩余内容写入sendbuf[14]开始的字符串。
						bytes=mtu_size + RTPHEADSIZE /*+ extlen*/ + 2;						//获得sendbuf的长度,为nalu的长度（除去原NALU头）加上rtp_header，fu_ind，fu_hdr的固定长度14字节

						offset += bytes;
						t++;
					}


					rtpSize[idx++] = bytes;//oSize;
					if((rtPacket->count + idx) >= MAXRTPNUM)
						return -1;
					seq_no++;
					//seq_no = seq_no > MAXVIDEOSEQNUM ? 0 : seq_no;
				}

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
	// 	videoSeqnum++;
	// 	if(videoSeqnum > max_video_seq_num)//VIDEOSEQNUM)
	// 		videoSeqnum = 0;
	// 	rtPacket->videoSeqnum = videoSeqnum;
	return size;
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
void H264_avResetRtPacket(H264enc_RTPacket *rtPacket, char *extBuf,unsigned short *pktSize, int count)
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
	}
	else /// 如果pktSize为NULL并且count为0
	{
		memset(rtPacket->size,0,MAXRTPNUM * sizeof(short)); /// 将rtPacket->size全部置0
	}
}

int H264_avInitRtPacket(H264enc_RTPacket *rtPacket,x264_param_t *params)
{
	int idx = 0;

	H264_avResetRtPacket(rtPacket, NULL,0,0);

	if(rtPacket->tmpBuf == NULL)
		rtPacket->tmpBuf = (char *)malloc(MAXRTPSIZE * sizeof(char));
	if(rtPacket->tmpBuf == NULL)
		return -1;

	//rtPacket->videoSeqnum = 0;
	rtPacket->mtu_size = 1260;//params->i_slice_max_size;

// 	if(rtPacket->packetInfo == NULL)
// 	{
// 		rtPacket->packetInfo = (void *)malloc(sizeof(RtpNalu));
// 		if(rtPacket->packetInfo == NULL)
// 			return -1;
// 		((RtpNalu *)rtPacket->packetInfo)->layerNum = 1;
// 		((RtpNalu *)rtPacket->packetInfo)->bnum = params->i_bframe;
// 	}

// 	if(rtPacket->rsvData == NULL)
// 	{
// 		idx = avGetVGAMode(params->i_width,params->i_height,params->i_slice_max_size);
// 		if(idx != -1)
// 			rtPacket->rsvData = (void *)SpsPpsTable[idx];
// 	}
	return 0;
}
void H264enc_avFreePacket(H264enc_RTPacket *rtPacket)
{
// 	if(rtPacket)
// 		rtPacket->rsvData = NULL;
// 	if(rtPacket->packetInfo)
// 	{
// 		free(rtPacket->packetInfo);
// 		rtPacket->packetInfo = NULL;
// 	}
	if(rtPacket->tmpBuf)
	{
		free(rtPacket->tmpBuf);
		rtPacket->tmpBuf = NULL;
	}
}

int H264enc_avRaw2RtpStream(H264enc_RTPacket *rtPacket,char *buffer,int size,int pts)
{
	int i = 0,x=0;
	int runflag = 0;
	char *buf = (char *)rtPacket->tmpBuf;//malloc(MAXRTPSIZE * sizeof(char));
	NALU_t nal[MAXRTPNUM];

	int layerPacketCount = 0;
	memset(nal,0,sizeof(NALU_t)*MAXRTPNUM);
	if(buf == NULL) ///<如果参数有误，返回-1
		return -1;
	i = H264enc_NalSort(buf,nal, buffer,size,pts, &layerPacketCount);//n); ///<为RTP packet的包头赋值
	if(i < 0)
		return -1;

	for( x=0;x<i;x++)
	{
		if(nal[x].len>H264MTU)
			break;
	}
//#ifdef H264_ENCODER_LINUX
	//runflag+=H264enc_RTPH264BaselinePacket(nal,i,rtPacket);
// #else
     runflag+=H264enc_RTPH264Packet(nal,i,rtPacket);
// #endif

	rtPacket->pos = 0;

	if(runflag < 0) 
		return -1;

	//free(buf);

	return runflag;
}
