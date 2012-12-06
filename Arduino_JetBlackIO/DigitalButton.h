/**
 * Class declaration for digital buttons directly connected to the Arduino chip.
 * 
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.22: Created
 * @version 1.1 - 2012.12.06: Modified to new button interface
 */
 
#ifndef DIGITAL_BUTTON_H_INCLUDED
#define DIGITAL_BUTTON_H_INCLUDED

#include "Button.h"

class DigitalButton : public Button
{
  public:
  
    /**
     * Creates a digital button class for a specific I/O pin.
     *
     * @param pinNo the number of the Arduino pin to use for this button
     */
    DigitalButton(byte pinNo);
    
    virtual boolean isPressed();
    
    virtual int getNumPresses();
    
    virtual void update(unsigned long time);

  private:
  
    byte          pinNo, numPresses;
    boolean       state, oldState;
    unsigned long nextPollTime;
};

#endif // DIGITAL_BUTTON_H_INCLUDED

