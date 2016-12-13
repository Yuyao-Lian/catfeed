/*

Automatic Auger Audiono pet feeder Copyright Roger Donoghue 28/03/2015 all rights reserved. For personal use only.
Not for commercial use or resale.
Allows you to set 2 feeding times and the quantity as a multiple of the default feed quantity.
Uses a DS1307 real time clock to keep the time, with a rechargable battery built in.
(You can use the arduino RTC example code in the IDE to set the clock , or use the rotary encoder as intended)

*/
// include the library code:
#include <LiquidCrystal.h>
#include <Wire.h>      // needed for the RTC libraty
#include <Time.h>
#include <DS1307RTC.h> // Real Time Clock Library
#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


#define PIN_SERVO 9
#define LEDPIN 6

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, LEDPIN, NEO_GRB + NEO_KHZ800);
Servo feedServo;
Servo stirServo;
int pos = 0;
volatile boolean TurnDetected;
volatile boolean up;
const int PinCLK=2;                   // Used for generating interrupts using CLK signal
const int PinDT=3;                    // Used for reading DT signal
const int PinSW=4;                    // Used for the push button switch of the Rotary Encoder
const int buttonPin = 8;             // the number of the pushbutton pin for manual feed 8
int buttonState = 0;                  // variable for reading the manual feed pushbutton status


int feed1hour = 07;                   // variables for feeding times and quantity
int feed1minute = 00;
int feed2hour = 20;
int feed2minute = 52;


int feedQty = 4;
int feedRate = 800;   //a pwm rate the triggers forward on the servo 75
int feedReversal = 80; //a pwm rate that triggers reverse on the servo
                       // play with these numbers for your servo. Mine is a Futaba digital servo
                       // that I removed the pot from and the plastic lug, to make it continuous.
//----------------------------------------------------------------------------------



void setup ()  {
   strip.begin();
   strip.show(); // Initialize all pixels to 'off'
   pinMode(buttonPin,INPUT_PULLUP);
   Serial.begin(9600);
}


  
void loop ()  {  //Main program loop - most things in here!
    static long virtualPosition=0;    // without STATIC it does not count correctly!!!
    tmElements_t tm;    // This sectionm reads the time from the RTC, sets it in tmElements tm (nice to work with), then displays it.
    pinMode(buttonPin,INPUT_PULLUP);
    strip.show();
RTC.read(tm); 
//
 Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
//

   // CHECK FOR MANUAL FEED BUTTON
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    Serial.println("manual");
    lightup();
    feed();

  }
  // CHECK FEEDING TIME AND FEED IF MATCHED
  
  if (tm.Hour == feed1hour && tm.Minute == feed1minute && tm.Second == 0)  {  // if I dont' check seconds are zero
    Serial.println("time1");
    feed();                                                                   // then it'll feed continuously for 1 minute!
      }
  if (tm.Hour == feed2hour && tm.Minute == feed2minute && tm.Second == 0)  {
    Serial.println("time2");
    feed();
      }  
  
}   // End of main Loop


//=========================================UTILITY FUNCTIONS BELOW============================================================

 
void feed() {
// rotate the Auger   
   feedServo.attach(PIN_SERVO);
    for (int cnt = 0; cnt < feedQty; cnt++)
    {
      feedServo.write(feedRate);  //the feedrate is really the feed direction and rate.
      delay(600);   //this delay sets how long the servo stays running from the previous command
      feedServo.write(feedReversal);  //...until this command sets the servo a new task!
      delay(200);
      feedServo.write(feedRate);  
      delay(600);   
      feedServo.write(feedReversal);  // if you want to increase the overall feedrate increase the forward delays (1000 at the moment)
      delay(200);                     // or better still just copy and past the forward & backwards code underneath to repeat
          }                           // that way the little reverse wiggle is always there to prevent jams
     feedServo.detach();
     for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
     // scroll one position left:
     // wait a bit:
     delay(150);
         }
             }

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}


void lightup(){
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127, 0, 0), 50); // Red
  theaterChase(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  for (uint16_t i=0; i < strip.numPixels(); i=i+1) {
        strip.setPixelColor(i, 0,0,0,0);     
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;
  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}



// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

