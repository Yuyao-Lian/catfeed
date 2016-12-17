c/*

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

// initialize the library with the numbers of the interface pins dor the LCD
LiquidCrystal lcd(12, 11, 5, 8, 7, 6);

#define PIN_SERVO 9

Servo feedServo;
Servo stirServo;
int pos = 0;
volatile boolean TurnDetected;
volatile boolean up;
const int PinCLK=2;                   // Used for generating interrupts using CLK signal
const int PinDT=3;                    // Used for reading DT signal
const int PinSW=4;                    // Used for the push button switch of the Rotary Encoder
const int buttonPin = A3;             // the number of the pushbutton pin for manual feed 13
int buttonState = 0;                  // variable for reading the manual feed pushbutton status
int feed1hour = 07;                   // variables for feeding times and quantity
int feed1minute = 00;
int feed2hour = 17;
int feed2minute = 30;
int feedQty = 4;
int feedRate = 800;   //a pwm rate the triggers forward on the servo 75
int feedReversal = 80; //a pwm rate that triggers reverse on the servo
                       // play with these numbers for your servo. Mine is a Futaba digital servo
                       // that I removed the pot from and the plastic lug, to make it continuous.

void isr ()  {                    // Interrupt service routine is executed when a HIGH to LOW transition is detected on CLK
 if (digitalRead(PinCLK))         // this keeps an eye out for the rotary encoder being turned regardless of where the program is
   up = digitalRead(PinDT);       // currently exectuting - in other words, during the main loop this ISR will always be active
 else
   up = !digitalRead(PinDT);
 TurnDetected = true;
}


void setup ()  {
   // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  // setup the Rotary encoder
 pinMode(PinCLK,INPUT);
 pinMode(PinDT,INPUT);  
 pinMode(PinSW,INPUT);
 pinMode(buttonPin, INPUT);
 attachInterrupt (0,isr,FALLING);   // interrupt 0 is always connected to pin 2 on Arduino UNO
    lcd.setCursor(17,0);            
    lcd.print("Roger Donoghue's");  // A bit of fun :-)
    lcd.setCursor(17,1);
    lcd.print("  Cat-O-Matic");
     for (int positionCounter = 0; positionCounter < 17; positionCounter++) {
     // scroll one position left:
     lcd.scrollDisplayLeft(); 
     // wait a bit:
     delay(150);
          }
      delay(3000);
      for (int positionCounter = 0; positionCounter < 17; positionCounter++) {
     // scroll one position left:
     lcd.scrollDisplayRight(); 
     // wait a bit:
     delay(150);
     
             }  // end of fun
     lcd.setCursor(17,0);
     lcd.print("                ");
     lcd.setCursor(17,1);
     lcd.print("                ");  
        }
  
void loop ()  {  //Main program loop - most things in here!
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
    lcd.print("Qty ");
    lcd.print(feedQty);
    lcd.print(" ");
    lcd.setCursor(0,1);
    lcd.print("1)");
    printDigits(feed1hour);
    lcd.print(":");
    printDigits(feed1minute);
    lcd.print(" 2)");
    printDigits(feed2hour);
    lcd.print(":");
    printDigits(feed2minute);
    
    
// MAIN BREAKOUT "IF" SECION BELOW THAT MONITORS THE PUSH BUTTON AND ENTERS PROGRAMMING IF IT'S PUSHED 

 if (!(digitalRead(PinSW))) {      // check if pushbutton is pressed
                                   // if YES then enter the programming subroutine
    lcd.blink();   // Turn on the blinking cursor:
    lcd.setCursor(5,0);
    lcd.print(" SET");
    virtualPosition = tm.Hour;  //needed or the hour will be zero each time you change the clock.
       do  {
            lcd.setCursor(0,0);   // put cursor at Time Hour  
            delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
            if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the hour of time -
            tm.Hour = virtualPosition;
            RTC.write(tm);
            lcd.setCursor(0, 0);
            printDigits(tm.Hour);  // then re-print the hour on the LCD
         } while ((digitalRead(PinSW)));  // do this "do" loop while the PinSW button is NOT pressed
         lcd.noBlink();
         delay(1000);
       
       //   SET THE MINS
        
        lcd.blink();   // Turn on the blinking cursor:
       virtualPosition = tm.Minute;  //needed or the minute will be zero each time you change the clock.
       do  {
       lcd.setCursor(3,0);   // put cursor at Time Mins
       delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
           if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the min of time -
            tm.Minute = virtualPosition;
            RTC.write(tm);
            lcd.setCursor(3, 0);
            printDigits(tm.Minute);  // then re-print the min on the LCD
         } while ((digitalRead(PinSW)));
         lcd.noBlink();
         delay(1000);
    
       //   SET THE QTY - Feed quantity
        
       lcd.blink();   // Turn on the blinking cursor:
       virtualPosition = feedQty;  //needed or the qty will be zero.
       do  {
       lcd.setCursor(14,0);   // put cursor at QTY
       delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
           if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the feed qty
            feedQty = virtualPosition;
            lcd.setCursor(14, 0);
            lcd.print(feedQty);
           } while ((digitalRead(PinSW)));
         lcd.noBlink();
         delay(1000);
         
     //   SET THE Feed1 Hour
        
       lcd.blink();   // Turn on the blinking cursor:
       virtualPosition = feed1hour;  //needed or will be zero to start with.
       do  {
       lcd.setCursor(2,1);   // put cursor at feed1hour
       delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
           if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the feed1 hour
            feed1hour = virtualPosition;
            lcd.setCursor(2,1);
            printDigits(feed1hour);
           } while ((digitalRead(PinSW)));
         lcd.noBlink();
         delay(1000);  
  
      //   SET THE Feed1 Mins
        
       lcd.blink();   // Turn on the blinking cursor:
       virtualPosition = feed1minute;  //needed or will be zero to start with.
       do  {
       lcd.setCursor(5,1);   // put cursor at feed1minute
       delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
           if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the feed1 minute
            feed1minute = virtualPosition;
            lcd.setCursor(5,1);
            printDigits(feed1minute);
           } while ((digitalRead(PinSW)));
         lcd.noBlink();
         delay(1000);   

   //   SET THE Feed2 Hour
        
       lcd.blink();   // Turn on the blinking cursor:
       virtualPosition = feed2hour;  //needed or will be zero to start with.
       do  {
       lcd.setCursor(10,1);   // put cursor at feed1hour
       delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
           if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the feed1 hour
            feed2hour = virtualPosition;
            lcd.setCursor(10,1);
            printDigits(feed2hour);
           } while ((digitalRead(PinSW)));
         lcd.noBlink();
         delay(1000);  
  
      //   SET THE Feed2 Mins
        
       lcd.blink();   // Turn on the blinking cursor:
       virtualPosition = feed2minute;  //needed or will be zero to start with.
       do  {
       lcd.setCursor(13,1);   // put cursor at feed1minute
       delay(500);   // Delay needed or same button press will exit do-while as while is checking for another button push!
           if (TurnDetected)  {		    // do this only if rotation was detected
            if (up)
              virtualPosition--;
            else
            virtualPosition++;
            TurnDetected = false;          // do NOT repeat IF loop until new rotation detected
                               }
            // Here I change the feed1 minute
            feed2minute = virtualPosition;
            lcd.setCursor(13,1);
            printDigits(feed2minute);
           } while ((digitalRead(PinSW)));
         lcd.noBlink();
         delay(1000);                         
      
  }  // end of main IF rotary encoder push button checker
 
   // CHECK FOR MANUAL FEED BUTTON
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    feed();
  }
  // CHECK FEEDING TIME AND FEED IF MATCHED
  
  if (tm.Hour == feed1hour && tm.Minute == feed1minute && tm.Second == 0)  {  // if I dont' check seconds are zero
    feed();                                                                   // then it'll feed continuously for 1 minute!
      }
  if (tm.Hour == feed2hour && tm.Minute == feed2minute && tm.Second == 0)  {
    feed();
      }  
  
}   // End of main Loop

void printDigits(int digits){   // utility function for digital clock display: prints leading 0
   if(digits < 10)
    lcd.print('0');
   lcd.print(digits);
 }
 
 void feed() {
   
   lcd.setCursor(17,0);
   lcd.print("   Meowwwww!");
    for (int positionCounter = 0; positionCounter < 16; positionCounter++) {
     // scroll one position left:
     lcd.scrollDisplayLeft(); 
     // wait a bit:
     delay(150);
         }
  
   // Stir servo section     If you don't need a stir servo simply comment out all fo this until the Auger rotate section  
   stirServo.attach(10);      // I don't know if I need one either but I'm adding it now as it's easiest before I build it!
    for(pos = 0; pos <= 180; pos += 1) {                                  
    stirServo.write(pos);              
    delay(5);   } 
   for(pos = 180; pos>=0; pos-=1)  {                                
    stirServo.write(pos);          
    delay(10);   } 
    delay(200);
   for(pos = 0; pos <= 180; pos += 1) {                                  
    stirServo.write(pos);              
    delay(10);   } 
   for(pos = 180; pos>=0; pos-=1)  {                                
    stirServo.write(pos);          
    delay(5);   }  
   stirServo.detach();  
   
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
     lcd.scrollDisplayRight(); 
     // wait a bit:
     delay(150);
         }
             }
