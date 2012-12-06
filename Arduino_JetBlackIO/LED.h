/**
 * Abstract base class declaration for LEDs.
 * 
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.14: Created
 */
 
#ifndef LED_H_INCLUDED
#define LED_H_INCLUDED

#include "Arduino.h"

/**
 * Abstract base class for LEDs connected to the board.
 */
class LED
{
  public:
  
    /**
     * Gets the brightness of an LED.
     * 
     * @return brightness (0: off, 99:fully lit)
     */
    virtual byte getBrightness();
    
    /**
     * Gets the brightness of an LED at the current point in time, including blinking.
     * 
     * @return brightness (0: off, 99:fully lit)
     */
    virtual byte getCurrentBrightness();
    
    /**
     * Sets the brightness of an LED.
     *
     * @param brightness the new brightness of the LED (0: off, 99: fully lit)
     */
    virtual void setBrightness(byte brightness);

    /**
     * Gets the LED blink interval.
     * 
     * @return blink interval in milliseconds (0: no blinking)
     */
    virtual unsigned int getBlinkInterval();
    
    /**
     * Sets the LED blink interval.
     * 
     * @param interval blink interval in milliseconds (0: no blinking)
     */
    virtual void setBlinkInterval(unsigned int interval);
    
    /**
     * Gets the LED blink ratio.
     * 
     * @return on/off blink ratio in percent
     */
    virtual byte getBlinkRatio();

    /**
     * Sets the LED blink ratio.
     * 
     * @param ratio on/off blink ratio in percent
     */
    virtual void setBlinkRatio(byte ratio);

	/**
	 * Checks if the LED supports multiple colours.
	 *
	 * @return <code>true</code> if the LED supports multiple colours,
	 *         <code>false</code> if not
	 */
	virtual bool supportsColour();
	
	/**
     * Sets the colour of the LED.
     *
     * @param red   the red   colour component (0-99)
     * @param green the green colour component (0-99)
     * @param blue  the blue  colour component (0-99)
     */
    virtual void setColour(byte red, byte green, byte blue);
	
    /**
     * This method needs to be called inside the main loop with the current millis() result
     * to allow for time-controlled events and control to function properly.
     *
     * @param time the current result of the millis() function
     */
    virtual void update(unsigned long time);

  protected:
  
    /**
     * Creates a generic LED class.
     */
    LED();
    
  private:
  
    /**
     * Sets the actual LED output pin state
     */
    virtual void updateLedState() = 0;
    
  protected:
  
    byte          brightness;
    unsigned int  interval;
    byte          ratio;
    boolean       state;
    unsigned long onTime, offTime;
};


#endif // LED_H_INCLUDED

