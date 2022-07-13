/**
 * @file   ymodem.c
 * @author Ed Holmes (edholmes2232(at)gmail.com)
 * @brief  Functions for YMODEM protocol
 *         Reference used: http://textfiles.com/programming/ymodem.txt
 *                         https://programmer.ink/think/ymodem-protocol-learning.html
 * 		   To program from unix: use picocom with --send-cmd "sb -vv"
 *         To program from Windows: Use TeraTerm
 */
#include "ymodem_conf.h"
#include "ymodem.h"
#include "stdlib.h"
#include "string.h"

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



#define YM_FILE_NAME_LENGTH			(256)
#define YM_FILE_SIZE_LENGTH			(16)

/** Regular packet size **/
#define YM_PACKET_SIZE				(128)
/** Data packet size **/
#define YM_PACKET_1K_SIZE			(1024)

#define YM_PACKET_SEQNO_INDEX     	(1)
#define YM_PACKET_SEQNO_COMP_INDEX 	(2)

#define YM_PACKET_HEADER           	(3)
#define YM_PACKET_TRAILER          	(2)
#define YM_PACKET_OVERHEAD         	(YM_PACKET_HEADER + YM_PACKET_TRAILER)

/**
 * @brief  YMODEM Control Characters
 * 
 */
enum YM_CC {
	SOH			= 0x01,	 /* start of 128-byte data packet */
	STX			= 0x02,  /* start of 1024-byte data packet */
	EOT			= 0x04,  /* end of transmission */
	ACK			= 0x06,  /* acknowledge */
	NAK			= 0x15,  /* negative acknowledge */
	CA			= 0x18,  /* two of these in succession aborts transfer */
	CRC16		= 0x43,  /* 'C' == 0x43, request 16-bit CRC */
	ABORT1		= 0x41,  /* 'A' == 0x41, abort by user */
	ABORT2		= 0x61,  /* 'a' == 0x61, abort by user */
};


/** Polynomial for CRC calculation **/
#define YM_CRC_POLY		(0x1021)

#define SWAP16(x)		(x >> 8) | ((x & 0xff) << 8)
#define ISVALIDDEC(c) 	((c >= '0') && (c <= '9'))
#define CONVERTDEC(c)	(c - '0')

/**
 * @brief  Internal return values
 * 
 */
typedef enum {
	YM_OK = 0,		/* OK, RETURN NOTHING */
	YM_ABORTED,		/* 2x CA Received, graceful abort, return ACK */
	YM_ABORT,		/* Initiate graceful abort, return 2x CA */
	YM_WRITE_ERR,	/* Error writing to flash */
	YM_SIZE_ERR,	/* File too big */
	YM_START_RX,	/* First frame ok, start receive, return ACK, CRC */
	YM_RX_ERROR,	/* Data receive error, return NAK */
	YM_RX_OK,		/* Data receive ok, return ACK */
	YM_RX_COMPLETE,	/* Data receive complete, return ACK */
	YM_SUCCESS,		/* Transfer complete, close */
} YM_RET_T;




static YM_RET_T YMODEM_ProcessPacket(void);
static YM_RET_T YMODEM_ProcessFirstPacket(void);
static YM_RET_T YMODEM_ProcessDataPacket(void);
static YM_RET_T YMODEM_CheckCRC(void);

static uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum);
static uint8_t packet_data[YM_PACKET_1K_SIZE + YM_PACKET_OVERHEAD];



static uint8_t 	fileName[YM_FILE_NAME_LENGTH];		/** Incoming file filename **/
static uint8_t 	fileSizeStr[YM_FILE_SIZE_LENGTH];	/** Incoming file size string **/
static uint32_t fileSize;						/** File size as int **/
static uint8_t 	prevC;							/** Previous byte character inputted **/
static uint8_t 	startOfPacket; 					/** Whether data is start of a packet **/
static uint8_t 	eotReceived; 					/** Expect one more packet after this to signal end **/
static uint16_t packetBytes; 					/** # of Bytes received of current packet **/
static uint16_t packetSize;						/** Size of current packet **/
static int32_t 	packetsReceived;				/** Num packets received **/
static uint32_t flashAddr; 						/** Flash memory address to write packet to **/
static YMODEM_T nextStatus; 					/** Status to return after closing a connection **/


/**
 * @brief  Initialise YMODEM Rx State 
 * 
 */
void YMODEM_Init(void) {
	memset(fileName, 	0, YM_FILE_NAME_LENGTH);
	memset(fileSizeStr, 0, YM_FILE_SIZE_LENGTH);
	fileSize 		= 0;
	prevC 			= 0;
	startOfPacket 	= 1;
	packetBytes 	= 0; 
	packetSize 		= 0;
	packetsReceived	= 0;
	eotReceived 	= 0;
	flashAddr 		= YMODEM_FLASH_START;
	nextStatus 		= YMODEM_OK;
}


/**
 * @brief  				Generates a payload to return to the YMODEM sender. 
 * 
 * @param  retVal		The internal YM_RET_T to translate to YMODEM_T 
 * @param  respBuff		Buffer to write to the response to
 * @param  len			Length of data written
 * @return YMODEM_T 	
 */
static YMODEM_T GenerateResponse(YM_RET_T retVal, uint8_t *respBuff, uint8_t *len) {
	switch (retVal) {
		case YM_OK:
			len = 0;
			return YMODEM_OK;
			break;
		case YM_ABORT:
			YMODEM_Abort(respBuff, len);
			return YMODEM_TX_PENDING;
			break;
		case YM_ABORTED:
			respBuff[0] = CRC16;
			*len = 1;
			nextStatus = YMODEM_ABORTED;
			return YMODEM_TX_PENDING;
			break;
		case YM_WRITE_ERR:
			YMODEM_Abort(respBuff, len);
			nextStatus = YMODEM_WRITE_ERR;
			return YMODEM_TX_PENDING;
		case YM_SIZE_ERR:
			YMODEM_Abort(respBuff, len);
			nextStatus = YMODEM_SIZE_ERR;
			return YMODEM_TX_PENDING;
		case YM_START_RX:
			respBuff[0] = ACK;
			respBuff[1] = CRC16;
			*len = 2;
			return YMODEM_TX_PENDING;
			break;
		case YM_RX_ERROR:
			respBuff[0] = NAK;
			*len = 1;
			return YMODEM_TX_PENDING;
			break;
		case YM_RX_OK:
			respBuff[0] = ACK;
			*len = 1;
			return YMODEM_TX_PENDING;
			break;
		case YM_RX_COMPLETE:
			respBuff[0] = ACK;
			respBuff[1] = CRC16;
			*len = 2;
			return YMODEM_TX_PENDING;
			break;
		case YM_SUCCESS:
			respBuff[0] = ACK;
			*len = 1;
			nextStatus = YMODEM_COMPLETE;
			return YMODEM_TX_PENDING;
			break;
		default: 
			break;
	}
	return YMODEM_ABORTED;
}

/**
 * @brief  				Aborts a YMODEM transfer. Writing the aborting commands payload to a provided buffer.
 * 						This buffer needs to be sent to the YMODEM sender.
 * 
 * @param  respBuff		Buffer to write payload to.
 * @param  len			Length of the payload.
 * @return YMODEM_T 		
 */
YMODEM_T YMODEM_Abort(uint8_t *respBuff, uint8_t *len) {
	respBuff[0] = CA;
	respBuff[1] = CA;
	*len = 2;
	nextStatus = YMODEM_ABORTED;
	return YMODEM_ABORTED;
}

/**
 * @brief  				Receives packets from a YMODEM Sender as a byte.
 * 						Bytes should be passed continuously while YMODEM_OK returned.
 * 
 * @param  c			A byte from the YMODEM Sender
 * @param  respBuff		Buffer to write the data to be sent back to the sender
 * @param  respLen		Length of the data to send to the sender.
 * @return YMODEM_T 	Return value indicating status after each byte.
 */
YMODEM_T YMODEM_ReceiveByte(uint8_t c, uint8_t *respBuff, uint8_t *respLen) { 
	YM_RET_T ret = YM_OK;

	/* Return status if just closed connection */
	if (nextStatus != YMODEM_OK) return nextStatus;

	do {	
		/* Receive full packet */
		if (startOfPacket) {
			/* Process start of packet */
			switch (c) {
				case SOH:
					packetSize = YM_PACKET_SIZE;
					/* start receiving payload */
					startOfPacket = 0;
					packetBytes++; //increment by 1 byte
					ret = YM_OK;
					break; 
				case STX:
					packetSize = YM_PACKET_1K_SIZE;
					/* start receiving payload */
					startOfPacket = 0;
					packetBytes++; //increment by 1 byte
					ret = YM_OK;
					break;
				case EOT: 
				/* One more packet comes after with 0,FF so reset this */
					eotReceived = 1;
					ret = YM_RX_COMPLETE;
					break;
				case CA:
					/* Two of these aborts transfer */
					if (prevC == CA) { 
						ret = YM_ABORTED;
						break;
					}
					ret = YM_OK;
					break;
				case ABORT1:
				case ABORT2: 
					ret = YM_ABORT;
					break;
				default: 
					ret = YM_RX_ERROR;
					break;
			}
		} else {
			/* receive rest of packet */
			if (packetBytes < (packetSize + YM_PACKET_OVERHEAD)-1) {
				packet_data[packetBytes++] = c; 
				ret = YM_OK;
				break;
			} else {
				/* Last byte of packet */
				packet_data[packetBytes++] = c; 
				if (packet_data[YM_PACKET_SEQNO_INDEX] != ((packet_data[YM_PACKET_SEQNO_COMP_INDEX] ^ 0xFF) & 0xFF)) {
					/* Check byte 1 == (byte 2 XOR 0xFF) */
					ret = YM_RX_ERROR;
					break;
				} else {
					/* Full packet received */
					ret = YMODEM_ProcessPacket();
					startOfPacket = 1;
					packetBytes = 0;
					break;
				}
			}
		}
	
	} while (0); // Empty do while to avoid multiple "return" statements
	prevC = c;
	return GenerateResponse(ret, respBuff, respLen);
}

static YM_RET_T YMODEM_ProcessPacket(void) {
	YM_RET_T ret = YM_OK;
	do {
		if (eotReceived == 1) {
			ret = YM_SUCCESS;
			break;
		/* Check byte 1 == num of bytes received */
		} else if ((packet_data[YM_PACKET_SEQNO_INDEX] & 0xFF) != (packetsReceived & 0xFF)) {
			/* Send a NAK */
			ret = YM_RX_ERROR;
			break;
		} else if (YMODEM_CheckCRC() != YM_OK) {
			ret = YM_RX_ERROR;
			break;
		} else {
			if (packetsReceived == 0) {
				ret = YMODEM_ProcessFirstPacket();
				break;
			} else {
				ret = YMODEM_ProcessDataPacket();
				break;
			}
		}
	} while(0);
	return ret;
}

/**
 * @brief  				Writes a data packet to the flash set in ymodem_conf.h
 * 						Optionally validates the writes.
 * 
 * @return YM_RET_T 	YM_WRITE_ERR if write fails, otherwise YM_RX_OK
 */
static YM_RET_T YMODEM_ProcessDataPacket(void) {
	YM_RET_T ret;
	do { 
		if (HAL_FLASH_Unlock() != HAL_OK) {
			ret = YM_WRITE_ERR;
			break;
		}

		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR );// FLASH_FLAG_PGSERR);

		const uint8_t *buffIn = (const uint8_t *)packet_data + YM_PACKET_HEADER;

		for (uint32_t i = 0; (i < packetSize) && (flashAddr <= YMODEM_FLASH_START + YMODEM_FLASH_SIZE); i++) {
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, flashAddr, buffIn[i]) != HAL_OK) {
				ret = YM_WRITE_ERR;
				break;
			}

			flashAddr++;
		}
							
		HAL_FLASH_Lock();
		
		#ifdef YMODEM_VALIDATE_PROGRAMMING
		if(memcmp((void *)(flashAddr-packetSize), (void *)&packet_data[YM_PACKET_HEADER], packetSize) != 0) {
			ret = YM_WRITE_ERR;
			break;
		}
		#endif

		ret = YM_RX_OK;
		packetsReceived++;

	} while(0);
	return ret;
}


/**
 * @brief  			Gets data from the first YMODEM packet.
 * 
 * @return YM_RET_T 
 */
static YM_RET_T YMODEM_ProcessFirstPacket(void) {
	YM_RET_T ret;
	int32_t i; 
	uint8_t *filePtr; 
	do {
		/* Filename packet */
		if (packet_data[YM_PACKET_HEADER] != 0) {
			/* Packet has valid data */
			/* Get File Name */
			for (i = 0, filePtr = packet_data + YM_PACKET_HEADER; (*filePtr != 0) && (i < YM_FILE_NAME_LENGTH); ) {
				fileName[i++] = *filePtr++;
			}
			fileName[i++] = '\0';

			for (i = 0, filePtr++; (*filePtr != ' ') && (i < YM_FILE_SIZE_LENGTH); ) {
				fileSizeStr[i++] = *filePtr++;
			}
			fileSizeStr[i++] = '\0';
			Str2Int(fileSizeStr, &fileSize);

			/* Test size of image < Flash size */
			if (fileSize > (YMODEM_FLASH_SIZE - 1)) {
				/* End session */
				ret = YM_SIZE_ERR;
				break;
			} else {
				/* Erase Flash Sector */
				FLASH_EraseInitTypeDef EraseInitStruct;
				uint32_t sectorError;

				EraseInitStruct.TypeErase		= FLASH_TYPEERASE_SECTORS;
				EraseInitStruct.Sector			= YMODEM_FLASH_FIRST_SECTOR_NUM;
				EraseInitStruct.NbSectors		= YMODEM_FLASH_NUM_OF_SECTORS;
				EraseInitStruct.VoltageRange	= FLASH_VOLTAGE_RANGE_3;
				
				HAL_FLASH_Unlock();
				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR );// FLASH_FLAG_PGSERR);
				if(HAL_FLASHEx_Erase(&EraseInitStruct, &sectorError) != HAL_OK) {
					HAL_FLASH_Lock();
					ret = YM_WRITE_ERR;
					break;
				}
				HAL_FLASH_Lock();
			}


			/* Send ACK AND CRC, READY FOR DATA */
			ret = YM_START_RX;
			packetsReceived++;
			break;

		} else {
			/* Filename packet is empty, end session */ 
			ret = YM_ABORT;
			break;
		}

	} while (0);
	return ret;
}


static uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum) {
	uint32_t i = 0, res = 0;
	uint32_t val = 0;

	/* max 10-digit decimal input */
	for (i = 0; i < 11; i++) {
		if (inputstr[i] == '\0') {
			*intnum = val;
			/* return 1 */
			res = 1;
			break;
		} else if (ISVALIDDEC(inputstr[i])) {
			val = val * 10 + CONVERTDEC(inputstr[i]);
		} else {
			/* return 0, Invalid input */
			res = 0;
			break;
		}
	}
	/* Over 10 digit decimal --invalid */
	if (i >= 11) {
		res = 0;
	}
	return res;
}

static uint16_t crc_update(uint16_t crc_in, int incr)
{
        uint16_t xor = crc_in >> 15;
        uint16_t out = crc_in << 1;

        if (incr)
                out++;

        if (xor)
                out ^= YM_CRC_POLY;

        return out;
}


static uint16_t crc16(const uint8_t *data, uint16_t size)
{
        uint16_t crc, i;

        for (crc = 0; size > 0; size--, data++)
                for (i = 0x80; i; i >>= 1)
                        crc = crc_update(crc, *data & i);

        for (i = 0; i < 16; i++)
                crc = crc_update(crc, 0);

        return crc;
}

static YM_RET_T YMODEM_CheckCRC(void) {
	uint16_t sourceCRC = 0;
	sourceCRC = packet_data[(packetSize+YM_PACKET_OVERHEAD) - 1];
	sourceCRC = (sourceCRC << 8) | packet_data[(packetSize+YM_PACKET_OVERHEAD) - 2];

	uint16_t newCRC = SWAP16(crc16(packet_data+YM_PACKET_HEADER, packetSize));
	if (newCRC != sourceCRC) {
		return YM_RX_ERROR;
	} else {
		return YM_OK;
	}
}