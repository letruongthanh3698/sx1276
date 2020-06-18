/*
 * LoRaApp.h
 *
 *  Created on: June 1, 2020
 *      Author: Le Truong Thanh
 *      Comment: Do whatever you want for implementing LoRaWan
 *      First version for RAK4200
 */

#ifndef SRC_LoRaApp_H_
#define SRC_LoRaApp_H_

#include <stdio.h>
#include <stdbool.h>
#include "HCI_SX1276.h"
#include "SCI_SX1276.h"

/*****************************************************************************************************************/
/*													MACROS DEFINITION											 */
/*****************************************************************************************************************/
#define DevEUI_Len		8
#define AppEUI_Len		8
#define AppKey_Len		16
/*****************************************************************************************************************/
/*													   DATA-STRUCTS												 */
/*****************************************************************************************************************/
typedef union uLoRaMacHeader
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to header bits
     */
    struct sHdrBits
    {
        /*!
         * Major version
         */
        uint8_t Major           : 2;
        /*!
         * RFU
         */
        uint8_t RFU             : 3;
        /*!
         * Message type
         */
        uint8_t MType           : 3;
    }Bits;
}LoRaMacHeader_t;

typedef union uLoRaMacFrameCtrl
{
    /*!
     * Byte-access to the bits
     */
    uint8_t Value;
    /*!
     * Structure containing single access to bits
     */
    struct sCtrlBits
    {
        /*!
         * Frame options length
         */
        uint8_t FOptsLen        : 4;
        /*!
         * Frame pending bit
         */
        uint8_t FPending        : 1;
        /*!
         * Message acknowledge bit
         */
        uint8_t Ack             : 1;
        /*!
         * ADR acknowledgment request bit
         */
        uint8_t AdrAckReq       : 1;
        /*!
         * ADR control in frame header
         */
        uint8_t Adr             : 1;
    }Bits;
}LoRaMacFrameCtrl_t;

typedef enum eLoRaMacFrameType
{
    /*!
     * LoRaMAC join request frame
     */
    FRAME_TYPE_JOIN_REQ              = 0x00,
    /*!
     * LoRaMAC join accept frame
     */
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    /*!
     * LoRaMAC unconfirmed up-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    /*!
     * LoRaMAC unconfirmed down-link frame
     */
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    /*!
     * LoRaMAC confirmed up-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    /*!
     * LoRaMAC confirmed down-link frame
     */
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    /*!
     * LoRaMAC RFU frame
     */
    FRAME_TYPE_RFU                   = 0x06,
    /*!
     * LoRaMAC proprietary frame
     */
    FRAME_TYPE_PROPRIETARY           = 0x07,
}LoRaMacFrameType_e;

typedef struct{
	uint8_t Port;
	void *Buffer;
	uint16_t BufferSize;
	int8_t Datarate;
}Request_t;

typedef struct{
	uint8_t DevEUI[DevEUI_Len];
	uint8_t AppEUI[AppEUI_Len];
	uint8_t AppKey[AppKey_Len];
}LoRaMac_t;

typedef struct{
	uint32_t DevAdr;

	uint32_t UpLinkCounter;

	uint32_t DownLinkCounter;

	TxConfig_t TxConfig;

	RxConfig_t RxConfig;
}Param_t;

/*
 * TxPower Definition Enum
 */
typedef enum{
	TxPower_20dBm=0,
	TxPower_14dBm,
	TxPower_11dBm,
	TxPower_8dBm,
	TxPower_5dBm,
	TxPower_2dBm,
}TxPower_e;

/*
 * Spreading Factor Definition Enum
 */
typedef enum{
	SF12=0,
	SF11,
	SF10,
	SF9,
	SF8,
	SF7
}SpreadingFactor_e;

/*
 * Bandwidth Definition Enum
 */
typedef enum{
	BW125=0,
	BW250
}Bandwidth_e;

/*
 * Datarate Definition Enum
 * type DR_ or SF_BW_
 */
typedef enum{
	DR0=0,
	DR1,
	DR2,
	DR3,
	DR4,
	DR5,
	DR6,
	SF12BW125=0,
	SF11BW125,
	SF10BW125,
	SF9BW125,
	SF8BW125,
	SF7BW125,
	SF7BW250
}Datarate_e;

/*
 * Coding Rate Definition Enum
 */
typedef enum{
	CR4_5=1,
	CR4_6,
	CR4_7,
	CR4_8
}CodingRate_e;
/*****************************************************************************************************************/
/*											Application Programming Interface     								 */
/*****************************************************************************************************************/
typedef struct{
	Request_t Req;

	LoRaMac_t LoRaMac;

	Param_t Param;

	bool IsJoin;

	/*
	 * Init LoRaApp.
	 * Frequency: 	868.1 MHz
	 * Datarate:	DR0
	 * Bandwidth:	125
	 * CodingRate:	4/5
	 */
	const void (*Init)(void);

	/*
	 * Set Tx/Rx Frequency
	 */
	const void (*SetFrequency)(uint32_t Freq);

	/*
	 * Set LoRa App Data Port
	 */
	const void (*SetPort)(uint8_t Port);

	/*
	 * Set Data Buffer to Send.
	 * Not Working
	 */
	const void (*SetBufferData)(uint8_t *Data, uint16_t size);

	/*
	 * PrepareFrame with macHdr
	 * macHdr type = Join => Prepare Join Frame with DEV EUI, APP EUI, APP KEY
	 * other macHdr Type => Not Working
	 */
	const void (*PrepareFrame)(LoRaMacHeader_t macHdr);

	/*
	 * Set DEV EUI for LoRa App
	 */
	const void (*SetDevEUI)(uint8_t *DevEUI);

	/*
	 * Set APP EUI for LoRa App
	 */
	const void (*SetAppEUI)(uint8_t *AppEUI);

	/*
	 * Set APP KEY for LoRa App
	 */
	const void (*SetAppKey)(uint8_t *AppKey);

	/*
	 * Set TxPower. Please Refer Enum TxPower_e
	 */
	const void (*SetTxPower)(TxPower_e TxPower);

	/*
	 * Set DataRate. Please Refer Enum Datarate_e
	 */
	const void (*SetDatarate)(Datarate_e Datarate);

	/*
	 * SetPreambleLen. Not Necessary if you dont't want using PreambleLen WakeUp
	 */
	const void (*SetPreambleLen)(uint16_t PreambleLen);

	/*
	 * Begin To Join.
	 * In this version, Join Loop every when TxDone
	 */
	const void (*Join)(void);

	/*
	 * Send Frame
	 */
	const void (*Send)(void);

	/*
	 * Not Working
	 */
	const void (*SetRxWindow1Timeout)(uint32_t timeout);

	/*
	 * Not Working
	 */
	const void (*SetRxWindow2Timeout)(uint32_t timeout);
}LoRaApp_t;

#endif
