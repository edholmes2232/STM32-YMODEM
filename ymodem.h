/**
 * @file   ymodem.h
 * @author Ed Holmes (edholmes2232(at)gmail.com)
 * @brief  YMODEM software function headers
 */
#ifndef YMODEM_H_
#define YMODEM_H_

#include "stdint.h"
#include "stddef.h"

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
