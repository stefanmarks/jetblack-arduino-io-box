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
 *
 * Command set:
 * E               : Echo version number
 * La,b[,c[,d]]    : Set LED a brightness to b (00-99) (and blink interval to c, and blink ratio to d)
 * la              : Get brightness of LED a
 * Tx,y,z "string" : Set text on LCD display, set to location row(x) and column(y) of where to start writing
 *                   turn screen on/off by sending 0 (on) or 1(off) as z. i.e. T101"hi" prints to row 1, column 0 with screen on                
 * 
 * Return value: "+" or value if command successful, "!" if an error occured
 */
 
#include <Wire.h>
#include "LED.h"
#include "Adafruit_MCP23017.h"
#include "Adafruit_RGBLCDShield.h"
 
 // version of the IO box
const char MODULE_NAME[]    = "JetBlack IO-Box";
const char MODULE_VERSION[] = "v1.2";

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

// LCD and text initialization
Adafruit_RGBLCDShield* pLCD = NULL; // if NO LCD is connnected, pLCD stays null
char incomingString[25]; // Set some space for the string (25 characters)
int  row = 0, column = 0; //default row and column locations set to 0
uint8_t oldButtons;
int cursorIterator = 0; // Iterator for large font locations
boolean largeFont = false;

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
byte LT[8]  = {B00111, B01111, B11111, B11111, B11111, B11111, B11111, B11111};
byte UB[8]  = {B11111, B11111, B11111, B00000, B00000, B00000, B00000, B00000};
byte RT[8]  = {B11100, B11110, B11111, B11111, B11111, B11111, B11111, B11111};
byte LL[8]  = {B11111, B11111, B11111, B11111, B11111, B11111, B01111, B00111};
byte LB[8]  = {B00000, B00000, B00000, B00000, B00000, B11111, B11111, B11111};
byte LR[8]  = {B11111, B11111, B11111, B11111, B11111, B11111, B11110, B11100};
byte UMB[8] = {B11111, B11111, B11111, B00000, B00000, B00000, B11111, B11111};
byte LMB[8] = {B11111, B00000, B00000, B00000, B00000, B11111, B11111, B11111};


/********************************************************************************
 * Setup and loop 
 ********************************************************************************/

/**
 * Startup of the I/O box code.
 */
void setup()
{
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
  processLCDButtons();
  unsigned long time = millis();
  // update the LEDs
  for ( int i = 0 ; i < numLEDs ; i++ ) 
  {
    arrLEDs[i]->update(time);
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
      switch ( readChar() )
      {
        // proper commands
        case 'E': processEchoCommand(); break;
        case 'l': processGetLedBrightnessCommand(); break;
        case 'L': processSetLedBrightnessCommand(); break;
        case 'T': processSetLcdTextCommand(); break;
        
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
 * and advances the read pointer accordingly.
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
    if ( hasInt() )
    {
      // optional blink interval in ms
      int interval = readInt();
      arrLEDs[ledIdx]->setBlinkInterval(interval);
    }
    if ( hasInt() )
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
  }
  // assignes each segment a write number
  pLCD->createChar(0,LT);
  pLCD->createChar(1,UB);
  pLCD->createChar(2,RT);
  pLCD->createChar(3,LL);
  pLCD->createChar(4,LB);
  pLCD->createChar(5,LR);
  pLCD->createChar(6,UMB);
  pLCD->createChar(7,LMB);
}


/**
 * Sets the text to display on the LCD panel
 * T: followed by the text to display
 */
void processSetLcdTextCommand()
{  
  if ( pLCD == NULL )
  {
    // no LCD connected -> get me out of here
    Serial.println(ERROR_CHAR);
    return;
  }
  
  char nextChar = ' ';
  column = 0, row = 0;
  boolean rowDone = false, columnDone = false, backLightDone = false;
  while(nextChar != '"' && charsAvailable() > 0)
  {
    nextChar = readChar();
    //check if row hasn't already been set, if characters are left
    //and if the found character is a number
    if (!rowDone && nextChar >='0' && nextChar <='9')
    {
      // sets the row Location on the LCD
      row = nextChar - '0';
      rowDone = true;
    }
    else if (!columnDone && nextChar >='0' && nextChar <='9')
    {
      // sets the column location on the LCD
      column = nextChar - '0'; 
      columnDone = true;
    }
    else if(!backLightDone && nextChar >= '0' && nextChar <= '1')
    {
      if(nextChar == '0')
      {
         pLCD->setBacklight(LCD_BLACK);
      }
      else
      {
         pLCD->setBacklight(LCD_WHITE); 
      }
      backLightDone = true;
    } 
  }
  
    pLCD->setCursor(column, row);
    
    //Now we check to see if any quotation marks are present
    //only print strings that begin with a quotation    
    if(nextChar == '"')
    {
      processText();
    }
 
  // all went well (?)
  Serial.println(SUCCESS_CHAR);  
}


/**
 * Function to process the text to be displayed
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
    pLCD->print(incomingString); 
    for(int i = 0; i < index; i++)
    {      
      if(incomingString[i] == CHAR_CR)
      {
        //pLCD->setCursor(column , row++);
      }
      if(incomingString[i] == '\0')
      {
        break;
      }
      
      if(largeFont)
      {
        switch(incomingString[i] - '0') {
            case 0: custom0();  cursorIterator += 4;  break;
            case 1: custom1();  cursorIterator += 4;  break;
            case 2: custom2();  cursorIterator += 4;  break;
            case 3: custom3();  cursorIterator += 4;  break;
            case 4: custom4();  cursorIterator += 4;  break;
            case 5: custom5();  cursorIterator += 4;  break;
            case 6: custom6();  cursorIterator += 4;  break;
            case 7: custom7();  cursorIterator += 4;  break;
            case 8: custom8();  cursorIterator += 4;  break;
            case 9: custom9();  cursorIterator += 4;  break;
            default: break;
        }       
      }
      else
      {
         pLCD->setCursor(column + i , row);
         pLCD->print(incomingString[i]);  
      }
      incomingString[i] = '\0';
    } 
   cursorIterator = 0;   
  } 
}


/**
 * Checks the LCD's default i2c to see if it is connected
 *
 * @return <code>true</code> if LCD is connected, <code>false</code> if not
 */
boolean checkLcdConnection()
{
  boolean found = false;
  
  // Serial.println();
  // Serial.println("Checking for LCD");
  
  Wire.begin();
  Wire.beginTransmission(32);
  if ( Wire.endTransmission() == 0 )
  {
    // Serial.print("LCD Device connected at ");
    // Serial.println(32, HEX);
    found = true;
  }
  else  
  {
     //Serial.println("LCD was not found"); 
  }
  return found;
}

/**
  * Checks if any buttons were pressed during each update and reacts accordingly
  *                               --Current Button Configurations--
  * Select Button - Switches size of fonts
  */
void processLCDButtons()
{
  uint8_t newButtons = pLCD->readButtons();
  uint8_t buttons = newButtons & ~oldButtons;
  oldButtons = newButtons;
  if (buttons) 
  {
    if (buttons & BUTTON_SELECT) 
    {
      if(largeFont)
      {
        //Serial.println("Large font off");
        largeFont = false;
      }
      else
      {
        //Serial.println("Large font on");
        largeFont = true;
      }
    }
  } 
}

 
// -- Large number character functions --  Will need to be refactored 

/**
  * functions which create the custom number font
  */  
void custom0()
{ // uses segments to build the number 0
  pLCD->setCursor(cursorIterator, 0); // set cursor to column 0, line 0 (first row)
  pLCD->write(0);  // call each segment to create
  pLCD->write(1);  // top half of the number
  pLCD->write(2);
  pLCD->setCursor(cursorIterator, 1); // set cursor to colum 0, line 1 (second row)
  pLCD->write(3);  // call each segment to create
  pLCD->write(4);  // bottom half of the number
  pLCD->write(5);
}

void custom1()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(1);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator+1,1);
  pLCD->write(5);
}

void custom2()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(6);
  pLCD->write(6);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator, 1);
  pLCD->write(3);
  pLCD->write(7);
  pLCD->write(7);
}

void custom3()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(6);
  pLCD->write(6);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator, 1);
  pLCD->write(7);
  pLCD->write(7);
  pLCD->write(5); 
}

void custom4()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(3);
  pLCD->write(4);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator+2, 1);
  pLCD->write(5);
}

void custom5()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(0);
  pLCD->write(6);
  pLCD->write(6);
  pLCD->setCursor(cursorIterator, 1);
  pLCD->write(7);
  pLCD->write(7);
  pLCD->write(5);
}

void custom6()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(0);
  pLCD->write(6);
  pLCD->write(6);
  pLCD->setCursor(cursorIterator, 1);
  pLCD->write(3);
  pLCD->write(7);
  pLCD->write(5);
}

void custom7()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(1);
  pLCD->write(1);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator+1, 1);
  pLCD->write(0);
}

void custom8()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(0);
  pLCD->write(6);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator, 1);
  pLCD->write(3);
  pLCD->write(7);
  pLCD->write(5);
}

void custom9()
{
  pLCD->setCursor(cursorIterator,0);
  pLCD->write(0);
  pLCD->write(6);
  pLCD->write(2);
  pLCD->setCursor(cursorIterator+2, 1);
  pLCD->write(5);
}

