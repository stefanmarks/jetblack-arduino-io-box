/**
 * Class declaration for LEDs connected to analog pins of the board.
 * 
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.22: Created
 */
 
#ifndef ANALOG_LED_H_INCLUDED
#define ANALOG_LED_H_INCLUDED

#include "LED.h"

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


#endif // ANALOG_LED_H_INCLUDED

