/*
 * HCI_SX1276.c
 *
 *  Created on: May 29, 2020
 *      Author: Le Truong Thanh
 *      Comment: Modified this file for hardware communicating between sensor and mcu
 *      First version for RAK4200
 */

#include "HCI_SX1276.h"

/*****************************************************************************************************************/
/*													GLOBAL VARIABLES											 */
/*****************************************************************************************************************/

/*****************************************************************************************************************/
/*													PRIVATE FUNCTION											 */
/*****************************************************************************************************************/
static void InitSPI(void);

static void InitGPIO(void);

static void GPIOWrite(GPIO_PORT_TypeDef Port,uint16_t Pin, uint8_t Value);

static void RunSPI(SPI_RUN_Mode_t);

static void SPI_COMMAND_IRQ(SPI_RUN_Mode_t mode);

static void SendData(uint8_t data);

static uint8_t GetData(void);

static void Delay(uint32_t timeout_ms);

static void InitVCTLPin(void);

static void InitVCTLPin(void);
/*****************************************************************************************************************/
/*											APPLICATION PROGRAMMING INTERFACE									 */
/*****************************************************************************************************************/
HCI_SX1276_t HCI_SX1276={
		.InitSPI				= &InitSPI,
		.InitGPIO				= &InitGPIO,
		.GPIOWrite				= &GPIOWrite,
		.RunSPI					= &RunSPI,
		.SPI_COMMAND_IRQ		= &SPI_COMMAND_IRQ,
		.SendData				= &SendData,
		.GetData				= &GetData,
		.Delay					= &Delay,
		.InitVCTLPin			= &InitVCTLPin,
		.DeInitVCTLPin			= &DeInitVCTLPin
};

/*****************************************************************************************************************/
/*													PRIVATE VARIABLES											 */
/*****************************************************************************************************************/
SPI_HandleTypeDef hspi1;

/*****************************************************************************************************************/
/*													FUNCTION IMPLEMENT									 	 	 */
/*****************************************************************************************************************/
void InitSPI(void)
{
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	if (HAL_SPI_Init(&hspi1) != HAL_OK)
	{
		Error_Handler();
	}

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* USER CODE BEGIN SPI1_MspInit 0 */

	/* USER CODE END SPI1_MspInit 0 */
	/* SPI1 clock enable */
	__HAL_RCC_SPI1_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**SPI1 GPIO Configuration
	PA5     ------> SPI1_SCK
	PA6     ------> SPI1_MISO
	PA7     ------> SPI1_MOSI
	*/
	GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void InitGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOC_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, NRESET_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, SPI1_NSS_Pin, GPIO_PIN_SET);

	  /*Configure GPIO pins : PAPin PAPin */
	  GPIO_InitStruct.Pin = NRESET_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : PAPin PAPin */
	  GPIO_InitStruct.Pin = SPI1_NSS_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : PB0 PB1 PB4 PB5 */
	  //GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
	  GPIO_InitStruct.Pin = DIO0|DIO1|DIO2|DIO3;
	  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  HAL_GPIO_Init(DIO_Port, &GPIO_InitStruct);

	  InitVCTLPin();

	  /* EXTI interrupt init*/
	  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

void InitVCTLPin(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, VCTL1_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, VCTL2_Pin, GPIO_PIN_SET);

	  /*Configure GPIO pins : PAPin PAPin */
	  GPIO_InitStruct.Pin = VCTL1_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : PAPin PAPin */
	  GPIO_InitStruct.Pin = VCTL2_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_PULLUP;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void DeInitVCTLPin(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, VCTL1_Pin, GPIO_PIN_RESET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, VCTL2_Pin, GPIO_PIN_SET);

	  /*Configure GPIO pins : PAPin PAPin */
	  GPIO_InitStruct.Pin = VCTL1_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : PAPin PAPin */
	  GPIO_InitStruct.Pin = VCTL2_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void GPIOWrite(GPIO_PORT_TypeDef Port,uint16_t Pin, uint8_t Value)
{
	HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port,SPI1_NSS_Pin,0);
}

void RunSPI(SPI_RUN_Mode_t mode)
{
	if (mode==SPI_MODE_ENABLE)
	{
		__HAL_SPI_ENABLE(&hspi1);
	} else
		__HAL_SPI_DISABLE(&hspi1);
}

void SPI_COMMAND_IRQ(SPI_RUN_Mode_t mode)
{
	if (mode==SPI_MODE_ENABLE)
		__enable_irq();
	else
		__disable_irq();
}

void SendData(uint8_t data)
{
	while( __HAL_SPI_GET_FLAG( &hspi1, SPI_FLAG_TXE ) == RESET );
	hspi1.Instance->DR = ( uint16_t ) ( data & 0xFF );
}

uint8_t GetData(void)
{
	uint8_t rxData;
	while( __HAL_SPI_GET_FLAG( &hspi1, SPI_FLAG_RXNE ) == RESET );
	rxData = ( uint8_t ) hspi1.Instance->DR;
	return rxData;
}

void Delay(uint32_t timeout_ms)
{
	HAL_Delay(timeout_ms);
}


