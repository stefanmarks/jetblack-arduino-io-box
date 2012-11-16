/**
 * Class file for analog LED control
 *
 * @version 1.0 - 2012.11.14: Created
 * @author  Stefan Marks
 */
 
#include "LED.h"

AnalogLED::AnalogLED(byte pinNo) : LED(pinNo)
{
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
