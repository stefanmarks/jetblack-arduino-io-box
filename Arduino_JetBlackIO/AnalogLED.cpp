/**
 * Analog LED class implementation.
 *
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.14: Created
 */
 
#include "AnalogLED.h"

AnalogLED::AnalogLED(byte pinNo) : LED()
{
  this->pinNo = pinNo;

  // prepare pin to output signal
  pinMode(pinNo, OUTPUT); 

  updateLedState();
}


void AnalogLED::updateLedState()
{
  if ( state ) 
  {
    analogWrite(pinNo, (int) brightness * 255 / 99);
  }
  else
  {
    analogWrite(pinNo, 0);
  } 
}
