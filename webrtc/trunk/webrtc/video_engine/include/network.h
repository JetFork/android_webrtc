#ifndef __NETWORK_H_
#define __NETWORK_H_
/****************************************************************************************
 *  Copyright (c) 2013 All Rights Reserved by
 ***************************************************************************************/
/**
 *  @file  network.h
 *  @brief  Define the interface of class network.cc
 *
 *  @author 
 *
 *  @date 5��30 2013
 *
 *  @version 0.0
 *
 *  Revision History
 ****************************************************************************************/
/*------------------------------- HEADER FILE INCLUDES ---------------------------------*/
#include "typedefs.h"
/**
 *  @defgroup Network_Feedback_Module
 *  @brief Network_Feedback module implementation
 *
 *  @defgroup Network_Feedback_Implement
 *  @ingroup Network_Feedback_Module
 *  @brief Network_Feedback module internal implementation
 *
 *  ����DTS���ݹ��������練����������Ӧ�ò�ȡ�Ķ�������֡�����ʵ�������
 *
 *  @defgroup Encode_Send_Statistics 
 *  @ingroup Network_Feedback_Module
 *  @brief Statistics information of encode & send for Network_Feedback implement
 *
 *  Ϊ���練�����ԵĲ�ȡ�ṩ������Ϣ��ͳ�ư����ʹ�С��
 *
 */
/*------------------------------------  DEFINITION ------------------------------------*/
#define RTPHEADSIZE 0x12      /*need update with X264 encoder struct RTPEXTENDHEADER real-time */
#define GOPINDEX  0x12        /*need update with X264 encoder struct PacketLoss real-time */
#define PACKET_LOSS_THRESHOLD_HIGH 2       /*�����ʸ���ֵ*/
#define PACKET_LOSS_THRESHOLD_LOW 0.5      /*�����ʵ���ֵ*/
#define RATIO_RTT_THRESHOLD_LOW 1.1        /*RTT����ֵ*/  
#define RATIO_RTT_THRESHOLD_HIGH 1.9       /*RTT����ֵ*/  
#define VIDEO_RATIO_TOTAL 0.2              /*��ƵĿ������������ܴ����еı���*/  
#define BITRATE_FLUCTUATE_THRESHOLD 15     /*���ʲ�����ֵ,������ֵˢ������*/  
#define MAXBITRATE                512      /*�Ϸ�������*/
#define RATIO_MOTIONPRIORITY      0.9      /*�˶�����ģʽ�����ʵ���ϵ��*/
#define RATIO_QUALITYPRIORITY     1.2      /*��������ģʽ�����ʵ���ϵ��*/
#define FALSE 0
#define TRUE 1
/*----------------------------------- ENUMERATIONS ------------------------------------*/
/*--------------------------------- STRUCT TYPES --------------------------------------*/
/**
 *  @addtogroup Network_Feedback_Module
 *  @brief ���練�����ݹ����Ĳ���
 *  @{
 */
typedef struct networkInf
{
	int phyTotalBandWidth;   ///<���������ܴ���
	int videoSendBandWidth;   ///<�ϴ���Ƶ���ʹ���
	int otherTotalBandWidth;   ///<����ҵ���ܴ���
	int lastOtherTotalBandWidth;   ///<��һ������ҵ���ܴ���
	int genBandWidth;   ///<��һ�ιۿ��������������
	int avgLossRate;   ///<ƽ��������
	int lastAvgLossRate;      ///<��ʷƽ��������
	int maxLossRate;   ///<��󶪰���
	int lastMaxLossRate;   ///<��ʷ��󶪰���
	int avgRtt;   ///<ƽ����ʱ
	int lastRtt;   ///<��ʷ��ʱ
	int maxRtt;   ///<�����ʱ
	int jitter;   ///����
	int minBandWidth;   ///<��Ƶ��С����
	int maxBandWidth;   ///<��Ƶ�������
	int peopleNumber;   ///<�λ�����
	int adjustType;   ///<���練����������
	int bitrate;      ///��Ҫ�µ�������
	bool resetBitrate; ///�����Ƿ����
}networkInf;

/**
 *  @brief ͳ�ư����ʹ�С�����������Ľṹ��
 */
typedef struct statPacketSize
{
	int sizeLayer0_I;   ///<0��I����С
	int sizeLayer0_P;   ///<0��P����С
	int sizeLayer0_B;   ///<0��B����С
	int sizeLayer0_B0;   ///<0��B0����С 
	int sizeLayer0_B1;   ///<0��B1����С
	int sizeLayer1_I;   ///<1��I����С
	int sizeLayer1_P;   ///<1��P����С
	int sizeLayer1_B;   ///<1��B����С
	int sizeLayer1_B0;   ///<1��B0����С
	int sizeLayer1_B1;   ///<1��B1����С
	int sizeLayer2_I;   ///<2��I����С
	int sizeLayer2_P;   ///<2��P����С
	int sizeLayer2_B;   ///<2��B����С
	int sizeLayer2_B0;  ///<2��B0����С
	int sizeLayer2_B1;   ///<2��B1����С
	int sizeLayer3_I;   ///<3��I����С
	int sizeLayer3_P;   ///<3��P����С
	int sizeLayer3_B;   ///<3��B����С
	int sizeLayer3_B0;   ///<3��B0����С
	int sizeLayer3_B1;   ///<3��B1����С
	int totalSize;   ///<һ��GOP���ܹ��������ݰ��Ĵ�С
}statPacketSize;
/**@}*/  
/*-------------------------------------- CONSTANTS ------------------------------------*/
/*------------------------------------- GLOBAL DATA -----------------------------------*/
/*-------------------------------- FUNCTION DEFINITION --------------------------------*/
/** 
 *  @name      webrtc
 *  @{
 */
namespace webrtc {
/**
 *  @addtogroup Network_Feedback_Module
 *  @brief class of network feedback
 *  @{
 *  @class
 */
class Network
{
friend class I420Encoder;
public:

	Network() ;   ///<Network constructor
	~Network() {};   ///<Network destructor

/**@}*/  
/**
 *  @addtogroup Network_Feedback_Implement 
 *  @{
 */

/**
 *  @brief  <b>������</b> ���ùۿ������������ƽ�������ʡ���󶪰��ʡ�ƽ����ʱ���λ����� 
 *  ����ۿ��������������
 *
 *  <b>�㷨������</b> \n
 *
 *  1���ۿ����������������RTTΪ����������Ϊ������ \n
 *  2�����������ж�RTT��ֵ���������ֵ�������Ƕ�����ֱ�ӽ��д���ĵ���  
 *  �����µ��ķ��ȸ���ʵ��������Եó� \n
 *  3�������ǰ�Ķ����ʴ�����ֵ��������RTTֱ�ӽ��д���ĵ���
 *  �����µ��ķ��ȸ���ʵ��������Եó� \n
 *  4�������ǰRTTС����ֵ��������ҲС����ֵ��С����ʷƽ��������
 *  ˵������״����ã��Ե�ǰ1/8������������� \n
 *  5�������ǰRTTС����ֵ��������С����ֵ����С����ʷƽ�������� 
 *  ˵������״�������ã�����ƽ�������ʡ���󶪰��ʺͲλ�����
 *  ����󲿷��˵�����״�� \n
 *  6������Ϊ�����ʷ�����̬�ֲ����󲿷��˵�����״����ƽ��ֵ+������ó�
 *  �����ǵ���84.1%����
 *  
 *  <b>˵����</b> \n
 *
 *  �ۿ���������������20s����һ�Σ�Ŀǰ���ڵ�Ƶ��Ϊһ��GOP 
 *
 *  @req		   GVE-SRS-154
 *  @param[in]     nInf        ��������ṹ��
 *  @returns       �ۿ��������������
 *  
 */
WebRtc_Word32 
GenNetworkBandwidth(networkInf nInf);
	
/**
 *  @brief  <b>������</b> ���ùۿ�����������������������ܴ�������ҵ���ܴ�����ƵĿ�������Ƶ���С����
 *  ������Ƶ���з��ʹ���
 *
 *  <b>�㷨������</b> \n
 *
 *  1��Ŀǰ���紫������Ĳ����п��������ܴ�������ҵ���ܴ�����ƵĿ����� \n
 *  2�����ݵ�ǰ�Ƿ��п��ô�������Ƿ��ϵ���ƵĿ����� \n
 *  3�����ȸ�����ʷ����ҵ���ܴ���Ԥ�⵱ǰ����ҵ���ܴ��� \n
 *  4�����ô����ɿ��������ܴ�����ǰ����ҵ���ܴ������ƵĿ��������ó� \n
 *  5�����������Ƶ���ô�����ۿ��������������Ƚ�ȡ��Сֵ \n
 *  6��������Ƶ���ô����޶�Ϊ<min, max>֮��
 *
 *  <b>˵����</b> \n
 *
 *  ���紫������Ĳ���Ŀǰ����Ƶ��Ϊ800ms��������ڵ�Ƶ��Ϊһ��GOP 
 *
 *  @req		    GVE-SRS-154
 *  @param[in]      nInf                    ��������ṹ��
 *  @returns        ��Ƶ���з��ʹ���
 *
 */
WebRtc_Word32 
UpBandWidth(networkInf nInf);

/**
 *  @brief  <b>������</b> ������Ƶ�ɷ��ʹ���֡�ʡ������С
 *  ������Ҫ����������
 *
 *  <b>�㷨������</b> \n
 *
 *  1�����Ȱѿɷ��͵���Ƶ����(��ʱ��sΪ��λ)ת��ΪGOPΪ��λ�Ĵ��� \n
 *  2��ʵ�ʱ����С��ȥ��Ƶ�ɷ��ʹ�С��Ϊ��Ҫ�����Ĵ���
 *
 *  <b>˵����</b> \n
 *
 *  �迼��֡�ʱ仯��Ӱ�� 
 *
 *  @req		    GVE-SRS-154
 *  @param[in]      frameRate                 ��ǰ����֡��
 *  @param[in]      videoBandWidth            ��Ƶ���з��ʹ���
 *  @param[in]      sizeGop                   һ��GOP������֡��
 *  @param[in]      totalSize                 һ��GOP���ܴ�С   
 *  @returns        ��Ҫ�����Ĵ���
 *
 */
WebRtc_Word32
AdjustBandWidth(int frameRate, int videoBandWidth, int sizeGop, int totalSize);

/**
 *  @brief  <b>������</b> ����ģʽ��������Ҫ�����Ĵ�������ͳ�ư��Ĵ�С 
 *  �����������
 *
 *  <b>�㷨������</b> \n
 *
 *  1������ģʽ�µ��ڴ����ʱ�򶪰������ʽ������ڵ�Ƶ�ʱȽϸ�
 *  ��Ҫ��Ϊ���ҳ�֡�ʺ�ͼ��������ƽ��� \n
 *  2����������Ĵ���С����ֵT0��ֻ���ж��� \n
 *  3����������Ĵ��������ֵT0��С����ֵT1���ڶ����Ļ������ٽ�һ��������
 *  ѭ����������
 *  
 *  <b>˵����</b> \n
 *
 *  ����ֵ������������������������֡�����ʵ���  \n
 *                                              
 ************************************adjustTypeÿ��bit��ʾ�ĺ���******************************  \n
 * -------------------------------------------------------------------------------------------  \n                                                                 
 * | 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 9  |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  \n
 * -------------------------------------------------------------------------------------------  \n
 * |    |    | I3 | I2 | I1 | I0 | P3 | P2 | P1 | P0 | B31| B30| B21| B20| B11| B10| B01| B00|  \n
 * -------------------------------------------------------------------------------------------  \n
 * �Ǹ�bitΪ1��˵����Ҫ���������͵İ�                           \n
 *********************************************************************************************  \n
 *
 *  @req		    GVE-SRS-156
 *  @param[in]      nInf                        ��������ṹ��
 *  @param[in]      sps                         ͳ�ư���С�ṹ��
 *  @param[in]      adjustBandWidth             ��Ҫ�����Ĵ��� 
 *  @param[in]      frameRate                   ֡��
 *  @param[in]      sizeGop                     һ��GOP�Ĵ�С 
 *  @returns        adjustType
 *
 */
 
WebRtc_Word32
GeneralMode(networkInf *nInf, statPacketSize sps, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>������</b> ��������ģʽ��������Ҫ�����Ĵ�������ͳ�ư��Ĵ�С 
 *  �����������
 *
 *  <b>�㷨������</b> \n
 *
 *  ��������ģʽ�¾�����������֡����������
 *
 *  <b>˵����</b> \n
 *
 *  ����ֵ������������������������֡�����ʵ��� \n
 *                                              
 *  @req		    GVE-SRS-157
 *  @param[in]      nInf                        ��������ṹ��
 *  @param[in]      sps                         ͳ�ư���С�ṹ��
 *  @param[in]      adjustBandWidth             ��Ҫ�����Ĵ��� 
 *  @param[in]      frameRate                   ֡��
 *  @param[in]      sizeGop                     һ��GOP�Ĵ�С 
 *  @returns        adjustType
 *
 */
WebRtc_Word32
QualityPriorityMode(networkInf *nInf, statPacketSize sps, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>������</b> �˶�����ģʽ��������Ҫ�����Ĵ�������ͳ�ư��Ĵ�С 
 *  �����������
 *
 *  <b>�㷨������</b> \n
 *
 *  �˶�����ģʽ�¾����Ƚ������ʣ�ֱ�����ʽ���4��B����0��1�����P����С�󣬿�ʼ���ж���
 *  
 *  <b>˵����</b> \n
 *
 *  ����ֵ������������������������֡�����ʵ���  \n
 *                                              
 *  @req		    GVE-SRS-158
 *  @param[in]      nInf                        ��������ṹ��
 *  @param[in]      sps                         ͳ�ư���С�ṹ��
 *  @param[in]      adjustBandWidth             ��Ҫ�����Ĵ��� 
 *  @param[in]      frameRate                   ֡��
 *  @param[in]      sizeGop                     һ��GOP�Ĵ�С 
 *  @returns        adjustType
 *
 */
WebRtc_Word32
MotionPriorityMode(networkInf *nInf, statPacketSize sps, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>������</b> ����ģʽֵѡ������ض���ģʽ  
 *  
 *  <b>˵����</b> \n
 *
 *  mode����0�������ģʽ;mode����1�����˶�����;mode����2������������ģʽ 
 *
 *  @req		    GVE-SRS-155
 *  @param[in]      mode                        ѡ����ģʽ
 *  @param[in]      sps                         ͳ�ư���С�ṹ��
 *  @param[in]      nInf                        ��������ṹ��
 *  @param[in]      adjustBandWidth             ��Ҫ�����Ĵ���  
 *  @param[in]      frameRate                   ֡��
 *  @param[in]      sizeGop                     һ��GOP�Ĵ�С 
 *  @returns        ��������
 *
 */
WebRtc_Word32
ModeSelect(int mode, statPacketSize sps, networkInf *nInf, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>������</b> ����ͳ�ư���С��ʵ�ʴ����ó����ڲ���  
 *  
 *  <b>˵����</b> \n
 *
 *  @req		    GVE-SRS---
 *  @param[in]      sps                         ͳ�ư���С�ṹ��
 *  @param[in]      nInf                        ��������ṹ��
 *  @param[in]      adjustBandWidth             ��Ҫ�����Ĵ���  
 *  @param[in]      frameRate                   ֡��
 *  @param[in]      sizeGop                     һ��GOP�Ĵ�С 
 *  @returns        ��������
 *
 */
WebRtc_Word32 
Network::LosePacketStrategy(statPacketSize sps, networkInf *nInf, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>������</b> ����ģʽֵ�������ʵ���  
 *  
 *  <b>˵����</b> \n
 *
 *  mode����0�������ģʽ;mode����1�����˶�����;mode����2������������ģʽ 
 *
 *  @req		    GVE-SRS---
 *  @param[in]      mode                        ѡ����ģʽ
 *  @param[in]      sps                         ͳ�ư���С�ṹ��
 *  @param[in]      nInf                        ��������ṹ��
 *  @param[in]      videoBandWidth              ��Ƶ�ɷ��ʹ���  
 *  @returns        ��������
 *
 */
WebRtc_Word32
Network::AdjustBitrate(int videoBandWidth, networkInf *nInf, statPacketSize *sps, int mode);

/**
 *  @brief  <b>������</b> ����������Filter:����RTP�����Ժ͵�������(adjustType)�����Ƿ񶪵������ݰ�
 *
 *  <b>�㷨������</b> \n
 *  
 *  ��RTP extend bit �ó������ݰ��Ĳ������ \n
 **************************RTP extend bit��ʾ�ĺ���********************** \n
 * -----------------------------------------------------------------------  \n                                                                
 * |  6 |  5 |  4 |  3 |  2 |  1 |                                          \n
 * -----------------------------------------------------------------------  \n
 * | picture_type |   layer id   |                                          \n
 * -----------------------------------------------------------------------  \n
 * layer id Ϊ���ٴ���ڼ���                                                \n
 * picture_type Ϊ1,2,3�ֱ��ʾI/P/B��                                      \n
 *************************************************************************
 *  �������õ��Ĳ�Ͱ�������adjustType����������ֵƥ�䣬���������ݰ� 
 *  �����������ʹ����ݰ�
 *
 *  @req		    GVE-SRS-160
 *  @param[in]      network                      ���練�������
 *  @param[in]      adjustType                   ��������
 *  @param[in]      dataBuffer                   RTP���ݰ���ַ    
 *  @returns        0��ʾ�˰����ͣ�1��ʾ�����˰�
 *
 */
WebRtc_Word32
LossPacketFilter(Network network, int adjustType, char *dataBuffer);

/**
 *  @brief  <b>������</b> ��������
 *  
 *  <b>˵����</b> \n
 *
 *  ��ƽ����ʱ��ƽ�������ʣ���󶪰��ʽ��и���
 *
 *  @req		    GVE-SRS-154
 *  @param[in]      nInf                    ��������ṹ��
 *  @returns        ---
 */
void
Update(networkInf *nInf);

 /**@}*/
/**
 *  @addtogroup Encode_Send_Statistics 
 *  @{
 */
/**
 *  @brief  <b>������</b> ������������Filter:����RTP�����Ժ���Ҫ�Ĳ���������Ƿ񶪵������ݰ� 
 *
 *  <b>�㷨������</b> \n
 *
 *  ��RTP extend bit �ó������ݰ��Ĳ������ \n
 **************************RTP extend bit��ʾ�ĺ���**********************   \n
 * -----------------------------------------------------------------------  \n                                                                
 * |  3 |  2 |  1 |                                                         \n
 * -----------------------------------------------------------------------  \n
 * |   layer id   |                                                         \n
 * -----------------------------------------------------------------------  \n
 * layer id Ϊ���ٴ���ڼ���                                                \n                                     \n
 *************************************************************************
 *  �������õ��Ĳ�Ͱ���������Ҫ�Ĳ����ֵƥ�䣬���������ݰ� 
 *  �����������ʹ����ݰ�
 *
 *  @req		    GVE-SRS-161
 *  @param[in]      layer                        ��Ҫ�Ĳ�
 *  @param[in]      dataBuffer                   RTP���ݰ���ַ 
 *  @returns        0��ʾ�˰����ͣ�1��ʾ�����˰�
 *
 */
WebRtc_Word32
UpThrowLayerFilter(int layer, char *dataBuffer);

/**
 *  @brief  <b>������</b> ͳ�Ƹ����������ݰ���С:���ݴ����ݰ���Сʵʱ�������ݰ���С�ṹ�����
 *
 *  <b>�㷨������</b> \n
 *
 *  1����������RTP�������жϳ��˰����ͺ����ڵĲ� \n
 *  2��Ȼ��������ݰ���С�ṹ���ж�Ӧ�Ĳ���
 *
 *  @req		    GVE-SRS-162
 *  @param[in]      network                      ���練�������
 *  @param[in]      dataBuffer                   RTP���ݰ���ַ    
 *  @param[in]      dataSize                     RTP���ݰ���С  
 *  @returns        ---
 *
 */
void
StatisticsPacketSize(Network *network, char *dataBuffer, int dataSize);
public:
//private:
	int layer;   ///<��Ҫ�Ĳ�
	int mode;        ///<ģʽѡ��
	int frameRate;   ///<��ǰ����֡��
	int gopSize;         ///GOP��С
	int gopIndex;        ///��ǰ֡��GOP��λ��
	networkInf nInf; ///<���練���յ������Ľṹ��
	statPacketSize sps;   ///<ͳ�ư����ʹ�С�Ľṹ��

 /**@}*/  
};

}
#endif /*__NETWORK_H_ */
