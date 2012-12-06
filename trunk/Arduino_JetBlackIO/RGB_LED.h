/**
 * Class declaration for RGB LEDs connected to analog pins of the board.
 * 
 * @author  Stefan Marks
 * @version 1.0 - 2012.12.05: Created
 */
 
#ifndef RGB_LED_H_INCLUDED
#define RGB_LED_H_INCLUDED

#include "LED.h"

class RGB_LED : public LED
{
  public:

    /**
     * Creates a multicolour LED class for three separate LEDs.
     *
     * @param pLEDred   the LED object that is controlling the red component
     * @param pLEDgreen the LED object that is controlling the green component
     * @param pLEDblue  the LED object that is controlling the blue component
     */
    RGB_LED(LED* pLEDred, LED* pLEDgreen, LED* pLEDblue);

  // overridden methods
  
    virtual bool supportsColour();
	
    virtual void setColour(byte red, byte green, byte blue);

    virtual void setBlinkInterval(unsigned int interval);
    
    virtual void setBlinkRatio(byte ratio);
    
    virtual void update(unsigned long time);

  private:
  
    virtual void updateLedState();

  private:
  
    LED* pLEDred;
    LED* pLEDgreen;
    LED* pLEDblue;
    byte red, green, blue;
};


#endif // RGB_LED_H_INCLUDED

