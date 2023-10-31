#ifndef __MB_INCLUDE_H
#define __MB_INCLUDE_H

// #include "main.h"
#ifndef USE_HAL_DRIVER
#define USE_HAL_DRIVER
#endif

#define CubeMX_Setting 1

#define COM_UART_MAX    2  // UART account in use
#define COM1    0
#define COM2    1
#define COM3    2
#define COM4    3

#include "mb_crc.h"
#include "mb_host.h"
#include "mb_port.h"
#include "mb_hook.h"

#endif

