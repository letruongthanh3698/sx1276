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

/*****************************************************************************************************************/
/*													MACROS DEFINITION											 */
/*****************************************************************************************************************/
#define USE_BAND_868
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
}Param_t;
/*****************************************************************************************************************/
/*											Application Programming Interface     								 */
/*****************************************************************************************************************/
typedef struct{
	Request_t Req;

	LoRaMac_t LoRaMac;

	Param_t Param;

	bool IsJoin;

	const void (*Init)(void);

	const void (*SetPort)(uint8_t Port);

	const void (*SetBufferData)(uint8_t *Data, uint16_t size);

	const void (*PrepareFrame)(void);

	const void (*SetDevEUI)(uint8_t *DevEUI);

	const void (*SetAppEUI)(uint8_t *AppEUI);

	const void (*SetAppKey)(uint8_t *AppKey);
}LoRaApp_t;

#endif
