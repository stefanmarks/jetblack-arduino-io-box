/**
 * Class implementation for the LCD backlight RGB LED.
 *
 * @author  Stefan Marks
 * @version 1.0 - 2012.12.06: Created
 */
 
#include "LCD_Backlight.h"

// Predefined constants for LCD backlight color
#define LCD_BLACK  0x0
#define LCD_RED    0x1
#define LCD_YELLOW 0x3
#define LCD_GREEN  0x2
#define LCD_TEAL   0x6
#define LCD_BLUE   0x4
#define LCD_VIOLET 0x5
#define LCD_WHITE  0x7


LCD_Backlight::LCD_Backlight(Adafruit_RGBLCDShield* pLCD) : LED()
{
  this->pLCD = pLCD;

  red   = 0;
  green = 0;
  blue  = 0;
  
  updateLedState();
}


bool LCD_Backlight::supportsColour()
{
  return true;
}


void LCD_Backlight::setColour(byte red, byte green, byte blue)
{
  this->red   = constrain(red,   0, 99);
  this->green = constrain(green, 0, 99);
  this->blue  = constrain(blue,  0, 99);
  updateLedState();
}


void LCD_Backlight::updateLedState()
{
  if ( state )
  {
    // create bitmask out of RGB values
    // 5000: to avoid division by 100 to include brightness
    pLCD->setBacklight(((red   * brightness >= 5000) ? LCD_RED   : 0) |
                       ((green * brightness >= 5000) ? LCD_GREEN : 0) |
                       ((blue  * brightness >= 5000) ? LCD_BLUE  : 0)
                      );
  }
  else
  {
    pLCD->setBacklight(LCD_BLACK);
  }
}

