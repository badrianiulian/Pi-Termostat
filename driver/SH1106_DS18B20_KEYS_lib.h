/******************************************************************
  SH1106_DS18B20_KEYS private lib
 ******************************************************************/

#ifndef _SH1106_DS18B20_KEYS_lib_H
#define _SH1106_DS18B20_KEYS_lib_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include "bcm2835.h"
#include "Font.h"

// Configuration Pin for ArduiPi board
#define OLED_SPI_RESET RPI_V2_GPIO_P1_22 /* RPI_V2_GPIO_P1_22 = GPIO 25, Pin P1-22  */
#define OLED_SPI_DC    RPI_V2_GPIO_P1_18 /* RPI_V2_GPIO_P1_18 = GPIO 24, Pin P1-18  */
/*                        By default CE0 is RPI_V2_GPIO_P1_24 = GPIO  8, Pin P1-24  */

// Arduino Compatible type
typedef uint8_t boolean;
typedef uint8_t byte;

#endif
