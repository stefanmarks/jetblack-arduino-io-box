/**
 * JetBlack IO Box
 *
 * @version 1.0 - 2012.11.14: Created
 * @author  Stefan Marks, Marcus Ball
 *
 *
 * Command set:
 * E             : Echo version number
 * La,b[,c[,d]]  : Set LED a brightness to b (00-99) (and blink interval to c, and blink ratio to d)
 * la            : Get brightness of LED a
 * T,x,y "string": Set text on LCD display, set to location row and column of where to start writing
 *                
 * 
 * Return value: "+" or value if command successful, "!" if an error occured
 */
 
#include <Wire.h>
#include "LED.h"
#include "Adafruit_MCP23017.h"
#include "Adafruit_RGBLCDShield.h"
 
 // version of the IO box
const char MODULE_VERSION[] = "1.0";

// array with LEDs
LED*      arrLEDs[] = {new AnalogLED(3), 
                       new AnalogLED(5), 
                       new AnalogLED(6), 
                       new AnalogLED(9), 
                       new AnalogLED(10), 
                       new AnalogLED(11), 
                       new DigitalLED(13)
                      };
const int numLEDs = sizeof(arrLEDs) / sizeof(arrLEDs[0]);

// lcd and text initialization
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
char incomingString[25]; // Set some space for the string (25 characters)
int selection = 0; 
int NEWLINE = 10;
//default row and column locations set to 0
int row = 0, column = 0;
boolean connectedLCD = false;

//Predefined #defines for LCD backlight color
#define BLACK 0x0
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7


// constants for communication
const char SUCCESS_CHAR = '+';
const char ERROR_CHAR   = '!';
const char CHAR_CR      = 13;
const char CHAR_LF      = 10;

// receive buffer
char       rxBuffer[128];
byte       rxBufferIdx = 0;
byte       rxReadIdx   = 0;
const byte rxBufferMax = sizeof(rxBuffer) / sizeof(rxBuffer[0]);

/**
 * Startup of the I/O box code.
 */
void setup()
{
  initializeLCD();
  // initialize serial communication at maximum bitrate
  Serial.begin(115200);
  connectedLCD = checkConnection();
}

/**
 * Main loop of the IO box code.
 */
void loop() 
{
  unsigned long time = millis();
  // update the LEDs
  for ( int i = 0 ; i < numLEDs ; i++ ) 
  {
    arrLEDs[i]->update(time);
  }
}


/**
 * This method is called whenever a byte over a the serial line is received.
 * It stores the bytes in the receive buffer
 * and triggers parsing of the recevied data when CR or LF is received.
 */
void serialEvent()
{
  while ( Serial.available() ) 
  {
    char rxIn = (char) Serial.read();
    rxBuffer[rxBufferIdx] = rxIn;
    
    // did we receive a CR or LF?
    if ( (rxIn == CHAR_LF) || (rxIn == CHAR_CR) )     
    {
      // look at what command it is
      switch ( readChar() )
      {
        // proper commands
        case 'E': processEchoCommand(); break;
        case 'l': processGetLedBrightnessCommand(); break;
        case 'L': processSetLedBrightnessCommand(); break;
        case 'T': 
                  if(connectedLCD)
                  {
                    processSetLcdTextCommand();
                  }
                  else
                  {
                    Serial.println("LCD is not connected");
                  }
                  break;
        
        // ignore extraneous bytes
        case CHAR_LF: break;
        case CHAR_CR: break;
        
        // everything else is wrong
        default : Serial.println(ERROR_CHAR); break;
      }
      // prepare for next command: reset read buffer
      rxBufferIdx = 0;
      rxReadIdx   = 0;
    }
    else if ( rxBufferIdx < rxBufferMax-1 )
    {
      // read a byte: advance read buffer index
      rxBufferIdx++;
    }    
  }
}

/**
 * Returns the number of characters available in the receive buffer.
 *
 * @return number of characters available
 */
byte charsAvailable()
{
  return rxBufferIdx - rxReadIdx;
}

/**
 * Reads the next character from the receive buffer
 * and advances the read pointer.
 *
 * @return next received character or '\0' if there is no next character
 */
char readChar()
{
  char c = '\0';
  // is there a next character?
  if ( charsAvailable() > 0 ) 
  {
    c = rxBuffer[rxReadIdx];
    rxReadIdx++;
  }
  return c;
}


/**
 * Reads the next integer value from the receive buffer
 * and advances the read pointer.
 *
 * @return next received integer or -1 if there is no next integer
 */
int readInt()
{
  int i = -1;
  boolean isNumeric;
  do 
  {
    char c = readChar();
    isNumeric = (c >= '0') && (c <= '9');
    
    if ( isNumeric )
    {
      // shift previous result one digit and add new digit
      if ( i < 0 )
      {
        i = 0;
      }
      i = (i * 10) + (c-'0');
    }
  } while ( isNumeric );
  
  return i;
}


/**
 * ECHO command was sent: return ID and serial number
 */
void processEchoCommand()
{
  Serial.print("JetBlack IO v"); 
  Serial.println(MODULE_VERSION);
}

/**
 * Gets LED brightness
 * la : a=LED number
 */
void processGetLedBrightnessCommand()
{
  int ledIdx = readInt(); // LED index is first parameter
  if ( (ledIdx >= 0) && (ledIdx < numLEDs) )
  {
    // return LED brightness
    Serial.println(arrLEDs[ledIdx]->getBrightness());
  }
  else
  {
    Serial.println(ERROR_CHAR);
  }
}

/**
 * Called to initialize LCD panel and set default text as JetBlack Simulator
 */
void initializeLCD()
{
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  //set the cursor to the 0 0 position, top left
  lcd.setCursor(0,0);
  //print JetBlck on the top line
  lcd.print("JetBlack");
  //move cursor to the bottom line
  lcd.setCursor(0,1);
  //Print Simulator
  lcd.print("Simulator");
  //Set the backlight to white
  lcd.setBacklight(WHITE); 
}

/**
 * Sets the text to display on the LCD panel
 * T: followed by the text to display
 */
void processSetLcdTextCommand()
{  
  char nextChar = readChar();
  boolean rowDone = false, columnDone = false;
  while(nextChar != '"' || charsAvailable <= 0)
  {
    if (!rowDone && charsAvailable() > 0 )
    {
      // sets the row Location on the LCD
      row = readInt();
      rowDone = true;
    }
    
    if (!columnDone && charsAvailable() > 0 )
    {
      // sets the column location on the LCD
      column = readInt(); 
      columnDone = true;
    }
    nextChar = readChar();
  }
  
    
    lcd.setCursor(column, row);
    
    //Now we check to see if any quotation marks are present
    //only print strings that begin with a quotation    
    if(nextChar == '"')
    {
      processText();
    }
    
}

/**
  * function to process the text to be displayed
  */
void processText()
{
  
  int index = 0;
  char incomingChar = '.';
  // While serial data is being received
  while (charsAvailable() > 0)
  {
    //only accept the first 25 characters (size of the string array)
    if(index < 24)
    {
      incomingChar = readChar();
      if(incomingChar == '"')
      {
        break;
      }
      //Store the incoming character into the string
      incomingString[index] = incomingChar;
      //Increment to next index in string array
      index++;
      // Null terminate the string
      incomingString[index] = '\0'; 
    }
  }
  if (index  > 0) 
  {  
    for(int i = 0; i < index; i++)
    {      
      if(incomingString[i] == NEWLINE)
      {
        lcd.setCursor(column , row++);
      }
      if(incomingString[i] == '\0')
      {
        break;
      }
      //lcd.setCursor(xLocation+i,yLocation);
      lcd.print(incomingString);
      //lcd.setBacklight(BLUE);
      incomingString[i] = '\0';
    }    
  } 
}

/**
  * checks the LCD's default i2c to see if it is connected
  */
boolean checkConnection() {
  Serial.println ();
  Serial.println ("Checking for LCD");
  byte count = 0;
  boolean found = false;
  
  Wire.begin();
  Wire.beginTransmission(32);
  if(Wire.endTransmission() == 0)
  {
    Serial.print("LCD Device connected at ");
    Serial.println(32, HEX);
    found = true;
  }
  
  if(!found)
  {
     Serial.println("LCD was not found"); 
  }
  return found;
}

 

/**
 * Sets the LED brightness (and optionally blink parameters).
 * La,b[,c[,d]] : a=LED number, b=brightness (0-99), c=blink interval in ms, d=blink ratio (0-99)
 */
void processSetLedBrightnessCommand()
{
  int ledIdx     = readInt(); // LED index is first parameter
  int brightness = readInt(); // LED brightness is second parameter
  if ( (ledIdx >= 0) && (ledIdx <= numLEDs) && (brightness >= 0) )
  {
    arrLEDs[ledIdx]->setBrightness(brightness);
    if ( charsAvailable() > 0 )
    {
      // optional blink interval in ms
      int interval = readInt();
      arrLEDs[ledIdx]->setBlinkInterval(interval);
    }
    if ( charsAvailable() > 0 )
    {
      // optional blink ratio in percent
      int ratio = readInt();
      arrLEDs[ledIdx]->setBlinkRatio(ratio);
    }
    Serial.println(SUCCESS_CHAR);
  }
  else
  {
    Serial.println(ERROR_CHAR);
  }
}

