/*
    MIT License

    Copyright (c) 2017-2018, Alexey Dynda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#include "ssd1306.h"
//#include "nano_gfx.h"
#include "sova.h"
#include "logo_improflex_table.h"

// Definitions of the three skrew terminals on the board 
#define NOPIN 0xFF
#define HWB_PIN 31
#define  TERMINALCOUNT  3
#define  POSCOUNT  10
uint8_t terminalPins[TERMINALCOUNT][POSCOUNT] = {
{17, 15, 16, 14, 8, 9, 10, 11, 5, NOPIN},
{13, 3, 2, 0, 1, 4, 30, 12, 6, NOPIN},
{NOPIN, NOPIN, 18, 19, 20, 21, 22, 23, 7, HWB_PIN}};

uint8_t terminalStatus[TERMINALCOUNT][POSCOUNT];

const int buttonPin = 7;     // the number of the pushbutton pin
const int txLedPin = 30;
const int rxLedPin = 17;

static byte buttonPressedCnt = 0;
uint32_t nextTime = 0;

void setAllTerminalPins(uint8_t mode)
{
  uint8_t terminal;
  uint8_t pos;
  uint8_t pin;
  for(terminal = 0; terminal < TERMINALCOUNT; terminal++)
  {
    for(pos = 0; pos < POSCOUNT; pos++)
    {
      pin = terminalPins[terminal][pos];
      if ((pin != NOPIN) && (pin != HWB_PIN))
      {
        pinMode(pin, mode);
      }
      else if (pin == HWB_PIN)
      {
        if(mode == INPUT)
        {
          DDRE &= 0xFB; //pinMode PE2 INPUT
        }
        else if (mode == OUTPUT)
        {
          DDRE |= 0x04;
        }
      }
    }
  }
}

void terminalInputStatus()
{
  uint8_t terminal;
  uint8_t pos;
  uint8_t pin;
  for(terminal = 0; terminal < TERMINALCOUNT; terminal++)
  {
    for(pos = 0; pos < POSCOUNT; pos++)
    {
      pin = terminalPins[terminal][pos];
      if ((pin != NOPIN) && (pin != HWB_PIN))
      {
        terminalStatus[terminal][pos] = digitalRead(pin);
      }
      else
      {
        terminalStatus[terminal][pos] = pin;
      }
    }
  }
}

static void bitmapDemo()
{
  ssd1306_drawBitmap(0, 0, 128, 64, Sova);
  delay(500);
  ssd1306_invertMode();
  delay(500);
  ssd1306_normalMode();
}

static void bitmapImpro()
{
  ssd1306_clearScreen();
  ssd1306_drawBitmap(0, 0, 128, 25, Logo_Improflex);
  ssd1306_printFixed(1,  24, "Technical Consultants", STYLE_NORMAL);
  ssd1306_printFixed(20,  48, "Jan Finnstrom", STYLE_NORMAL);
}


static void textDemo()
{
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen();
  ssd1306_printFixed(0,  8, "Normal text", STYLE_NORMAL);
  ssd1306_printFixed(0, 16, "Bold text", STYLE_BOLD);
  ssd1306_printFixed(0, 24, "Italic text", STYLE_ITALIC);
  ssd1306_negativeMode();
  ssd1306_printFixed(0, 32, "Inverted bold", STYLE_BOLD);
  ssd1306_positiveMode();
  delay(500);
  ssd1306_clearScreen();
  ssd1306_printFixedN(0, 0, "N3", STYLE_NORMAL, FONT_SIZE_8X);
  delay(500);
}

void moreTextDemos()
{
  ssd1306_clearScreen();
  ssd1306_setCursor(0, 0);
  ssd1306_print("Pos 0,0");
  ssd1306_print("New print");
  ssd1306_printFixed(0, 8, "J1 1 2 3 4 5 6 7 8 9", STYLE_NORMAL);
  ssd1306_printFixed(0, 16, "12345 ", STYLE_NORMAL);
  ssd1306_setCursor(0, 32);
  ssd1306_print("Rad 32 ");
  ssd1306_print("New print");

}

void printDigitalInput()
{
  uint8_t pos;
  uint8_t pin;
  ssd1306_clearScreen();
  ssd1306_printFixed(0, 0, "J1 1 2 3 4 5 6 7 8 9", STYLE_NORMAL);
  ssd1306_setCursor(0, 16);
     for(pos = 0; pos < POSCOUNT; pos++)
    {
      pin = terminalPins[0][pos];
      if ((pin != NOPIN) && (pin != HWB_PIN))
      {
        if(terminalStatus[0][pos] == 0)
        {
          ssd1306_print("L "); 
        }
        else
        {
          ssd1306_print("H "); 
        }
      }
    }
 }


static void drawLinesDemo()
{
  ssd1306_clearScreen();
  for (uint8_t y = 0; y < ssd1306_displayHeight(); y += 8)
  {
    ssd1306_drawLine(0, 0, ssd1306_displayWidth() - 1, y);
  }
  for (uint8_t x = ssd1306_displayWidth() - 1; x > 7; x -= 8)
  {
    ssd1306_drawLine(0, 0, x, ssd1306_displayHeight() - 1);
  }
  delay(500);
}

static void buttonISR(void)
{
  buttonPressedCnt++;
}

void setup()
{
  int i;

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(txLedPin, OUTPUT);
  pinMode(rxLedPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, RISING);

  /* Select the font to use with menu and all font functions */
  ssd1306_setFixedFont(ssd1306xled_font6x8);

  ssd1306_128x64_i2c_init();
  //    ssd1306_128x64_spi_init(3,4,5);     // Use this line for Atmega328p (3=RST, 4=CE, 5=D/C)
  //    ssd1306_128x64_spi_init(24, 0, 23); // Use this line for Raspberry  (gpio24=RST, 0=CE, gpio23=D/C)
  //    ssd1306_128x64_spi_init(22, 5, 21); // Use this line for ESP32 (VSPI)  (gpio22=RST, gpio5=CE for VSPI, gpio21=D/C)

  ssd1306_fillScreen( 0x00 );

  for (i = 200; i > 0; i -= 20)
  {
    digitalWrite(txLedPin, LOW);  //Turn ON TxLED
    digitalWrite(rxLedPin, HIGH);  //Turn OFF RxLED
    delay(i);
    digitalWrite(txLedPin, HIGH);  //Turn OFF TxLED
    digitalWrite(rxLedPin, LOW);  //Turn ON RxLED
    delay(i);
  }
  digitalWrite(rxLedPin, HIGH);  //Turn OFF RxLED
  nextTime = millis() + 500;
}

void loop()
{
  enum runState{ShowLogoState, ReadDigitalInState, ReadAnalogInState, ShowBitMapState};
  static byte currentRunState = 1;
  static bool invMode = false;
/* If button have been pressed, wait for switch bounces to settle and move to next run state */
  if (buttonPressedCnt > 0)
  {
    while (buttonPressedCnt > 0)
    {
      buttonPressedCnt = 0;
      delay(50);
    }
    runState++;
  }

/* Invert display every 2 second */
  if ( nextTime < millis())
  {
    nextTime = millis() + 2000;
    if (invMode)
    {
      ssd1306_invertMode();
      invMode = false;
    }
    else
    {
      ssd1306_normalMode();
      invMode = true;
    }
  }

  if ( currentRunState != runState)
  {
    currentRunState = runState;
    switch (runState)
    {
      case 0:
        bitmapImpro();
        break;

      case 1:
        terminalInputStatus();
        printDigitalInput();
        break;

      case 2:
        moreTextDemos();
        break;

      case 3:
        drawLinesDemo();
        break;

      case 4:
        bitmapDemo();
        break;

      default:
        runState = 0;
        break;
    }
  }
}
