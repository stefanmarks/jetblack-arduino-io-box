/**
 * Class file for digital buttons directly connected to the Arduino chip.
 * 
 * @version 1.0 - 2012.11.22: Created
 * @author  Stefan Marks
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
    
    virtual boolean hasChanged();
    
    virtual void update(unsigned long time);

  private:
  
    byte    pinNo;
    boolean state, oldState, changed;
};

#endif // DIGITAL_BUTTON_H_INCLUDED

