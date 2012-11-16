/*
  JetBlack IO Box
  Command set:
  E   : Echo version number
  Bxyy: Set LED x brightness to y (00-99)
  bx  : Get brightness of LED x
  
  Return value: "+" or value if command successful, "!" if an error occured
 */
 
const char MODULE_VERSION[] = "0.2";

const char SUCCESS_CHAR = '+';
const char ERROR_CHAR   = '!';
const char CHAR_CR      = 13;
const char CHAR_LF      = 10;

byte          arrLED_Pins[]       = {3, 5, 6, 9, 13};  // Array with pin numbers of the specific LEDs
byte          arrLED_Brightness[] = {0, 0, 0, 0,  0};  // Array with LED brightnesses
unsigned long arrLED_OffTime[]    = {0, 0, 0, 0,  1};  // Array with the OFF timestamp (0: PWM output)
const int     maxLED              = sizeof(arrLED_Pins);

int           iLED_PulseIterval   = 10;   // PWM interval in ms
unsigned long nextPWM_Time        = 0;    // next timer value for PWM

// the setup routine runs once when you press reset:
void setup()
{
  // initialize the LED pins as an outputs
  for ( byte i = 0 ; i < maxLED ; i++ ) 
  {
    pinMode(arrLED_Pins[i], OUTPUT);    
  }
  
  // initialize serial communication at maximum bitrate
  Serial.begin(115200);
}

// the loop routine runs over and over again forever:
void loop() 
{
  unsigned long currentTime = millis();
 
  // check if next PWM interval is due
  if ( currentTime > nextPWM_Time ) 
  {
    // yes: calculate next PWM interval time
    nextPWM_Time = currentTime + iLED_PulseIterval;
    // calculate LED off-times
    for ( byte i = 0 ; i < maxLED ; i++ )
    {
      if ( arrLED_OffTime[i] > 0 )
      {
        arrLED_OffTime[i] = currentTime + (iLED_PulseIterval * arrLED_Brightness[i] / 99);
        if ( arrLED_Brightness[i] > 0 )
        {
          // only switch LED on when brightness is > 0
          digitalWrite(arrLED_Pins[i], HIGH);
        }
      }
      else
      {
        // this is a PWM output: just set the analog value
        analogWrite(arrLED_Pins[i], arrLED_Brightness[i] * 255 / 99);
      }
    }
  }
  else 
  {
    // check if LEDs can be turned off
    for ( byte i = 0 ; i < maxLED ; i++ )
    {
      if ( (arrLED_OffTime[i] > 0) && (currentTime >= arrLED_OffTime[i]) )
      {
        digitalWrite(arrLED_Pins[i], LOW);
      }
    }
  }  
}

char       rxBuffer[16];
byte       rxBufferIdx = 0;
const byte rxBufferMax = sizeof(rxBuffer);

// In case of a serial command, this method is executed
void serialEvent()
{
  char rxIn = 0;
  while ( Serial.available() ) 
  {
    rxIn = (char) Serial.read();
    rxBuffer[rxBufferIdx] = rxIn;
    
    // did we receive a CR 
    if ( (rxIn == CHAR_LF) || (rxIn == CHAR_CR) )     
    {
      // look at what command it is
      switch ( rxBuffer[0] )
      {
        case 'E': processEchoCommand(); break;
        case 'b': processGetBrightnessCommand(); break;
        case 'B': processSetBrightnessCommand(); break;
        default : Serial.println(ERROR_CHAR); break;
      }
      // prepare for next command
      rxBufferIdx = 0;
    }
    else if ( rxBufferIdx < rxBufferMax-1 )
    {
      rxBufferIdx++;
    }    
  }
}

// ECHO command was sent: return ID and serial number
void processEchoCommand()
{
  Serial.print("JetBlack IO v"); 
  Serial.println(MODULE_VERSION);
}

// Get LED brightness command (x: x=LED number)
void processGetBrightnessCommand()
{
  if ( rxBufferIdx >= 2 )
  {
    // parse LED idx
    byte ledIdx = constrain(rxBuffer[1] - '0', 0, maxLED);
    // return LED brightness
    Serial.println(arrLED_Brightness[ledIdx]);
  }
  else
  {
    Serial.println(ERROR_CHAR);
  }
}

// Set LED brightness command (xyy: x=LED number, yy=brightness as decimal number 00-99)
void processSetBrightnessCommand()
{
  if ( rxBufferIdx >= 4 )
  {
    // parse LED idx
    byte ledIdx = constrain(rxBuffer[1] - '0', 0, maxLED);
    // parse LED brightness
    arrLED_Brightness[ledIdx] = constrain((rxBuffer[2] - '0') * 10 + (rxBuffer[3] - '0'), 0, 99);
    Serial.println(SUCCESS_CHAR);
  }
  else
  {
    Serial.println(ERROR_CHAR);
  }
}

