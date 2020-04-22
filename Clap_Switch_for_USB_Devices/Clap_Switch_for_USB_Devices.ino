/*code by Charles Janoah Policarpio
This project controls an LED (or switch any device) after 3 well-timed claps.
*/

#define pin 12
int reading;
boolean state = false;

#define led 13

int eventStatus; //1 = detect 1st clap
                 //2 = delay after 1st clap
                 //3 = detect 2nd clap
                 //4 = delay after 2nd clap
                 //5 = detect 3rd clap



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(pin,INPUT);
  pinMode(led,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(digitalRead(pin));
  eventStatus=0;

  //if there is enough ambient light, turn the lamp off
  reading = digitalRead(pin);
  if(reading == LOW){                //detect 1st clap
    eventStatus++;
    Serial.println(eventStatus);
    delay(500);
  }
  if( eventStatus == 1){             //delay after 1st clap
    for (int i = 0; i<1000; i++){
      reading = digitalRead(pin);
      if(reading == LOW){
        eventStatus=0;
        break;
      }
      if(i = 999){
        eventStatus++;
        Serial.println(eventStatus);
      }
      delay(1);      
    }
  }
  if(eventStatus ==2){               //detect 2nd clap
    for (int i = 0; i<1000; i++){
      reading = digitalRead(pin);
      if (reading == LOW){
        eventStatus++;
        Serial.println(eventStatus);
        delay(500);
        break;        
      }
      delay(1);
    }
  }
  if( eventStatus == 3){            //delay after 2nd clap
    for (int i = 0; i<1000; i++){
      reading = digitalRead(pin);
      if(reading == LOW){
        eventStatus=0;
        break;
      }
      if(i = 999){
        eventStatus++;
        Serial.println(eventStatus);
      }
      delay(1);      
    }
  }
  if(eventStatus ==4){            //detect 3rd clap
    for (int i = 0; i<1000; i++){
      reading = digitalRead(pin);
      if (reading == LOW){
        eventStatus++;
        Serial.println(eventStatus);
        delay(500);
        break;        
      }
      delay(1);
    }
  }
  if(eventStatus ==5){            //switch control
    if(state==false){
      state = true;
      Serial.println("Now HIGH!");
    }
    else{
      state = false;
      Serial.println("Now LOW!");
    }
    digitalWrite(led, state);
    Serial.println("Chaanged!");
    delay(500);
  }
}
