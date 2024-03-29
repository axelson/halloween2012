#include "LPD8806.h"
#include "SPI.h"

#define Check_Return checkSerial(); \
  if( needChange ) return;

// Example to control LPD8806-based RGB LED Modules in a strip

/*****************************************************************************/

// Choose which 2 pins you will use for output.
// Can be any valid output pins.
int dataPin = 2;
int clockPin = 3;
int ledPin = 13;

// Set the first variable to the NUMBER of pixels. 32 = 32 pixels in a row
// The LED strips are 32 LEDs per meter but you can extend/cut the strip
LPD8806 strip = LPD8806(32, dataPin, clockPin);

// you can also use hardware SPI, for ultra fast writes by leaving out the
// data and clock pin arguments. This will 'fix' the pins to the following:
// on Arduino 168/328 thats data = 11, and clock = pin 13
// on Megas thats data = 51, and clock = 52
//LPD8806 strip = LPD8806(32);

// Flag for coooperative multitasking
static int needChange = 0;

// Counter sent back to android (probably remove later)
static int counter = 0;

// Sleep time per cycle
static int sleepTime = 0;

// Flag for using rainbow colors
static int niceColors = 1;

static int ledSwitchPin = 8;
static int val = 0;

void setup() {
  // Start up the LED strip
  strip.begin();

  // Update the strip, to start they are all 'off'
  strip.show();

  Serial.begin(115200);

  pinMode(ledSwitchPin, INPUT);
  pinMode(ledPin, OUTPUT);
}


void loop() {
  lightsOff();
  if(val == 1) {
    delay(10);
    
    lightsOff();
    checkSerial();
    return;
  }
  else {

  Serial.print("Tick #");
  Serial.print(counter++, DEC);
  Serial.print("\n");

  delay(sleepTime);
  /*
  colorChase(strip.Color(127,127,127), 10);

  // Send a simple pixel chase in...
  colorChase(strip.Color(127,0,0), 10);  	// full brightness red
  colorChase(strip.Color(127,127,0), 10);	// orange
  colorChase(strip.Color(0,127,0), 10);		// green
  colorChase(strip.Color(0,127,127), 10);	// teal
  colorChase(strip.Color(0,0,127), 10);		// blue
  colorChase(strip.Color(127,0,127), 10);	// violet

  // fill the entire strip with...
  colorWipe(strip.Color(127,0,0), 10);		// red
  */
  //if( niceColors ) {
  if( niceColors ) {
    rainbowCycle(0);  // make it go through the cycle fairly fast
    //lightsOff();
  }
  else {
    lightsOff();
    colorWipe(strip.Color(0, 127,0), 10);		// green
    colorWipe(strip.Color(0,0,127), 10);		// blue
  }
//  rainbow(10);

  //rainbowCycle(0);  // make it go through the cycle fairly fast
  }
}

void rainbow(uint8_t wait) {
  int i, j;

  for (j=0; j < 384; j++) {     // 3 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      Check_Return
      strip.setPixelColor(i, Wheel( (i + j) % 384));
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed
// along the chain
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j=0; j < 384 * 5; j++) {     // 5 cycles of all 384 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      Check_Return
      // tricky math! we use each pixel as a fraction of the full 384-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 384 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 384 / strip.numPixels()) + j) % 384) );
    }
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;

  for (i=0; i < strip.numPixels(); i++) {
      Check_Return
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

// Chase a dot down the strip
// good for testing purposes
void colorChase(uint32_t c, uint8_t wait) {
  int i;

  for (i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);  // turn all pixels off
  }

  for (i=0; i < strip.numPixels(); i++) {
      Check_Return
      strip.setPixelColor(i, c);
      if (i == 0) {
        strip.setPixelColor(strip.numPixels()-1, 0);
      } else {
        strip.setPixelColor(i-1, 0);
      }
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

//Input a value 0 to 384 to get a color value.
//The colours are a transition r - g -b - back to r

uint32_t Wheel(uint16_t WheelPos)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   //Red down
      g = WheelPos % 128;      // Green up
      b = 0;                  //blue off
      break;
    case 1:
      g = 127 - WheelPos % 128;  //green down
      b = WheelPos % 128;      //blue up
      r = 0;                  //red off
      break;
    case 2:
      b = 127 - WheelPos % 128;  //blue down
      r = WheelPos % 128;      //red up
      g = 0;                  //green off
      break;
  }
  return(strip.Color(r,g,b));
}

void lightsOff() {
  int i;

  //for (i=0; i < strip.numPixels(); i++) {
    for (i=0; i < 32; i++) {
    strip.setPixelColor(i, 0);  // turn all pixels off
  }
  strip.show();
}


void checkSerial() {
  val = digitalRead(ledSwitchPin);
  digitalWrite(ledPin, val);

  
  needChange = 0;
  if(val == 1) {
    needChange = 1;
  }
  int readCommand = 0;
  if (Serial.peek() != -1) {
    Serial.print("Read: ");
    do {
      if(readCommand) {
        needChange = 1;
        readCommand = 0;
        if(Serial.peek() == '1') {
          niceColors = 1;
          sleepTime = 0000;
        }
        if(Serial.peek() == '2') {
          sleepTime = 0000;
          niceColors = 0;
        }
      }
      if(Serial.peek() == 'Z') {
        readCommand = 1;
      }
      Serial.print((char) Serial.read());
    } while (Serial.peek() != -1);
    Serial.print("\n");
  }
}
