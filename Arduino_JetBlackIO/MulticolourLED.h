/**
 * Class for multicolour LEDs connected to analog pins of the board.
 * 
 * @version 1.0 - 2012.11.22: Created
 * @author  Stefan Marks
 */
 
#ifndef MULTICOLOUR_LED_H_INCLUDED
#define MULTICOLOUR_LED_H_INCLUDED

#include "LED.h"

class MulticolourLED : public LED
{
  public:

    /**
     * Creates a multicolour LED class for three separate LEDs.
     *
     * @param pLEDred   the LED object that is controlling the red component
     * @param pLEDgreen the LED object that is controlling the green component
     * @param pLEDblue  the LED object that is controlling the green component
     */
    MulticolourLED(LED* pLEDred, LED* pLEDgreen, LED* pLEDblue);

    /**
     * Sets the colour of the LED.
     *
     * @param red   the red   colour component (0-99)
     * @param green the green colour component (0-99)
     * @param blue  the blue  colour component (0-99)
     */
    void setColour(byte red, byte green, byte blue);

    /**
     * Sets the LED blink interval.
     * 
     * @param interval blink interval in milliseconds (0: no blinking)
     */
    virtual void setBlinkInterval(unsigned int interval);
    
    /**
     * Sets the LED blink ratio.
     * 
     * @param ratio on/off blink ratio in percent
     */
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


#endif // MULTICOLOUR_LED_H_INCLUDED

