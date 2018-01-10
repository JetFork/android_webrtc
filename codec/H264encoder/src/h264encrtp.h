/********************************************************************
* @file		svcrtp.h
* file path	E:\work\webrtc_HLD_0516\trunk\third_party\ffmpeg_v1.3\x264
*	
* @brief	SVC 的RTP相关类型和函数
*
*********************************************************************/

#ifndef SVCRTP_H
#define SVCRTP_H

#include "../inc/h264_enc_api.h"
#include "gve_h264encode.h"
#ifdef _WIN32
#include <winsock2.h>   
#pragma comment( lib, "ws2_32.lib" )   
#elif defined SVC_ENCODER_ANDROID
#include <arpa/inet.h>
#endif

#define MAXVIDEOSEQNUM		(1<<14)
#define RTPHEADSIZE			12
#define MAXRTPNUM			256//64


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


void H264_avResetRtPacket(H264enc_RTPacket *rtPacket, char *extBuf,unsigned short *pktSize, int count);
int H264_avInitRtPacket(H264enc_RTPacket *rtPacket,x264_param_t *params);
void H264enc_avFreePacket(H264enc_RTPacket *rtPacket);
int H264enc_avRaw2RtpStream(H264enc_RTPacket *rtPacket,char *buffer,int size,int pts);

#endif
