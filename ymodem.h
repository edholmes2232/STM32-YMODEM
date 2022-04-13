/**
 * @file   ymodem.h
 * @author Ed Holmes (eholmes@lablogic.com)
 * @brief  YMODEM software function headers
 * @date   2022-03-09
 */
#ifndef YMODEM_H
#define YMODEM_H

#include "stdint.h"
#include "stddef.h"

typedef enum {
	YMODEM_OK = 0,			/* All OK, send next byte */
	YMODEM_TX_PENDING,		/* Data waiting to be transmitted */
	YMODEM_ABORTED,			/* Transfer aborted */
	YMODEM_WRITE_ERR,		/* Error writing to flash */
	YMODEM_SIZE_ERR,		/* File is bigger than flash */
	YMODEM_COMPLETE,		/* Transfer completed succesfully */
} YMODEM_T;


/* Size of flash to save to */
#define FLASH_SIZE				(256 * 1024)	// 256 kB
/* Start flash addresss */
#define FLASH_START				0x08080000
/* Flash sector num */
#define FLASH_SECTOR_NUM		6 


void 		YMODEM_Init(void);
YMODEM_T 	YMODEM_ReceiveByte(uint8_t c, uint8_t *respBuff, uint8_t *respLen);
YMODEM_T 	YMODEM_Abort(uint8_t *respBuff, uint8_t *len);

#endif // YMODEM_H
