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
 *  @date 5月30 2013
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
 *  利用DTS传递过来的网络反馈参数计算应该采取的丢包、丢帧、码率调整策略
 *
 *  @defgroup Encode_Send_Statistics 
 *  @ingroup Network_Feedback_Module
 *  @brief Statistics information of encode & send for Network_Feedback implement
 *
 *  为网络反馈策略的采取提供辅助信息，统计包类型大小等
 *
 */
/*------------------------------------  DEFINITION ------------------------------------*/
#define RTPHEADSIZE 0x12      /*need update with X264 encoder struct RTPEXTENDHEADER real-time */
#define GOPINDEX  0x12        /*need update with X264 encoder struct PacketLoss real-time */
#define PACKET_LOSS_THRESHOLD_HIGH 2       /*丢包率高阈值*/
#define PACKET_LOSS_THRESHOLD_LOW 0.5      /*丢包率低阈值*/
#define RATIO_RTT_THRESHOLD_LOW 1.1        /*RTT低阈值*/  
#define RATIO_RTT_THRESHOLD_HIGH 1.9       /*RTT高阈值*/  
#define VIDEO_RATIO_TOTAL 0.2              /*视频目标带宽在物理总带宽中的比例*/  
#define BITRATE_FLUCTUATE_THRESHOLD 15     /*码率波动阈值,超过此值刷新码率*/  
#define MAXBITRATE                512      /*上发最大带宽*/
#define RATIO_MOTIONPRIORITY      0.9      /*运动优先模式，码率调节系数*/
#define RATIO_QUALITYPRIORITY     1.2      /*质量优先模式，码率调节系数*/
#define FALSE 0
#define TRUE 1
/*----------------------------------- ENUMERATIONS ------------------------------------*/
/*--------------------------------- STRUCT TYPES --------------------------------------*/
/**
 *  @addtogroup Network_Feedback_Module
 *  @brief 网络反馈传递过来的参数
 *  @{
 */
typedef struct networkInf
{
	int phyTotalBandWidth;   ///<可用物理总带宽
	int videoSendBandWidth;   ///<上次视频发送带宽
	int otherTotalBandWidth;   ///<其他业务总带宽
	int lastOtherTotalBandWidth;   ///<上一次其他业务总带宽
	int genBandWidth;   ///<上一次观看端整体网络带宽
	int avgLossRate;   ///<平均丢包率
	int lastAvgLossRate;      ///<历史平均丢包率
	int maxLossRate;   ///<最大丢包率
	int lastMaxLossRate;   ///<历史最大丢包率
	int avgRtt;   ///<平均延时
	int lastRtt;   ///<历史延时
	int maxRtt;   ///<最大延时
	int jitter;   ///抖动
	int minBandWidth;   ///<视频最小码率
	int maxBandWidth;   ///<视频最大码率
	int peopleNumber;   ///<参会人数
	int adjustType;   ///<网络反馈调整参数
	int bitrate;      ///需要下调的码率
	bool resetBitrate; ///码率是否调整
}networkInf;

/**
 *  @brief 统计包类型大小及调整参数的结构体
 */
typedef struct statPacketSize
{
	int sizeLayer0_I;   ///<0层I包大小
	int sizeLayer0_P;   ///<0层P包大小
	int sizeLayer0_B;   ///<0层B包大小
	int sizeLayer0_B0;   ///<0层B0包大小 
	int sizeLayer0_B1;   ///<0层B1包大小
	int sizeLayer1_I;   ///<1层I包大小
	int sizeLayer1_P;   ///<1层P包大小
	int sizeLayer1_B;   ///<1层B包大小
	int sizeLayer1_B0;   ///<1层B0包大小
	int sizeLayer1_B1;   ///<1层B1包大小
	int sizeLayer2_I;   ///<2层I包大小
	int sizeLayer2_P;   ///<2层P包大小
	int sizeLayer2_B;   ///<2层B包大小
	int sizeLayer2_B0;  ///<2层B0包大小
	int sizeLayer2_B1;   ///<2层B1包大小
	int sizeLayer3_I;   ///<3层I包大小
	int sizeLayer3_P;   ///<3层P包大小
	int sizeLayer3_B;   ///<3层B包大小
	int sizeLayer3_B0;   ///<3层B0包大小
	int sizeLayer3_B1;   ///<3层B1包大小
	int totalSize;   ///<一个GOP内总共发送数据包的大小
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
 *  @brief  <b>简述：</b> 利用观看端整体网络的平均丢包率、最大丢包率、平均延时、参会人数 
 *  计算观看端整体网络带宽
 *
 *  <b>算法描述：</b> \n
 *
 *  1、观看端整体网络带宽以RTT为主，丢包率为辅计算 \n
 *  2、函数首先判断RTT的值如果超过阈值，不考虑丢包率直接进行带宽的调整  
 *  具体下调的幅度根据实际网络测试得出 \n
 *  3、如果当前的丢包率大于阈值，不考虑RTT直接进行带宽的调整
 *  具体下调的幅度根据实际网络测试得出 \n
 *  4、如果当前RTT小于阈值，丢包率也小于阈值且小于历史平均丢包率
 *  说明网络状况变好，以当前1/8带宽的速率增长 \n
 *  5、如果当前RTT小于阈值，丢包率小于阈值但不小于历史平均丢包率 
 *  说明网络状况还不好，根据平均丢包率、最大丢包率和参会人数
 *  计算大部分人的网络状况 \n
 *  6、现认为丢包率符合正态分布，大部分人的网络状况以平均值+均方差得出
 *  即考虑的是84.1%的人
 *  
 *  <b>说明：</b> \n
 *
 *  观看端整体网络数据20s更新一次，目前调节的频率为一个GOP 
 *
 *  @req		   GVE-SRS-154
 *  @param[in]     nInf        网络参数结构体
 *  @returns       观看端整体网络带宽
 *  
 */
WebRtc_Word32 
GenNetworkBandwidth(networkInf nInf);
	
/**
 *  @brief  <b>简述：</b> 利用观看端整体网络带宽、可用物理总带宽、其他业务总带宽、视频目标带宽、视频最大、小码率
 *  计算视频上行发送带宽
 *
 *  <b>算法描述：</b> \n
 *
 *  1、目前网络传输过来的参数有可用物理总带宽、其他业务总带宽、视频目标带宽 \n
 *  2、根据当前是否有可用带宽决定是否上调视频目标带宽 \n
 *  3、首先根据历史其他业务总带宽预测当前其他业务总带宽 \n
 *  4、可用带宽由可用物理总带宽、当前其他业务总带宽和视频目标带宽计算得出 \n
 *  5、计算出的视频可用带宽与观看端整体网络带宽比较取最小值 \n
 *  6、最后把视频可用带宽限定为<min, max>之间
 *
 *  <b>说明：</b> \n
 *
 *  网络传输过来的参数目前更新频率为800ms，带宽调节的频率为一个GOP 
 *
 *  @req		    GVE-SRS-154
 *  @param[in]      nInf                    网络参数结构体
 *  @returns        视频上行发送带宽
 *
 */
WebRtc_Word32 
UpBandWidth(networkInf nInf);

/**
 *  @brief  <b>简述：</b> 利用视频可发送带宽、帧率、编码大小
 *  计算需要调整带宽量
 *
 *  <b>算法描述：</b> \n
 *
 *  1、首先把可发送的视频带宽(此时以s为单位)转化为GOP为单位的带宽 \n
 *  2、实际编码大小减去视频可发送大小即为需要调整的带宽
 *
 *  <b>说明：</b> \n
 *
 *  需考虑帧率变化的影响 
 *
 *  @req		    GVE-SRS-154
 *  @param[in]      frameRate                 当前发送帧率
 *  @param[in]      videoBandWidth            视频上行发送带宽
 *  @param[in]      sizeGop                   一个GOP包含的帧数
 *  @param[in]      totalSize                 一个GOP包总大小   
 *  @returns        需要调整的带宽
 *
 */
WebRtc_Word32
AdjustBandWidth(int frameRate, int videoBandWidth, int sizeGop, int totalSize);

/**
 *  @brief  <b>简述：</b> 均衡模式下利用需要调整的带宽量和统计包的大小 
 *  计算调整参数
 *
 *  <b>算法描述：</b> \n
 *
 *  1、均衡模式下调节带宽的时候丢包和码率交互调节的频率比较高
 *  主要是为了找出帧率和图像质量的平衡点 \n
 *  2、如果调整的带宽小于阈值T0，只进行丢包 \n
 *  3、如果调整的带宽大于阈值T0，小于阈值T1，在丢包的基本上再降一部分码率
 *  循环上面两步
 *  
 *  <b>说明：</b> \n
 *
 *  返回值调整参数用来决定丢包、丢帧和码率调整  \n
 *                                              
 ************************************adjustType每个bit表示的涵义******************************  \n
 * -------------------------------------------------------------------------------------------  \n                                                                 
 * | 18 | 17 | 16 | 15 | 14 | 13 | 12 | 11 | 10 | 9  |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  \n
 * -------------------------------------------------------------------------------------------  \n
 * |    |    | I3 | I2 | I1 | I0 | P3 | P2 | P1 | P0 | B31| B30| B21| B20| B11| B10| B01| B00|  \n
 * -------------------------------------------------------------------------------------------  \n
 * 那个bit为1，说明需要丢掉此类型的包                           \n
 *********************************************************************************************  \n
 *
 *  @req		    GVE-SRS-156
 *  @param[in]      nInf                        网络参数结构体
 *  @param[in]      sps                         统计包大小结构体
 *  @param[in]      adjustBandWidth             需要调整的带宽 
 *  @param[in]      frameRate                   帧率
 *  @param[in]      sizeGop                     一个GOP的大小 
 *  @returns        adjustType
 *
 */
 
WebRtc_Word32
GeneralMode(networkInf *nInf, statPacketSize sps, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>简述：</b> 质量优先模式下利用需要调整的带宽量和统计包的大小 
 *  计算调整参数
 *
 *  <b>算法描述：</b> \n
 *
 *  质量优先模式下尽量丢包、丢帧不降低码率
 *
 *  <b>说明：</b> \n
 *
 *  返回值调整参数用来决定丢包、丢帧和码率调整 \n
 *                                              
 *  @req		    GVE-SRS-157
 *  @param[in]      nInf                        网络参数结构体
 *  @param[in]      sps                         统计包大小结构体
 *  @param[in]      adjustBandWidth             需要调整的带宽 
 *  @param[in]      frameRate                   帧率
 *  @param[in]      sizeGop                     一个GOP的大小 
 *  @returns        adjustType
 *
 */
WebRtc_Word32
QualityPriorityMode(networkInf *nInf, statPacketSize sps, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>简述：</b> 运动优先模式下利用需要调整的带宽量和统计包的大小 
 *  计算调整参数
 *
 *  <b>算法描述：</b> \n
 *
 *  运动优先模式下尽量先降低码率，直到码率降了4层B包和0，1两层的P包大小后，开始进行丢包
 *  
 *  <b>说明：</b> \n
 *
 *  返回值调整参数用来决定丢包、丢帧和码率调整  \n
 *                                              
 *  @req		    GVE-SRS-158
 *  @param[in]      nInf                        网络参数结构体
 *  @param[in]      sps                         统计包大小结构体
 *  @param[in]      adjustBandWidth             需要调整的带宽 
 *  @param[in]      frameRate                   帧率
 *  @param[in]      sizeGop                     一个GOP的大小 
 *  @returns        adjustType
 *
 */
WebRtc_Word32
MotionPriorityMode(networkInf *nInf, statPacketSize sps, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>简述：</b> 依据模式值选择进行特定的模式  
 *  
 *  <b>说明：</b> \n
 *
 *  mode等于0进入均衡模式;mode等于1进入运动优先;mode等于2进入质量优先模式 
 *
 *  @req		    GVE-SRS-155
 *  @param[in]      mode                        选定的模式
 *  @param[in]      sps                         统计包大小结构体
 *  @param[in]      nInf                        网络参数结构体
 *  @param[in]      adjustBandWidth             需要调整的带宽  
 *  @param[in]      frameRate                   帧率
 *  @param[in]      sizeGop                     一个GOP的大小 
 *  @returns        调整参数
 *
 */
WebRtc_Word32
ModeSelect(int mode, statPacketSize sps, networkInf *nInf, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>简述：</b> 依据统计包大小和实际带宽，得出调节参数  
 *  
 *  <b>说明：</b> \n
 *
 *  @req		    GVE-SRS---
 *  @param[in]      sps                         统计包大小结构体
 *  @param[in]      nInf                        网络参数结构体
 *  @param[in]      adjustBandWidth             需要调整的带宽  
 *  @param[in]      frameRate                   帧率
 *  @param[in]      sizeGop                     一个GOP的大小 
 *  @returns        调整参数
 *
 */
WebRtc_Word32 
Network::LosePacketStrategy(statPacketSize sps, networkInf *nInf, int adjustBandWidth, int frameRate, int sizeGop);

/**
 *  @brief  <b>简述：</b> 依据模式值进行码率调整  
 *  
 *  <b>说明：</b> \n
 *
 *  mode等于0进入均衡模式;mode等于1进入运动优先;mode等于2进入质量优先模式 
 *
 *  @req		    GVE-SRS---
 *  @param[in]      mode                        选定的模式
 *  @param[in]      sps                         统计包大小结构体
 *  @param[in]      nInf                        网络参数结构体
 *  @param[in]      videoBandWidth              视频可发送带宽  
 *  @returns        调整参数
 *
 */
WebRtc_Word32
Network::AdjustBitrate(int videoBandWidth, networkInf *nInf, statPacketSize *sps, int mode);

/**
 *  @brief  <b>简述：</b> 行主动丢包Filter:根据RTP包属性和调整参数(adjustType)决定是否丢掉此数据包
 *
 *  <b>算法描述：</b> \n
 *  
 *  由RTP extend bit 得出此数据包的层和类型 \n
 **************************RTP extend bit表示的涵义********************** \n
 * -----------------------------------------------------------------------  \n                                                                
 * |  6 |  5 |  4 |  3 |  2 |  1 |                                          \n
 * -----------------------------------------------------------------------  \n
 * | picture_type |   layer id   |                                          \n
 * -----------------------------------------------------------------------  \n
 * layer id 为多少代表第几层                                                \n
 * picture_type 为1,2,3分别表示I/P/B包                                      \n
 *************************************************************************
 *  如果上面得到的层和包类型与adjustType调整参数的值匹配，丢掉此数据包 
 *  否则正常发送此数据包
 *
 *  @req		    GVE-SRS-160
 *  @param[in]      network                      网络反馈类对象
 *  @param[in]      adjustType                   调整参数
 *  @param[in]      dataBuffer                   RTP数据包地址    
 *  @returns        0表示此包发送，1表示丢掉此包
 *
 */
WebRtc_Word32
LossPacketFilter(Network network, int adjustType, char *dataBuffer);

/**
 *  @brief  <b>简述：</b> 参数更新
 *  
 *  <b>说明：</b> \n
 *
 *  对平均延时，平均丢包率，最大丢包率进行更新
 *
 *  @req		    GVE-SRS-154
 *  @param[in]      nInf                    网络参数结构体
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
 *  @brief  <b>简述：</b> 上行主动丢层Filter:根据RTP包属性和需要的层参数决定是否丢掉此数据包 
 *
 *  <b>算法描述：</b> \n
 *
 *  由RTP extend bit 得出此数据包的层和类型 \n
 **************************RTP extend bit表示的涵义**********************   \n
 * -----------------------------------------------------------------------  \n                                                                
 * |  3 |  2 |  1 |                                                         \n
 * -----------------------------------------------------------------------  \n
 * |   layer id   |                                                         \n
 * -----------------------------------------------------------------------  \n
 * layer id 为多少代表第几层                                                \n                                     \n
 *************************************************************************
 *  如果上面得到的层和包类型与需要的层参数值匹配，丢掉此数据包 
 *  否则正常发送此数据包
 *
 *  @req		    GVE-SRS-161
 *  @param[in]      layer                        需要的层
 *  @param[in]      dataBuffer                   RTP数据包地址 
 *  @returns        0表示此包发送，1表示丢掉此包
 *
 */
WebRtc_Word32
UpThrowLayerFilter(int layer, char *dataBuffer);

/**
 *  @brief  <b>简述：</b> 统计各种类型数据包大小:根据此数据包大小实时更新数据包大小结构体参数
 *
 *  <b>算法描述：</b> \n
 *
 *  1、首先依据RTP包属性判断出此包类型和所在的层 \n
 *  2、然后更新数据包大小结构体中对应的参数
 *
 *  @req		    GVE-SRS-162
 *  @param[in]      network                      网络反馈类对象
 *  @param[in]      dataBuffer                   RTP数据包地址    
 *  @param[in]      dataSize                     RTP数据包大小  
 *  @returns        ---
 *
 */
void
StatisticsPacketSize(Network *network, char *dataBuffer, int dataSize);
public:
//private:
	int layer;   ///<需要的层
	int mode;        ///<模式选择
	int frameRate;   ///<当前发送帧率
	int gopSize;         ///GOP大小
	int gopIndex;        ///当前帧在GOP中位置
	networkInf nInf; ///<网络反馈收到参数的结构体
	statPacketSize sps;   ///<统计包类型大小的结构体

 /**@}*/  
};

}
#endif /*__NETWORK_H_ */
