/*****************************************************************************
 * x264: top-level x264cli functions
 *****************************************************************************
 * Copyright (C) 2003-2012 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Steven Walters <kemuri9@gmail.com>
 *          Jason Garrett-Glaser <darkshikari@gmail.com>
 *          Kieran Kunhya <kieran@kunhya.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/

#define  DATA_MAX  3000000
#define X264_ANALYSE_I4x4       0x0001  /* Analyse i4x4 */
#define X264_ANALYSE_I8x8       0x0002  /* Analyse i8x8 (requires 8x8 transform) */
#define X264_ANALYSE_PSUB16x16  0x0010  /* Analyse p16x8, p8x16 and p8x8 */
#define X264_ANALYSE_PSUB8x8    0x0020  /* Analyse p8x4, p4x8, p4x4 */
#define X264_ANALYSE_BSUB16x16  0x0100  /* Analyse b16x8, b8x16 and b8x8 */

#include <time.h>
#include "../../inc/svc_enc_api.h"


char* InputName = "video.yuv";
char* OutputName = "str.264";
char* RtpSizeName = "size.size";
char* RtpCountName = "count.count";

int main()
{
	clock_t start,end;
	double cost_time = 0;
	double pertime;
	int i;
	int j,number = 0,discard = 0;
	int write_size ;
	int relcount = 0;
	unsigned int frameNum = 0;
	unsigned int file_len,read_len = 0;
	short size[MAXRTPNUM];
	FILE * fpInfile = NULL;
	FILE * fpOutfile = NULL;
	FILE * fpRtpSizefile = NULL;
	FILE * fpRtpCountfile = NULL;
	int ii=0;
	int s=0;
	unsigned char* pdata = NULL;
	unsigned int	GVE_H264Enc_Handle = 0;
	GVE_SVCEnc_OperatePar  OperatePar = {0};
	GVE_SVCEnc_OutPutInfo  OutPutInfo = {0};
	GVE_SVCEnc_ConfigPar   ConfigPar  = {/*{640,320,160},*/{1280,640,320},
																/*{480,240,120},*/{720,360,180},
																/*{1280,640,320},*/
																15,
																33,
																3,
																1000,
																{550,300,150},
																{{20,35},{20,35},{20,35}},
																6,
																40,
																28,
																0,
																3,
																0,
																1260,
																1,
																1,
																/*32*/16,
																0,
																2,
																/*1,*/2,
																/*5*/3,
																0,
																1,
																0,
															X264_ANALYSE_I4x4,
	X264_ANALYSE_I4x4 | X264_ANALYSE_I8x8 | X264_ANALYSE_PSUB16x16,
	"medium",
	NULL,
	1,
	1};

	if((fpInfile = fopen(InputName,"rb")) == NULL)
	{
		printf("Fail to open input yuv file!\n");	  
		return 1;
	}
	if((fpOutfile = fopen(OutputName,"wb")) == NULL)
	{
		printf("Fail to open output stream file!\n");	  
		return 1;
	}
	if((fpRtpSizefile = fopen(RtpSizeName,"wb")) == NULL)
	{
		printf("Fail to open RTP Size file!\n");	  
		return 1;
	}
	if((fpRtpCountfile = fopen(RtpCountName,"wb")) == NULL)
	{
		printf("Fail to open RTP count file!\n");	  
		return 1;
	}
	OperatePar.rtpsize = size;



	OperatePar.InPutLen = ConfigPar.Inwidth[0] * ConfigPar.InHeight[0] * 3/2;


	if(NULL == (OperatePar.InBuf = (unsigned char *)malloc(OperatePar.InPutLen))) 
	{
		printf("Fail to initialize yuv buffer! \n");
		return 1;
	}

	if(NULL == (OperatePar.OutBuf = (unsigned char *)malloc(sizeof(unsigned char)*DATA_MAX))) 
	{
		printf("Fail to initialize stream buffer! \n");
		return 1;
	}

	fseek(fpInfile,0,SEEK_END);
	file_len = ftell(fpInfile);
	fseek(fpInfile,0,SEEK_SET);

	GVE_SVC_Encoder_Create(&GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

	ConfigPar.debugfilename = "x264_enc_0.264";
	ConfigPar.debugfp = fopen(ConfigPar.debugfilename, "wb");
	ConfigPar.debuglayer = 0;
	ConfigPar.debugflag = 1;
	GVE_SVC_Encoder_SetDebugFile(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

	ConfigPar.debugfilename = "x264_enc_1.264";
	ConfigPar.debugfp = fopen(ConfigPar.debugfilename, "wb");
	ConfigPar.debuglayer = 1;
	ConfigPar.debugflag = 1;
	GVE_SVC_Encoder_SetDebugFile(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

	ConfigPar.debugfilename = "x264_enc_2.264";
	ConfigPar.debugfp = fopen(ConfigPar.debugfilename, "wb");
	ConfigPar.debuglayer = 2;
	ConfigPar.debugflag = 1;
	GVE_SVC_Encoder_SetDebugFile(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);

	do 
	{
		frameNum++;
		read_len += fread(OperatePar.InBuf,1,OperatePar.InPutLen,fpInfile);

		start = clock();
		GVE_SVC_Encoder_Encoder(GVE_H264Enc_Handle,&OperatePar,&ConfigPar,&OutPutInfo);
		end = clock();
		cost_time += (double)(end - start);
		
  		if (OperatePar.OutputLen)
  		{
//  			discard  = 0;
//  			write_size = 0;
//  			for (j = 0;j < OperatePar.rtpcount;j++)
//  			{
//  				if (((number + j) % 10)||((number + j) == 0))
//  				{
//  					fwrite(&OperatePar.rtpsize[j],1,sizeof(short),fpRtpSizefile);
//  					fwrite(OperatePar.OutBuf + write_size,1,OperatePar.rtpsize[j],fpOutfile);
//  				}
//  				else
//  				{
//  					discard += 1;
//  				}
//  				write_size += OperatePar.rtpsize[j];
//  			}
//  			number += j;
//  			relcount = OperatePar.rtpcount - discard;
//  			fwrite(&relcount,1,4,fpRtpCountfile);
			//fwrite(&OperatePar.OutputLen,1,4,fpOutfile);
   			fwrite(OperatePar.OutBuf,1,OperatePar.OutputLen,fpOutfile);
   			fwrite(&OperatePar.rtpcount,1,4,fpRtpCountfile);
   			fwrite(OperatePar.rtpsize,1,OperatePar.rtpcount*sizeof(short),fpRtpSizefile);

// 			pdata = OperatePar.OutBuf;
// 			for (ii=0;ii<=OperatePar.rtpcount;ii++)
// 			{
// 				s = OperatePar.rtpsize[ii];
// 				fwrite(&s,1,4,fpOutfile);
// 				fwrite(pdata,1,s,fpOutfile);
// 				pdata+=s;
// 			}
  		}


	} while (read_len < file_len);

	cost_time = cost_time/CLOCKS_PER_SEC;

	pertime = cost_time/frameNum;

	printf("cost total time = %f s\n, tatal frame  = %d \n,every fame cost number is %f s\n",cost_time,frameNum,pertime);

	GVE_SVC_Encoder_Destroy(GVE_H264Enc_Handle);

	if (OperatePar.InBuf != NULL)
	{
		free(OperatePar.InBuf);
	}

	if (OperatePar.OutBuf != NULL)
	{
		free(OperatePar.OutBuf);
	}

	if (fpInfile)
	{
		fclose(fpInfile);
	}
	if (fpOutfile)
	{
		fclose(fpOutfile);
	}
	if (fpRtpCountfile)
	{
		fclose(fpRtpCountfile);
	}
	if (fpRtpSizefile)
	{
		fclose(fpRtpSizefile);
	}

	getchar();

	return 0;
}