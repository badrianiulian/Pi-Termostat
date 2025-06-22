/* ======================================================================
    SH1106-Buttons-DS18B20 driver header
====================================================================== */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include "bcm2835.h"
#include "Font.h"
#include <iostream>

/* ======================================================================
Define SPI pins
====================================================================== */
#define OLED_SPI_RESET RPI_V2_GPIO_P1_22 /* RPI_V2_GPIO_P1_22 = GPIO 25, Pin P1-22  */
#define OLED_SPI_DC    RPI_V2_GPIO_P1_18 /* RPI_V2_GPIO_P1_18 = GPIO 24, Pin P1-18  */
/*                        By default CE0 is RPI_V2_GPIO_P1_24 = GPIO  8, Pin P1-24  */
#define RELAY_PIN      RPI_V2_GPIO_P1_11 /* RPI_V2_GPIO_P1_11 = GPIO 17, Pin P1-11  */

// Arduino Compatible type
typedef uint8_t boolean;
typedef uint8_t byte;

// GCC Missing
//#define max(a,b) (a>b?a:b)
//#define min(a,b) (a<b?a:b)

#ifndef _SH1106_DS18B20_KEYS_H
#define _SH1106_DS18B20_KEYS_H

/* ======================================================================
    SH1106 Commands (Manual)
====================================================================== */
//  1 : Set lower column address (values from 0x00 to 0x0F)
#define SH1106_Set_Lower_Column_Start_Address 0x02
//  2 : Set higher column address (values from 0x10 to 0x1F)
#define SH1106_Set_Higher_Column_Start_Address 0x10
//  3 : Set pump voltage value (values from 0x30 to 0x33)
#define SH1106_Set_Pump_Voltage 0x30
//  4 : Set display start line (values from 0x40 to 0x7F)
#define SH1106_Set_Display_Start_Line 0x40
//  5 : Set contrast control register - double bytes: 0x00 - 0xFF
#define SH1106_Set_Contrast_Level 0x81
#define SH1106_Contrast_Standard 0x80
//  6 : Set segment re-map (0xA0/0xA1 - Horizontal flip)
#define SH1106_Set_Segment_Remap 0xA0
//  7 : Set entire display Off / On
#define SH1106_Entire_Display_Off 0xA4
#define SH1106_Entire_Display_On 0xA5
//  8 : Set Normal/Invert Display
#define SH1106_Normal_Display 0xA6
#define SH1106_Invert_Display 0xA7
//  9 : Set multiplex ratio - double bytes: 0x00 - 0x3F
#define SH1106_Set_Muliplex_Ratio 0xA8
// 10 : DC-DC mode - double bytes: 0x8A - 0x8B
#define SH1106_Set_DC_DC_Control_Mode 0xAD
#define SH1106_DC_DC_Disable 0x8A // external DC-DC must be used
#define SH1106_DC_DC_Enable 0x8B // built-in DC-DC is used
// 11 : Display Off/On
#define SH1106_Display_Off 0xAE
#define SH1106_Display_On 0xAF
// 12 : Set Page Address (values from 0xB0 to 0xB7)
#define SH1106_Set_Page_Address 0xB0
// 13 : Set common output scan directon - vertical flip
#define SH1106_Set_Com_Output_Scan_Direction_Normal 0xC0
#define SH1106_Set_Com_Output_Scan_Direction_Remap 0xC8
// 14 : Set display offset - double bytes: 0x00 - 0x3F
#define SH1106_Set_Display_Offset 0xD3
// 15 : Set display clock divide ratio/oscilator frequency - double bytes: 0x00 - 0xFF
#define SH1106_Set_Display_DCLK_FRQ 0xD5
// 16 : Set dis-charge/pre-charge period - double bytes: 0x00 - 0xFF
#define SH1106_Set_Precharge_Period 0xD9
// 17 : Set common pads hardware configuration - double bytes: 0x02 - 0x12
#define SH1106_Set_Com_Pins 0xDA
// 18 : Set VCOM deselect level - double bytes: 0x00 - 0xFF
#define SH1106_Set_Vcomh_Deselect_Level 0xDB
// 19 : Read-Modify-Write Start
#define SH1106_Read_Modif_Write_Begin 0xE0
// 20 : Read-Modify-Write End
#define SH1106_Read_Modif_Write_End 0xEE
// 21 : NOP
#define SH1106_NOP_Command 0xE3

/* ======================================================================
Define buttons
====================================================================== */
#define BTN_ON_OFF RPI_V2_GPIO_P1_29 // RPI_V2_GPIO_P1_29 = GPIO  5, Pin P1-29, ON/OFF
#define BTN_LEFT   RPI_V2_GPIO_P1_31 // RPI_V2_GPIO_P1_31 = GPIO  6, Pin P1-31, LEFT
#define BTN_RIGHT  RPI_V2_GPIO_P1_32 // RPI_V2_GPIO_P1_32 = GPIO 12, Pin P1-32, RIGHT
#define BTN_UP     RPI_V2_GPIO_P1_33 // RPI_V2_GPIO_P1_33 = GPIO 13, Pin P1-33, UP
#define BTN_DOWN   RPI_V2_GPIO_P1_36 // RPI_V2_GPIO_P1_36 = GPIO 16, Pin P1-36, DOWN
#define BTN_MENU   RPI_V2_GPIO_P1_37 // RPI_V2_GPIO_P1_37 = GPIO 26, Pin P1-37, MENU

/* ======================================================================
Define one wire pin (with external pullup resistor)
====================================================================== */
#define ONE_WIRE   RPI_V2_GPIO_P1_07 // RPI_V2_GPIO_P1_07 = GPIO  4, Pin P1-07

/* ======================================================================
    DS18B20 Commands (Manual)
====================================================================== */
// DS18B20 ROM Commands
#define DS18B20_SEARCH_ROM 0XF0
#define DS18B20_READ_ROM 0x33
#define DS18B20_MATCH_ROM 0x55
#define DS18B20_SKIP_ROM 0xCC
#define DS18B20_ALARM_SEARCH 0xEC
// DS18B20 FUNCTION Commands
#define DS18B20_CONVERT_T 0x44
#define DS18B20_WRITE_SCRATCHPAD 0x4E
#define DS18B20_READ_SCRATCHPAD 0xBE
#define DS18B20_COPY_SCRATCHPAD 0x48
#define DS18B20_RECALL_E2 0xB8
#define DS18B20_READ_POWER_SUPPLY 0xB4


/* ======================================================================
    Button status
====================================================================== */
/*! PUSH_UP means pin is HIGH, true, 3.3volts on a pin. */
#define PUSH_UP 0x1
/*! PUSH_DOWN means pin is LOW, false, 0volts on a pin. */
#define PUSH_DOWN  0x0

/* ======================================================================
    Main class header
====================================================================== */
class SH1106_DS18B20_KEYS
{
 public:
  SH1106_DS18B20_KEYS();

  boolean init_buff(void);

  // SPI Init
  boolean module_init(void);
  void close(void);

  // SH1106 OLED functions
  void start(void);
  void SH1106_setContrast(uint8_t Contrast);
  void SH1106_invertDisplay(uint8_t i);

  void SH1106_sendCommand(uint8_t c);
  void SH1106_sendCommand(uint8_t c0, uint8_t c1);
  void SH1106_sendCommand(uint8_t c0, uint8_t c1, uint8_t c2);
  void SH1106_sendData(uint8_t c);

  void SH1106_display(void);
  void SH1106_clearDisplay(void);
  void SH1106_printchar6(uint8_t row, const char * text, boolean = 1, boolean = 0); // centered = 1, inverted = 0
  void SH1106_printchar6(uint8_t row, uint8_t start, const char * text, uint8_t = 6, boolean = 0, boolean = 0); // inverted = 0, rotation = 0
  void SH1106_printspecial(uint8_t row, uint8_t offset, const char * text);
  void SH1106_printcustom(uint8_t row, uint8_t start, const char * buffer, int len_buffer, boolean = 0); // inverted = 0

  // KEYS functions
  int KEYS_check(void);
  /* returns:
      0 = none
      1 = left
      2 = right
      3 = up
      4 = down
      5 = menu
      6 = on/off
  */

  // Temperature data
  double temperature = 00.000;
  // Function to read the above temperature variable
  void Read_Temperature(void);

  // Relay status
  void Relay_ON(void);
  void Relay_OFF(void);
  // Retain relay status in a boolean variable
  boolean relay_status;

 private:
  uint8_t *poledbuff; // Pointer to OLED data buffer in memory
  int8_t dc, rst, cs, relay;
  size_t oled_width, oled_height;
  size_t oled_buff_size, ds18b20_size;
  unsigned long long *ptempbuff; // Pointer to DS18B20 ID buffer in memory
  unsigned char ScratchPad[9];
  unsigned short wait_time;

  void fastSPIwrite(uint8_t c);
  void fastSPIwrite(char* tbuf, uint32_t len);

  int len_utf8(const char * text);
  int sp_count_utf8(const char * text);
  int find_space_21_utf8(const char * text);

  // Text rotation related
  std::string old_text;
  uint8_t *old_buff;
  size_t old_buff_size;

  // Buttons status
  uint8_t s_on_off, s_left, s_right, s_up, s_down, s_menu, multi_buttons;

  // DS18B20 functions
  void DelayMicrosecondsNoSleep(int delay_us);
  // Default resolution for sensor
  void DS18B20_WriteByte(unsigned char value);
  void DS18B20_WriteBit(unsigned char value);
  unsigned char DS18B20_ReadBit(void);
  unsigned char DS18B20_ReadByte(void);
  int DS18B20_Reset(void);
  void DS18B20_ReadScratchPad(void);
  unsigned char DS18B20_CalcCRC(unsigned char * data, unsigned char byteSize);
  char DS18B20_IDGetBit(unsigned long long *llvalue, char bit);
  unsigned long long DS18B20_IDSetBit(unsigned long long *llvalue, char bit, unsigned char newValue);
  void DS18B20_SelectSensor(unsigned long long ID);
  int DS18B20_SearchSensor(unsigned long long * ID, int * LastBitChange);
  double DS18B20_ReadSensor(void);
  int DS18B20_GlobalStartConversion(void);
  void DS18B20_WriteScratchPad(unsigned char TH, unsigned char TL, unsigned char config);
  void DS18B20_CopyScratchPad(void);
  // Standard Conversion is set to 12-bit resolution
  void DS18B20_ChangeSensorsResolution(int = 12);
  void DS18B20_ScanForSensor(void);
  void set_max_priority(void);
  void set_default_priority(void);

};
#endif
