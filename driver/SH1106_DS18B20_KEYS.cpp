/*
  1.3" SPI OLED with SH1106 driver
*/

#include "SH1106_DS18B20_KEYS.h"

/* ======================================================================
Low level SPI Write function
====================================================================== */
inline void SH1106_DS18B20_KEYS::fastSPIwrite(uint8_t d) {
  bcm2835_spi_transfer(d);
}
inline void SH1106_DS18B20_KEYS::fastSPIwrite(char* tbuf, uint32_t len) {
  bcm2835_spi_writenb(tbuf, len);
}

/* ======================================================================
Function to get length of a UTF-8 text
====================================================================== */
inline int SH1106_DS18B20_KEYS::len_utf8(const char * text)
{
  int len = 0;
  while (*text) len += ((*text++ & 0xc0) != 0x80);
  /* UTF-8 characters have a defined code page inside this range:
               0b11000000 .. 0b11110000
                     0xC0 .. 0xF0
     What is below 0xC0 is also less or equal than 0xBF:
               0b00000000 .. 0b10111111
                     0x00 .. 0xBF                              */
  return len;
}

/* ======================================================================
Function to number of spaces from a UTF-8 text
====================================================================== */
inline int SH1106_DS18B20_KEYS::sp_count_utf8(const char * text)
{
  int count = 0;
  while (*text) count += (*text++ == 0x20);
  /* There is no other UTF-8 character that has this value */
  return count;
}

/* ======================================================================
Function that finds last space position in a UTF-8 text using
the maximum number of characters on a row length of 21
====================================================================== */
inline int SH1106_DS18B20_KEYS::find_space_21_utf8(const char * text)
{
  // When using 6x8 font style we have 21 chars per line
  int len = 0, max_size = 0;
  while ((*text) && (len < 21))
  {
    if (*text <= 0x20)
    { // Retain last space position in the UTF-8 text
      len += ((*text++ & 0xc0) != 0x80);
      max_size = len;
    }
    else len += ((*text++ & 0xc0) != 0x80);
  }
  if ((!max_size) || ((max_size) && (!(*text)))) max_size = len;
  if ((max_size <= 21) && (*text) && (*(text-1) <= 0x20)) max_size--;
  return max_size;
}

/* ======================================================================
OLED display instantiation
====================================================================== */
SH1106_DS18B20_KEYS::SH1106_DS18B20_KEYS()
{
  // Init all var, and clean
  // Command I/O
  rst = 0 ;
  dc  = 0 ;
  cs =  0 ;
  relay = 0 ;

  // Lcd size
  oled_width  = 0;
  oled_height = 0;

  // Empty pointer to OLED buffer
  poledbuff = NULL;

  // Empty pointer to DS18B20 buffer
  ptempbuff = NULL;

  // Empty pointer to backup buffer
  old_buff = NULL;
}

/* ======================================================================
Init buffers for OLED, DS18B20 and also init BCM GPIO
====================================================================== */
boolean SH1106_DS18B20_KEYS::init_buff(void)
{
  // Default values SH1106
  oled_width = 128;
  oled_height = 64;

  // OLED buffer size : 1 pixel is one bit
  oled_buff_size = oled_width * oled_height ;

  // De-Allocate memory for OLED buffer if any
  if (poledbuff)
    free(poledbuff);

  // Allocate memory for OLED buffer
  poledbuff = (uint8_t *) malloc ( oled_buff_size );

  if (!poledbuff)
    return false;

  // DS18B20 buffer size : 64 bits/sensor
  ds18b20_size = 64;

  // De-Allocate memory for DS18B20 buffer if any
  if (ptempbuff)
    free(ptempbuff);

  // Allocate memory for DS18B20 buffer
  ptempbuff = (unsigned long long *) malloc ( ds18b20_size );

  if (!ptempbuff)
    return false;

  // De-Allocate memory for backup buffer if any
  if (old_buff)
    free(old_buff);

  // Allocate memory for backup buffer max size of display
  old_buff = (uint8_t *) malloc ( oled_buff_size );

  if (!old_buff)
    return false;

  // Init Raspberry PI GPIO
  if (!bcm2835_init())
    return false;
  return true;
}

/* ======================================================================
Init SPI comunication through BCM GPIO
====================================================================== */
boolean SH1106_DS18B20_KEYS::module_init(void)
{
  /* ========================= OLED section ========================= */
  rst = OLED_SPI_RESET;  // Reset Pin
  dc = OLED_SPI_DC;      // Data / command Pin
  //cs = OLED_SPI_CS;    // Raspberry Pi Zero SPI chip Enable (CE0)
  relay = RELAY_PIN;     // Relay control Pin

  // Verify if buffers can be initialized
  if (!init_buff())
    return (false);

  // Init & Configure Raspberry PI Zero SPI
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);

  // 16 MHz SPI bus, but Worked at 62 MHz also
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16);

  // Set the pin that will control DC as output
  bcm2835_gpio_fsel(dc, BCM2835_GPIO_FSEL_OUTP);

  // Setup reset pin direction as output
  bcm2835_gpio_fsel(rst, BCM2835_GPIO_FSEL_OUTP);

  /* ======================== Buttons section ======================= */
  // Set all defined buttons as inputs
  bcm2835_gpio_fsel(BTN_ON_OFF, BCM2835_GPIO_FSEL_INPT);
  bcm2835_gpio_fsel(BTN_LEFT, BCM2835_GPIO_FSEL_INPT);
  bcm2835_gpio_fsel(BTN_RIGHT, BCM2835_GPIO_FSEL_INPT);
  bcm2835_gpio_fsel(BTN_UP, BCM2835_GPIO_FSEL_INPT);
  bcm2835_gpio_fsel(BTN_DOWN, BCM2835_GPIO_FSEL_INPT);
  bcm2835_gpio_fsel(BTN_MENU, BCM2835_GPIO_FSEL_INPT);
  // Pull up internal resistors for defined buttons
  bcm2835_gpio_set_pud(BTN_ON_OFF, BCM2835_GPIO_PUD_UP);
  bcm2835_gpio_set_pud(BTN_LEFT, BCM2835_GPIO_PUD_UP);
  bcm2835_gpio_set_pud(BTN_RIGHT, BCM2835_GPIO_PUD_UP);
  bcm2835_gpio_set_pud(BTN_UP, BCM2835_GPIO_PUD_UP);
  bcm2835_gpio_set_pud(BTN_DOWN, BCM2835_GPIO_PUD_UP);
  bcm2835_gpio_set_pud(BTN_MENU, BCM2835_GPIO_PUD_UP);

  /* ======================== DS18B20 section ======================= */
  // Check for pull up resistor
  // Signal input should be high
  // Set PIN to INPUT MODe
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  int Flag=0;
  for(int loop=0; loop<100; loop++)
  {
    usleep(1000); // sleep 1ms
    if (( bcm2835_gpio_lev(ONE_WIRE) != 0 ))
    {
      Flag=1;
      break;
    }
  }
  if(Flag==0)
  {
    return (false);
  }

  return (true);
}

/* ======================================================================
Init OLED display by sending commands on SPI pins
Start thread that fills temperature data value
====================================================================== */
void SH1106_DS18B20_KEYS::start(void)
{
  DS18B20_ChangeSensorsResolution();
  DS18B20_ScanForSensor();

  uint8_t multiplex;
  uint8_t offset;
  uint8_t compins;
  uint8_t precharge;
  uint8_t osc_div;
  uint8_t vcomh;
  uint8_t contrast;

  // Setup reset pin direction (used by SPI)
  bcm2835_gpio_fsel(rst, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(rst, HIGH);

  // VDD (3.3V) goes high at start, one milisecond wait
  usleep(1000);

  // bring reset low
  bcm2835_gpio_write(rst, LOW);

  // wait 10ms
  usleep(10000);

  // bring out of reset
  bcm2835_gpio_write(rst, HIGH);

  // typical values for SH1106 128x64
  multiplex = 0x3F;  // duty = 1/32
  offset = 0x00;     // 0x00 = no offset
  compins   = 0x12;  // alternative signals pad configuration
  precharge  = 0x22; // typical precharge period
  osc_div = 0x80;    // the suggested ratio 0x80
  vcomh = 0x40;      // typical vcomh value
  contrast = 0x80;   // contrast control is not used value

  // OLED_SH1106_SPI_128x64 commands start
  SH1106_sendCommand(SH1106_Display_Off);
  SH1106_sendCommand(SH1106_Set_Muliplex_Ratio, multiplex);
  SH1106_sendCommand(SH1106_Set_Lower_Column_Start_Address);
  SH1106_sendCommand(SH1106_Set_Higher_Column_Start_Address);
  SH1106_sendCommand(SH1106_Set_Display_Start_Line);
  SH1106_sendCommand(SH1106_Set_Page_Address);
  SH1106_sendCommand(SH1106_Set_Segment_Remap);
  SH1106_sendCommand(SH1106_Normal_Display);
  SH1106_sendCommand(SH1106_Set_DC_DC_Control_Mode);
  SH1106_sendCommand(SH1106_DC_DC_Enable);
  SH1106_sendCommand(SH1106_Set_Pump_Voltage);
  SH1106_sendCommand(SH1106_Set_Com_Output_Scan_Direction_Normal);
  SH1106_sendCommand(SH1106_Set_Display_Offset,offset);
  SH1106_sendCommand(SH1106_Set_Display_DCLK_FRQ,osc_div);
  SH1106_sendCommand(SH1106_Set_Precharge_Period,precharge);
  SH1106_sendCommand(SH1106_Set_Com_Pins,compins);
  SH1106_sendCommand(SH1106_Set_Vcomh_Deselect_Level,vcomh);
  SH1106_sendCommand(SH1106_Set_Contrast_Level, contrast);

  // Empty uninitialized buffer
  SH1106_clearDisplay();

  // turn on oled panel
  SH1106_sendCommand(SH1106_Display_On);

  // wait 100ms
  usleep(100000);

  bcm2835_gpio_fsel(relay, BCM2835_GPIO_FSEL_OUTP);
  Relay_OFF();

}

/* ======================================================================
Closes SPI communication and releases BCM control
====================================================================== */
void SH1106_DS18B20_KEYS::close(void)
{
  // Turn off relay
  Relay_OFF();
  // Clear OLED
  SH1106_clearDisplay();
  SH1106_display();

  // De-Allocate memory for OLED buffer if any
  if (poledbuff)
    free(poledbuff);

  poledbuff = NULL;

  // De-Allocate memory for DS18B20 buffer if any
  if (ptempbuff)
    free(ptempbuff);

  ptempbuff = NULL;

  // De-Allocate memory for backup buffer if any
  if (old_buff)
    free(old_buff);

  old_buff = NULL;

  // Release Raspberry SPI
  bcm2835_spi_end();

  // Release Raspberry I/O control
  bcm2835_close();
}

/* ======================================================================
Sets contrast level for OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_setContrast(uint8_t Contrast)
{
   SH1106_sendCommand(SH1106_Set_Contrast_Level);
   SH1106_sendCommand(Contrast);
}

/* ======================================================================
Inverts pixels on OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_invertDisplay(uint8_t i)
{
  if (i)
    SH1106_sendCommand(SH1106_Invert_Display);
  else
    SH1106_sendCommand(SH1106_Normal_Display);
}

/* ======================================================================
Send one byte command to OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_sendCommand(uint8_t c)
{
  // Setup D/C line to low to switch to command mode
  bcm2835_gpio_write(dc, LOW);
  // Write Data on SPI
  fastSPIwrite(c);
}

/* ======================================================================
Send two bytes command to OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_sendCommand(uint8_t c0, uint8_t c1)
{
  char buff[3];
  buff[1] = c0;
  buff[2] = c1;

  // Setup D/C line to low to switch to command mode
  bcm2835_gpio_write(dc, LOW);

  // Write Data
  fastSPIwrite(&buff[1], 2);
}

/* ======================================================================
Send three bytes command to OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_sendCommand(uint8_t c0, uint8_t c1, uint8_t c2)
{
  char buff[4] ;

  buff[1] = c0;
  buff[2] = c1;
  buff[3] = c2;

  // Setup D/C line to low to switch to command mode
  bcm2835_gpio_write(dc, LOW);

  // Write Data
  fastSPIwrite(&buff[1], 3);
}

/* ======================================================================
Send one byte data to OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_sendData(uint8_t c)
{
  // Setup D/C line to high to switch to data mode
  bcm2835_gpio_write(dc, HIGH);

  // write value
  fastSPIwrite(c);
}

/* ======================================================================
Send entire buffer data to OLED display
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_display(void)
{
  size_t i=0 ;

  // pointer to OLED data buffer
  uint8_t * p = poledbuff;

  for (uint8_t k=0; k<8; k++)
  {
    SH1106_sendCommand(SH1106_Set_Page_Address + k);
    SH1106_sendCommand(SH1106_Set_Lower_Column_Start_Address);
    SH1106_sendCommand(SH1106_Set_Higher_Column_Start_Address);

    // Setup D/C line to high to switch to data mode
    bcm2835_gpio_write(dc, HIGH);

    for (i=0; i<oled_width; i++)
    {
      fastSPIwrite(*p++);
    }
  }
}

/* ======================================================================
Clear entire OLED buffer
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_clearDisplay(void)
{
  memset(poledbuff, 0, oled_buff_size);
}

/* ======================================================================
Write standard 6x8 character in OLED buffer starting at left margin
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_printchar6(uint8_t row, const char * text, boolean centered, boolean invert)
{ // Spaces are constant of 6x8
  uint8_t * p = poledbuff;
  // Get where to do the change in the buffer
  p = poledbuff + (row * oled_width);

  while (*text)
  {
    int max_size, text_len;
    text_len = len_utf8(text);
    max_size = find_space_21_utf8(text);
    if (centered)
    { // 128 = 21 * 6 + 2 ; if centered we split blank bytes in half
      for (int k=0;k<((21-max_size)*3+1);k++)
      {
        // [(21 chars)-(length of text to display)]x(6/2 blank bytes)+1
        *p++ = ((invert) ? ((text_len > max_size) ? 0xFF : 0x7F) : 0x00);
      }
    }
    size_t q = 0;
    int iter = max_size;
    while(iter > 0)
    {
      // Ignore non-printable ASCII characters.
      if (*text <= 0x20) q = 0; // Space character
      // UTF-8 get character starting byte location in SMALL_FONT
      // Character (*(text+1)) is taken from code page (*text)
      else if (((*text & 0xc0) == 0xc0) || ((*text & 0xd0) == 0xd0))
        switch (*text) {
          case 0xC2: q = (*++text) - (64); /* 0xA0 */ break;
          case 0xC3: q = (*++text); /* 0x80 */ break;
          case 0xC4: q = (*++text) + (64); /* 0x80 */ break;
          case 0xC5: q = (*++text) + (128); /* 0x80 */ break;
          case 0xD0: q = (*++text) + (192); /* 0x80 */ break;
          case 0xD1: q = (*++text) + (256); /* 0x80 */ break;
          default:   q = 0; // Space character
        }
      else if ((*text & 0xe0) == 0xe0) text += 2;
      else if ((*text & 0xf0) == 0xf0) text += 3;
      else q = (*text - ' '); // Normal ASCII character
      for (int k = 0; k < 6; k++)
      {
        *p++ = ((invert) ? (~SMALL_FONT[( q * 6 + k )]&((text_len > max_size) ? 0xFF : 0x7F)) : SMALL_FONT[( q * 6 + k )]);
      }
      text++;
      iter--;
    }
    if (centered)
    { // 128 = 21 * 6 + 2 ; if centered we split blank bytes in half
      for (int k=0;k<((21-max_size)*3+1);k++)
      {
        // [(21 chars)-(length of text to display)]x(6/2 blank bytes)+1
        *p++ = ((invert) ? ((text_len > max_size) ? 0xFF : 0x7F) : 0x00);
      }
    }
    else
    { // 128 = 21 * 6 + 2 ; if not centered we write blank at the end
      for (int k=0;k<((21-max_size)*6+2);k++)
      {
        // [(21 chars)-(length of text to display)]x(6 blank bytes)+2
        *p++ = ((invert) ? ((text_len > max_size) ? 0xFF : 0x7F) : 0x00);
      }
    }
    if (*text)
    { // If the next first character is " " and text is centered,
      // we increment to the next character (used for text wrapping)
      if ((*text<=0x20)&&(text_len>max_size))
      text++;
    }
  }
}

/* ======================================================================
Write standard 6x8 character in OLED buffer (alternative)
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_printchar6(uint8_t row, uint8_t start, const char * text, uint8_t splen, boolean invert, boolean rotation)
{
  uint8_t * p = poledbuff;
  // Get where to do the change in the buffer
  p = poledbuff + (row * oled_width) + start;

  // Buffer countdown iterator (printing only on one line and disregarding the rest of the text)
  int max_print = 127 - start;
  int byte_size = ( len_utf8(text) * 6 + sp_count_utf8(text) * ( splen - 6 ) );
  size_t q; // UTF-8 char position in Fonts section
  uint8_t * n;

  if ((rotation) && (byte_size > max_print))
  { // Rotation section
    if (old_text.compare(text) != 0)
    {
      old_text = std::string(text);
      // Set new size for the new old buffer (number of bytes)
      old_buff_size = byte_size + 6; // 6 bytes of space separator at the end
      n = old_buff;
      int do_print = max_print; // limiter for display on one row
      while(*text)
      { // Ignore non-printable ASCII characters.
        if (*text <= 0x20) q = 0; // Space character
        // UTF-8 get character starting byte location in SMALL_FONT
        // Character (*(text+1)) is taken from code page (*text)
        else if (((*text & 0xc0) == 0xc0) || ((*text & 0xd0) == 0xd0))
          switch (*text) {
            case 0xC2: q = (*++text) - (64); /* 0xA0 */ break;
            case 0xC3: q = (*++text); /* 0x80 */ break;
            case 0xC4: q = (*++text) + (64); /* 0x80 */ break;
            case 0xC5: q = (*++text) + (128); /* 0x80 */ break;
            case 0xD0: q = (*++text) + (192); /* 0x80 */ break;
            case 0xD1: q = (*++text) + (256); /* 0x80 */ break;
            default:   q = 0; // Space character
          }
        else if  ((*text & 0xe0) == 0xe0) text += 2;
        else if  ((*text & 0xf0) == 0xf0) text += 3;
        else q = (*text - ' '); // Normal ASCII character
        // Now filling the backup buffer and the display buffer with data
        for (size_t k = 0; k < ( (q == 0) ? splen : 6 ); k++, do_print--)
        {
          if (q != 0)
            *n++ = ((invert) ? (~SMALL_FONT[( q * 6 + k )]&(0x7F)) : SMALL_FONT[( q * 6 + k )]);
          else
            *n++ = ((invert) ? (0x7F) : (0x00)); // If splen is defined we don't use SMALL_FONT[]
          if (do_print >= 0) *p++ = *(n-1);
        }
        // Let's get the next character
        text++;
      }
      for (size_t k = 0; k < 6; k++) *n++ = ((invert) ? (0x7F) : (0x00)); // add space at the end
    }
    else
    {
      n = old_buff; // get to start position of backup buffer
      // Write data to display buffer
      int do_print = max_print; // limiter for display on one row
      while (do_print >= 0)
      { // while pointer has data and the display limiter has not been reached write to display buffer
        *p++ = *n++;
        // Countdown iterator (number of chars remaining till end of line)
        do_print--;
      }
      // Data rotation in backup buffer
      n = old_buff; // get to start position of backup buffer
      uint8_t r = *n; // get data from first position of the backup buffer
      size_t old_buff_pos = 1;
      while (old_buff_pos < old_buff_size)
      { // go through the backup buffer and shift data to the left one position
        *n = *(n+1);
        n++;
        old_buff_pos++;
      } // reached last buffer position
      *n = r; // write data taken from the first position
    }
  }
  else
  { // If rotation is disabled or text display byte size is lower than limiter
    int do_print = max_print; // limiter for display on one row
    while((*text) && (do_print >= 0))
    {
      // Ignore non-printable ASCII characters.
      if (*text <= 0x20) q = 0; // Space character
      // UTF-8 get character starting byte location in SMALL_FONT
      // Character (*(text+1)) is taken from code page (*text)
      else if (((*text & 0xc0) == 0xc0) || ((*text & 0xd0) == 0xd0))
        switch (*text) {
          case 0xC2: q = (*++text) - (64); /* 0xA0 */ break;
          case 0xC3: q = (*++text); /* 0x80 */ break;
          case 0xC4: q = (*++text) + (64); /* 0x80 */ break;
          case 0xC5: q = (*++text) + (128); /* 0x80 */ break;
          case 0xD0: q = (*++text) + (192); /* 0x80 */ break;
          case 0xD1: q = (*++text) + (256); /* 0x80 */ break;
          default:   q = 0; // Space character
        }
      else if  ((*text & 0xe0) == 0xe0) text += 2;
      else if  ((*text & 0xf0) == 0xf0) text += 3;
      else q = (*text - ' '); // Normal ASCII character
      // Now filling the display buffer with data
      for (size_t k = 0; ((k < ((q == 0) ? splen : 6)) && (do_print >= 0)); k++, do_print--)
      { // If the display limiter has not been reached write to display buffer
        if (q != 0)
          *p++ = ((invert) ? (~SMALL_FONT[( q * 6 + k )]&(0x7F)) : SMALL_FONT[( q * 6 + k )]);
        else
          *p++ = ((invert) ? (0x7F) : (0x00)); // If splen is defined we don't use SMALL_FONT[]
      }
      // Let's get the next character
      text++;
    }
  }
}

/* ======================================================================
Write special 12x16 character in OLED buffer
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_printspecial(uint8_t row, uint8_t offset, const char * text)
{
  uint8_t * p = poledbuff;
  // Get where to do the change in the buffer
  p = poledbuff + (row * oled_width) + offset;

  unsigned char i=0;
  while(text[i])
  {
    if (text[i] == 44 || text[i] == 46 || text[i] == 58) //If separators are ',','.' or ':'.
    {
      for (size_t k=0; k<6; k++)
      {
        *p = SEPARATOR[((text[i]==44?0:(text[i]==46?12:24))+k)];
        p = p + oled_width;
        *p = SEPARATOR[((text[i]==44?6:(text[i]==46?18:30))+k)];
        p = p - oled_width + 1;
      }
    } else
    if (text[i] > 47 && text[i] < 58) //If we have digits '0' to '9'.
    {
      for (size_t k=0; k<12; k++)
      {
        *p = BIG_DIGITS[((text[i]-48)*24+k)];
        p = p + oled_width;
        *p = BIG_DIGITS[((text[i]-48)*24+k+12)];
        p = p - oled_width + 1;
      }
    } else
    if (text[i] == 33) //If we have special symbol '!'.
    {
      for (size_t k=0; k<12; k++)
      {
        *p = BIG_DIGITS[(240+k)];
        p = p + oled_width;
        *p = BIG_DIGITS[(252+k)];
        p = p - oled_width + 1;
      }
    } else
    if (text[i] == 69) //If we have the letter 'E' - stands for error.
    {
      for (size_t k=0; k<12; k++)
      {
        *p = BIG_DIGITS[(264+k)];
        p = p + oled_width;
        *p = BIG_DIGITS[(276+k)];
        p = p - oled_width + 1;
      }
    } else {
      for (size_t k=0; k<12; k++)
      {
        *p = 0x00;
        p = p + oled_width;
        *p = 0x00;
        p = p - oled_width + 1;
      }
    }
    i++;
  }
}

/* ======================================================================
Write custom data in OLED buffer (user defined)
====================================================================== */
void SH1106_DS18B20_KEYS::SH1106_printcustom(uint8_t row, uint8_t start, const char * buffer, int len_buffer, boolean invert)
{
  uint8_t * p = poledbuff;
  // Get where to do the change in the buffer
  p = poledbuff + (row * oled_width) + start;

  // Now filling the buffer with data
  for(int k = 0; k < len_buffer; k++)
  {
    *p++ = ((invert) ? (~(*buffer++)&(0x7F)) : (*buffer++));
  }
}

/* ======================================================================
Buttons section
====================================================================== */

/* ======================================================================
Check event detection status for all buttons
====================================================================== */
int SH1106_DS18B20_KEYS::KEYS_check(void)
{
  s_on_off = !bcm2835_gpio_lev(BTN_ON_OFF);
  s_left = !bcm2835_gpio_lev(BTN_LEFT);
  s_right = !bcm2835_gpio_lev(BTN_RIGHT);
  s_up = !bcm2835_gpio_lev(BTN_UP);
  s_down = !bcm2835_gpio_lev(BTN_DOWN);
  s_menu = !bcm2835_gpio_lev(BTN_MENU);
  // check if multiple buttons are pressed
  multi_buttons = (uint8_t)( (int)s_on_off + (int)s_left + (int)s_right + (int)s_up + (int)s_down + (int)s_menu );
  if (( multi_buttons == HIGH ))
  { // check witch button is pressed
    if ( s_left   == HIGH ) return 1;
    if ( s_right  == HIGH ) return 2;
    if ( s_up     == HIGH ) return 3;
    if ( s_down   == HIGH ) return 4;
    if ( s_menu   == HIGH ) return 5;
    if ( s_on_off == HIGH ) return 6;
  }
  return 0;
}
/*
int SH1106_DS18B20_KEYS::KEYS_check(void)
{
//  uint8_t s_on_off, s_left, s_right, s_up, s_down, s_menu, multi_buttons;
  s_on_off = !bcm2835_gpio_lev(BTN_ON_OFF);
  s_left = !bcm2835_gpio_lev(BTN_LEFT);
  s_right = !bcm2835_gpio_lev(BTN_RIGHT);
  s_up = !bcm2835_gpio_lev(BTN_UP);
  s_down = !bcm2835_gpio_lev(BTN_DOWN);
  s_menu = !bcm2835_gpio_lev(BTN_MENU);
  multi_buttons = (uint8_t)( (int)s_on_off + (int)s_left + (int)s_right + (int)s_up + (int)s_down + (int)s_menu );
  usleep(50000); // debounce max time
  if (( multi_buttons == HIGH ))
  { // check witch button is still pressed
    if (( HIGH == (!bcm2835_gpio_lev(BTN_LEFT)) )   && ( s_left == HIGH ))   return 1;
    if (( HIGH == (!bcm2835_gpio_lev(BTN_RIGHT)) )  && ( s_right == HIGH ))  return 2;
    if (( HIGH == (!bcm2835_gpio_lev(BTN_UP)) )     && ( s_up == HIGH ))     return 3;
    if (( HIGH == (!bcm2835_gpio_lev(BTN_DOWN)) )   && ( s_down == HIGH ))   return 4;
    if (( HIGH == (!bcm2835_gpio_lev(BTN_MENU)) )   && ( s_menu == HIGH ))   return 5;
    if (( HIGH == (!bcm2835_gpio_lev(BTN_ON_OFF)) ) && ( s_on_off == HIGH )) return 6;
  }
  return 0;
}
*/

/* ======================================================================
One wire comunication section

One wire timings source:
https://www.maximintegrated.com/en/app-notes/index.mvp/id/126
====================================================================== */
inline void SH1106_DS18B20_KEYS::DelayMicrosecondsNoSleep(int delay_us)
{
   long int start_time;
   long int time_difference;
   struct timespec gettime_now;

   clock_gettime(CLOCK_REALTIME, &gettime_now);
   start_time = gettime_now.tv_nsec;      //Get nS value
   while (1)
   {
      clock_gettime(CLOCK_REALTIME, &gettime_now);
      time_difference = gettime_now.tv_nsec - start_time;
      if (time_difference < 0)
         time_difference += 1000000000;            //(Rolls over every 1 second)
      if (time_difference > (delay_us * 1000))     //Delay for # nS
         break;
   }
}

/* ======================================================================
Write 1 bit: Send a '1' bit to the 1-Wire slaves (Write 1 time slot)
  Drive bus low, delay A (Standard 6us, Overdrive 1.0us)
  Release bus, delay B (Standard 64us, Overdrive 7.5us)
====================================================================== */

/* ======================================================================
Write 0 bit: Send a '0' bit to the 1-Wire slaves (Write 0 time slot)
  Drive bus low, delay C (Standard 60us, Overdrive 7.5us)
  Release bus, delay D (Standard 10us, Overdrive 2.5us)
====================================================================== */

inline void SH1106_DS18B20_KEYS::DS18B20_WriteByte(unsigned char value)
{
  unsigned char Mask=1;
  int loop;

  for(loop=0;loop<8;loop++)
  {
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
    // INP_GPIO(DS_PIN);
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_OUTP);
    // OUT_GPIO(DS_PIN);
    bcm2835_gpio_write(ONE_WIRE, LOW);
    // GPIO_CLR= 1 <<DS_PIN;

    if ((value & Mask)!=0)
    {
      DelayMicrosecondsNoSleep(1);
      bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
      // INP_GPIO(DS_PIN);
      DelayMicrosecondsNoSleep(60);
    }
    else
    {
      DelayMicrosecondsNoSleep(60);
      bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
      // INP_GPIO(DS_PIN);
      DelayMicrosecondsNoSleep(1);
    }
    Mask*=2;
    DelayMicrosecondsNoSleep(60);
  }
   usleep(100);
}

inline void SH1106_DS18B20_KEYS::DS18B20_WriteBit(unsigned char value)
{
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  // INP_GPIO(DS_PIN);
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_OUTP);
  // OUT_GPIO(DS_PIN);
  bcm2835_gpio_write(ONE_WIRE, LOW);
  // GPIO_CLR=1 <<DS_PIN;
  if(value)
  {
    DelayMicrosecondsNoSleep(1);
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
    // INP_GPIO(DS_PIN);
    DelayMicrosecondsNoSleep(60);
  }
  else
  {
    DelayMicrosecondsNoSleep(60);
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
    // INP_GPIO(DS_PIN);
    DelayMicrosecondsNoSleep(1);
  }
  DelayMicrosecondsNoSleep(60);
}

/* ======================================================================
Read bit: Read a bit from the 1-Wire slaves (Read time slot)
  Drive bus low, delay A (Standard 6us, Overdrive 1.0us)
  Release bus, delay E (Standard 9us, Overdrive 1.0us)
  Sample bus to read bit from slave
  Delay F (Standard 55us, Overdrive 7us)
====================================================================== */

inline unsigned char SH1106_DS18B20_KEYS::DS18B20_ReadBit(void)
{
  unsigned char rvalue=0;
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  // INP_GPIO(DS_PIN);
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_OUTP);
  // OUT_GPIO(DS_PIN);
  bcm2835_gpio_write(ONE_WIRE, LOW);
  // GPIO_CLR= 1 << DS_PIN;
  DelayMicrosecondsNoSleep(1);
   // set INPUT
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  // INP_GPIO(DS_PIN);
  DelayMicrosecondsNoSleep(2);
  if (( bcm2835_gpio_lev(ONE_WIRE) != 0 ))
  // if(GPIO_READ(DS_PIN)!=0)
    rvalue=1;
  DelayMicrosecondsNoSleep(60);
  return rvalue;
}

inline unsigned char SH1106_DS18B20_KEYS::DS18B20_ReadByte(void)
{
  unsigned char Mask=1;
  int loop;
  unsigned char data=0;

  for(loop=0;loop<8;loop++)
  {
    //  set output
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
    // INP_GPIO(DS_PIN);
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_OUTP);
    // OUT_GPIO(DS_PIN);
    bcm2835_gpio_write(ONE_WIRE, LOW);
    // GPIO_CLR= 1 << DS_PIN;
    DelayMicrosecondsNoSleep(1);
     // set input
    bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
    // INP_GPIO(DS_PIN);
     // Wait  2 us
    DelayMicrosecondsNoSleep(2);
    if (( bcm2835_gpio_lev(ONE_WIRE) != 0 ))
    // if(GPIO_READ(DS_PIN)!=0)
      data |= Mask;
    Mask*=2;
    DelayMicrosecondsNoSleep(60);
  }
  return data;
}

/* ======================================================================
Reset: Reset the 1-Wire bus slave devices and ready them for a command
  Delay G (Standard 0us, Overdrive 2.5us)
  Drive bus low, delay H (Standard 480us, Overdrive 70us)
  Release bus, delay I (Standard 70us, Overdrive 8.5us)
  Sample bus, 0 = device(s) present, 1 = no device present
  Delay J (Standard 410us, Overdrive 40us)
====================================================================== */
inline int SH1106_DS18B20_KEYS::DS18B20_Reset(void)
{
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  // INP_GPIO(DS_PIN);

  DelayMicrosecondsNoSleep(10);

  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  // INP_GPIO(DS_PIN);
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_OUTP);
  // OUT_GPIO(DS_PIN);

   // pin low for 480 us
  bcm2835_gpio_write(ONE_WIRE, LOW);
  // GPIO_CLR=1<<DS_PIN;
  usleep(480);
  bcm2835_gpio_fsel(ONE_WIRE, BCM2835_GPIO_FSEL_INPT);
  // INP_GPIO(DS_PIN);
  DelayMicrosecondsNoSleep(60);
  if (( bcm2835_gpio_lev(ONE_WIRE) == 0 ))
  // if(GPIO_READ(DS_PIN)==0)
  {
    DelayMicrosecondsNoSleep(420);
    return 1;
  }
  return 0;
}

inline void SH1106_DS18B20_KEYS::DS18B20_ReadScratchPad(void)
{
  DS18B20_WriteByte(DS18B20_READ_SCRATCHPAD);
  for(int loop=0;loop<9;loop++)
  {
    ScratchPad[loop]=DS18B20_ReadByte();
  }
}

inline unsigned char SH1106_DS18B20_KEYS::DS18B20_CalcCRC(unsigned char * data, unsigned char byteSize)
{
  unsigned char shift_register = 0;
  unsigned char loop,loop2;
  char DataByte;

  for(loop = 0; loop < byteSize; loop++)
  {
    DataByte = *(data + loop);
    for(loop2 = 0; loop2 < 8; loop2++)
    {
      if((shift_register ^ DataByte)& 1)
      {
        shift_register = shift_register >> 1;
        shift_register ^=  0x8C;
      }
      else
        shift_register = shift_register >> 1;
      DataByte = DataByte >> 1;
    }
  }
  return shift_register;
}

inline char SH1106_DS18B20_KEYS::DS18B20_IDGetBit(unsigned long long *llvalue, char bit)
{
  unsigned long long Mask = 1ULL << bit;
  return ((*llvalue & Mask) ? 1 : 0);
}

inline unsigned long long SH1106_DS18B20_KEYS::DS18B20_IDSetBit(unsigned long long *llvalue, char bit, unsigned char newValue)
{
  unsigned long long Mask = 1ULL << bit;
  if((bit >= 0) && (bit < 64))
  {
    if(newValue==0)
      *llvalue &= ~Mask;
    else
      *llvalue |= Mask;
  }
  return *llvalue;
}

inline void SH1106_DS18B20_KEYS::DS18B20_SelectSensor(unsigned long long ID)
{
  int BitIndex;
  DS18B20_WriteByte(DS18B20_MATCH_ROM);
  for(BitIndex=0;BitIndex<64;BitIndex++)
    DS18B20_WriteBit(DS18B20_IDGetBit(&ID,BitIndex));
}

inline int SH1106_DS18B20_KEYS::DS18B20_SearchSensor(unsigned long long * ID, int * LastBitChange)
{
  int BitIndex;
  char Bit, NoBit;

  if(*LastBitChange <0) return 0;

  // Set bit at LastBitChange Position to 1
  // Every bit after LastbitChange will be 0
  if(*LastBitChange <64)
  {
    DS18B20_IDSetBit(ID,*LastBitChange,1);
    for(BitIndex=*LastBitChange+1;BitIndex<64;BitIndex++)
      DS18B20_IDSetBit(ID,BitIndex,0);
  }
  *LastBitChange=-1;

  // every action begins with reset
  if(!DS18B20_Reset()) return -1;
  DS18B20_WriteByte(DS18B20_SEARCH_ROM);
  for(BitIndex=0;BitIndex<64;BitIndex++)
  {
    NoBit = DS18B20_ReadBit();
    Bit = DS18B20_ReadBit();
    if(Bit && NoBit)
      return -2;
    if(!Bit && !NoBit)
    {
      // ok 2 possibilities
      if(DS18B20_IDGetBit(ID,BitIndex))
      {
        // Bit High already set
        DS18B20_WriteBit(1);
      }
      else
      {
        // ok let's try LOW value first
        *LastBitChange=BitIndex;
        DS18B20_WriteBit(0);
      }
    }
    else if(!Bit)
    {
      DS18B20_WriteBit(1);
      DS18B20_IDSetBit(ID,BitIndex,1);
    }
    else
    {
      DS18B20_WriteBit(0);
      DS18B20_IDSetBit(ID,BitIndex,0);
    }
  }
  return 1;
}

inline double SH1106_DS18B20_KEYS::DS18B20_ReadSensor(void)
{
  int RetryCount;
  unsigned long long * ID = ptempbuff;
  unsigned char CRCByte;
  union {
   short SHORT;
   unsigned char CHAR[2];
  } IntTemp;

  //time_t t = time(NULL);
  //struct tm tm = *localtime(&t);

  double temp=-999.99;

  for(RetryCount=0;RetryCount<10;RetryCount++)
  {
    // every read begins with reset
    if(!DS18B20_Reset()) continue;
    // start a conversion
    DS18B20_SelectSensor(*ID);
    DS18B20_ReadScratchPad();
    // OK Check sum Check;
    CRCByte= DS18B20_CalcCRC(ScratchPad,8);
    if(CRCByte!=ScratchPad[8]) continue;
    //Check Resolution
    /* double increment=0.0;
    switch(ScratchPad[4])
    {
      case 0x1f: increment=0.5;break;
      case 0x3f: increment=0.25;break;
      case 0x5f: increment=0.125;break;
      case 0x7f: increment=0.0625;break;
    } */

    // Read Temperature
    IntTemp.CHAR[0]=ScratchPad[0];
    IntTemp.CHAR[1]=ScratchPad[1];

    temp =  0.0625 * (double) IntTemp.SHORT;
  }
  return temp;
}

inline int SH1106_DS18B20_KEYS::DS18B20_GlobalStartConversion(void)
{
  int retry=0;

  while(retry<10)
  {
    if(!DS18B20_Reset())
      usleep(10000);
    else
    {
      DS18B20_WriteByte(DS18B20_SKIP_ROM);
      DS18B20_WriteByte(DS18B20_CONVERT_T);

#define USE_CONSTANT_DELAY
#ifdef USE_CONSTANT_DELAY
      usleep(wait_time * 1000);
      return 1;
#else
      int maxloop=0;
      // wait until ready
      while(!DS18B20_ReadBit())
      {
        maxloop++;
        if(maxloop>100000) break;
      }
      if(maxloop<=100000)  return 1;
#endif
    }
    retry++;
  }
  return 0;
}

inline void SH1106_DS18B20_KEYS::DS18B20_WriteScratchPad(unsigned char TH, unsigned char TL, unsigned char config)
{
  // First reset device
  DS18B20_Reset();
  usleep(10);
  // Skip ROM command
  DS18B20_WriteByte(DS18B20_SKIP_ROM);
  // Write Scratch pad
  DS18B20_WriteByte(DS18B20_WRITE_SCRATCHPAD);
  // Write TH
  DS18B20_WriteByte(TH);
  // Write TL
  DS18B20_WriteByte(TL);
  // Write config
  DS18B20_WriteByte(config);
}

inline void SH1106_DS18B20_KEYS::DS18B20_CopyScratchPad(void)
{
  // Reset device
  DS18B20_Reset();
  usleep(1000);
  // Skip ROM Command
  DS18B20_WriteByte(DS18B20_SKIP_ROM);
  // copy scratch pad
  DS18B20_WriteByte(DS18B20_COPY_SCRATCHPAD);
  usleep(100000);
}

inline void SH1106_DS18B20_KEYS::DS18B20_ChangeSensorsResolution(int resolution)
{
  unsigned char config=0;
  switch(resolution)
  {
    case 9:
      config=0x1f;
      wait_time=93.75;
      break;
    case 10:
      config=0x3f;
      wait_time=187.5;
      break;
    case 11:
      config=0x5f;
      wait_time=375;
      break;
    default:
      config=0x7f;
      wait_time=750;
      break;
  }
  DS18B20_WriteScratchPad(0xff,0xff,config);
  usleep(1000);
  DS18B20_CopyScratchPad();
}

inline void SH1106_DS18B20_KEYS::DS18B20_ScanForSensor(void)
{
  unsigned long long ID=0ULL;
  int NextBit=64;
  int _NextBit;
  int rcode;
  int retry=0;
  unsigned long long _ID;
  unsigned char _ID_CRC;
  unsigned char _ID_Calc_CRC;

  while (retry<10)
  {
    _ID=ID;
    _NextBit=NextBit;
    rcode=DS18B20_SearchSensor(&_ID,&_NextBit);
    if(rcode==1)
    {
      _ID_CRC =  (unsigned char)  (_ID>>56);
      _ID_Calc_CRC =  DS18B20_CalcCRC((unsigned char *) &_ID,7);
      if(_ID_CRC == _ID_Calc_CRC)
      {
        ID=_ID;
        NextBit=_NextBit;
        *ptempbuff=ID;
      }
      else
        retry++;
    }
    else
      if (rcode==0)
        break;
      else
        retry++;
  }
}

// Adafruit   set_max_priority and set_default_priority add-on

inline void SH1106_DS18B20_KEYS::set_max_priority(void)
{
  struct sched_param sched;
  memset(&sched, 0, sizeof(sched));
  // Use FIFO scheduler with highest priority for the lowest chance of the kernel context switching.
  sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
  sched_setscheduler(0, SCHED_FIFO, &sched);
}

inline void SH1106_DS18B20_KEYS::set_default_priority(void)
{
  struct sched_param sched;
  memset(&sched, 0, sizeof(sched));
  // Go back to default scheduler with default 0 priority.
  sched.sched_priority = 0;
  sched_setscheduler(0, SCHED_OTHER, &sched);
}

// Function to read temperature into the temperature variable

void SH1106_DS18B20_KEYS::Read_Temperature(void)
{
    set_max_priority();
    DS18B20_GlobalStartConversion();
    double local_temperature = DS18B20_ReadSensor();
    temperature = local_temperature;
    set_default_priority();
}

// Relay control
void SH1106_DS18B20_KEYS::Relay_ON(void)
{
  if (!(relay_status))
  {
    bcm2835_gpio_write(relay, LOW);
    relay_status = 1;
  }
}

void SH1106_DS18B20_KEYS::Relay_OFF(void)
{
  if (relay_status)
  {
    bcm2835_gpio_write(relay, HIGH);
    relay_status = 0;
  }
}
