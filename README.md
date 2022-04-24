# STM32-YMODEM

STM32 HAL based YMODEM Library.

Currently only supports writing to a single flash sector.

Needs a valid `ymodem_conf.h` file. 

Tested with STM32F4, STM32F7

## How to Use
Use as a guide.
```C
#define SERIAL_UART	huart2

/**
 * @brief  Sends "C" to initialise YModem transfer until data is received or timeout
 * 
 * @param  output
 */
static void startFWTx(SERIAL_DEVICE_ID_T output) {
	uint8_t timeout = 100;
	/* Initialise all YMODEM vars */
	YMODEM_Init();
	/* Wait for data in queue */
	while (osMessageQueueGetCount(SERIAL_CMDInHandle) == 0) {
		HAL_UART_Transmit(&SERIAL_UART, "C", 1);
		if (--timeout == 0) {
			break;
		}
		osDelay(1000);
	}
}

void RTOSThread(void *arg) {
	uint8_t buff[100];
	uint8_t payload[5];
	uint8_t payloadLen;
	uint8_t pcOutputStr[100];
	uint8_t fwDownloading = 0;
	for(;;) {
		/* Data in. Can be from UART */
		osMessageQueueGet(SERIAL_CMDInHandle, &buff, 0, osWaitForever);
		if (fwDownloading) {	
			/* ABORT if "a" sent */
			if ((strcmp(buff, "a") == 0) && (strlen(buff) <= 3)) { 
				/* Send abort */

				YMODEM_Abort(payload, &payloadLen);
				HAL_UART_Transmit(&SERIAL_UART, payload, payloadLen);
			} else {
				for (uint8_t i = 0; i < strlen(buff); i++) {
					YMODEM_T ret = YMODEM_ReceiveByte(buff[i], payload, &payloadLen);
					switch (ret) {
						case YMODEM_OK:
							break; 
						case YMODEM_TX_PENDING:
							HAL_UART_Transmit(&SERIAL_UART, payload, payloadLen);
							break;
						case YMODEM_ABORTED:
							fwDownloading = 0;
							sprintf(pcOutputString, "Aborted\r\n");
							HAL_UART_Transmit(&SERIAL_UART, pcOutputString, strlen(pcOutputString));
							break;
						case YMODEM_WRITE_ERR:
							fwDownloading = 0;
							sprintf(pcOutputString, "Write Error\r\n");
							HAL_UART_Transmit(&SERIAL_UART, pcOutputString, strlen(pcOutputString));
							break;
						case YMODEM_SIZE_ERR:
							fwDownloading = 0;
							sprintf(pcOutputString, "File too big\r\n");
							HAL_UART_Transmit(&SERIAL_UART, pcOutputString, strlen(pcOutputString));
							break;
						case YMODEM_COMPLETE:
							fwDownloading = 0;
							sprintf(pcOutputString, "Download Complete\r\n");
							HAL_UART_Transmit(&SERIAL_UART, pcOutputString, strlen(pcOutputString));
							break;
					}
					// If transfer stopped, dont process more data
					if (ret > YMODEM_TX_PENDING) break;
				}
			}
		}
	}
}

```

# Programming
## Via serial:
```bash
$ picocom -b 115200 /dev/ttyS0 --send-cmd "sb -vv"
```

## Via telnet
```bash
$ socat pty,link=/dev/ttyS0,raw tcp:192.168.1.150:23
$ picocom -b 115200 /dev/ttyS0 --send-cmd "sb -vv"
```



Todo:

* Remove STM32 HAL dependency. Split erase/write functions to custom IO file.