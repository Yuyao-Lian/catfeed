/*

Automatic Auger Audiono pet feeder Copyright Roger Donoghue 28/03/2015 all rights reserved. For personal use only.
Not for commercial use or resale.
Modified by Yuyao Lian 2016-December-15
Allows you to set 2 feeding times and the quantity as a multiple of the default feed quantity.
Uses a DS1307 real time clock to keep the time, with a rechargable battery built in.
(You can use the arduino RTC example code in the IDE to set the clock , or use the rotary encoder as intended)

*/
// include the library code:
#include <LiquidCrystal.h> // LCD Screen
#include <Wire.h>      // needed for the RTC libraty
#include <Time.h>      // needed for RTC chip to work
#include <DS1307RTC.h> // Real Time Clock Library
#include <Servo.h>
#include <Adafruit_NeoPixel.h> // LED Ring

#ifdef __AVR__
  #include <avr/power.h>
#endif

// Define Pins for LED light and servo
#define PIN_SERVO 9
#define LEDPIN 6

// Set up LCD, LED ring, servo.
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, LEDPIN, NEO_GRB + NEO_KHZ800);
Servo feedServo;


int pos = 0;
volatile boolean TurnDetected;
volatile boolean up;
const int pushbuttonPin = 8;    // the number of the pushbutton pin for manual feed 8
int buttonState = 0;        // variable for reading the manual feed pushbutton status

//Set feed time and Qty.
int feed1hour = 07;         // variables for feeding times and quantity
int feed1minute = 30;
int feed2hour = 13;
int feed2minute = 30;
int feed3hour = 22;
int feed3minute = 30;
int feedQty = 4;
int feedRate = 120;    //a pwm rate the triggers forward on the servo 
int feedReversal = 20; //a pwm rate that triggers reverse on the servo

//----------------------------------------------------------------------------------

void setup ()  {
   lcd.begin(16, 2);
   strip.begin();
   strip.show(); // Initialize all pixels to 'off'
   pinMode(pushbuttonPin,INPUT_PULLUP);
//    Serial.begin(9600);
}

//----------------------------------------------------------------------------------
// Main function
void loop ()  {  
    pinMode(pushbuttonPin,INPUT_PULLUP);
    strip.show();
    static long virtualPosition=0;    // without STATIC it does not count correctly!!!
    tmElements_t tm;    // This sectionm reads the time from the RTC, sets it in tmElements tm (nice to work with), then displays it.
    RTC.read(tm); 
    lcd.setCursor(0, 0);
    printDigits(tm.Hour); //call to print digit function that adds leading zeros that may be missing
    lcd.print(":");
    printDigits(tm.Minute);
    lcd.print(":");
    printDigits(tm.Second);
    lcd.print("  ");
    lcd.print("*RORO*");
    lcd.print(" ");
    lcd.setCursor(0,1);
    lcd.print(" ");
    printDigits(feed1hour);
    printDigits(feed1minute);
    lcd.print(" ");
    printDigits(feed2hour);
    printDigits(feed2minute);
    lcd.print(" ");
    printDigits(feed3hour);
    printDigits(feed3minute);    
// Used for debugging.
//     Serial.print("Ok, Time = ");
//     print2digits(tm.Hour);
//     Serial.write(':');
//     print2digits(tm.Minute);
//     Serial.write(':');
//     print2digits(tm.Second);
//     Serial.print(", Date (D/M/Y) = ");
//     Serial.print(tm.Day);
//     Serial.write('/');
//     Serial.print(tm.Month);
//     Serial.write('/');
//     Serial.print(tmYearToCalendar(tm.Year));
//    Serial.println();

   // CHECK FOR MANUAL FEED BUTTON
  buttonState = digitalRead(pushbuttonPin);
  if (buttonState == HIGH) {
    // Serial.println("manual");
    meow();
    lightup();
    feed();
    lcd.begin(16,2); // Refresh the LCD screen, other wise will be stuck at meow.
  }

  // CHECK FEEDING TIME AND FEED IF MATCHED
  if (tm.Hour == feed1hour && tm.Minute == feed1minute && tm.Second == 0)  {  // if I dont' check seconds are zero
    // Serial.println("time1");
    meow();
    lightup();
    feed();                                                                   
    lcd.begin(16,2);
  }
  if (tm.Hour == feed2hour && tm.Minute == feed2minute && tm.Second == 0)  {
    // Serial.println("time2");
    meow();
    lightup();
    feed();
    lcd.begin(16,2);
  }
   if (tm.Hour == feed3hour && tm.Minute == feed3minute && tm.Second == 0)  {
    // Serial.println("time2");
    meow();
    lightup();
    feed();
    lcd.begin(16,2);
  }  
}   // End of main Loop


//======================UTILITY FUNCTIONS BELOW==================================
void feed() {
// rotate the Auger   
  feedServo.attach(PIN_SERVO);
  for (int cnt = 0; cnt < feedQty; cnt++){
    feedServo.write(feedReversal); 
    delay(300);
    feedServo.write(feedRate);  
    delay(400);   
    
    feedServo.write(feedReversal);  
    delay(300);
    feedServo.write(feedRate);  
    delay(400);   
    
    feedServo.write(feedReversal); 
    delay(300);                     
    feedServo.write(feedRate);  
    delay(400);   

    feedServo.write(feedReversal); 
    delay(300);                     
    feedServo.write(feedRate);  
    delay(400);  
  }                         
  feedServo.detach();
}
//-------------------
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}
//-------------------
void lightup(){
  colorWipe(strip.Color(127, 127, 127), 50); // White RGBW
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  for (uint16_t i=0; i < strip.numPixels(); i=i+1) {
        strip.setPixelColor(i, 0,0,0,0);     
  }
}
//-------------------
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
//-------------------
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
//-------------------
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
//-------------------
void printDigits(int digits){   // utility function for digital clock display: prints leading 0
   if(digits < 10)
    lcd.print('0');
   lcd.print(digits);
 }
//-------------------
 void meow(){
   lcd.setCursor(17,0);
   lcd.print("   Meowwwwww!");
    for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
     lcd.scrollDisplayLeft(); 
     delay(100);
    }
   delay(100);
 }
//-------------------
