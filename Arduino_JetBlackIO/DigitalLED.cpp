/**
 * Class file for digital LED control
 *
 * @version 1.0 - 2012.11.14: Created
 * @author  Stefan Marks
 */
 
#include "LED.h"

DigitalLED::DigitalLED(byte pinNo) : LED(pinNo)
{
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

