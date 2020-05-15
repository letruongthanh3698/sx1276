/*
 * Lora.c
 *
 *  Created on: May 15, 2020
 *      Author: Le Truong Thanh
 *      Comment: None
 *      First version for RAK4200
 */

#include "lora.h"

/*****************************************************************************************************************/
/*													GLOBAL VARIABLES											 */
/*****************************************************************************************************************/

/*****************************************************************************************************************/
/*													PRIVATE FUNCTION											 */
/*****************************************************************************************************************/
static void SX1276Reset(void);
static uint8_t SX1276Read(uint8_t adr);
static void SX1276ReadBuffer(uint8_t adr,uint8_t *buffer,uint8_t length);
static void SX1276Write(uint8_t adr, uint8_t data);
static void SX1276WriteBuffer(uint8_t adr, uint8_t *buffer, uint8_t length);
static uint8_t SX1276InOut(uint8_t adr);
static void RxChainCalibration(void);
static void SX1276SetOpMode(uint8_t OpMode);
static void SX1276SetModem( RadioModems_t modem );
static void SX1276SetPublicNetwork(bool enable);
static void SX1276Init(void);
static void SX1276Sleep(void);
static void SX1276Send();
static void SX1276PrepareFrame();
static void SX1276SetChannel(uint32_t freq);
static void SX1276SetMacPayloadLength(RadioModems_t Modem, uint8_t length);
static void SX1276SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
        uint32_t bandwidth, uint32_t datarate,
        uint8_t coderate, uint16_t preambleLen,
        bool fixLen, bool crcOn, bool freqHopOn,
        uint8_t hopPeriod, bool iqInverted, uint32_t timeout );
static void SX1276SetStby();
static void SX1276SetRfTxPower( int8_t power );
static void SX1276SetTx();
/*****************************************************************************************************************/
/*											APPLICATION PROGRAMMING INTERFACE									 */
/*****************************************************************************************************************/
SX1276_t SX1276 ={
		.InOut				= &SX1276InOut,
		.Read				= &SX1276Read,
		.ReadBuffer			= &SX1276ReadBuffer,
		.Write				= &SX1276Write,
		.WriteBuffer		= &SX1276WriteBuffer,
		.RxChainCalibration	= &RxChainCalibration,
		.SetOpMode			= &SX1276SetOpMode,
		.SetModem			= &SX1276SetModem,
		.Init				= &SX1276Init,
		.Sleep				= &SX1276Sleep,
		.Send				= &SX1276Send,
		.Prepareframe		= &SX1276PrepareFrame,
		.SetChannel			= &SX1276SetChannel,
		.SetMaxPayloadLength= &SX1276SetMacPayloadLength,
		.SetTxConfig		= &SX1276SetTxConfig,
		.SetStby			= &SX1276SetStby
};
/*****************************************************************************************************************/
/*													PRIVATE VARIABLES											 */
/*****************************************************************************************************************/
const RadioRegisters_t RadioRegsInit[] = RADIO_INIT_REGISTERS_VALUE;
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*****************************************************************************************************************/
/*													FUNCTION IMPLEMENT									 	 	 */
/*****************************************************************************************************************/
void SX1276Reset(void)
{
	HAL_GPIO_WritePin(NRESET_GPIO_Port,NRESET_Pin,0);
	HAL_Delay(10);
	HAL_GPIO_WritePin(NRESET_GPIO_Port,NRESET_Pin,1);
	HAL_Delay(60);
}

uint8_t SX1276InOut(uint8_t outData)
{
    uint8_t rxData = 0;

    __HAL_SPI_ENABLE(&hspi1);

    __disable_irq();

    while( __HAL_SPI_GET_FLAG( &hspi1, SPI_FLAG_TXE ) == RESET );
    hspi1.Instance->DR = ( uint16_t ) ( outData & 0xFF );

    while( __HAL_SPI_GET_FLAG( &hspi1, SPI_FLAG_RXNE ) == RESET );
    rxData = ( uint8_t ) hspi1.Instance->DR;

    __enable_irq();

    __HAL_SPI_DISABLE(&hspi1);

    return( rxData );
}

uint8_t SX1276Read(uint8_t adr)
{
	uint8_t data;
	SX1276ReadBuffer(adr,&data,1);
	return data;
}

void SX1276ReadBuffer(uint8_t adr, uint8_t *buffer, uint8_t length)
{
    uint8_t i;

    //NSS = 0;
    HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin,0);

    SX1276InOut( adr & 0x7F );

    for( i = 0; i < length; i++ )
    {
        buffer[i] = SX1276InOut( 0 );
    }

    //NSS = 1;
    HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin,1);
}

void SX1276Write(uint8_t adr, uint8_t data)
{
	SX1276WriteBuffer(adr,&data,1);
}

void SX1276WriteBuffer(uint8_t adr, uint8_t *buffer, uint8_t length)
{
    //NSS = 0;
	HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin,0);


    //__HAL_SPI_ENABLE(&SX1276.Spi.Spi);
    SX1276InOut( adr | 0x80 );
    for( uint8_t i = 0; i < length; i++ )
    {
        SX1276InOut( buffer[i] );
    }

    //NSS = 1;
    //__HAL_SPI_DISABLE(&SX1276.Spi.Spi);
    HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin,1);
}

void RxChainCalibration()
{
	uint8_t regPaConfigInitVal;
	uint32_t initialFreq;

	// Save context
	regPaConfigInitVal = SX1276Read( REG_PACONFIG );
	initialFreq = ( double )( ( ( uint32_t )SX1276Read( REG_FRFMSB ) << 16 ) |
							  ( ( uint32_t )SX1276Read( REG_FRFMID ) << 8 ) |
							  ( ( uint32_t )SX1276Read( REG_FRFLSB ) ) ) * ( double )FREQ_STEP;

	// Cut the PA just in case, RFO output, power = -1 dBm
	SX1276Write( REG_PACONFIG, 0x00 );

	// Launch Rx chain calibration for LF band
	SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
	while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
	{
	}

	// Sets a Frequency in HF band
	SX1276SetChannel( 868000000 );

	// Launch Rx chain calibration for HF band
	SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
	while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
	{
	}

	// Restore context
	SX1276Write( REG_PACONFIG, regPaConfigInitVal );
	SX1276SetChannel( initialFreq );
}

void SX1276SetOpMode(uint8_t OpMode)
{
    /*if( OpMode == RF_OPMODE_SLEEP )
    {
        SX1276SetAntSwLowPower( true );
    }
    else
    {
        SX1276SetAntSwLowPower( false );
        SX1276SetAntSw( OpMode );
    }*/
    volatile uint8_t temp;
    temp=SX1276Read( REG_OPMODE );

    SX1276Write( REG_OPMODE, ( temp & RF_OPMODE_MASK ) | OpMode );
    temp=SX1276Read( REG_OPMODE );
}

void SX1276SetModem( RadioModems_t modem )
{
	//if (modem==MODEM_FSK) return;
    volatile uint8_t temp;
    temp=SX1276Read( REG_OPMODE );
    assert_param( ( &hspi1.Instance != NULL ) );

    if( ( temp & RFLR_OPMODE_LONGRANGEMODE_ON ) != 0 )
    {
        SX1276.Modem = MODEM_LORA;
    }
    else
    {
        SX1276.Modem = MODEM_FSK;
    }

    if( SX1276.Modem == modem )
    {
        return;
    }

    SX1276.Modem = modem;
    switch( SX1276.Modem )
    {
    default:
    case MODEM_FSK:
        SX1276Sleep( );
        temp=SX1276Read( REG_OPMODE );
        SX1276Write( REG_OPMODE, ( temp & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_OFF );

        SX1276Write( REG_DIOMAPPING1, 0x00 );
        SX1276Write( REG_DIOMAPPING2, 0x30 ); // DIO5=ModeReady
        break;
    case MODEM_LORA:
        SX1276Sleep( );
        temp=SX1276Read( REG_OPMODE );
        SX1276Write( REG_OPMODE, ( temp & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON );

        SX1276Write( REG_DIOMAPPING1, 0x00 );
        SX1276Write( REG_DIOMAPPING2, 0x00 );
        break;
    }
}

void SX1276SetPublicNetwork( bool enable )
{
    SX1276SetModem( MODEM_LORA );
    //SX1276.Settings.LoRa.PublicNetwork = enable;
    if( enable == true )
    {
        // Change LoRa modem SyncWord
        SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

void SX1276Init( )
{
    uint8_t i;

    //RadioEvents = events;

    // Initialize driver timeout timers
    //TimerInit( &TxTimeoutTimer, SX1276OnTimeoutIrq );
    //TimerInit( &RxTimeoutTimer, SX1276OnTimeoutIrq );
    //TimerInit( &RxTimeoutSyncWord, SX1276OnTimeoutIrq );

    SX1276Reset( );

    RxChainCalibration( );

    SX1276SetOpMode( RF_OPMODE_SLEEP );

    //SX1276IoIrqInit( DioIrq );

    for( i = 0; i < sizeof( RadioRegsInit ) / sizeof( RadioRegisters_t ); i++ )
    {
        SX1276SetModem( RadioRegsInit[i].Modem );
        SX1276Write( RadioRegsInit[i].Addr, RadioRegsInit[i].Value );
    }

    SX1276SetModem( MODEM_FSK );
    SX1276SetPublicNetwork(false);

    //SX1276.Settings.State = RF_IDLE;

}

void SX1276Sleep()
{
	SX1276SetOpMode( RF_OPMODE_SLEEP );
}

void SX1276PrepareFrame()
{
	SX1276.buffer[0]=0x30;
	SX1276.buffer[1]=0x30;
	SX1276.buffer[2]=0x30;
	SX1276.length=3;
}

void SX1276SetChannel(uint32_t freq)
{
    //SX1276.Settings.Channel = freq;
    freq = ( uint32_t )( ( double )freq / ( double )FREQ_STEP );

    SX1276Write( REG_FRFMSB, ( uint8_t )( ( freq >> 16 ) & 0xFF ) );
    SX1276Write( REG_FRFMID, ( uint8_t )( ( freq >> 8 ) & 0xFF ) );
    SX1276Write( REG_FRFLSB, ( uint8_t )( freq & 0xFF ) );
}

void SX1276SetMacPayloadLength(RadioModems_t Modem, uint8_t length)
{
    SX1276SetModem( Modem );

    switch( Modem )
    {
    case MODEM_FSK:
        /*if( SX1276.Settings.Fsk.FixLen == false )
        {
            SX1276Write( REG_PAYLOADLENGTH, max );
        }*/
        break;
    case MODEM_LORA:
        SX1276Write( REG_LR_PAYLOADMAXLENGTH, length );
        break;
    }
}

void SX1276SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
        uint32_t bandwidth, uint32_t datarate,
        uint8_t coderate, uint16_t preambleLen,
        bool fixLen, bool crcOn, bool freqHopOn,
        uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
	SX1276SetModem( modem );
	SX1276SetRfTxPower( power );
	uint8_t LowDatarateOptimize;

	switch( modem )
	    {
	    case MODEM_FSK:
	    	break;
	    case MODEM_LORA:
	    	bandwidth += 7;

	    	if( datarate > 12 )
				{
					datarate = 12;
				}
				else if( datarate < 6 )
				{
					datarate = 6;
				}
				if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
					( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
				{
					LowDatarateOptimize = 0x01;
				}
				else
				{
					LowDatarateOptimize = 0x00;
	    	    }

	    	if(freqHopOn == true )
				{
					SX1276Write( REG_LR_PLLHOP, ( SX1276Read( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
					SX1276Write( REG_LR_HOPPERIOD, hopPeriod );
				}
	    	SX1276Write( REG_LR_MODEMCONFIG1,
	    	                         ( SX1276Read( REG_LR_MODEMCONFIG1 ) &
	    	                           RFLR_MODEMCONFIG1_BW_MASK &
	    	                           RFLR_MODEMCONFIG1_CODINGRATE_MASK &
	    	                           RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
	    	                           ( bandwidth << 4 ) | ( coderate << 1 ) |
	    	                           fixLen );

			SX1276Write( REG_LR_MODEMCONFIG2,
						 ( SX1276Read( REG_LR_MODEMCONFIG2 ) &
						   RFLR_MODEMCONFIG2_SF_MASK &
						   RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK ) |
						   ( datarate << 4 ) | ( crcOn << 2 ) );

			SX1276Write( REG_LR_MODEMCONFIG3,
						 ( SX1276Read( REG_LR_MODEMCONFIG3 ) &
						   RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) |
						   (LowDatarateOptimize << 3 ) );

			SX1276Write( REG_LR_PREAMBLEMSB, ( preambleLen >> 8 ) & 0x00FF );
			SX1276Write( REG_LR_PREAMBLELSB, preambleLen & 0xFF );

			if( datarate == 6 )
			{
				SX1276Write( REG_LR_DETECTOPTIMIZE,
							 ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
							   RFLR_DETECTIONOPTIMIZE_MASK ) |
							   RFLR_DETECTIONOPTIMIZE_SF6 );
				SX1276Write( REG_LR_DETECTIONTHRESHOLD,
							 RFLR_DETECTIONTHRESH_SF6 );
			}
			else
			{
				SX1276Write( REG_LR_DETECTOPTIMIZE,
							 ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
							 RFLR_DETECTIONOPTIMIZE_MASK ) |
							 RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
				SX1276Write( REG_LR_DETECTIONTHRESHOLD,
							 RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
			}
			break;
	    }
}

void SX1276Send()
{
	uint32_t freq=868525000;
	uint8_t txPower =TxPowers[4];
	uint8_t datarate =Datarates[5];
	uint32_t fdev=0;
	uint32_t bandwidth=0;
	uint8_t coderate=1;
	uint16_t preambleLen=5;
	bool fixLen=false;
	bool crcOn=true;
	bool freqHopOn=0;
	uint8_t hopPeriod=0;
	bool iqInverted=false;
	uint32_t timeout=3e3;

	HAL_GPIO_WritePin(VCTL1_GPIO_Port,VCTL1_Pin,1);
	HAL_GPIO_WritePin(VCTL2_GPIO_Port,VCTL2_Pin,0);

	SX1276SetChannel(freq);
	SX1276SetMacPayloadLength(MODEM_LORA, SX1276.length);
	//SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
	SX1276SetTxConfig( MODEM_LORA, txPower, fdev, bandwidth, datarate, coderate, preambleLen, fixLen, crcOn, freqHopOn, hopPeriod, iqInverted, timeout );
	if( iqInverted == true )
	{
		SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_ON ) );
		SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
	}
	else
	{
		SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
		SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
	}

	// Initializes the payload size
	SX1276Write( REG_LR_PAYLOADLENGTH, SX1276.length );

	// Full buffer used for Tx
	SX1276Write( REG_LR_FIFOTXBASEADDR, 0 );
	SX1276Write( REG_LR_FIFOADDRPTR, 0 );
	volatile uint8_t tmp=SX1276Read(REG_OPMODE);
	// FIFO operations can not take place in Sleep mode
	if( ( SX1276Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
	{
		SX1276SetStby( );
		HAL_Delay( 10 );
	}
	// Write payload buffer
	SX1276WriteBuffer(0, SX1276.buffer, SX1276.length );
	//SX1276SetTx();

	if( freqHopOn == true )
	{
		SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
										  RFLR_IRQFLAGS_RXDONE |
										  RFLR_IRQFLAGS_PAYLOADCRCERROR |
										  RFLR_IRQFLAGS_VALIDHEADER |
										  //RFLR_IRQFLAGS_TXDONE |
										  RFLR_IRQFLAGS_CADDONE |
										  //RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
										  RFLR_IRQFLAGS_CADDETECTED );

		// DIO0=TxDone, DIO2=FhssChangeChannel
		SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK ) | RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO2_00 );
	}
	else
	{
		SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
										  RFLR_IRQFLAGS_RXDONE |
										  RFLR_IRQFLAGS_PAYLOADCRCERROR |
										  RFLR_IRQFLAGS_VALIDHEADER |
										  //RFLR_IRQFLAGS_TXDONE |
										  RFLR_IRQFLAGS_CADDONE |
										  RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
										  RFLR_IRQFLAGS_CADDETECTED );

		// DIO0=TxDone
		SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK ) | RFLR_DIOMAPPING1_DIO0_01 );
	}

	SX1276SetOpMode( RF_OPMODE_TRANSMITTER );

	//HAL_GPIO_WritePin(VCTL1_GPIO_Port,VCTL1_Pin,1);
	//HAL_GPIO_WritePin(VCTL2_GPIO_Port,VCTL2_Pin,0);
}
void SX1276SetStby()
{
	SX1276SetOpMode( RF_OPMODE_STANDBY );
}

void SX1276SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | RF_PACONFIG_PASELECT_PABOOST;//SX1276GetPaSelect( SX1276.Settings.Channel );
    paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK ) | 0x70;

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power < -1 )
        {
            power = -1;
        }
        if( power > 14 )
        {
            power = 14;
        }
        paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power + 1 ) & 0x0F );
    }
    SX1276Write( REG_PACONFIG, paConfig );
    SX1276Write( REG_PADAC, paDac );
}
