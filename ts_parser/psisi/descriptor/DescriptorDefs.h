#ifndef _DESCRIPTOR_DEFS_H_
#define _DESCRIPTOR_DEFS_H_


#define DESC_TAG__NETWORK_NAME					(0x40)  // ネットワーク名記述子
#define DESC_TAG__SERVICE_LIST					(0x41)  // サービスリスト記述子
#define DESC_TAG__STUFFING						(0x42)  // スタッフ記述子
#define DESC_TAG__SATELLITE_DELIVERY_SYSTEM		(0x43)  // 衛星分配システム記述子
#define DESC_TAG__CABLE_DELIVERY_SYSTEM			(0x44)  // 有線分配システム記述子
#define DESC_TAG__BOUQET_NAME					(0x47)  // ブーケ名記述子
#define DESC_TAG__SERVICE						(0x48)  // サービス記述子
#define DESC_TAG__COUNTRY_AVAILABILITY			(0x49)  // 国別受信可否記述子
#define DESC_TAG__LINKAGE						(0x4a)  // リンク記述子
#define DESC_TAG__NVOD_REFERENCE				(0x4b)  // NVOD基準サービス記述子
#define DESC_TAG__TIME_SHIFTED_SERVICE			(0x4c)  // タイムシフトサービス記述子
#define DESC_TAG__SHORT_EVENT					(0x4d)  // 短形式イベント記述子
#define DESC_TAG__EXTENDED_EVENT				(0x4e)  // 拡張形式イベント記述子
#define DESC_TAG__TIME_SHIFTED_EVENT			(0x4f)  // タイムシフトイベント記述子

#define DESC_TAG__COMPONENT						(0x50)  // コンポーネント記述子
#define DESC_TAG__MOSAIC						(0x51)  // モザイク記述子
#define DESC_TAG__STREAM_IDENTIFIER				(0x52)  // ストリーム識別記述子
#define DESC_TAG__CA_IDENTIFIER					(0x53)  // CA識別記述子
#define DESC_TAG__CONTENT						(0x54)  // コンテント記述子
#define DESC_TAG__PARENTAL_RATING				(0x55)  // パレンタルレート記述子
#define DESC_TAG__TELETEXT						(0x56)  // 
#define DESC_TAG__TELEPHONE						(0x57)  // 
#define DESC_TAG__LOCAL_TIME_OFFSET				(0x58)  // ローカル時間オフセット記述子
#define DESC_TAG__SUBTITLING					(0x59)  // 
#define DESC_TAG__TERRESTRIAL_DLIVRERY_SYSTEM	(0x5a)  // 
#define DESC_TAG__MULTILINGUAL_NETWORK_NAME		(0x5b)  // 
#define DESC_TAG__MULTILINGUAL_BOUQET_NAME		(0x5c)  // 
#define DESC_TAG__MULTILINGUAL_SERVICE_NAME		(0x5d)  // 
#define DESC_TAG__MULTILINGUAL_COMPONENT		(0x5e)  // 
#define DESC_TAG__PRIVATE_DATA_SPECIFIER		(0x5f)  // 

#define DESC_TAG__SERVICE_MOVE					(0x60)  // 
#define DESC_TAG__SHORT_SMOOTHING_BUFFER		(0x61)  // 
#define DESC_TAG__FREQUENCY_LIST				(0x62)  // 
#define DESC_TAG__PARTIAL_TRANSPORT_STREAM		(0x63)  // パーシャルTS記述子
#define DESC_TAG__DATA_BROADCAST				(0x64)  // 
#define DESC_TAG__CA_SYSTEM						(0x65)  // 
#define DESC_TAG__DATA_BROADCAST_ID				(0x66)  // 

#define DESC_TAG__TRANSCODE_MODE_REGISTRATION	(0x05)  // 
#define DESC_TAG__REGISTRATION					(0x05)  // 
#define DESC_TAG__DTCP							(0x88)  // 



#include "ShortEventDescriptor.h"


#endif
