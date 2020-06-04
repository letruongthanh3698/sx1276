/*
 * LoRaApp.h
 *
 *  Created on: June 1, 2020
 *      Author: Le Truong Thanh
 *      Comment: Do whatever you want for implementing LoRaWan
 *      First version for RAK4200
 */
#include <stdlib.h>
#include "LoRaApp.h"
#include "SCI_SX1276.h"

/*****************************************************************************************************************/
/*													GLOBAL VARIABLES											 */
/*****************************************************************************************************************/
extern SX1276_t SX1276;
extern const int8_t TxPowers[];
extern const uint8_t Datarates[];
const uint8_t MaxPayloadOfDatarate[];
/*****************************************************************************************************************/
/*													PRIVATE FUNCTION											 */
/*****************************************************************************************************************/
/*
 * LoRaApp Function declaration for sx1276
 */
static void DIO0_IRQ(void);
static void DIO1_IRQ(void);
static void DIO2_IRQ(void);
static void DIO3_IRQ(void);
static void OnTxDone(void);
static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
static void OnTxTimeout(void);
static void OnRxWindow1Timeout(void);
static void OnRxWindow2Timeout(void);

/*
 * LoRaApp Function
 */
static void Init(void);
static void SetPort(uint8_t Port);
static void SetBufferData(uint8_t *Data, uint16_t size);
static void PrepareFrame(LoRaMacHeader_t macHdr);
static void SetDevEUI(uint8_t *DevEUI);
static void SetAppEUI(uint8_t *AppEUI);
static void SetAppKey(uint8_t *AppKey);
static void SetTxPower(TxPower_e TxPower);
static void SetDatarate(Datarate_e Datarate);
static void SetPreambleLen(uint16_t PreambleLen);
static void Join(void);
static void Send(void);
static void SetRxWindow1Timeout(uint32_t timeout);
static void SetRxWindow2Timeout(uint32_t timeout);
/*****************************************************************************************************************/
/*											APPLICATION PROGRAMMING INTERFACE									 */
/*****************************************************************************************************************/
LoRaApp_t LoRaApp={
		.Init				= &Init,
		.SetDevEUI			= &SetDevEUI,
		.SetAppEUI			= &SetAppEUI,
		.SetAppKey			= &SetAppKey,
		.SetPort			= &SetPort,
		.SetBufferData		= &SetBufferData,
		.PrepareFrame		= &PrepareFrame,
		.SetPreambleLen		= &SetPreambleLen,
		.SetTxPower			= &SetTxPower,
		.SetDatarate		= &SetDatarate,
		.Join				= &Join,
		.Send				= &Send
};
/*****************************************************************************************************************/
/*													PRIVATE VARIABLES											 */
/*****************************************************************************************************************/
static bool AdrCtrlOn = false;

static uint16_t LoRaMacBufferPktLen = 0;
static uint16_t LoRaMacTxPayloadLen = 0;
static uint8_t LoRaMacBuffer[100];
static uint16_t LoRaMacDevNonce;
static bool NodeAckRequested = false;

/*****************************************************************************************************************/
/*													FUNCTION IMPLEMENT									 	 	 */
/*****************************************************************************************************************/
void Init(void)
{
	SX1276.SCI->Init();
	//DIO IRQ Implementation
	SX1276_UserFunction_t UserFunction;
	UserFunction.DIO0_IRQ			= &DIO0_IRQ;
	UserFunction.DIO1_IRQ			= &DIO1_IRQ;
	UserFunction.DIO2_IRQ			= &DIO2_IRQ;
	UserFunction.DIO3_IRQ			= &DIO3_IRQ;
	//Radio Funtion Implementation
	UserFunction.OnRxDone			= &OnRxDone;
	UserFunction.OnTxDone			= &OnTxDone;
	UserFunction.OnRxWindow1Timeout	= &OnRxWindow1Timeout;
	UserFunction.OnRxWindow2Timeout	= &OnRxWindow2Timeout;
	UserFunction.OnTxTimeout		= &OnTxTimeout;

	SX1276.SCI->SetUserFunction(UserFunction);

	LoRaApp.Param.TxConfig.Modem_Type		= MODEM_LORA;
	LoRaApp.Param.TxConfig.Frequency		= 868100000;
	LoRaApp.Param.TxConfig.TxPower			= TxPowers[TxPower_14dBm];
	LoRaApp.Param.TxConfig.Datarate			= Datarates[DR0];
	LoRaApp.Param.TxConfig.fDev				= 0;
	LoRaApp.Param.TxConfig.Bandwidth		= BW125;
	LoRaApp.Param.TxConfig.Coderate			= CR4_5;
	LoRaApp.Param.TxConfig.PreambleLen		= 8;
	LoRaApp.Param.TxConfig.FixLen			= false;
	LoRaApp.Param.TxConfig.CrcOn			= true;
	LoRaApp.Param.TxConfig.FreqHopOn		= false;
	LoRaApp.Param.TxConfig.HopPeriod		= 0;
	LoRaApp.Param.TxConfig.IqInverted		= false;
	LoRaApp.Param.TxConfig.Timeout			= 3e3;

	//( MODEM_LORA, RxConfig.Bandwidth, downlinkDatarate, 1, 0, 8, RxConfig.SymbTimeout, false, 0, false, 0, 0, true, RxConfig.RxContinuous );
	LoRaApp.Param.RxConfig.Frequency		= 868100000;
	LoRaApp.Param.RxConfig.Modem_Type		= MODEM_LORA;
	LoRaApp.Param.RxConfig.Bandwidth		= BW125;
	LoRaApp.Param.RxConfig.Datarate			= Datarates[DR0];
	LoRaApp.Param.RxConfig.Coderate			= CR4_5;
	LoRaApp.Param.RxConfig.BandwidthAfc		= 0;
	LoRaApp.Param.RxConfig.PreambleLen		= 8;
	LoRaApp.Param.RxConfig.SymbTimeout		= 8;
	LoRaApp.Param.RxConfig.FixLen			= false;
	LoRaApp.Param.RxConfig.PayloadLen		= 0;
	LoRaApp.Param.RxConfig.CrcOn			= false;
	LoRaApp.Param.RxConfig.FreqHopOn		= 0;
	LoRaApp.Param.RxConfig.HopPeriod		= 0;
	LoRaApp.Param.RxConfig.IqInverted		= true;
	LoRaApp.Param.RxConfig.RxContinuous		= false;
	LoRaApp.Param.RxConfig.MaxRxWindow1		= 5000;
	LoRaApp.Param.RxConfig.MaxRxWindow2		= 6000;
	LoRaApp.Param.RxConfig.MaxPayload		= MaxPayloadOfDatarate[DR0];

}

void SetPort(uint8_t Port)
{
	LoRaApp.Req.Port=Port;
}

void SetBufferData(uint8_t *Data, uint16_t size)
{
	LoRaApp.Req.Buffer=Data;
	LoRaApp.Req.BufferSize=size;
}


void SetDevEUI(uint8_t *DevEUI)
{
	for (uint8_t i=0;i<DevEUI_Len;i++)
	{
		LoRaApp.LoRaMac.DevEUI[i]=DevEUI[i];
	}
}

void SetAppEUI(uint8_t *AppEUI)
{
	for (uint8_t i=0;i<AppEUI_Len;i++)
	{
		LoRaApp.LoRaMac.AppEUI[i]=AppEUI[i];
	}
}

void SetAppKey(uint8_t *AppKey)
{
	for (uint8_t i=0;i<AppKey_Len;i++)
	{
		LoRaApp.LoRaMac.AppKey[i]=AppKey[i];
	}
}

void PrepareFrame(LoRaMacHeader_t macHdr)
{
	LoRaMacFrameCtrl_t Ctrl;
	uint8_t pktHeaderLen = 0;
	uint32_t mic = 0;

    Ctrl.Value = 0;
    Ctrl.Bits.FOptsLen      = 0;
    Ctrl.Bits.FPending      = 0;
    Ctrl.Bits.Ack           = false;
    Ctrl.Bits.AdrAckReq     = false;
    Ctrl.Bits.Adr           = AdrCtrlOn;

    NodeAckRequested = false;

    LoRaMacBufferPktLen = 0;

    LoRaMacTxPayloadLen = LoRaApp.Req.BufferSize;

	LoRaMacBuffer[pktHeaderLen++] = macHdr.Value;

	switch( macHdr.Bits.MType )
	{
		case FRAME_TYPE_JOIN_REQ:
			LoRaMacBufferPktLen = pktHeaderLen;

			memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaApp.LoRaMac.AppEUI, AppEUI_Len );
			LoRaMacBufferPktLen += 8;
			memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaApp.LoRaMac.DevEUI, DevEUI_Len );
			LoRaMacBufferPktLen += 8;

			LoRaMacDevNonce = SX1276.SCI->Random();

			LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

			LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaApp.LoRaMac.AppKey, &mic );

			LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

			break;
		case FRAME_TYPE_DATA_CONFIRMED_UP:
			NodeAckRequested = true;
			//Intentional fallthrough
		case FRAME_TYPE_DATA_UNCONFIRMED_UP:
			if( LoRaApp.IsJoin == false )
			{
				return;// LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
			}

			//Ctrl->Bits.AdrAckReq = AdrNextDr( Ctrl->Bits.Adr, true, &LoRaApp.Req.Datarate );

			/*if( SrvAckRequested == true )
			{
				SrvAckRequested = false;
				fCtrl->Bits.Ack = 1;
			}*/

			LoRaMacBuffer[pktHeaderLen++] = ( LoRaApp.Param.DevAdr ) & 0xFF;
			LoRaMacBuffer[pktHeaderLen++] = ( LoRaApp.Param.DevAdr >> 8 ) & 0xFF;
			LoRaMacBuffer[pktHeaderLen++] = ( LoRaApp.Param.DevAdr >> 16 ) & 0xFF;
			LoRaMacBuffer[pktHeaderLen++] = ( LoRaApp.Param.DevAdr >> 24 ) & 0xFF;

			LoRaMacBuffer[pktHeaderLen++] = Ctrl.Value;

			LoRaMacBuffer[pktHeaderLen++] = LoRaApp.Param.UpLinkCounter & 0xFF;
			LoRaMacBuffer[pktHeaderLen++] = ( LoRaApp.Param.UpLinkCounter >> 8 ) & 0xFF;

			// Copy the MAC commands which must be re-send into the MAC command buffer
			/*memcpy1( &MacCommandsBuffer[MacCommandsBufferIndex], MacCommandsBufferToRepeat, MacCommandsBufferToRepeatIndex );
			MacCommandsBufferIndex += MacCommandsBufferToRepeatIndex;

			if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
			{
				if( ( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_LENGTH ) && ( MacCommandsInNextTx == true ) )
				{
					fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

					// Update FCtrl field with new value of OptionsLength
					LoRaMacBuffer[0x05] = fCtrl->Value;
					for( i = 0; i < MacCommandsBufferIndex; i++ )
					{
						LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
					}
				}
			}
			else
			{
				if( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx ) )
				{
					LoRaMacTxPayloadLen = MacCommandsBufferIndex;
					payload = MacCommandsBuffer;
					framePort = 0;
				}
			}
			MacCommandsInNextTx = false;
			// Store MAC commands which must be re-send in case the device does not receive a downlink anymore
			MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCommandsBuffer, MacCommandsBufferIndex, MacCommandsBufferToRepeat );
			if( MacCommandsBufferToRepeatIndex > 0 )
			{
				MacCommandsInNextTx = true;
			}

			if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
			{
				LoRaMacBuffer[pktHeaderLen++] = framePort;

				if( framePort == 0 )
				{
					LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacNwkSKey, LoRaApp.Param.DevAdr, UP_LINK, LoRaApp.Param.UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
				}
				else
				{
					LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaApp.Param.DevAdr, UP_LINK, LoRaApp.Param.UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
				}
			}
			LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;

			LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaApp.Param.DevAdr, UP_LINK, LoRaApp.Param.UpLinkCounter, &mic );

			LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
			LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

			LoRaMacBufferPktLen += LORAMAC_MFR_LEN;*/

			break;
		default:
			return;// LORAMAC_STATUS_SERVICE_UNKNOWN;
	}


}

void Send(void)
{
	SX1276.SCI->Prepareframe(LoRaMacBuffer,LoRaMacBufferPktLen);
	SX1276.SCI->Send(LoRaApp.Param.TxConfig);
}

void Join()
{
	LoRaMacHeader_t macHdr;
    macHdr.Value = 0;
    macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

    PrepareFrame(macHdr);
    Send();
}

void SetTxPower(TxPower_e TxPower)
{
	LoRaApp.Param.TxConfig.TxPower=TxPowers[TxPower];
}

void SetDatarate(Datarate_e Datarate)
{
	LoRaApp.Param.TxConfig.Datarate=Datarate;
	LoRaApp.Param.RxConfig.Datarate=Datarate;
	LoRaApp.Param.RxConfig.MaxPayload=MaxPayloadOfDatarate[Datarate];
}

void SetPreambleLen(uint16_t PreambleLen)
{
	LoRaApp.Param.TxConfig.PreambleLen=PreambleLen;
	LoRaApp.Param.RxConfig.PreambleLen=PreambleLen;
}

void SetRxWindow1Timeout(uint32_t timeout)
{
	LoRaApp.Param.RxConfig.MaxRxWindow1=timeout;
}

void SetRxWindow2Timeout(uint32_t timeout)
{
	LoRaApp.Param.RxConfig.MaxRxWindow2=timeout;
}

void DIO0_IRQ(void)
{

}

void DIO1_IRQ(void)
{

}

void DIO2_IRQ(void)
{

}

void DIO3_IRQ(void)
{

}

void OnTxDone(void)
{
	//SX1276.SCI->SetOpMode(RF_OPMODE_RECEIVER);
	HAL_Delay(5000);
	SX1276.SCI->RxWindowSetup(LoRaApp.Param.RxConfig);
	volatile uint8_t tmp;
	tmp=SX1276.SCI->Read(REG_OPMODE);
	tmp=SX1276.SCI->Read(REG_LNA);
	tmp=SX1276.SCI->Read(REG_LR_MODEMCONFIG1);
	tmp=SX1276.SCI->Read(REG_LR_MODEMCONFIG2);
	tmp=SX1276.SCI->Read(REG_LR_SYMBTIMEOUTLSB);
	tmp=SX1276.SCI->Read(REG_LR_PAYLOADMAXLENGTH);
	tmp=SX1276.SCI->Read(REG_LR_SYNCWORD);
	tmp=SX1276.SCI->Read(REG_LR_INVERTIQ);
	tmp=SX1276.SCI->Read(REG_LR_INVERTIQ2);
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{

}

void OnTxTimeout(void)
{

}

void OnRxWindow1Timeout(void)
{

}

void OnRxWindow2Timeout(void)
{

}
