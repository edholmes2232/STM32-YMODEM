# STM32-YMODEM

STM32 HAL based YMODEM Library.

Currently only supports writing to a single flash sector.

Needs a valid `ymodem_conf.h` file. 

Tested with STM32F4, STM32F7

Todo:

* Remove STM32 HAL dependency. Split erase/write functions to custom IO file.