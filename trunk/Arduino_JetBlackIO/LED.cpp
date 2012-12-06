/**
 * LED base class implementation.
 *
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.14: Created
 */
 
#include "LED.h"

LED::LED()
{
  // initialise variables
  brightness = 0;
  interval   = 0;
  ratio      = 50;
  onTime     = 0;
  offTime    = 0;
  state      = true;
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


bool LED::supportsColour()
{
  return false; // default case: unicolour
}


void LED::setColour(byte /*red*/, byte /*green*/, byte /*blue*/)
{
  // do nothing here
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


