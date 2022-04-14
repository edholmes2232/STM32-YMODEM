/**
 * @file   ymodem.h
 * @author Ed Holmes (edholmes2232(at)gmail.com)
 * @brief  YMODEM software function headers
 */
#ifndef YMODEM_H_
#define YMODEM_H_

#include "ymodem_conf.h"
#include "stdint.h"
#include "stddef.h"

#ifdef DEVICE_FAMILY_STM32F0
	#include "stm32f0xx_hal.h"
	#include "stm32f0xx_hal_def.h"
	#include "stm32f0xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32F1
	#include "stm32f1xx_hal.h"
	#include "stm32f1xx_hal_def.h"
	#include "stm32f1xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32F2
	#include "stm32f2xx_hal.h"
	#include "stm32f2xx_hal_def.h"
	#include "stm32f2xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32F3
	#include "stm32f3xx_hal.h"
	#include "stm32f3xx_hal_def.h"
	#include "stm32f3xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32F4
	#include "stm32f4xx_hal.h"
	#include "stm32f4xx_hal_def.h"
	#include "stm32f4xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32F7
	#include "stm32f7xx_hal.h"
	#include "stm32f7xx_hal_def.h"
	#include "stm32f7xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32G0
	#include "stm32g0xx_hal.h"
	#include "stm32g0xx_hal_def.h"
	#include "stm32g0xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32G4
	#include "stm32g4xx_hal.h"
	#include "stm32g4xx_hal_def.h"
	#include "stm32g4xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32H7
	#include "stm32h7xx_hal.h"
	#include "stm32h7xx_hal_def.h"
	#include "stm32h7xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32L0
	#include "stm32l0xx_hal.h"
	#include "stm32l0xx_hal_def.h"
	#include "stm32l0xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32L1
	#include "stm32l1xx_hal.h"
	#include "stm32l1xx_hal_def.h"
	#include "stm32l1xx_hal_flash.h"
#endif
#ifdef DEVICE_FAMILY_STM32L4
	#include "stm32l4xx_hal.h"
	#include "stm32l4xx_hal_def.h"
	#include "stm32l4xx_hal_flash.h"
#endif

/**
 * @brief  Return values for YMODEM functions
 * 
 */
typedef enum {
	YMODEM_OK = 0,			/* All OK, send next byte */
	YMODEM_TX_PENDING,		/* Data waiting to be transmitted */
	YMODEM_ABORTED,			/* Transfer aborted */
	YMODEM_WRITE_ERR,		/* Error writing to flash */
	YMODEM_SIZE_ERR,		/* File is bigger than flash */
	YMODEM_COMPLETE,		/* Transfer completed succesfully */
} YMODEM_T;


void 		YMODEM_Init(void);
YMODEM_T 	YMODEM_ReceiveByte(uint8_t c, uint8_t *respBuff, uint8_t *respLen);
YMODEM_T 	YMODEM_Abort(uint8_t *respBuff, uint8_t *len);

#endif // YMODEM_H
