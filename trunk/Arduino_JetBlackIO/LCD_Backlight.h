/**
 * Class declaration for the LCD backlight RGB LED.
 * 
 * @author  Stefan Marks
 * @version 1.0 - 2012.12.06: Created
 */
 
#ifndef LCD_BACKLIGHT_H_INCLUDED
#define LCD_BACKLIGHT_H_INCLUDED

#include "LED.h"
#include "Adafruit_RGBLCDShield.h"

class LCD_Backlight : public LED
{
  public:

    /**
     * Creates the LCD backlight LED object.
     *
     * @param pLCD the LCD module object
     */
    LCD_Backlight(Adafruit_RGBLCDShield* pLCD);


  // overridden methods

    virtual bool supportsColour();

    virtual void setColour(byte red, byte green, byte blue);

  private:
  
    virtual void updateLedState();

  private:
  
    Adafruit_RGBLCDShield* pLCD;
    byte                   red, green, blue;
};


#endif // LCD_BACKLIGHT_H_INCLUDED

