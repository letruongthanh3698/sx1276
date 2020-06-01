/*
 * HCI_SX1276.h
 *
 *  Created on: May 29, 2020
 *      Author: Le Truong Thanh
 *      Comment: Modified this file for hardware communicating between sensor and mcu
 *      First version for RAK4200
 */
#ifndef SRC_HCI_SX1276_H_
#define SRC_HCI_SX1276_H_

#include "main.h"
#include <stdbool.h>
/*****************************************************************************************************************/
/*													MACROS DEFINITION											 */
/*****************************************************************************************************************/
#define GPIO_PORT_TypeDef 		GPIO_TypeDef
#define DIO0					GPIO_PIN_0
#define DIO1					GPIO_PIN_1
#define DIO2					GPIO_PIN_5
#define DIO3					GPIO_PIN_4
#define DIO_Port				GPIOB


#define NRESET_Pin 			GPIO_PIN_0
#define NRESET_GPIO_Port 	GPIOA
#define SPI1_NSS_Pin 		GPIO_PIN_4
#define SPI1_NSS_GPIO_Port 	GPIOA
#define VCTL2_Pin 			GPIO_PIN_8
#define VCTL2_GPIO_Port 	GPIOA
#define VCTL1_Pin 			GPIO_PIN_11
#define VCTL1_GPIO_Port 	GPIOA
/*****************************************************************************************************************/
/*													   DATA-STRUCTS												 */
/*****************************************************************************************************************/
typedef enum{
	SPI_MODE_DISABLE=0,
	SPI_MODE_ENABLE
}SPI_RUN_Mode_t;
/*****************************************************************************************************************/
/*											    Software Command Interface     									 */
/*****************************************************************************************************************/
typedef struct{
	const void (*InitSPI)(void);

	const void (*InitGPIO)(void);

	const void (*GPIOWrite)(GPIO_PORT_TypeDef *Port,uint16_t Pin, uint8_t Value);

	const void (*RunSPI)(SPI_RUN_Mode_t mode);

	const void (*SPI_COMMAND_IRQ)(SPI_RUN_Mode_t mode);

	const uint8_t (*SendGetData)(uint8_t data);

	const uint8_t (*GetData)(void);

	const void (*Delay)(uint32_t timeout_ms);

	const void (*InitVCTLPin)(void);

	const void (*DeInitVCTLPin)(void);
}HCI_SX1276_t;

#endif
