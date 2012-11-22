/**
 * Class for LEDs connected to purely digital pins of the board.
 * Digitally driven LEDs cannot be set to intermediate brightnesses.
 * A brightness value >= 50 is interpreted as "on", otherwise as "off".
 * 
 * @version 1.0 - 2012.11.22: Created
 * @author  Stefan Marks
 */
 
#ifndef DIGITAL_LED_H_INCLUDED
#define DIGITAL_LED_H_INCLUDED

#include "LED.h"

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


#endif // DIGITAL_LED_H_INCLUDED

