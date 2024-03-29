/**
 * Digital LED class implementation.
 *
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.14: Created
 */
 
#include "DigitalLED.h"

DigitalLED::DigitalLED(byte pinNo) : LED()
{
  this->pinNo = pinNo;

  // prepare pin to output signal
  pinMode(pinNo, OUTPUT); 

  updateLedState();
}


void DigitalLED::setBrightness(byte brightness)
{
  // no middle values, only on or off
  LED::setBrightness((brightness >= 50) ? 99 : 0);
}


void DigitalLED::updateLedState()
{
  digitalWrite(pinNo, ((brightness > 0) && state) ? HIGH : LOW);
}

