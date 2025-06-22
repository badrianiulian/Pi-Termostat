/*********************************************************************
[ SH1106 DS18B20 KEYS ] Control Module service
*********************************************************************/

#include "SH1106_DS18B20_KEYS_lib.h"
#include "SH1106_DS18B20_KEYS.h"
#include <mysql.h>
#include <math.h>
#include <thread>
#include <ctime>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <bitset>

/* Special class : program = (id,text) */
class program {
 public:
  int id = 0;
  std::string text = "";
};

#define PRG_NAME        "shdskeys_srv"
#define PRG_VERSION     "0.0.1"

/* ======================================================================
Instantiate the module
====================================================================== */
SH1106_DS18B20_KEYS module;

/* ======================================================================
Variables section
====================================================================== */

/* Shutdown check across threads */
boolean turn_off_check = 0;
#define TIMEOUT 10
boolean suspend = 0;
int button_pushed = 0, button_timer = TIMEOUT;

/* Time related variables */
struct tm * tstr;
char buf_date[11], buf_time[9];//, buf_wday[1], buf_hour[2];
int day, month, year, hour, minute;
struct timespec gettime;
int start_timer, timer_difference;

/* Menu position selectors */
uint8_t select_1, select_2, select_3;
/* menu: 0 = main screen, 1 = simple menu, 2 = keyboard menu */
int menu = 0;

/* Buttons related variables */
int check_button = 0, new_button = 0, count = 0, unit = 35000;

/* Mode : 0 = OFF / 1 = PROGRAM / 2 = MANUAL */
int mode;

/* Temperature variables */
std::string temp_target_text; // used for destination temperature
std::string temp_tolerance = "0"; // temperature tolerance
std::string temp = "0.0000"; // used for actual temperature

/* Program variable (used for display) */
program *prog = new program;

/* SQL connection variables */
MYSQL *mysqlConn = NULL;
std::string statement;
MYSQL_RES *sqlresult;
MYSQL_ROW sqlrow;
boolean sql_error;

/* Menu related constants */
#define BIT0 0x00 //   0 // 00000000
#define BIT1 0x01 //   1 // 00000001
#define BIT2 0x02 //   2 // 00000010
#define BIT3 0x04 //   4 // 00000100
#define BIT4 0x08 //   8 // 00001000
#define BIT5 0x10 //  16 // 00010000
#define BIT6 0x20 //  32 // 00100000
#define BIT7 0x40 //  64 // 01000000
#define BIT8 0x80 // 128 // 10000000

/* ======================================================================
  8 bit -> 3 bit hash using a de-Brujin sequence
  bits2bosition[(( byte * hexadecimal ) >> shift_left ) &    0x07   ]
           with              0x17              4          0b00000111
In theory using hexadecimal = 0x17 and shift_left = 4 is the best option
since it they are the smallest numbers and the operations are faster
====================================================================== */
int bits2position[] = { 8 , 1 , 2 , 4 , 7 , 3 , 6 , 5 };
uint8_t bit[] = {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

/* WLAN related variables */
std::string wlan = "";
std::vector<std::string> WiFi_Data;
std::vector<std::string>::iterator it_W;
#define DC_W 5 //data set count for WiFi_Data
std::string wlan_country_code;

/* Region settings variables */
std::vector<std::string> Countries_Data;
std::vector<std::string>::iterator it_C;
#define DC_C 2 //data set count for Countries_Data
std::string sys_country_code;
std::string sys_timezone;
std::string sys_localtime;

/* Status data gatherer */
std::vector<std::string> Status_Data;
std::vector<std::string>::iterator it_S;
#define DC_S 1 //data set count for Status_Data

/* Custom characters for keyboard */
const char    bk_char[] = {0x00,0x08,0x1C,0x2A,0x08,0x08,0x00};
const char    up_char[] = {0x00,0x30,0x38,0x3E,0x38,0x30,0x00};
const char  down_char[] = {0x00,0x06,0x0E,0x3E,0x0E,0x06,0x00};
const char right_char[] = {0x00,0x3E,0x3E,0x1C,0x08,0x08,0x00};
const char  left_char[] = {0x00,0x08,0x08,0x1C,0x3E,0x3E,0x00};
const char    ok_char[] = {0x00,0x10,0x38,0x54,0x10,0x1F,0x00};
/*
  Characters allowed in SSID names and/or a WPA2-PSK password:
       abcdefghijklmnopqrstuvwxyz
       ABCDEFGHIJKLMNOPQRSTUVWXYZ
       1234567890
       $@^`,|%;.~()/\{}:?[]=-+_#!
  Characters not allowed in SSID names and/or a WPA2-PSK password:
       "<>'&
*/
std::string keyboard = "qwertyuiop!@#$<>\u0008asdfghjkl{}123()%~zxcvbnm;:,456[]\"QWERTYUIOP+789*?'ASDFGHJKL|-.0/\u0011^&`ZXCVBNM  \\=_\u0014\u0012\u0013\u000D";
uint8_t select_k, pos_k = 0;
/*
qwertyuiop!@#$<> 
asdfghjkl{}123()%
~zxcvbnm;:,456[]"
QWERTYUIOP+789*?'
ASDFGHJKL|-.0/ ^&
`ZXCVBNM  \=_    
*/
std::string k_input = ""; // result string after keyboard input

/* ======================================================================
Commands section
====================================================================== */
#define DELIMITER "\x1e"

#define RESTART_SQL "\
service mysql restart\
"

#define COUNTRIES_LIST "\
cat /usr/share/zoneinfo/zone.tab | cut -f 1,3 | sed -e '1,/TZ/d' | tr '\t' '\x1e' | tr '\n' '\x1e' \
"

#define GET_WIRELESS_COUNTRY_CODE "\
grep country= /etc/wpa_supplicant/wpa_supplicant.conf | cut -d '=' -f 2 \
"

#define GET_LOCALZONE "\
timedatectl status | grep 'Time zone' | cut -f 10 -d ' ' \
"

#define GET_LOCALTIME "\
timedatectl status | grep 'Time zone' | cut -f 12 -d ' ' | cut -f 1 -d ')' | sed 's/^\\(.\\{3\\}\\)/\\1:/' \
"

#define LIST_WLAN_INTERFACES "\
echo $(#!/bin/sh \n\
  for dir in /sys/class/net/*/wireless; do \n\
    if [ -d \"$dir\" ]; then \n\
      basename \"$(dirname \"$dir\")\" \n\
    fi \n\
  done \n) | head -n 1\
"

/* ======================================================================
Prototypes section
====================================================================== */
void static_read_temperature(SH1106_DS18B20_KEYS *instance);
void control_module(void);
void display_menu(void);
void display_screen(void);
void display_error(const char * = NULL);
void display_keyboard(void);
void set_time(uint8_t oct);
void get_time(void);
boolean check_timer(void);
void init_timer(void);
void get_temp(void);
void rotate_program(int direction);
void rotate_mode(int direction);
void get_mode(void);
void mode_check(void);
void alter_manual_temp(int direction);
void get_active_program(void);
void sql_check_conn(void);
boolean finish_with_error(MYSQL *mysqlConn,const char *reason);
int shiftBitR(uint8_t *oct,uint8_t target);
int shiftBitL(uint8_t *oct,uint8_t target);
int getBitPos(uint8_t oct);
boolean getBit(uint8_t oct,uint8_t bit);
void GetStatusData(void);
void ScanWiFi(void);
void GetWiFiData(void);
boolean GetWlanInterface(void);
void GetCountriesData(void);
