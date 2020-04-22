#include <Arduino.h>
#include <TM1637Display.h>
#include <EEPROM.h>


#include <Wire.h>
//#include <Time.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

// The amount of time (in milliseconds) between tests
#define TEST_DELAY   2000

const uint8_t SEG_DONE[] = {
	SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
	SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
	SEG_C | SEG_E | SEG_G,                           // n
	SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
	};

TM1637Display display(CLK, DIO);

int hr = 0;  //hour
int m = 0;   //minute

int number; //display to TM1637

unsigned long currentMillis;        
unsigned long previousMillis = 0;    
const long interval = 100;           // interval at which to update information

unsigned long currentBlinkMillis;        
unsigned long previousBlinkMillis = 0;        
const long blinkInterval = 1000;           // interval at which colon will blink (milliseconds)

boolean colon = false;

int ldrPin = A0;                         //LDR
int ldr[10];
int ldrCount = 0;
int brightness;

unsigned long ldrCurrentMillis;        
unsigned long ldrPreviousMillis = 0;
const long ldrInterval = 10;        // interval at which to read ambient light

#define setModePin 7     //change to normal or military time in normal time mode
boolean milTime = false;
int reading;

#define setAlPin 4       //button to et  to alarm mode
int j;                   //when button is pressed, increment to 20 (2s) to set alarm
boolean setAlarmMode = false;   
boolean activateAlarm = false;  //allow/ don't allow alarm at set time

#define setAlHrPin 5     //adjust hours at alarm
#define setAlMinPin 6    //adjust minutes at alarm
int alHr = 0;
int k = 0;
int alMin = 0;
int l = 0;
boolean alarming = false;

const int test = 13;                 //now for buzzer
const int test2 = 12;                //now indicates if you would want to use alarm or not  

void setup()
{
  number = 0;
  display.setBrightness(8);  //0,8-15
  
  pinMode(ldrPin,INPUT);
  
  for(int i = 0; i<10; i++){
    ldr[i]=0;
  }
  
  pinMode(setModePin, INPUT);
  pinMode(setAlPin, INPUT);
  pinMode(setAlHrPin, INPUT);
  pinMode(setAlMinPin, INPUT);
  
  alHr = EEPROM.read(0);         //slHr saves here
  alMin = EEPROM.read(1);        //alMin saves here
  
  pinMode(test, OUTPUT);       
  pinMode(test2, OUTPUT);       
  
  Serial.begin(9600);
}

void loop()
{
  tmElements_t tm;
  
  currentMillis = millis();
  currentBlinkMillis = millis();
  ldrCurrentMillis = millis();
  
  if(currentMillis - previousMillis >= interval && setAlarmMode == false) {   
    previousMillis = currentMillis;
    reading = digitalRead(setModePin);
    if (reading == HIGH){                       //change to normal or military time
      milTime = !milTime;
      reading = digitalRead(setModePin);
      while(reading==HIGH){
        reading = digitalRead(setModePin);
      }
    }
    if (RTC.read(tm)) {                    //Read the time from RTC
      m = tm.Minute;
      if(milTime==true){                   //military time
        hr = tm.Hour;
      }
      else if(milTime == false){           //normal time
        if(tm.Hour == 12)
          hr = tm.Hour;
        else if (tm.Hour == 0)
          hr = 12;
        else
          hr = tm.Hour%12;
      }
    }
    
    if(digitalRead(setAlPin)==HIGH){   //hold button to enter setAlarmMode
      while(digitalRead(setAlPin)==HIGH){
        j++;
        delay(100);
        if(j>=20){                         //enter setArarm mode
          setAlarmMode = true;
          number = 8888;
          display.showNumberDec(number,true,4,0);  //to preserve colon, use this one at a time. 
        }
      }
    }
    j=0;
    number = (hr*100) + m;
    
    if(activateAlarm==true && tm.Minute == alMin && tm.Hour == alHr){ //alarming
      alarming = true;
      while (tm.Minute == alMin && tm.Hour == alHr){
        while (alarming){
          tone(test,3000);
          display.setColon(true);
          delay(100);
          noTone(test);
          display.setColon(false);
          delay(100);
          if (RTC.read(tm)) {                    //Read the time from RTC
            m = tm.Minute;
            if(milTime==true){                   //military time
              hr = tm.Hour;
            }
            else if(milTime == false){           //normal time
              if(tm.Hour == 12)
                hr = tm.Hour;
              else if (tm.Hour == 0)
                hr = 12;
              else
                hr = tm.Hour%12;
            }
          }
          number = (hr*100) + m;
          display.showNumberDec(number,true,4,0);  //to preserve colon, use this one at a time. 
          if(digitalRead(setAlPin)==HIGH || digitalRead(setAlHrPin)==HIGH || digitalRead(setAlMinPin)==HIGH || digitalRead(setModePin)==HIGH){
            alarming = false;
          }
        }
        if (RTC.read(tm)) {                    //Read the time from RTC
          m = tm.Minute;
          if(milTime==true){                   //military time
            hr = tm.Hour;
          }
          else if(milTime == false){           //normal time
            if(tm.Hour == 12)
              hr = tm.Hour;
            else if (tm.Hour == 0)
              hr = 12;
            else
              hr = tm.Hour%12;
          }
        }
      }
    }
  }
  if(currentMillis - previousMillis >= interval && setAlarmMode == true) {   
    if(digitalRead(setAlHrPin)==HIGH){                 //when button is pressed, increment alHr
      alHr++;
      while(digitalRead(setAlHrPin)==HIGH){
        k++;
        delay(500);
        if(k>=5){
          alHr++;
          if(alHr >= 24){
            alHr = 0;
          }
          number = alHr*100 + alMin;            //number will hold alarm values for hours and minutes
          display.showNumberDec(number,true,4,0);  //to preserve colon, use this one at a time. 
        }
      }
      k=0;
      if(alHr >= 24){
        alHr = 0;
      }
    }
    if(digitalRead(setAlMinPin)==HIGH){                 //when button is pressed, increment alMin
      alMin++;
      while(digitalRead(setAlMinPin)==HIGH){
        l++;
        delay(500);
        if(l>=5){
          alMin++;
          if(alMin >= 60){
            alMin = 0;
          }
          number = alHr*100 + alMin;            //number will hold alarm values for hours and minutes
          display.showNumberDec(number,true,4,0);  //to preserve colon, use this one at a time.     
        }
      }
      l=0;
      if(alMin >= 60){
        alMin = 0;
      }
    }
    if(digitalRead(setAlPin)==HIGH){   //press to exit setAlarmMode
      setAlarmMode = false;
      if( EEPROM.read(0) != alHr ){    //update alHr EEPROM only if it's a different value
        EEPROM.write(0, alHr);
      }
      if( EEPROM.read(1) != alMin ){    //update alMin EEPROM only if it's a different value
        EEPROM.write(1, alMin);
      }
    } 
    
    if(digitalRead(setModePin)==HIGH){                 //when button is pressed, alarm is activated
      activateAlarm = !activateAlarm;
      digitalWrite(test2, activateAlarm);
      while(digitalRead(setModePin)==HIGH){

      }
    }
    
    number = alHr*100 + alMin;            //number will hold alarm values for hours and minutes
  }
  
  if(ldrCurrentMillis - ldrPreviousMillis >= ldrInterval) {  //adaptive brightness
    ldrPreviousMillis = ldrCurrentMillis;

    ldr[ldrCount]= analogRead(ldrPin);
    ldrCount++;
    if(ldrCount==10){
      //average LDR readings
      for(int i = 0; i<10; i++){  
        brightness += ldr[i];
        
      }
      brightness = brightness/10;
      ldrCount=0;
      
      Serial.print("Brightness: ");
      Serial.println(brightness);
      
      if (brightness>=0 && brightness <= 20){
        display.setBrightness(8);  //0,8-15
      }
      else if (brightness>=21 && brightness <= 40){
        display.setBrightness(9);  //0,8-15
      }
      else if (brightness>=41 && brightness <= 60){
        display.setBrightness(10);  //0,8-15
      }
      else if (brightness>=61 && brightness <= 80){
        display.setBrightness(11);  //0,8-15
      }
      else if (brightness>=81 && brightness <= 100){
        display.setBrightness(12);  //0,8-15
      }
      else if (brightness>=101 && brightness <= 120){
        display.setBrightness(13);  //0,8-15
      }
      else if (brightness>=121 && brightness <= 140){
        display.setBrightness(14);  //0,8-15
      }
      
      else if (brightness>=141){
        display.setBrightness(15);  //0,8-15
      }
      brightness=0;
    }
  }
  
  if(currentBlinkMillis - previousBlinkMillis >= blinkInterval) {  //blink colon to indicate the clock is ticking every second
    previousBlinkMillis = currentBlinkMillis;

    display.setColon(colon);
    colon = !colon;
  }
  
  
  
  
  
  display.showNumberDec(number,true,4,0);  //to preserve colon, use this one at a time. 
  
}
