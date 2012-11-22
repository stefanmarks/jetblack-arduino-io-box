/**
 * Class file for LED control.
 * 
 * @version 1.0 - 2012.11.14: Created
 * @author  Stefan Marks
 */
 
#ifndef LED_H_INCLUDED
#define LED_H_INCLUDED

#include "Arduino.h"

/**
 * Abstract base class for LED's connected to the board.
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


/**
 * Class for LED's connected to purely digital pins of the board.
 * Digitally driven LED's cannot be set to intermediate brightnesses.
 * A brightness value >= 50 is interpreted as "on", otherwise as "off".
 */
class DigitalLED : public LED
{
  public:
  
    /**
     * Creates a digital LED class for a specific I/O pin.
     *
     * @param pinNo the number of the Arduino pin to use for this LED
     */
    DigitalLED(byte pinNo);
    
    virtual void setBrightness(byte brightness);

  private:
  
    virtual void updateLedState();

  private:
  
    byte pinNo;

};


/**
 * Class for LED's connected to analog pins of the board.
 */
class AnalogLED : public LED
{
  public:

    /**
     * Creates an analog LED class for a specific I/O pin.
     *
     * @param pinNo the number of the Arduino pin to use for this LED
     */
    AnalogLED(byte pinNo);

  private:
  
    virtual void updateLedState();

  private:
  
    byte pinNo;

};

#endif // LED_H_INCLUDED

