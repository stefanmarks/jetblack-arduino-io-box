/**
 * Digital button class implementation.
 *
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.22: Created
 * @version 1.1 - 2012.12.06: Modified to new button interface 
 */
 
#include "DigitalButton.h"

const unsigned long DEBOUNCE_TIME = 100; // time in ms to avoid bouncing contacts

DigitalButton::DigitalButton(byte pinNo) : Button()
{
  this->pinNo  = pinNo;
  state        = false;
  oldState     = false;
  numPresses   = 0;
  nextPollTime = 0;
  
  // prepare pin to output signal
  pinMode(pinNo, INPUT); 
}


boolean DigitalButton::isPressed()
{
  return state;
}

    
int DigitalButton::getNumPresses()
{
  byte retPresses = numPresses;
  numPresses = 0; // reset counter
  return retPresses;
}

    
void DigitalButton::update(unsigned long time)
{
  if ( time > nextPollTime ) // avoid bouncing contacts
  {
    state = (digitalRead(pinNo) == HIGH);
    if ( state != oldState ) 
    {
      if ( state )
      {
        numPresses++;
      }
      nextPollTime = time + DEBOUNCE_TIME;
      oldState = state;
    }
  }
}


