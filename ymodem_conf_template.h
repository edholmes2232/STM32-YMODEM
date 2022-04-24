/**
 * @file   ymodem_conf_template.h
 * @author Ed Holmes (edholmes2232(at)gmail.com)
 * @brief  Template for configuring the YMODEM library.
 */

/**
 * @brief  Select STM32 Device family used. Options are:
 * 		   - DEVICE_FAMILY_STM32F0
 * 		   - DEVICE_FAMILY_STM32F1
 * 		   - DEVICE_FAMILY_STM32F2
 * 		   - DEVICE_FAMILY_STM32F3
 * 		   - DEVICE_FAMILY_STM32F4
 * 		   - DEVICE_FAMILY_STM32F7
 * 		   - DEVICE_FAMILY_STM32G0
 * 		   - DEVICE_FAMILY_STM32G4
 * 		   - DEVICE_FAMILY_STM32H7
 * 		   - DEVICE_FAMILY_STM32L0
 * 		   - DEVICE_FAMILY_STM32L1
 * 		   - DEVICE_FAMILY_STM32L4
 * 
 */
#define DEVICE_FAMILY_STM32F7

/**
 * @brief	Whether to check programming was successful
 * 
 */
//#define YMODEM_VALIDATE_PROGRAMMING	


/**
 * @brief	Size of flash to save file to  
 * 
 */
#define YMODEM_FLASH_SIZE				(256 * 1024) // 256kB

/**
 * @brief  Starting address of flash
 * 
 */
#define YMODEM_FLASH_START				0x08080000

/**
 * @brief  Flash sector number
 * 
 */
#define YMODEM_FLASH_FIRST_SECTOR_NUM			(6) 

/**
 * @brief  Num of flash sectors 
 * 
 */
#define YMODEM_FLASH_NUM_OF_SECTORS		(1)
