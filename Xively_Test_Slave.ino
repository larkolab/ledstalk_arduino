#include <Wire.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

// Matrix Display
#define	MAX_DEVICES	1
#define	CLK_PIN		5
#define	DATA_PIN	7
#define	CS_PIN		6

MD_Parola P = MD_Parola(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

uint8_t	frameDelay = 100;
textEffect_t scrollEffect = SCROLL_LEFT;

#define	BUF_SIZE 75
char message[BUF_SIZE];
char tmp_message[BUF_SIZE];
unsigned char i;
boolean new_message = false;

void setup()
{
  Wire.begin(4);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(57600);          // start serial for output
  
  P.begin();
  P.displayClear();
  P.displaySuspend(false);

  P.displayScroll(message, CENTER, scrollEffect, frameDelay);

  strcpy(message, "...");
  //message[0] = '\0';
}

void loop()
{
  if (P.displayAnimate()) 
  {
    if (new_message)
    {
      strcpy(message, tmp_message);
      new_message = false;
    }
    P.displayReset();
  }
  delay(100);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  i = 0;
  new_message = false;
  while(Wire.available() > 0) // loop through all but the last
  {
    tmp_message[i] = Wire.read(); // receive byte as a character
    i++;
  }
  tmp_message[i] = '\0';
  new_message = true;
  Serial.println();         // print the integer
}
