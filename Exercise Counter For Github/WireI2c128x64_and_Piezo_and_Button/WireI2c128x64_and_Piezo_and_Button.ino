// Simple I2C test for ebay 128x64 oled.

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiWire oled;

//------------------------------------------------------------------------------

const int selButton = 1;
const int stButton = 3;
const int piezo = 4;

byte exerciseIndex = 0;

const char* const exercises[]  ={"Core Central", "Left: Unilateral \nKnee to Chest", "Right: Unilateral \nKnee to Chest","Bilateral Knee \nto Chest", "Partial Leg Raise",
"Partial Sit-ups", "Wall Squats","Quadruped", "Left Side Stretch","Right Side \nContraction","Segmental Rotation"};

const byte hold[11]  = {7,7,7,7,7,7,7,15,15,15,15};
const byte sets[11] = {10,10,10,10,10,10,10,6,3,3,6};

void setup() {
  pinMode(selButton,INPUT);
  pinMode(stButton,INPUT);
  pinMode(piezo,OUTPUT);
  
  Wire.begin();
  Wire.setClock(400000L);

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);

  //startupSequence();  
}
//------------------------------------------------------------------------------
void loop() {
  if(digitalRead(selButton) == HIGH){  //exercise selection
    oled.clear();
    exerciseIndex++;
    if (exerciseIndex >=11){
      exerciseIndex = 0;
    }
  }

  if(digitalRead(stButton)== HIGH){
    oled.clear();
    for(int i = 1;i<sets[exerciseIndex]+1;i++){
      for(int j=1;j<hold[exerciseIndex]+1; j++){
        oled.print("Seconds: ");
        oled.println(j);
        oled.print("Sets: ");
        oled.println(i);
        beep(10,250);
        oled.clear();
      }
      beep(12,100);
      beep(12,100);
    }
    beep(15,50);
    beep(15,50);
    beep(15,50);
  }
  
  oled.println(exercises[exerciseIndex]);
  oled.println();
  
  oled.print("Hold for ");
  oled.print(hold[exerciseIndex]);
  oled.println(" seconds");
  oled.println();
  
  oled.print("Sets:");
  oled.println(sets[exerciseIndex]);
  
  while(digitalRead(selButton)== LOW && digitalRead(stButton)== LOW){

  }
}

/*void startupSequence(){                    //causing stability issues
  oled.clear();
  for(int i = 0; i<5; i++){
    oled.println("Initializing...");
    delay(200);
    oled.clear();
    delay(200);
  }
  oled.println();
  oled.println();
  oled.println();
  oled.println("Noah's ATTiny85-based");
  oled.println("   Exercise Counter");
  delay(2000);
  oled.clear();
}*/

void beep(byte sound, byte time){
  tone(piezo,sound*200);
  delay(time*3);
  noTone(piezo);
  delay(time);
}

