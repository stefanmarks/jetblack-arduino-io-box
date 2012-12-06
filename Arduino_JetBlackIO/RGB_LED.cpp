/**
 * RGB LED class implementation.
 *
 * @author  Stefan Marks
 * @version 1.0 - 2012.11.23: Created
 */
 
#include "RGB_LED.h"

RGB_LED::RGB_LED(LED* pLEDred, LED* pLEDgreen, LED* pLEDblue) : LED()
{
  this->pLEDred   = pLEDred;
  this->pLEDgreen = pLEDgreen;
  this->pLEDblue  = pLEDblue;

  red   = 0;
  green = 0;
  blue  = 0;
  
  updateLedState();
}


bool RGB_LED::supportsColour()
{
  return true;
}


void RGB_LED::setColour(byte red, byte green, byte blue)
{
  this->red   = constrain(red,   0, 99);
  this->green = constrain(green, 0, 99);
  this->blue  = constrain(blue,  0, 99);
  updateLedState();
}


void RGB_LED::setBlinkInterval(unsigned int interval)
{
  // set all components synchronously
  if ( pLEDred   != NULL ) pLEDred->setBlinkInterval(interval);
  if ( pLEDgreen != NULL ) pLEDgreen->setBlinkInterval(interval);
  if ( pLEDblue  != NULL ) pLEDblue->setBlinkInterval(interval);
  LED::setBlinkInterval(interval);
}


void RGB_LED::setBlinkRatio(byte ratio)
{
  // set all components synchronously
  if ( pLEDred   != NULL ) pLEDred->setBlinkRatio(ratio);
  if ( pLEDgreen != NULL ) pLEDgreen->setBlinkRatio(ratio);
  if ( pLEDblue  != NULL ) pLEDblue->setBlinkRatio(ratio);
  LED::setBlinkRatio(ratio);
}


void RGB_LED::update(unsigned long /* time */)
{
  // do nothing here, LED's are updated by themselves
}


void RGB_LED::updateLedState()
{
  if ( pLEDred   != NULL ) pLEDred->setBrightness(  (int) red   * brightness / 99);
  if ( pLEDgreen != NULL ) pLEDgreen->setBrightness((int) green * brightness / 99);
  if ( pLEDblue  != NULL ) pLEDblue->setBrightness( (int) blue  * brightness / 99);
}

