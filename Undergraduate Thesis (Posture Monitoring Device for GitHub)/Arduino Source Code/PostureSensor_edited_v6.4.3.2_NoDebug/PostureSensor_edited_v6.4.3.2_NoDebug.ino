//#include "pitches.h"
#include<SoftwareSerial.h>
#include<EEPROM.h>

#include "I2Cdev.h"
#include "MPU6050.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

MPU6050 accelgyro;

int16_t ax, ay, az;    
int16_t gx, gy, gz;

#include <math.h>              //access to advanced math functions

SoftwareSerial btSerial(8,9); 


//momentarty switch pin
const int buttonPin = 6; //initialization of posture
int buttonState = 0;      //state of button of initialization of posture


//test LED Pin
const int LEDPin = 4;

int offsetX;      //calibrate for game offset
int offsetY;
int offsetZ;

//vibration motor digital pin
const int vibePin = 5;            //pin to control vibrating motor
const int vibSwitch = 0;   //switch to control vibration or buzzer
int vibState = 0;           //state of switch to control vibration or buzzer

//buzzer digital pin
const int buzzerPin = 2;         //pin to control buzzer

//length of delay for loop: 1s 
long interval = 1000;
int v;

// will store last time buzzer was updated
unsigned long previousMillis = 0;     

unsigned long currentMillis;

//counters
int counter=0;            //bad posture counter
int counterThresh;   //set by the slider in software for how long bad posture counter alert should be
bool counterBool = false;
int inactivityCtr =0;     //inactivity counter
int inactivityThresh;//how long inactivity posture counter alert should be



const int gameModePin = 7;
int game = 0;
int prevGame;

unsigned int seconds;
unsigned int minutes;
unsigned int hours;
unsigned int totalSeconds;

unsigned int tempSlouchTimeS = 0;
unsigned int slouchTimeS;      //in s
unsigned int inactivityTimeS;
unsigned int slouchTimeM;      //in min
unsigned int inactivityTimeM;
unsigned int slouchTimeH;      //in hr
unsigned int inactivityTimeH;
unsigned int totalSlouchS;
unsigned int totalInactivityS;

unsigned int goodTimeS;        //Total time - slouching 
unsigned int goodTimeM;
unsigned int goodTimeH;
unsigned int totalGoodTimeS;

unsigned int activeS;
unsigned int activeM;
unsigned int activeH;
unsigned int totalActiveS;

bool displayData= false; //should display data the when switching to gameMode from posture monitoring mode

//bool values for slouching or inactive too long
bool slouching = false;
bool inactivity = false;

//no of times alerted and rxn time
int rxns = 0;       //used or indexing rxnTimes, averaging rxnTimes, counting rxns
int rxnTimes[50];
int rxnTime = 0;

int maxReading;

char receiveByte='z';

bool currentVib=true;
bool previousVib;
bool currentBuzz=true;
bool previousBuzz;

//variable for battery life
long batt=0;
int battPercent=0;

//for reading sensitivity readings from software
String myString;
int commaIndex;
String data1;     //for posture sensitivity
String data2;     //for inactivity sensitivity

// will store last time buzzer was updated
unsigned long aPMillis = 0;     

unsigned long aCMillis;

int aInterval = 100;

double goodAcX;
double goodAcY;
double goodAcZ;

double badFAcX;
double badFAcY;
double badFAcZ;

double badBAcX;
double badBAcY;
double badBAcZ;

double wrtX;       //with respect to x, etc.
double wrtY;       //main readings
double wrtZ;

double initwrtX;       //with respect to x, etc.
double initwrtY;       //for good posture initialization readings
double initwrtZ;

double gwrtX;     //good with respect to x, etc.
double gwrtY;
double gwrtZ;

double bfwrtX;     //bad forward with respect to x, etc.
double bfwrtY;
double bfwrtZ;

double bbwrtX;     //bad backward with respect to x, etc.
double bbwrtY;
double bbwrtZ;

double devfwrtX;     //bad forward deviation with respect to x, etc.
double devfwrtY;
double devfwrtZ;

double devbwrtX;     //bad backward deviation with respect to x, etc.
double devbwrtY;
double devbwrtZ;

double pThreshX;     //+ posture angle thresholds
double pThreshZ;

double nThreshX;     //- posture angle thresholds
double nThreshZ;

long AcX; //averaged values
long AcY; 
long AcZ; 
long GyX;
long GyY; 
long GyZ;

int j=0;

int calibrated;   //check EEPROM(2); 0 is not calibrated, 1 is calibrated

void setup(){
  //BT: RX->9,TX->8
  //Initialize the appropriate pins to outputs or inputs 
  pinMode(vibePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(vibSwitch, INPUT);
  pinMode(gameModePin,INPUT);
  pinMode(LEDPin, OUTPUT);

  digitalWrite(vibePin,HIGH);
  delay(500);
  digitalWrite(vibePin,LOW);

  noTone(buzzerPin);

  for (int i =0; i<50; i++)
    rxnTimes[i] = 0;

  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(1);  //0 = +/- 2g | 1 = +/- 4g | 2 = +/- 8g | 3 =  +/- 16g 

  counterThresh = EEPROM.read(0);
  inactivityThresh = EEPROM.read(1);

  btSerial.begin(9600);

  //set offsets for different MPU6050s
  accelgyro.setXAccelOffset(-756);
  accelgyro.setYAccelOffset(-1078);
  accelgyro.setZAccelOffset(872);

  calibrated = EEPROM.read(2);
  if(calibrated == 0){        // posture deviation initialization
    btSerial.println("***Calibrating posture deviation***");
    while(true){
      btSerial.println("Stay in good posture, then send 'a'.");
      if (btSerial.available()){
        //accelgyro.getAcceleration(&goodAcX, &goodAcY, &goodAcZ);
        receiveByte = btSerial.read();

      }
      if (receiveByte == 'a'){
        receiveByte = 'z';

        for(int i = 0; i<10; i++){
          accelgyro.getAcceleration(&ax, &ay, &az);
          goodAcX+=ax;
          goodAcY+=ay;
          goodAcZ+=az;
        }
        goodAcX = goodAcX/10;
        goodAcY = goodAcY/10;
        goodAcZ = goodAcZ/10;

      

        break;
      }

      delay(333);
    }  
    while(true){
      btSerial.println("Lean forward to bad posture, then send 'a'.");
      if (btSerial.available()){
        //accelgyro.getAcceleration(&goodAcX, &goodAcY, &goodAcZ);
        receiveByte = btSerial.read();


      }
      if (receiveByte == 'a'){
        receiveByte = 'z';
        for(int i = 0; i<10; i++){
          accelgyro.getAcceleration(&ax, &ay, &az);
          badFAcX+=ax;
          badFAcY+=ay;
          badFAcZ+=az;
        }
        badFAcX = badFAcX/10;
        badFAcY = badFAcY/10;
        badFAcZ = badFAcZ/10;

        
        break;
      }
      delay(333);
    }
    

    gwrtX = atan2(goodAcX,sqrt((goodAcZ*goodAcZ)+(goodAcY*goodAcY)))*(180/3.14159265);
    gwrtY = atan2(goodAcY,sqrt((goodAcZ*goodAcZ)+(goodAcX*goodAcX)))*(180/3.14159265);
    gwrtZ = atan2(goodAcZ,sqrt((goodAcX*goodAcX)+(goodAcY*goodAcY)))*(180/3.14159265);


    bfwrtX = atan2(badFAcX,sqrt((badFAcZ*badFAcZ)+(badFAcY*badFAcY)))*(180/3.14159265);
    bfwrtY = atan2(badFAcY,sqrt((badFAcZ*badFAcZ)+(badFAcX*badFAcX)))*(180/3.14159265);
    bfwrtZ = atan2(badFAcZ,sqrt((badFAcX*badFAcX)+(badFAcY*badFAcY)))*(180/3.14159265);

    

    bbwrtX = atan2(badBAcX,sqrt((badBAcZ*badBAcZ)+(badBAcY*badBAcY)))*(180/3.14159265);
    bbwrtY = atan2(badBAcY,sqrt((badBAcZ*badBAcZ)+(badBAcX*badBAcX)))*(180/3.14159265);
    bbwrtZ = atan2(badBAcZ,sqrt((badBAcX*badBAcX)+(badBAcY*badBAcY)))*(180/3.14159265);

    

    devfwrtX = gwrtX - bfwrtX;
    devfwrtX = abs(devfwrtX);
    EEPROM.write(3,devfwrtX);
    devfwrtY = gwrtY - bfwrtY;
    devfwrtY = abs(devfwrtY);
    EEPROM.write(4,devfwrtY);
    devfwrtZ = gwrtZ - bfwrtZ;
    devfwrtZ = abs(devfwrtZ);
    EEPROM.write(5,devfwrtZ);

    

    devbwrtX = gwrtX - bbwrtX;
    devbwrtX = abs(devbwrtX);
    EEPROM.write(6,devbwrtX);
    devbwrtY = gwrtY - bbwrtY;
    devbwrtY = abs(devbwrtY);
    EEPROM.write(7,devbwrtY);
    devbwrtZ = gwrtZ - bbwrtZ;
    devbwrtZ = abs(devbwrtZ);
    EEPROM.write(8,devbwrtZ);

   

    EEPROM.write(2,1);
  }


  while(buttonState != HIGH){          //good posture initialization
    btSerial.println("Uninitialized");

    AcX = 0;
    AcY = 0;
    AcZ = 0;

    for(int i = 0; i<10; i++){
      accelgyro.getAcceleration(&ax, &ay, &az);
      AcX+=ax;
      AcY+=ay;
      AcZ+=az;
    }
    AcX = AcX/10;
    AcY = AcY/10;
    AcZ = AcZ/10;



    //USING ANGLES
    initwrtX = atan2(AcX,sqrt((AcZ*AcZ)+(AcY*AcY)))*(180/3.14159265);
    initwrtY = atan2(AcY,sqrt((AcX*AcX)+(AcZ*AcZ)))*(180/3.14159265);
    initwrtZ = atan2(AcZ,sqrt((AcX*AcX)+(AcY*AcY)))*(180/3.14159265);

    buttonState = digitalRead(buttonPin);

    delay(333);
  }

  btSerial.print("wrtX: ");
  btSerial.print(wrtX);
  btSerial.print(" || wrtY: ");
  btSerial.print(wrtY);
  btSerial.print(" || wrtZ: ");
  btSerial.println(wrtZ);    

  pThreshX = initwrtX+EEPROM.read(5);
  nThreshX = initwrtX-EEPROM.read(5);

  pThreshZ = initwrtZ+EEPROM.read(5);
  nThreshZ = initwrtZ;

  btSerial.print("pThreshX: ");
  btSerial.print(pThreshX);
  btSerial.print(" || nThreshX: ");
  btSerial.println(nThreshX);    

  btSerial.print("pThreshZ: ");
  btSerial.print(pThreshZ);
  btSerial.print(" || nThreshZ: ");
  btSerial.println(nThreshZ);    

  btSerial.print("Initialized: ");

  digitalWrite(vibePin, HIGH);
  delay(100);
  digitalWrite(vibePin, LOW);
  delay(100);
  digitalWrite(vibePin, HIGH);
  delay(100);
  digitalWrite(vibePin, LOW);  

  btSerial.println("PostureSensor_edited_v6.4.3.2");
}

void loop(){
  //Start reading values coming from the accelerometer. 
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  //buzzing indicator for changing modes
  previousVib = currentVib;
  previousBuzz = currentBuzz;

  prevGame = game;
  game = digitalRead(gameModePin);

  if(prevGame != game){
    digitalWrite(vibePin, HIGH);
    delay(100);
    digitalWrite(vibePin, LOW);
    delay(100);
    digitalWrite(vibePin, HIGH);
    delay(100);
    digitalWrite(vibePin, LOW);
  }

  if(digitalRead(vibSwitch)==HIGH){
    currentVib = true;
    currentBuzz = false;
  }
  else {
    currentVib = false;
    currentBuzz = true;
  }

  if(currentVib){
    if (!previousVib){
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
      delay(100);
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
    }
  }
  if(currentBuzz){
    if (!previousBuzz){
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
      delay(100);
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
    }
  }

  aCMillis = millis();
  if(aCMillis -aPMillis>= aInterval){      //accelerometer smoothing
    AcX = 0;
    AcY = 0;
    AcZ = 0;
    for(int i = 0; i<10; i++){
      accelgyro.getAcceleration(&ax, &ay, &az);
      AcX+=ax;
      AcY+=ay;
      AcZ+=az;
    }
    AcX = AcX/10;
    AcY = AcY/10;
    AcZ = AcZ/10;

    wrtX = atan2(AcX,sqrt((AcZ*AcZ)+(AcY*AcY)))*(180/3.14159265);
    wrtY = atan2(AcY,sqrt((AcX*AcX)+(AcZ*AcZ)))*(180/3.14159265);
    wrtZ = atan2(AcZ,sqrt((AcX*AcX)+(AcY*AcY)))*(180/3.14159265);

  }

  currentMillis = millis();
  if(currentMillis -previousMillis>= interval){
    previousMillis = currentMillis;


    if(btSerial.available())
    {
      receiveByte = btSerial.read();
    }

    if(receiveByte == 'a'){
      btSerial.print("pr-");
      //Display time used
      totalSeconds = (hours*60*60)+(minutes*60)+seconds;
      btSerial.print(totalSeconds);
      btSerial.print("-");

      //Compute time in good posture
      goodTimeH = hours-slouchTimeH-inactivityTimeH;
      goodTimeM = minutes-slouchTimeM-inactivityTimeM;
      goodTimeS = seconds-slouchTimeS-inactivityTimeS;

      //Display time in good posture
      totalGoodTimeS = (goodTimeH*60*60)+(goodTimeM*60)+goodTimeS;
      btSerial.print(totalGoodTimeS);
      btSerial.print("-");

      //Display time in bad posture
      totalSlouchS = (slouchTimeH*60*60)+(slouchTimeM*60)+slouchTimeS;
      btSerial.print(totalSlouchS);
      btSerial.print("-");

      //Display time inactive
      totalInactivityS = (inactivityTimeH*60*60)+(inactivityTimeM*60)+inactivityTimeS;
      btSerial.print(totalInactivityS);
      btSerial.print("-");

      //Display time active
      activeH = hours-inactivityTimeH;
      activeM = minutes-inactivityTimeM;
      activeS = seconds-inactivityTimeS;
      totalActiveS = (activeH*60*60)+(activeM*60)+activeS;
      btSerial.print(totalActiveS);
      btSerial.print("-");

      //display no. of alerts
      btSerial.print(rxns);
      btSerial.print("-");

      //display avg rxn. time
      if(rxns == 0){
        rxnTime = 0;
      }
      else {
        for(int i = 0; i<50; i++)
          rxnTime+=rxnTimes[i];
        rxnTime = rxnTime/rxns;
      }
      btSerial.print(rxnTime);
      btSerial.print("-");

      //display posture threshold
      btSerial.print(counterThresh);
      btSerial.print("-");

      //display inactivity threshold
      btSerial.print(inactivityThresh);


      btSerial.println();

    }
    if(receiveByte == 'b'){
      while(btSerial.available()){
        myString = btSerial.readString();
        btSerial.println(myString);
        commaIndex = myString.indexOf(',');

        data1=myString.substring(0, commaIndex);
        data2=myString.substring(commaIndex+1);

        

        counterThresh = data1.toInt();
        inactivityThresh = data2.toInt();

        EEPROM.write(0, counterThresh);
        EEPROM.write(1, inactivityThresh);
      }  
    }
    if(receiveByte == 'c'){
      btSerial.println("***Calibrating posture deviation***");
      while(true){
        btSerial.println("Stay in good posture, then send 'a'.");
        if (btSerial.available()){
          //accelgyro.getAcceleration(&goodAcX, &goodAcY, &goodAcZ);
          receiveByte = btSerial.read();

        }
        if (receiveByte == 'a'){
          receiveByte = 'z';

          for(int i = 0; i<10; i++){
            accelgyro.getAcceleration(&ax, &ay, &az);
            goodAcX+=ax;
            goodAcY+=ay;
            goodAcZ+=az;
          }
          goodAcX = goodAcX/10;
          goodAcY = goodAcY/10;
          goodAcZ = goodAcZ/10;

         

          break;
        }

        delay(333);
      }  
      while(true){
        btSerial.println("Lean forward to bad posture, then send 'a'.");
        if (btSerial.available()){
          //accelgyro.getAcceleration(&goodAcX, &goodAcY, &goodAcZ);
          receiveByte = btSerial.read();


        }
        if (receiveByte == 'a'){
          receiveByte = 'z';
          for(int i = 0; i<10; i++){
            accelgyro.getAcceleration(&ax, &ay, &az);
            badFAcX+=ax;
            badFAcY+=ay;
            badFAcZ+=az;
          }
          badFAcX = badFAcX/10;
          badFAcY = badFAcY/10;
          badFAcZ = badFAcZ/10;

         
          break;
        }
        delay(333);
      }
      

      gwrtX = atan2(goodAcX,sqrt((goodAcZ*goodAcZ)+(goodAcY*goodAcY)))*(180/3.14159265);
      gwrtY = atan2(goodAcY,sqrt((goodAcZ*goodAcZ)+(goodAcX*goodAcX)))*(180/3.14159265);
      gwrtZ = atan2(goodAcZ,sqrt((goodAcX*goodAcX)+(goodAcY*goodAcY)))*(180/3.14159265);

      

      bfwrtX = atan2(badFAcX,sqrt((badFAcZ*badFAcZ)+(badFAcY*badFAcY)))*(180/3.14159265);
      bfwrtY = atan2(badFAcY,sqrt((badFAcZ*badFAcZ)+(badFAcX*badFAcX)))*(180/3.14159265);
      bfwrtZ = atan2(badFAcZ,sqrt((badFAcX*badFAcX)+(badFAcY*badFAcY)))*(180/3.14159265);

      

      bbwrtX = atan2(badBAcX,sqrt((badBAcZ*badBAcZ)+(badBAcY*badBAcY)))*(180/3.14159265);
      bbwrtY = atan2(badBAcY,sqrt((badBAcZ*badBAcZ)+(badBAcX*badBAcX)))*(180/3.14159265);
      bbwrtZ = atan2(badBAcZ,sqrt((badBAcX*badBAcX)+(badBAcY*badBAcY)))*(180/3.14159265);

      

      devfwrtX = gwrtX - bfwrtX;
      devfwrtX = abs(devfwrtX);
      EEPROM.write(3,devfwrtX);
      devfwrtY = gwrtY - bfwrtY;
      devfwrtY = abs(devfwrtY);
      EEPROM.write(4,devfwrtY);
      devfwrtZ = gwrtZ - bfwrtZ;
      devfwrtZ = abs(devfwrtZ);
      EEPROM.write(5,devfwrtZ);

 

      devbwrtX = gwrtX - bbwrtX;
      devbwrtX = abs(devbwrtX);
      EEPROM.write(6,devbwrtX);
      devbwrtY = gwrtY - bbwrtY;
      devbwrtY = abs(devbwrtY);
      EEPROM.write(7,devbwrtY);
      devbwrtZ = gwrtZ - bbwrtZ;
      devbwrtZ = abs(devbwrtZ);
      EEPROM.write(8,devbwrtZ);

     

      pThreshX = initwrtX+EEPROM.read(5);
      nThreshX = initwrtX-EEPROM.read(5);

      pThreshZ = initwrtZ+EEPROM.read(5);
      nThreshZ = initwrtZ-EEPROM.read(8);

      btSerial.print("pThreshX: ");
      btSerial.print(pThreshX);
      btSerial.print(" || nThreshX: ");
      btSerial.println(nThreshX);    

      btSerial.print("pThreshZ: ");
      btSerial.print(pThreshZ);
      btSerial.print(" || nThreshZ: ");
      btSerial.println(nThreshZ);
    }
    else{
      batt = readVcc(); //read battery voltage; value is in mV
      battPercent = map(batt,3300,4000,0,100);

      if(battPercent>100)
        battPercent = 100;

      btSerial.print(battPercent);
      btSerial.print("%, ");

      game = digitalRead(gameModePin);
      //posture mode
      if(game == LOW){
        //interval for posture mode is 1s
        interval = 1000;

        btSerial.print("Posture,");

        vibState = digitalRead(vibSwitch);
        if(vibState == HIGH){
          btSerial.print("Motor,");
        }
        else{
          btSerial.print("Buzzer,");
        }

        buttonState = digitalRead(buttonPin);
        if(buttonState == HIGH){
          btSerial.print(wrtX);
          btSerial.print(",");
          btSerial.print(wrtY);
          btSerial.print(",");
          btSerial.print(wrtZ);
          btSerial.print(",");
          btSerial.print("Reinitializing... ");
          btSerial.println();
          
          //USING ANGLES
          initwrtX = atan2(AcX,sqrt((AcZ*AcZ)+(AcY*AcY)))*(180/3.14159265);
          initwrtY = atan2(AcY,sqrt((AcX*AcX)+(AcZ*AcZ)))*(180/3.14159265);
          initwrtZ = atan2(AcZ,sqrt((AcX*AcX)+(AcY*AcY)))*(180/3.14159265);

          
          pThreshX = initwrtX+EEPROM.read(5);
          nThreshX = initwrtX-EEPROM.read(5);

          pThreshZ = initwrtZ+EEPROM.read(5);
          nThreshZ = initwrtZ;

          btSerial.print("pThreshX: ");
          btSerial.print(pThreshX);
          btSerial.print(" || nThreshX: ");
          btSerial.println(nThreshX);    

          btSerial.print("pThreshZ: ");
          btSerial.print(pThreshZ);
          btSerial.print(" || nThreshZ: ");
          btSerial.println(nThreshZ); 

       

          digitalWrite(vibePin, HIGH);
          delay(100);
          digitalWrite(vibePin, LOW);
          delay(100);
          digitalWrite(vibePin, HIGH);
          delay(100);
          digitalWrite(vibePin, LOW);
        }
        else{


          btSerial.print(wrtX);
          btSerial.print(",");
          btSerial.print(wrtY);
          btSerial.print(",");
          btSerial.print(wrtZ);
          btSerial.print(",");

          //bad posture
          if((wrtZ >= pThreshZ) || ((wrtZ < nThreshZ)) || (wrtX >= pThreshX) || (wrtX < nThreshX)){
            btSerial.print("Bad");
            counter+=1;
            inactivityCtr = 0;

            
            if(inactivity){
              digitalWrite(vibePin, LOW);
              noTone(buzzerPin);
              inactivity = false;
            }
            if (slouching){
              tempSlouchTimeS++;

              slouchTimeS++;
              vibeMotorSlouch();
              alarmSlouch();
              delay(100);
              digitalWrite(vibePin, LOW);
              noTone(buzzerPin);
              delay(100);
              vibeMotorSlouch();
              alarmSlouch();
              delay(100);
              digitalWrite(vibePin, LOW);
              noTone(buzzerPin);

              //debug tempSlouchTimeS
              btSerial.print(",");
              btSerial.println(tempSlouchTimeS);


            }
          }
          //good posture
          else{
            btSerial.print("Good");
            counter = 0;
            inactivityCtr +=1;


            
            if(slouching){
              digitalWrite(vibePin, LOW);
              noTone(buzzerPin);
              slouching = false;

              rxnTimes[rxns] = (counterThresh*60) + tempSlouchTimeS;  //set rxnTimes as tempSlouchTimeS
              rxns++;                            //add no. of rxns
            }
            if(inactivity){
              inactivityTimeS++;
              vibeMotorSlouch();
              alarmSlouch();
              delay(100);
              digitalWrite(vibePin, LOW);
              noTone(buzzerPin);
              delay(100);
              vibeMotorSlouch();
              alarmSlouch();
              delay(100);
              digitalWrite(vibePin, LOW);
              noTone(buzzerPin);

            } 

            tempSlouchTimeS = 0;
          }
          if(counter == (counterThresh*60) && !slouching){ //time the device will buzz for bad posture 
            vibeMotorSlouch();
            alarmSlouch();
            delay(100);
            digitalWrite(vibePin, LOW);
            noTone(buzzerPin);
            delay(100);
            vibeMotorSlouch();
            alarmSlouch();
            delay(100);
            digitalWrite(vibePin, LOW);
            noTone(buzzerPin);

            slouchTimeM+=counterThresh;

            slouching = true;
          }
          if(inactivityCtr == (inactivityThresh*60) && !inactivity){ //time the device will buzz for inactivity 
            vibeMotorSlouch();
            alarmSlouch();
            delay(100);
            digitalWrite(vibePin, LOW);
            noTone(buzzerPin);
            delay(100);
            vibeMotorSlouch();
            alarmSlouch();
            delay(100);
            digitalWrite(vibePin, LOW);
            noTone(buzzerPin);

            inactivityTimeM+=inactivityThresh;
            inactivity = true;
          }
          btSerial.print(",");
          btSerial.print(counterThresh);
          btSerial.print(",");
          btSerial.print(inactivityThresh);
          btSerial.print(",");
          btSerial.print(pThreshX);
          btSerial.print(",");
          btSerial.print(nThreshX);
          btSerial.print(",");
          btSerial.print(pThreshZ);
          btSerial.print(",");
          btSerial.print(nThreshZ);
          btSerial.println();
        }
      }
      //game mode
      else if (game == HIGH){
        //interval for game is 1/4s
        interval = 250;


        //override all posture monitoring outputs to false/ off
        digitalWrite(vibePin, LOW);
        noTone(buzzerPin);
        slouching = false;
        inactivity = false;


        btSerial.print("Game,");

        buttonState = digitalRead(buttonPin);
        
        if(buttonState == HIGH){
          
        }
        
        else{

          if (((wrtX)>-11.25 && (wrtX)<11.25) && (wrtY>-10.95 && wrtY<11.55) && ((wrtZ)>78.45 && (wrtZ)<89.7)){ //plank
            btSerial.print("1");
            btSerial.print(",");
          }
          if (((wrtX)>-84.27 && (wrtX)<-39.27) && (wrtY>5.73 && wrtY<50.73) && ((wrtZ)>-15 && (wrtZ)<30)) { //half side plank right
            btSerial.print("2");
            btSerial.print(",");
          }
          if (((wrtX)>39.27 && (wrtX)<84.27) && (wrtY>5.73 && wrtY<50.73) && ((wrtZ)>-15 && (wrtZ)<30)) { //half side plank left
            btSerial.print("3");
            btSerial.print(",");
          }
          if (((wrtX)>-83 && (wrtX)<-38) && (wrtY>6.99 && wrtY<51.99) && ((wrtZ)>-15 && (wrtZ)<30)) { //full right side plank
            btSerial.print("4");
            btSerial.print(",");
          }
          if (((wrtX)>38 && (wrtX)<83) && (wrtY>6.99 && wrtY<51.99) && ((wrtZ)>-15 && (wrtZ)<30)) { //full left side plank
            btSerial.print("5");
            btSerial.print(",");
          }
          if (((wrtX)>-35 && (wrtX)<-14) && (wrtY>55 && wrtY<76) && ((wrtZ)>-22.5 && (wrtZ)<22.5)){ //double arm reach right
            btSerial.print("10");
            btSerial.print(",");
          }
          if (((wrtX)>14 && (wrtX)<35) && (wrtY>55 && wrtY<76) && ((wrtZ)>-22.5 && (wrtZ)<22.5)) { //double arm reach left
            btSerial.print("11");
            btSerial.print(",");
          }
          if (((wrtX)>-22.5 && (wrtX)<22.5) && (wrtY>-1.94 && wrtY<29) && ((wrtZ)>46.94 && (wrtZ)<72.25)) { //superman
            btSerial.print("12");
            btSerial.print(",");
          }
          if (((wrtX)>0 && (wrtX)<57.92) && (wrtY>41.37 && wrtY<66.25) && ((wrtZ)>18.44 && (wrtZ)<59.20)) { //standing bicycle right
            btSerial.print("13");
            btSerial.print(",");
          }
          if (((wrtX)>-57.92 && (wrtX)<0) && (wrtY>41.37 && wrtY<66.25) && ((wrtZ)>18.44 && (wrtZ)<59.20)) { //standing bicycle left
            btSerial.print("14");
            btSerial.print(",");
          }
          if (((wrtX)>-11.25 && (wrtX)<11.25) && (wrtY>48.75 && wrtY<71.25) && ((wrtZ)>18.75 && (wrtZ)<41.25)) { //chair pose
            btSerial.print("19");
            btSerial.print(",");
          }
          if (((wrtX)>-42.48 && (wrtX)<2.52) && (wrtY>-17.31 && wrtY<16.44) && ((wrtZ)>62.31 && (wrtZ)<84.81)) { //right one-legged suatreach
            btSerial.print("20");
            btSerial.print(",");
          }
          if (((wrtX)>-2.52 && (wrtX)<42.48) && (wrtY>-17.31 && wrtY<16.44) && ((wrtZ)>62.31 && (wrtZ)<84.81)) { //left one-legged squat reach
            btSerial.print("21");
            btSerial.print(",");
          }
          if (((wrtX)>-22.5 && (wrtX)<22.5) && (wrtY>-24 && wrtY<21) && ((wrtZ)>66 && (wrtZ)<111)) { //liftoff right leg up
            btSerial.print("22");
            btSerial.print(",");
          }
          if (((wrtX)>-22.5 && (wrtX)<22.5) && (wrtY>-24 && wrtY<21) && ((wrtZ)>66 && (wrtZ)<111)) { //liftoff ledt leg up
            btSerial.print("23");
            btSerial.print(",");
          }
          if (((wrtX)>-22.5 && (wrtX)<22.5) && (wrtY>54.86 && wrtY<99.86) && ((wrtZ)>-9.86 && (wrtZ)<35.14)) { //prone press up
            btSerial.print("24");
            btSerial.print(",");
          }
          //repetition exercises
          if (((wrtX)>0 && (wrtX)<28.96) && (wrtY>39.6 && wrtY<72) && ((wrtZ)>18.44 && (wrtZ)<59.20)) { //right knee cross crunch
            btSerial.print("6");
            btSerial.print(",");
          }
          if (((wrtX)>-28.96 && (wrtX)<0) && (wrtY>39.6 && wrtY<72) && ((wrtZ)>18.44 && (wrtZ)<59.20)) { //left knee cross crunch
            btSerial.print("7");
            btSerial.print(",");
          }
          if (((wrtX)>-43.53 && (wrtX)<-18.44) && (wrtY>46.47 && wrtY<75.47) && ((wrtZ)>15 && (wrtZ)<37.5)) { //right side imprint
            btSerial.print("8");
            btSerial.print(",");
          }
          if (((wrtX)>18.44 && (wrtX)<43.53) && (wrtY>46.47 && wrtY<71.57) && ((wrtZ)>15 && (wrtZ)<37.5)) { //left side imprint
            btSerial.print("9");
            btSerial.print(",");
          }
          if (((wrtX)>-43.53 && (wrtX)<-22.53) && (wrtY>46.47 && wrtY<67.47) && ((wrtZ)>-22.5 && (wrtZ)<22.5)) { //leaning lifting crunch to the right
            btSerial.print("17");
            btSerial.print(",");
          }
          if (((wrtX)>22.53 && (wrtX)<43.53) && (wrtY>46.47 && wrtY<67.47) && ((wrtZ)>-22.5 && (wrtZ)<22.5)) { //leaning lifting crunch to the left
            btSerial.print("18");
          }

          btSerial.println(",");

      

          btSerial.print("wrtX: ");
          btSerial.print(wrtX);
          btSerial.print(" || wrtY: ");
          btSerial.print(wrtY);
          btSerial.print(" || wrtZ: ");
          btSerial.println(wrtZ);  

        }
        btSerial.println();
      }
    }

    //Clear receiveByte so it can receive again from the computer
    receiveByte = 'z';


    //time the device is running
    if(interval==250){
      v++;
      if(v==4){
        seconds++;
        v=0;
      }
    }
    else if(interval == 1000){
      seconds++;
    }
    if(seconds == 60){
      minutes++;
      seconds=0;
    }
    if(minutes ==60){
      hours++;
      minutes=0;
    }

    //time the user is slouching
    if(slouchTimeS == 60){
      slouchTimeM++;
      slouchTimeS=0;
    }
    if(slouchTimeM ==60){
      slouchTimeH++;
      slouchTimeM=0;
    }
  }

  //print time device is on when voltage reaches 3.35 V, before the BT module will die
  if(batt <= 3350 && batt >= 3200){
    //Display time used
    btSerial.print("Hours: ");
    btSerial.print(hours);
    btSerial.print("||Minutes: ");
    btSerial.print(minutes);
    btSerial.print("||Seconds: ");
    btSerial.println(seconds);
    btSerial.println("Please charge your device.");
    while(true){//vibrate 5 times atset intervals if low batt
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
      delay(100);
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
      digitalWrite(vibePin, HIGH);
      delay(100);
      digitalWrite(vibePin, LOW);
      delay(1000);
    }
  }
}

//fuction that controls the piezo buzzer for slouching
void alarmSlouch(){
  vibState = digitalRead(vibSwitch);
  if(vibState == LOW){
    tone(buzzerPin, 200);
  }
  else if(vibState == HIGH)
    noTone(buzzerPin);
}

//fuction that controls the vibration motor for slouching
void vibeMotorSlouch(){
  vibState = digitalRead(vibSwitch);
  if(vibState == HIGH){
    digitalWrite(vibePin, HIGH); 
  }
  else if(vibState == LOW)
    digitalWrite(vibePin, LOW);
}

//fuction that controls the piezo buzzer for inactivity
void alarmInactivity(){
  vibState = digitalRead(vibSwitch);
  if(vibState == LOW){
    tone(buzzerPin, 65);
  }
  else if(vibState == HIGH)
    noTone(buzzerPin);
}

//fuction that controls the vibration motor for inactivity
void vibeMotorInactivity(){
  vibState = digitalRead(vibSwitch);
  if(vibState == HIGH){
    digitalWrite(vibePin, HIGH);
  }
  else if(vibState == LOW)
    digitalWrite(vibePin, LOW);
}

long readVcc() { 
  long result; // Read 1.1V reference against AVcc 
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); 
  delay(2); // Wait for Vref to settle 
  ADCSRA |= _BV(ADSC); // Convert 
  while (bit_is_set(ADCSRA,ADSC)); 
  result = ADCL; 
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV 
  return result; 
}















