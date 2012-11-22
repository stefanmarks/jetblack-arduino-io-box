/**
 * Abstract base class for buttons.
 * 
 * @version 1.0 - 2012.11.22: Created
 * @author  Stefan Marks
 */
 
#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include "Arduino.h"

class Button
{
  public:
  
    /**
     * Checks if the button is currently pressed.
     * 
     * @return <code>true</code> if the button is pressed, <code>false</code> if not
     */
    virtual boolean isPressed() = 0;
    
    /**
     * Checks if the button state has changed since the last hasChanged() call.
     * 
     * @return <code>true</code> if the button state has changed, <code>false</code> if not
     */
    virtual boolean hasChanged() = 0;
    
    /**
     * This method needs to be called inside the main loop with the current millis() result
     * to allow for time-controlled events and control to function properly.
     *
     * @param time the current result of the millis() function
     */
    virtual void update(unsigned long time) = 0;

  protected:
  
    /**
     * Creates a generic button object.
     */
    Button();
  
};


#endif // BUTTON_H_INCLUDED

