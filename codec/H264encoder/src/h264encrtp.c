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
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1) return 0; //�ж��Ƿ�Ϊ0x000001,����Ƿ���1
	else return 1;
}

static int H264enc_FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1) return 0;//�ж��Ƿ�Ϊ0x00000001,����Ƿ���1
	else return 1;
}

static int H264enc_GetAnnexbNALU (NALU_t *nalu,void *inbuf,int len)
{
	int pos = 0;
	int StartCodeFound, rewind;
	unsigned char *Buf = (unsigned char *)inbuf;
	static int info2=0, info3=0;
	nalu->startcodeprefix_len=3;//��ʼ���������еĿ�ʼ�ַ�Ϊ3���ֽ�
	info2 = H264enc_FindStartCode2 (Buf);//�ж��Ƿ�Ϊ0x000001 
	if(info2 != 1) 
	{
		info3 = H264enc_FindStartCode3 (Buf);//�ж��Ƿ�Ϊ0x00000001
		if (info3 != 1)//������ǣ�����-1
		{ 
			return -1;
		}
		else 
		{
			//�����0x00000001,�õ���ʼǰ׺Ϊ4���ֽ�
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	}

	else
	{
		//�����0x000001,�õ���ʼǰ׺Ϊ3���ֽ�
		nalu->startcodeprefix_len = 3;
		pos = 3;
	}
	//������һ����ʼ�ַ��ı�־λ
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
		info3 = H264enc_FindStartCode3(&Buf[pos-4]);//�ж��Ƿ�Ϊ0x00000001
		if(info3 != 1)
			info2 = H264enc_FindStartCode2(&Buf[pos-3]);//�ж��Ƿ�Ϊ0x000001
		StartCodeFound = (info2 == 1 || info3 == 1);
	} 
	// Here, we have found another start code (and read length of startcode bytes more than we should
	// have.  Hence, go back in the file
	rewind = (info3 == 1)? -4 : -3;
	// Here the Start code, the complete NALU, and the next start code is in the Buf.  
	// The size of Buf is pos, pos+rewind are the number of bytes excluding the next
	// start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code
	nalu->len = (pos+rewind)-nalu->startcodeprefix_len;
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//����һ������NALU����������ʼǰ׺0x000001��0x00000001
	nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
	nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
	return (pos+rewind);//����������ʼ�ַ�֮�������ֽ�������������ǰ׺��NALU�ĳ���
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
		//����RTP HEADER��   
		rtp_hdr->version = 2;   //�汾�ţ��˰汾�̶�Ϊ2   
		rtp_hdr->marker  = 0;   //��־λ���ɾ���Э��涨��ֵ��   
		rtp_hdr->payload = PAYLOADTYPE_H264;//�������ͺţ�   
		rtp_hdr->ssrc    = htonl(10);//���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ   
		//����rtp M λ��   
		rtp_hdr->marker= 0;//1;  //add wyh
		rtp_hdr->seq_no = htons(seq_num ++); //���кţ�ÿ����һ��RTP����1  
		//����NALU HEADER,�������HEADER����sendbuf[12]   
		nalu_hdr =(NALU_HEADER*)&sendbuf[12]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�   
		nalu_hdr->F=n->forbidden_bit;  
		nalu_hdr->NRI=n->nal_reference_idc>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��   
		nalu_hdr->TYPE=n->nal_unit_type;  
		nalu_payload=&sendbuf[13];//ͬ��sendbuf[13]����nalu_payload   
		memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����   
		ts_current=ts_current+timestamp_increse;  
		rtp_hdr->timestamp=htonl(ts_current);  
		bytes=n->len + 12 ;  //���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�   
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
			rtp_hdr->payload     = PAYLOADTYPE_H264; //rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //�������ͺţ�									PT
			rtp_hdr->version     = 2;  //�汾�ţ��˰汾�̶�Ϊ2								V
			rtp_hdr->padding	 = 0;//														P
			rtp_hdr->csrc_len	 = 0;//														CC
			rtp_hdr->marker		 = 0;//(size >= len);   //��־λ���ɾ���Э��涨��ֵ��		M
			rtp_hdr->ssrc        = /*0;*/htonl(10);    //���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ	SSRC
			rtp_hdr->extension	 = 0;//(!codecType);//1;//														X
			rtp_hdr->seq_no		 = /*seq_no;*/htons(seq_no); //���кţ�ÿ����һ��RTP����1	SQUENCE NUMBER
			ts_current=ts_current+timestamp_increse;   //add wyh
			rtp_hdr->timestamp = /*0;*/htonl(ts_current);
			nalu_hdr =(NALU_HEADER*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�
			nalu_hdr->F=n->forbidden_bit;
			nalu_hdr->NRI=n->nal_reference_idc>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��
			nalu_hdr->TYPE=n->nal_unit_type;

			nalu_payload=&sendptr[1];//sendbuf[13 + offset];//ͬ��sendbuf[13]����nalu_payload
			memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����

			bytes=n->len + RTPHEADSIZE + 0/*extlen*/;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�

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

		//ע������sps/pps��һ��Ͳ���һ��������
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

			//�õ���nalu��Ҫ�ö��ٳ���Ϊ1400�ֽڵ�RTP��������
			int k=0,l=0;
			int t=0;//����ָʾ��ǰ���͵��ǵڼ�����ƬRTP��
			k=n->len / mtu_size;//��Ҫk��1400�ֽڵ�RTP��
			l=n->len % mtu_size;//���һ��RTP������Ҫװ�ص��ֽ���
			//layerPacketCount = k + (l > 0);
			if (t == k)		//add wyh
			{
				rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
				rtp_hdr->payload     = PAYLOADTYPE_H264;//rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //�������ͺţ�									PT
				rtp_hdr->version     = 2;  //�汾�ţ��˰汾�̶�Ϊ2								V
				rtp_hdr->padding	 = 0;//														P
				rtp_hdr->csrc_len	 = 0;//														CC
				rtp_hdr->marker		 = 0;//(size >= len);   //��־λ���ɾ���Э��涨��ֵ��		M
				rtp_hdr->ssrc        = /*0;*/htonl(10);    //���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ	SSRC
				rtp_hdr->extension	 = 0;//(!codecType);//1;//														X	
				ts_current=ts_current+timestamp_increse;   //add wyh
				rtp_hdr->timestamp=/*0;*/htonl(ts_current);
				sendptr = &sendbuf[RTPHEADSIZE + offset];
				rtp_hdr->seq_no =/* seq_no;*/htons(seq_no); //���кţ�ÿ����һ��RTP����1

				nalu_hdr =(NALU_HEADER*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�
				nalu_hdr->F=n->forbidden_bit;
				nalu_hdr->NRI=n->nal_reference_idc>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��
				nalu_hdr->TYPE=n->nal_unit_type;

				nalu_payload=&sendptr[1];//sendbuf[13 + offset];//ͬ��sendbuf[13]����nalu_payload
				memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����

				bytes=n->len + RTPHEADSIZE + 0/*extlen*/;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�

				offset += bytes;
				rtpSize[idx++] = bytes;
				seq_no++;
			}
			else
			{
				while(t<=k)
				{
					rtp_hdr =(RTP_FIXED_HEADER*)&sendbuf[offset];
					rtp_hdr->payload     = PAYLOADTYPE_H264;//rtPacket->payLoadType;//VCM_SVC_PAYLOAD_TYPE;  //�������ͺţ�									PT
					rtp_hdr->version     = 2;  //�汾�ţ��˰汾�̶�Ϊ2								V
					rtp_hdr->padding	 = 0;//														P
					rtp_hdr->csrc_len	 = 0;//														CC
					rtp_hdr->marker		 = 0;//(size >= len);   //��־λ���ɾ���Э��涨��ֵ��		M
					rtp_hdr->ssrc        = /*0;*/htonl(10);    //���ָ��Ϊ10�������ڱ�RTP�Ự��ȫ��Ψһ	SSRC
					rtp_hdr->extension	 = 0;//(!codecType);//1;//														X	
					ts_current=ts_current+timestamp_increse;   //add wyh
					rtp_hdr->timestamp=/*0;*/htonl(ts_current);
					sendptr = &sendbuf[RTPHEADSIZE + offset];
					rtp_hdr->seq_no =/* seq_no;*/htons(seq_no); //���кţ�ÿ����һ��RTP����1

					if(!k)
					{
						//layerPacketCount = layerPacketCnt[n->layer];
						if(firstSlice && nextFirstSlice)
						{
							rtp_hdr->seq_no     = /*seq_no;*/htons(seq_no); //���кţ�ÿ����һ��RTP����1	SQUENCE NUMBER
							nalu_hdr =(NALU_HEADER*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����nalu_hdr��֮���nalu_hdr��д��ͽ�д��sendbuf�У�
							nalu_hdr->F=n->forbidden_bit;
							nalu_hdr->NRI=n->nal_reference_idc>>5;//��Ч������n->nal_reference_idc�ĵ�6��7λ����Ҫ����5λ���ܽ���ֵ����nalu_hdr->NRI��
							nalu_hdr->TYPE=n->nal_unit_type;

							nalu_payload=&sendptr[ 1];//sendbuf[13 + offset];//ͬ��sendbuf[13]����nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��naluͷ��naluʣ������д��sendbuf[13]��ʼ���ַ�����

							ts_current=ts_current+timestamp_increse;  //add wyh
							rtp_hdr->timestamp = /*0;*/htonl(ts_current);										//TIMESTAMP
							bytes=n->len + RTPHEADSIZE /*+ extlen*/;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ�����NALUͷ����ȥ��ʼǰ׺������rtp_header�Ĺ̶�����12�ֽ�

							offset += bytes;
							t++;
							firstSlice = 0;
							nextFirstSlice = 0;
						}
						else if(firstSlice && !nextFirstSlice)//����һ����Ҫ��Ƭ��NALU�ĵ�һ����Ƭ����FU HEADER��Sλ
						{
							rtp_hdr->extension	 = 0;//(!codecType);
							fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
							fu_ind->F=n->forbidden_bit;
							fu_ind->NRI=n->nal_reference_idc>>5;
							fu_ind->TYPE=28;

							//����FU HEADER,�������HEADER����sendbuf[13]
							fu_hdr =(FU_HEADER*)&sendptr[1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
							fu_hdr->E=0;
							fu_hdr->R=0;
							fu_hdr->S=1;
							fu_hdr->TYPE=n->nal_unit_type;						

							nalu_payload=&sendptr[ 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//ͬ��sendbuf[14]����nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ��NALUͷ

							bytes=n->len + RTPHEADSIZE /*+ extlen*/ + 1;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥ��ʼǰ׺��NALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�

							offset += bytes;
							t++;
							firstSlice = 0;

						}					
						else if(nextFirstSlice && !firstSlice)//���͵������һ����Ƭ��ע�����һ����Ƭ�ĳ��ȿ��ܳ���1400�ֽڣ���l>1386ʱ����
						{
							fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
							fu_ind->F=n->forbidden_bit;
							fu_ind->NRI=n->nal_reference_idc>>5;
							fu_ind->TYPE=28;

							//����FU HEADER,�������HEADER����sendbuf[13]
							fu_hdr =(FU_HEADER*)&sendptr[ 1];//sendbuf[13 + offset];
							fu_hdr->R=0;
							fu_hdr->S=0;
							fu_hdr->TYPE=n->nal_unit_type;
							fu_hdr->E=1;

							nalu_payload=&sendptr[2 /*+ extlen*/];//sendbuf[14 + offset];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//��nalu���ʣ���l-1(ȥ����һ���ֽڵ�NALUͷ)�ֽ�����д��sendbuf[14]��ʼ���ַ�����
							bytes=n->len + RTPHEADSIZE /*+ extlen*/ + 1;		//���sendbuf�ĳ���,Ϊʣ��nalu�ĳ���l-1����rtp_header��FU_INDICATOR,FU_HEADER������ͷ��14�ֽ�

							offset += bytes;
							t++;
						}
						else
						{
							fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
							fu_ind->F=n->forbidden_bit;
							fu_ind->NRI=n->nal_reference_idc>>5;
							fu_ind->TYPE=28;

							//����FU HEADER,�������HEADER����sendbuf[13]
							fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13 + offset];
							fu_hdr->R=0;
							fu_hdr->S=0;
							fu_hdr->E=0;
							fu_hdr->TYPE=n->nal_unit_type;

							nalu_payload=&sendptr[2 /*+ extlen*/];//sendbuf[14 + offset];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
							memcpy(nalu_payload,n->buf+1,n->len-1);//ȥ����ʼǰ׺��naluʣ������д��sendbuf[14]��ʼ���ַ�����
							bytes=n->len + RTPHEADSIZE /*+ extlen*/ + 1;			//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥԭNALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�

							offset += bytes;
							t++;
						}
					}
					else if(!t)
					{
						rtp_hdr->extension	 = 0;//(!codecType);
						fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//sendbuf[RTPHEADSIZE + VIDIORASHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
						fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//����FU HEADER,�������HEADER����sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13  + VIDIORASHEADSIZE + offset];
						fu_hdr->E=0;
						fu_hdr->R=0;
						fu_hdr->S=1;
						fu_hdr->TYPE=n->nal_unit_type;						

						nalu_payload=&sendptr[/*extlen +*/ 2];//sendbuf[14 + VIDIORASHEADSIZE + offset];//ͬ��sendbuf[14]����nalu_payload
						memcpy(nalu_payload,n->buf+t*mtu_size+1,mtu_size-1);//ȥ��NALUͷ

						bytes=mtu_size + RTPHEADSIZE /*+ extlen*/ + 1;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥ��ʼǰ׺��NALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�

						offset += bytes;
						t++;					
					}
					//����һ����Ҫ��Ƭ��NALU�ķǵ�һ����Ƭ������FU HEADER��Sλ������÷�Ƭ�Ǹ�NALU�����һ����Ƭ����FU HEADER��Eλ
					else if(k==t)//���͵������һ����Ƭ��ע�����һ����Ƭ�ĳ��ȿ��ܳ���1400�ֽڣ���l>1386ʱ����
					{
						fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];//&sendbuf[RTPHEADSIZE + offset]; //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
						fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//����FU HEADER,�������HEADER����sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13 + offset];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->TYPE=n->nal_unit_type;
						fu_hdr->E=1;

						nalu_payload=&sendptr[/*extlen +*/ 2];//sendbuf[14 + offset];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
						memcpy(nalu_payload,n->buf+t*mtu_size,l);//��nalu���ʣ���l-1(ȥ����һ���ֽڵ�NALUͷ)�ֽ�����д��sendbuf[14]��ʼ���ַ�����
						bytes=l + RTPHEADSIZE /*+ extlen */+ 2;		//���sendbuf�ĳ���,Ϊʣ��nalu�ĳ���l-1����rtp_header��FU_INDICATOR,FU_HEADER������ͷ��14�ֽ�

						offset += bytes;
						t++;
					}
					else if(t<k&&0!=t)
					{
						fu_ind =(FU_INDICATOR*)&sendptr[0/*extlen*/];// //��sendbuf[12]�ĵ�ַ����fu_ind��֮���fu_ind��д��ͽ�д��sendbuf�У�
						fu_ind->F = 1;//n->forbidden_bit;//!!!!!!!!!!!!!!!!!!
						fu_ind->NRI=n->nal_reference_idc>>5;
						fu_ind->TYPE=28;

						//����FU HEADER,�������HEADER����sendbuf[13]
						fu_hdr =(FU_HEADER*)&sendptr[/*extlen +*/ 1];//sendbuf[13 + offset];
						fu_hdr->R=0;
						fu_hdr->S=0;
						fu_hdr->E=0;
						fu_hdr->TYPE=n->nal_unit_type;

						nalu_payload=&sendptr[/*extlen +*/ 2];//sendbuf[14 + offset];//ͬ��sendbuf[14]�ĵ�ַ����nalu_payload
						memcpy(nalu_payload,n->buf+t*mtu_size,mtu_size);//ȥ����ʼǰ׺��naluʣ������д��sendbuf[14]��ʼ���ַ�����
						bytes=mtu_size + RTPHEADSIZE /*+ extlen*/ + 2;						//���sendbuf�ĳ���,Ϊnalu�ĳ��ȣ���ȥԭNALUͷ������rtp_header��fu_ind��fu_hdr�Ĺ̶�����14�ֽ�

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
* @details	�������̣�
*
* @verbatim
STEP1: rtPacket->buf = extBuf;
STEP2: ���pktSize��ΪNULL����count��Ϊ0������RTP�������丳ֵ��rtPacket->size��rtPacket->count�����򣬽�����0.
STEP3: ����ֵ��0.
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
	if(pktSize && count) /// �ж�pktSize��ΪNULL����count��Ϊ0
	{
		for (i = 0; i < count; i++) /// ΪrtPacket->size��rtPacket->count��ֵ
		{
			rtPacket->size[i] = pktSize[i];

		}
		rtPacket->count = count;
	}
	else /// ���pktSizeΪNULL����countΪ0
	{
		memset(rtPacket->size,0,MAXRTPNUM * sizeof(short)); /// ��rtPacket->sizeȫ����0
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
	if(buf == NULL) ///<����������󣬷���-1
		return -1;
	i = H264enc_NalSort(buf,nal, buffer,size,pts, &layerPacketCount);//n); ///<ΪRTP packet�İ�ͷ��ֵ
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
