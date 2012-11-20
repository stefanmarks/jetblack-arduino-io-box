/**
 * Abstract base class for LED control
 *
 * @version 1.0 - 2012.11.14: Created
 * @author  Stefan Marks
 */
 
#include "LED.h"

LED::LED(byte pinNo)
{
  this->pinNo = pinNo;
  
  // initialise variables
  brightness = 0;
  interval   = 0;
  ratio      = 50;
  onTime     = 0;
  offTime    = 0;
  state      = true;

  // prepare pin to output signal
  pinMode(pinNo, OUTPUT); 
}


byte LED::getBrightness()
{
  return brightness;
}


byte LED::getCurrentBrightness()
{
  return state ? brightness : 0;
}


void LED::setBrightness(byte brightness)
{
  this->brightness = constrain(brightness, 0, 99);
  updateLedState();
}


unsigned int LED::getBlinkInterval()
{
  return interval;
}


void LED::setBlinkInterval(unsigned int interval)
{
  this->interval = interval;
  if ( interval == 0 )
  {
    // no more blinking: activate LED
    state = true;
    updateLedState();
  }
  else
  {
    // blinking: force recalculation NOW
    onTime  = 0;
    offTime = 0;
  }
}


byte LED::getBlinkRatio()
{
  return ratio;
}


void LED::setBlinkRatio(byte ratio)
{
  this->ratio = constrain(ratio, 1, 99);
}


void LED::update(unsigned long time)
{
  if ( interval > 0 )
  {
    if ( time >= onTime )
    {
      offTime = time + (interval * ratio / 100);
      onTime  = time + interval;
      state   = true;
      updateLedState();
    }
    else if ( time >= offTime )
    {
      state = false;
      updateLedState();
    }
  }
}

