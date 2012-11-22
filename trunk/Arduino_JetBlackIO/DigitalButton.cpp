/**
 * Digital button class implementation.
 *
 * @version 1.0 - 2012.11.22: Created
 * @author  Stefan Marks
 */
 
#include "DigitalButton.h"

DigitalButton::DigitalButton(byte pinNo) : Button()
{
  this->pinNo = pinNo;
  state    = false;
  oldState = false;
  changed  = false;
  
  // prepare pin to output signal
  pinMode(pinNo, INPUT); 

  update(0); 
  changed = false;
}


boolean DigitalButton::isPressed()
{
  return state;
}

    
boolean DigitalButton::hasChanged()
{
  boolean retChanged = changed;
  changed = false;
  return retChanged;
}

    
void DigitalButton::update(unsigned long /* time */)
{
  oldState = state;
  state    = (digitalRead(pinNo) == HIGH);
  if ( state != oldState )
  {
    changed = true;
  }
}


