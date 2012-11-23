/**
 * Multicolour LED class implementation.
 *
 * @version 1.0 - 2012.11.23: Created
 * @author  Stefan Marks
 */
 
#include "MulticolourLED.h"

MulticolourLED::MulticolourLED(LED* pLEDred, LED* pLEDgreen, LED* pLEDblue) : LED()
{
  this->pLEDred   = pLEDred;
  this->pLEDgreen = pLEDgreen;
  this->pLEDblue  = pLEDblue;

  red   = 0;
  green = 0;
  blue  = 0;
  
  updateLedState();
}


void MulticolourLED::setColour(byte red, byte green, byte blue)
{
  this->red   = constrain(red,   0, 99);
  this->green = constrain(green, 0, 99);
  this->blue  = constrain(blue,  0, 99);
  updateLedState();
}


void MulticolourLED::setBlinkInterval(unsigned int interval)
{
  // set all components synchronously
  if ( pLEDred   != NULL ) pLEDred->setBlinkInterval(interval);
  if ( pLEDgreen != NULL ) pLEDgreen->setBlinkInterval(interval);
  if ( pLEDblue  != NULL ) pLEDblue->setBlinkInterval(interval);
  LED::setBlinkInterval(interval);
}


void MulticolourLED::setBlinkRatio(byte ratio)
{
  // set all components synchronously
  if ( pLEDred   != NULL ) pLEDred->setBlinkRatio(ratio);
  if ( pLEDgreen != NULL ) pLEDgreen->setBlinkRatio(ratio);
  if ( pLEDblue  != NULL ) pLEDblue->setBlinkRatio(ratio);
  LED::setBlinkRatio(ratio);
}


void MulticolourLED::update(unsigned long /* time */)
{
  // do nothing here, LED's are updated by themselves
}


void MulticolourLED::updateLedState()
{
  if ( pLEDred   != NULL ) pLEDred->setBrightness(  (int) red   * brightness / 99);
  if ( pLEDgreen != NULL ) pLEDgreen->setBrightness((int) green * brightness / 99);
  if ( pLEDblue  != NULL ) pLEDblue->setBrightness( (int) blue  * brightness / 99);
}

