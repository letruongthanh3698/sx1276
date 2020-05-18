/*
 * Lora.h
 *
 *  Created on: May 15, 2020
 *      Author: Le Truong Thanh
 *      Comment: None
 *      First version for RAK4200
 */
#ifndef SRC_LORA_H_
#define SRC_LORA_H_

#include "main.h"
#include <stdbool.h>
#include "sx1276Regs-LoRa.h"
#include "sx1276Regs-Fsk.h"
/*****************************************************************************************************************/
/*													MACROS DEFINITION											 */
/*****************************************************************************************************************/
#define RADIO_INIT_REGISTERS_VALUE                \
{                                                 \
    { MODEM_FSK , REG_LNA                , 0x23 },\
    { MODEM_FSK , REG_RXCONFIG           , 0x1E },\
    { MODEM_FSK , REG_RSSICONFIG         , 0xD2 },\
    { MODEM_FSK , REG_AFCFEI             , 0x01 },\
    { MODEM_FSK , REG_PREAMBLEDETECT     , 0xAA },\
    { MODEM_FSK , REG_OSC                , 0x07 },\
    { MODEM_FSK , REG_SYNCCONFIG         , 0x12 },\
    { MODEM_FSK , REG_SYNCVALUE1         , 0xC1 },\
    { MODEM_FSK , REG_SYNCVALUE2         , 0x94 },\
    { MODEM_FSK , REG_SYNCVALUE3         , 0xC1 },\
    { MODEM_FSK , REG_PACKETCONFIG1      , 0xD8 },\
    { MODEM_FSK , REG_FIFOTHRESH         , 0x8F },\
    { MODEM_FSK , REG_IMAGECAL           , 0x02 },\
    { MODEM_FSK , REG_DIOMAPPING1        , 0x00 },\
    { MODEM_FSK , REG_DIOMAPPING2        , 0x30 },\
    { MODEM_LORA, REG_LR_PAYLOADMAXLENGTH, 0x40 },\
}

#define RADIO_INIT_REGISTERS_VALUE                \
{                                                 \
    { MODEM_FSK , REG_LNA                , 0x23 },\
    { MODEM_FSK , REG_RXCONFIG           , 0x1E },\
    { MODEM_FSK , REG_RSSICONFIG         , 0xD2 },\
    { MODEM_FSK , REG_AFCFEI             , 0x01 },\
    { MODEM_FSK , REG_PREAMBLEDETECT     , 0xAA },\
    { MODEM_FSK , REG_OSC                , 0x07 },\
    { MODEM_FSK , REG_SYNCCONFIG         , 0x12 },\
    { MODEM_FSK , REG_SYNCVALUE1         , 0xC1 },\
    { MODEM_FSK , REG_SYNCVALUE2         , 0x94 },\
    { MODEM_FSK , REG_SYNCVALUE3         , 0xC1 },\
    { MODEM_FSK , REG_PACKETCONFIG1      , 0xD8 },\
    { MODEM_FSK , REG_FIFOTHRESH         , 0x8F },\
    { MODEM_FSK , REG_IMAGECAL           , 0x02 },\
    { MODEM_FSK , REG_DIOMAPPING1        , 0x00 },\
    { MODEM_FSK , REG_DIOMAPPING2        , 0x30 },\
    { MODEM_LORA, REG_LR_PAYLOADMAXLENGTH, 0x40 },\
}

#define XTAL_FREQ                                   32000000
#define FREQ_STEP                                   61.03515625

#define RX_BUFFER_SIZE                              256

/*!
 * Sync word for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Sync word for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34

/*****************************************************************************************************************/
/*													   DATA-STRUCTS												 */
/*****************************************************************************************************************/
typedef enum
{
    MODEM_FSK = 0,
    MODEM_LORA,
}RadioModems_t;

typedef struct
{
    RadioModems_t Modem;
    uint8_t       Addr;
    uint8_t       Value;
}RadioRegisters_t;
/*****************************************************************************************************************/
/*											    Software Command Interface     									 */
/*****************************************************************************************************************/
typedef struct{
	RadioModems_t Modem;

	uint8_t length;

	uint8_t buffer[100];

	const void (*Reset)(void);

	const void (*RxChainCalibration)(void);

	const void (*SetOpMode)(uint8_t OpMode);

	const void (*SetModem)(RadioModems_t modem);

	const void (*SetPublicNetwork)(bool enable);

	const uint8_t (*Read)(uint8_t adr);

	const void (*ReadBuffer)(uint8_t adr,uint8_t *buffer,uint8_t length);

	const void (*Write)(uint8_t adr, uint8_t data);

	const void (*WriteBuffer)(uint8_t adr, uint8_t *buffer, uint8_t length);

	const uint8_t (*InOut)(uint8_t outData);

	const void (*Init)(void);

	const void (*Sleep)(void);

	const void (*Send)();

	const void (*Prepareframe)();

	const void (*SetChannel)();

	const void (*SetMaxPayloadLength)(RadioModems_t Modem, uint8_t length);

	const void (*SetTxConfig)( RadioModems_t modem, int8_t power, uint32_t fdev,
            uint32_t bandwidth, uint32_t datarate,
            uint8_t coderate, uint16_t preambleLen,
            bool fixLen, bool crcOn, bool freqHopOn,
            uint8_t hopPeriod, bool iqInverted, uint32_t timeout );

	const void (*SetStby)();

}SX1276_t;

extern SX1276_t SX1276;
#endif
