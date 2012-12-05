/**
 * JetBlack IO Box
 *
 * @author  Stefan Marks, Marcus Ball
 * @version 1.0 - 2012.11.14: - Created
 * @version 1.1 - 2012.11.16: - Merged LED and LCD projects
 * @version 1.2 - 2012.11.20: - Restructured the code, removed unnecessary or duplicate variables
 *                            - Changed LCD variable to pointer which is NULL if no LCD is connected
 *                            - Added handshake characters to text command
 *                            - Added methods for checking parameters and reading string parameters
 * @version 1.3 - 2012.11.22: - Added big numeric characters
 *                            - Refactored LED class
 * @version 1.4 - 2012.11.23: - Added button command
 *                            - Refactored big numbers code
 *                            - Added multicolour LEDs
 * @version 1.5 - 2012.12.05: - Bugfixes in the receive routine and the multicolour LEDs
 *                            - Cursor position column parameter optional
 *
 * Command set:
 * E               : Echo version number
 * ba              : Get state of button a (00:off, no change / 10: on, no change / 01:off, changed / 11:on, changed)
 * Ln,b[,i[,r]]    : Set LED n brightness to b (00-99) (and blink interval to i, and blink ratio to r)
 * ln              : Get brightness of LED n
 * Mn,r,g,b[,i[,r]]: Set multicolour LED n colour to r,g,b (00-99) (and blink interval to i, and blink ratio to r)
 * T"string"       : Set text on LCD display, the string to be displayed must be enclosed with quotation marks               
 * Nx              : Displays large numerical text on the LCD where x is the number to be displayed
 * Pr[,c]          : Sets the row r [and column c] for the cursor
 *
 * Return value: "+" or value if command successful, "!" if an error occured
 */
 
#include <Wire.h>
#include "DigitalButton.h"
#include "DigitalLED.h"
#include "AnalogLED.h"
#include "MulticolourLED.h"
#include "Adafruit_MCP23017.h"
#include "Adafruit_RGBLCDShield.h"

// version of the IO box
const char MODULE_NAME[]    = "JetBlack IO-Box";
const char MODULE_VERSION[] = "v1.5";

// macro for the size of an array
#define ARRSIZE(x) (sizeof(x) / sizeof(x[0] ))
 
// array with LEDs
LED* arrLEDs[] = {
  new AnalogLED(3), 
  new AnalogLED(5), 
  new AnalogLED(6), 
  NULL, // LED 3 will be Multicolour LED 0
  new AnalogLED(9), 
  new AnalogLED(10), 
  new AnalogLED(11), 
  NULL, // LED 7 will be Multicolour LED 1
  new DigitalLED(13), // LED on the board
  NULL 
};

// array with multicolour LEDs
MulticolourLED* arrMulticolourLEDs[] = {
  NULL,
  NULL,
  NULL,
  new MulticolourLED(arrLEDs[0], arrLEDs[1], arrLEDs[2]), 
  NULL,
  NULL,
  NULL,
  new MulticolourLED(arrLEDs[4], arrLEDs[5], arrLEDs[6]),
  NULL,
  NULL
};

// array with buttons
Button* arrButtons[] = { 
  new DigitalButton(2), 
  new DigitalButton(4), 
  new DigitalButton(7)
};

// LCD and text initialization
Adafruit_RGBLCDShield* pLCD = NULL; // if NO LCD is connnected, pLCD stays null

// Predefined constants for LCD backlight color
#define LCD_BLACK  0x0
#define LCD_RED    0x1
#define LCD_YELLOW 0x3
#define LCD_GREEN  0x2
#define LCD_TEAL   0x6
#define LCD_BLUE   0x4
#define LCD_VIOLET 0x5
#define LCD_WHITE  0x7


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

// the 8 arrays that form each segment of the large custom numbers
byte bigNumberSegments[][8] = { { B00111, B01111, B11111, B11111, B11111, B11111, B11111, B11111 },
                                { B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000 },
                                { B11100, B11110, B11111, B11111, B11111, B11111, B11111, B11111 },
                                { B11111, B11111, B11111, B11111, B11111, B11111, B01111, B00111 },
                                { B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111 },
                                { B11111, B11111, B11111, B11111, B11111, B11111, B11110, B11100 },
                                { B11111, B11111, B11111, B00000, B00000, B00000, B11111, B11111 },
                                { B11111, B00000, B00000, B00000, B00000, B11111, B11111, B11111 } 
                              };
// the 10 arrays for building a big number out of the special characters
byte bigNumberChars[10][6] =  { { 0,  1,  2,   3,  4,  5 }, // 0
                                { 1,  2, 32,   4,255,  4 }, // 1
                                { 6,  6,  2,   3,  7,  7 }, // 2
                                { 6,  6,  2,   7,  7,  5 }, // 3
                                { 3,  4,  2,  32, 32,  5 }, // 4
                                { 0,  6,  6,   7,  7,  5 }, // 5
                                { 0,  6,  6,   3,  7,  5 }, // 6
                                { 1,  1,  2,  32,  0, 32 }, // 7
                                { 0,  6,  2,   3,  7,  5 }, // 8
                                { 0,  6,  2,   4,  4,  5 }  // 9
                              };


/********************************************************************************
 * Setup and loop 
 ********************************************************************************/

/**
 * Startup of the I/O box code.
 */
void setup()
{
  arrLEDs[3] = arrMulticolourLEDs[3]; // LED 3 is a multicolour LED
  arrLEDs[7] = arrMulticolourLEDs[7]; // LED 7 is a multicolour LED
  
  // initialise the LCD (if present)
  initializeLCD();
  
  // initialize serial communication at maximum bitrate
  Serial.begin(115200);
}


/**
 * Main loop of the IO box code.
 */
void loop() 
{
  unsigned long time = millis();
  // update the LEDs
  for ( int i = 0 ; i < ARRSIZE(arrLEDs) ; i++ ) 
  {
    if ( arrLEDs[i] != NULL ) arrLEDs[i]->update(time);
  }
  // update the Buttons
  for ( int i = 0 ; i < ARRSIZE(arrButtons) ; i++ ) 
  {
    if ( arrButtons[i] != NULL ) arrButtons[i]->update(time);
  }
}


/********************************************************************************
 * Methods for receiving serial data and reading parameters
 ********************************************************************************/

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
      // look at what the received command is
      char cmd = readChar();
      switch ( cmd )
      {
        // proper commands
        case 'E': processEchoCommand(); break;
        case 'b': processGetButtonStateCommand(); break;
        case 'l': processGetLedBrightnessCommand(); break;
        case 'L': processSetLedBrightnessCommand(); break;
        case 'M': processSetMulticolourLedColourCommand(); break;
        case 'T': processSetLcdTextCommand(); break;
        case 'P': processSetCursorCommand(); break;
        case 'N': processSetBigNumberCommand(); break;
        
        // ignore extraneous bytes
        case CHAR_LF: break;
        case CHAR_CR: break;
        case '\0'   : break;
        
        // everything else is wrong
        default : Serial.println('?'); break;
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
  char c = pollChar();
  // is there a next character?
  if ( c != '\0' )
  {
    // if so, advance read pointer
    rxReadIdx++;
  }
  return c;
}


/**
 * Reads the next character from the receive buffer
 * but does not advance the read pointer.
 *
 * @return next received character or '\0' if there is no next character
 */
char pollChar()
{
  char c = '\0';
  // is there a next character?
  if ( charsAvailable() > 0 ) 
  {
    c = rxBuffer[rxReadIdx];
  }
  return c;
}


/**
 * Checks if there is a next parameter.
 *
 * @return <code>true</code> if there is a next parameter,
 *         <code>false</code> if not
 */
boolean hasNextParameter()
{
  // skip any whitespace
  boolean skipChar = false;
  do
  {
    char c = pollChar();
    skipChar = ( (c == ',') || (c == ' ') || (c == '\t') );
    if ( skipChar )
    {
      // "consume" skippable characters
      c = readChar();
    }
  } while ( (charsAvailable() > 0) && skipChar);
    
  // is there more inthe receive buffer? yes: must be a next parameter
  return (charsAvailable() > 0);
}


/**
 * Checks if there is a next parameter and if it is a number.
 *
 * @return <code>true</code> if there is a next integer parameter,
 *         <code>false</code> if not
 */
boolean hasInt()
{
  char c = pollChar();
  return (c >= '0') && (c <= '9');
}


/**
 * Reads the next integer value from the receive buffer
 * and advances the read pointer to the last valid digit.
 *
 * @return next received integer or -1 if there is no next integer
 */
int readInt()
{
  int i = -1;
  boolean isNumeric;
  do 
  {
    char c = pollChar();
    isNumeric = (c >= '0') && (c <= '9');
    
    if ( isNumeric )
    {
      c = readChar(); // actually read the character
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
 * Checks if there is a next parameter and if it is a string.
 *
 * @return <code>true</code> if there is a next string parameter,
 *         <code>false</code> if not
 */
boolean hasString()
{
  char c = pollChar();
  return (c == '"');
}


/**
 * Reads a string enclosed in quotation marks from the receive buffer
 * and advances the read pointer to the end of the terminating ".
 *
 * @return next received string or an empty string if there is no next string
 */
String readString()
{
  String str = "";
  // needs to start with a "
  if ( readChar() == '"' )
  {
    // read characters to the next "
    while ( charsAvailable() > 0 ) 
    {
      char c = readChar();
      if ( c != '"' )
      {
        str += c;
      }
      else
      {
        // found the terminating "
        break;
      }
    }
  }
  return str;
}


/********************************************************************************
 * Methods for processing commands
 ********************************************************************************/

/**
 * ECHO command was sent: return ID and serial number
 */
void processEchoCommand()
{
  Serial.print(MODULE_NAME); 
  Serial.print(" ");
  Serial.println(MODULE_VERSION);
}


/**
 * Gets button state.
 * ba : a=Button number
 */
void processGetButtonStateCommand()
{
  int buttonIdx = readInt(); // Button index is first parameter
  if ( (buttonIdx >= 0) && 
       (buttonIdx < ARRSIZE(arrButtons)) && 
       (arrButtons[buttonIdx] != NULL) )
  {
    // return button state (0,1)
    Serial.print(arrButtons[buttonIdx]->isPressed() ? '1' : '0');
    // return button change flag (0,1)
    Serial.println(arrButtons[buttonIdx]->hasChanged() ? '1' : '0');
  }
  else
  {
    Serial.println(ERROR_CHAR);
  }
}

/**
 * Gets LED brightness
 * la : a=LED number
 */
void processGetLedBrightnessCommand()
{
  int ledIdx = readInt(); // LED index is first parameter
  if ( (ledIdx >= 0) && 
       (ledIdx < ARRSIZE(arrLEDs)) && 
       (arrLEDs[ledIdx] != NULL) )
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
 * Sets the LED brightness (and optionally blink parameters).
 * Ln,b[,i[,r]] : n=LED number, b=brightness (0-99), i=blink interval in ms, r=blink ratio (0-99)
 */
void processSetLedBrightnessCommand()
{
  boolean success = false;
  // LED index is first parameter
  int ledIdx = readInt(); 
  if ( (ledIdx >= 0) && (ledIdx < ARRSIZE(arrLEDs)) && 
       (arrLEDs[ledIdx] != NULL) &&
       hasNextParameter() && hasInt() )
  {
    // LED brightness is second parameter
    int brightness = readInt(); 
    arrLEDs[ledIdx]->setBrightness(brightness);
    
    // optional blink interval in ms
    if ( hasNextParameter() && hasInt() )
    {
      int interval = readInt();
      arrLEDs[ledIdx]->setBlinkInterval(interval);
    }
    
    // optional blink ratio in percent
    if ( hasNextParameter() && hasInt() )
    {
      int ratio = readInt();
      arrLEDs[ledIdx]->setBlinkRatio(ratio);
    }

    success = true;
  }
  Serial.println(success ? SUCCESS_CHAR : ERROR_CHAR);
}



/**
 * Sets the colour (and optionally blink parameters) of a multicolour LED.
 * Mn,r,g,b,[,i[,r]] : n=LED number, r,g,b=RGB brightness (0-99), i=blink interval in ms, r=blink ratio (0-99)
 */
void processSetMulticolourLedColourCommand()
{
  boolean success = false;
  // LED index is first parameter
  int ledIdx = readInt(); 
  if ( (ledIdx >= 0) && (ledIdx < ARRSIZE(arrMulticolourLEDs)) && 
       (arrMulticolourLEDs[ledIdx] != NULL) &&
       hasNextParameter() && hasInt() )
  {
    // Red/Green/Blue brightness are the next three parameters
    int red   = readInt(); hasNextParameter();
    int green = readInt(); hasNextParameter();
    int blue  = readInt(); 
    if ( (red >= 0) && (green >=0) && (blue >= 0) )
    {
      arrMulticolourLEDs[ledIdx]->setColour(red, green, blue);
      
      // optional blink interval in ms
      if ( hasNextParameter() && hasInt() )
      {
        int interval = readInt();
        arrMulticolourLEDs[ledIdx]->setBlinkInterval(interval);
      }
      
      // optional blink ratio in percent
      if ( hasNextParameter() && hasInt() )
      {
        int ratio = readInt();
        arrMulticolourLEDs[ledIdx]->setBlinkRatio(ratio);
      }
  
      success = true;
    }
  }
  Serial.println(success ? SUCCESS_CHAR : ERROR_CHAR);
}


/**
 * Checks if the LCD panel is connected.
 * If so, initialises the LCD panel and sets the default text to the module name and version.
 */
void initializeLCD()
{
  if ( checkLcdConnection() )
  {
    pLCD = new Adafruit_RGBLCDShield();
    
    // set up the LCD's number of columns and rows: 
    pLCD->begin(16, 2);
    
    // set the cursor to the top left position
    pLCD->setCursor(0,0);
    // print module name on the top line
    pLCD->print(MODULE_NAME);
    // move cursor to the bottom line
    pLCD->setCursor(0,1);
    // print version number
    pLCD->print(MODULE_VERSION);
    // set the backlight to white
    pLCD->setBacklight(LCD_WHITE); 
  
    // define custom segments for big numbers
    for ( int i = 0 ; i < ARRSIZE(bigNumberSegments) ; i++ )
    {
      pLCD->createChar(i, bigNumberSegments[i]);
    }
  }
}


/**
 * Sets the cursor to a defined position
 * default positions are row - 0 and column - 0
 * e.g. P1,2 shifts the cursor to row 1, column 2,
 * any text printed afterwards will start from this location
 */
void processSetCursorCommand()
{
  if ( pLCD == NULL )
  {
    // no LCD connected -> get me out of here
    Serial.println(ERROR_CHAR);
    return;
  } 
  
  //read any integers passed through
  int row    = readInt(); 
  int column = 0;
  if ( hasNextParameter() ) // column is optional
  {
     column = readInt();
  }
  pLCD->setCursor(column, row); 
  Serial.println(SUCCESS_CHAR);
}


/**
 * Sets received numbers to display in big font on the LCD screen
 * e.g. N123 will print 123 in big font on the screen
 */
void processSetBigNumberCommand()
{
  int cursorIterator = 0; // Iterator for large font locations
  while(charsAvailable() > 0)
  {
    int num = readChar() - '0';
    if ( (num >=0) && (num <= 9) )
    {
      byte arrIter = 0; // iterator through character array
      for ( byte y = 0 ; y < 2 ; y++ ) // two lines
      {
        pLCD->setCursor(cursorIterator, y);
        for ( byte x = 0 ; x < 3 ; x++ ) // three chars each line
        {
          pLCD->write(bigNumberChars[num][arrIter++]);
        }
      }
      cursorIterator += 4; // advance cursor 4 spaces
    }
  }
   
  //use cursor iterator to check if anything was printed 
  if ( cursorIterator > 0 )
  {
    Serial.println(SUCCESS_CHAR);  
    cursorIterator = 0; 
  } 
  else
  {
     //deemed unsuccessful if nothing was printed
     Serial.println(ERROR_CHAR);
  }
  
}


/**
 * Sets the text to display on the LCD panel
 * 'T' followed by the text to display within quotation marks "text"
 */
void processSetLcdTextCommand()
{  
  if ( pLCD == NULL )
  {
    // no LCD connected -> get me out of here
    Serial.println(ERROR_CHAR);
    return;
  }
  
  String receivedString = readString();
  if ( receivedString.length() > 0 )
  {
    pLCD->print(receivedString); 
  }
  Serial.println(SUCCESS_CHAR); 
}


/**
 * Checks the LCD's default i2c to see if it is connected
 *
 * @return <code>true</code> if LCD is connected, <code>false</code> if not
 */
boolean checkLcdConnection()
{
  Wire.begin();
  Wire.beginTransmission(32);
  boolean found = (Wire.endTransmission() == 0);
  return found;
}

