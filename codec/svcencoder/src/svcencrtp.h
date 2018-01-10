/********************************************************************
* @file		svcrtp.h
* file path	E:\work\webrtc_HLD_0516\trunk\third_party\ffmpeg_v1.3\x264
*	
* @brief	SVC ��RTP������ͺͺ���
*
*********************************************************************/

#ifndef SVCRTP_H
#define SVCRTP_H

// #ifdef SVC_ENCODER_WIN32
// #include "stdint.h"
// #endif
#include "../inc/svc_enc_api.h"
#include "gve_svcencode.h"
#ifdef _WIN32
#include <winsock2.h>   
#pragma comment( lib, "ws2_32.lib" )   
#endif


#define MAXVIDEOSEQNUM		(1<<14)

#define RTPEXTENDSIZE		2//2��for svc��4��for tang
#define	VIDIORASHEADSIZE	4
#define RTPHEADSIZE			12
#define VIDEOSEQNUM			127//((1 << 7) - 1)

typedef struct 
{
    /**//* byte 0 */
    unsigned char csrc_len:4;        /**//* expect 0 */
    unsigned char extension:1;        /**//* expect 1, see RTP_OP below */
    unsigned char padding:1;        /**//* expect 0 */
    unsigned char version:2;        /**//* expect 2 */
    /**//* byte 1 */
    unsigned char payload:7;        /**//* RTP_PAYLOAD_RTSP */
    unsigned char marker:1;        /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short seq_no;            
    /**//* bytes 4-7 */
    unsigned  int timestamp;        
    /**//* bytes 8-11 */
    unsigned int ssrc;            /**//* stream number is used here. */
} RTP_FIXED_HEADER;

typedef struct 
{
    //byte 0
	unsigned char TYPE:5;
    unsigned char NRI:2;
	unsigned char F:1;    
         
} NALU_HEADER; /**//* 1 BYTES */

typedef struct 
{
    //byte 0
    unsigned char TYPE:5;
	unsigned char NRI:2; 
	unsigned char F:1;                
} FU_INDICATOR; /**//* 1 BYTES */

typedef struct 
{
    //byte 0
    unsigned char TYPE:5;
	unsigned char R:1;
	unsigned char E:1;
	unsigned char S:1;    
} FU_HEADER; /**//* 1 BYTES */

typedef struct {	
		unsigned short rtp_extend_profile0;/**< pts*/
		short rtp_extend_length;  /**< length, for tang, it is 2 for two 32 bit */
		short rtp_extend_rtplen;//bit[0,10]:rtp packet len;bit[12,15]:rtp_count_high
		short rtp_extend_profile; /**< profile used*///bit[0,2]:layer id; bit[3,5]:pic type; bit[6,8]:layer num; bit[9,11]:b num;bit[12,15]:rtp_count_low//gop = n * (bnum+1) + 1
		short rtp_extend_globInfo;//bit[0,3]:enc resolution;bit[4,7]:alloc resolution;bit[8,15]:share id;
		unsigned short rtp_extend_svcheader;//bit0:is first packet;bit1: with redundancy packet;bit[2,7]: layer packet count;bit[8,15]:gop size
	//	int rtp_extend_tang1;
	//	int rtp_extend_tang2;
} RTPEXTENDHEADER;

typedef struct
{
  int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned int len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned int max_size;            //! Nal Unit Buffer size
  int forbidden_bit;            //! should be always FALSE
  int nal_reference_idc;        //! NALU_PRIORITY_xxxx
  int nal_unit_type;            //! NALU_TYPE_xxxx  
  int layer;
  char *buf;                    //! contains the first byte followed by the EBSP
  unsigned short pts;
} NALU_t;

typedef struct
{
	short size[MAXRTPNUM];///< ��ǰ�Ӱ��ĳ���
	short type[MAXRTPNUM];///< Ƭ����
	short layer[MAXRTPNUM];///< �����
	unsigned short videoSeqnum[MAXRTPNUM];///< ��Ƶ��չ�����7λ��ʾÿ��������Ƶ���İ�����СΪ0�����Ϊ127��
	unsigned short layerPacketCount[MAXRTPNUM];///< ��Ƶ��չ�����8λ��ʾһ��������Ƶ���и��Ӱ�����ţ���СΪ0�����Ϊ255��
	unsigned short rtpSeqnum[MAXRTPNUM];///< RTP����
	short layerNum;///< ��ǰ�ֱ����µĲ�����ͨ���������ã�
	short maxLayerNum;///< ��������ͨ��RTP��ã�
	short bnum;///< B֡������Ĭ��ֵΪ3,ͨ��RTP��ã�
	//short gop;///< Group of picture
	unsigned short pts[MAXRTPNUM];
	int count;///< ���Ӱ�����
}RtpNalu;

int SVCEnc_avInitRtPacket(RTPacket *rtPacket,int mtu_size,int layer,int bframenum,int width,int height,int mult_slice);
int SVCEnc_avInitPacket(SVCPacket *pMultPacket[MAXLAYER],int m);
void SVCEnc_avResetRtPacket(RTPacket *rtPacket, char *extBuf,unsigned short *pktSize, int count);
int SVCEnc_avRaw2RtpStream(RTPacket *rtPacket,SVCPacket *pMultPacket[MAXLAYER],int layer);
void SVCEnc_avFreePacket(RTPacket *rtPacket);

#ifdef SVC_ENCODER_IOS
extern void memcpy_neon(void *,const void *,size_t);
#endif

#endif
