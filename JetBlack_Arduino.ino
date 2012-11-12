/**
 * Used to interface with the Unity Engine and print objects to the Adafruit LCD screen
 * @author Marcus Ball 
 * @date 10/11/12
 */

//Libraries
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>

//Initialise the adafruit LCD
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

//Predefined #defines for backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

char incomingString[25]; // Set some space for the string (18 characters)
char incomingCharacter=-1; // Where to store the character read
byte index = 0; // Index into array; where to store the character
int selection = 0; 
int changeColor = 0;
boolean buttonReleased = true;
byte smiley[8] = {
  B00000,
  B10001,
  B00000,
  B00000,
  B10001,
  B01110,
  B00000,
};

void setup() {
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
  //Reset the LCD and set the cursor to 0,0
  //lcd.home(); 
  //allow auto scroll
  lcd.noAutoscroll();
  //lcd.scrollDisplayLeft();
  //Left allignment 
  lcd.leftToRight();
  //delay(2000);
  //Set up serial communication to 9600bps
  Serial.begin(9600);
  lcd.clear();
  lcd.createChar(0, smiley);
  lcd.begin(16, 2);  
  lcd.write(0);
}


uint8_t i=0;
uint8_t oldButtons;
void loop() 
{
  // While serial data is being received
  while (Serial.available())
  {
    //only accept the first 18 characters (size of the string array)
    if(index < 24 && Serial.available() > 0)
    {
      //set a small delay to allow for buffer to fill
      delay(10);
      //read the incoming character, one character at a time over serial
      incomingCharacter = Serial.read();
      //Store the incoming character into the string
      incomingString[index] = incomingCharacter; 
      //Increment to next index in string array
      index++;
      // Null terminate the string
      incomingString[index] = '\0'; 
    }
  }

  if (index  > 0) 
  {
    //lcd.clear();
    lcd.setCursor(0,0);
    boolean newline = false;
    int j = 0;
    for(int i = 0; i < 16; i++)
    {
      if(incomingString[i] == '#')
      {
        newline = true;
        j = i;
        break;
      }
      if(incomingString[i] == '*')
      {
        break;
      }
      if(incomingString[i] != '\0')
      {
        lcd.setCursor(i,0);
        lcd.print(incomingString[i]); 
      }
    }
    int k = 0;
    if(newline)
    {
      for(int i = j+1; i < 25; i++)
      {
        //dont print null characters
        if(incomingString[i] != '\0')
        {
          lcd.setCursor(k,1);
          k++;
          lcd.print(incomingString[i]);
        }
      }

    }
    else
    {
      for(int i = 17; i < 25; i++)
      {
        if(incomingString[i] != '\0')
        {
          lcd.setCursor(k,1);
          k++;
          lcd.print(incomingString[i]);
        }
      }
    }
    //lcd.print(incomingString);
    //reset each char in string array to 0
    for (int i=0;i<24;i++) 
    { 
      incomingString[i]=0;
    }
    //reset index
    index=0;      
  }

  //read any button inputs from LCD shield
  //uint8_t buttons = lcd.readButtons();
  uint8_t newButtons = lcd.readButtons();
  uint8_t buttons = newButtons & ~oldButtons;
  oldButtons = newButtons;

  if (buttons) {
    if (buttons & BUTTON_UP) {
      lcd.setBacklight(changeColor);
      //selection++; 
      changeColor++;
      lcd.clear();
    }
    if (buttons & BUTTON_DOWN) {
      lcd.setBacklight(changeColor);
      lcd.clear();
      //selection++;
    }
    if (buttons & BUTTON_LEFT) {
      selection--;
      lcd.clear();
    }
    if (buttons & BUTTON_RIGHT) {
      selection++;
      lcd.clear();
    }
    if (buttons & BUTTON_SELECT) {
      lcd.setBacklight(0x0);
    }
    if(selection > 5)
    {
      selection = 0;
    }
    else if(selection < 0)
    {
      selection = 5;
    }
    if(changeColor > 8)
    {
      changeColor = 0;
    }
    
    Serial.println(selection, DEC);
  }

}



