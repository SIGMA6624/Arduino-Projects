//try putting a 500uF cap

char input;
int pin[8] = {4,5,6,7,8,9,10,11}; 

void setup()                    // run once, when the sketch starts
{
 Serial.begin(9600);            // set the baud rate to 9600, same should be of your Serial Monitor
 //Serial.println("Initializing...");
 for (int i = 2;i<6;i++)
   pinMode(i,OUTPUT);
}

void loop(){                   //runs again and again
  if(Serial.available()){
    //Serial.println("Reading...");
    input = Serial.read();
  }
  if(input == 'a'){
    forward();
  }
  else if(input == 'b'){
    backward();
  }
  else if(input == 'c'){
    hardLeft();
  }
  else if(input == 'd'){
    hardRight();
  }
  else if(input == 'e'){
    stop();
  }
  //delay(5);
  //Serial.clear();
}

void forward(){
  digitalWrite(pin[0],HIGH);
  digitalWrite(pin[1],LOW);
  digitalWrite(pin[2],HIGH);
  digitalWrite(pin[3],LOW);
  digitalWrite(pin[4],HIGH);
  digitalWrite(pin[5],LOW);
  digitalWrite(pin[6],HIGH);
  digitalWrite(pin[7],LOW);
}

void backward(){
  digitalWrite(pin[0],LOW);
  digitalWrite(pin[1],HIGH);
  digitalWrite(pin[2],LOW);
  digitalWrite(pin[3],HIGH);
  digitalWrite(pin[4],LOW);
  digitalWrite(pin[5],HIGH);
  digitalWrite(pin[6],LOW);
  digitalWrite(pin[7],HIGH);
}

void left(){
  digitalWrite(pin[0],HIGH);
  digitalWrite(pin[1],LOW);
  digitalWrite(pin[2],HIGH);
  digitalWrite(pin[3],LOW);
  digitalWrite(pin[4],LOW);
  digitalWrite(pin[5],LOW);
  digitalWrite(pin[6],LOW);
  digitalWrite(pin[7],LOW);
}

void hardLeft(){
  digitalWrite(pin[0],HIGH);
  digitalWrite(pin[1],LOW);
  digitalWrite(pin[2],HIGH);
  digitalWrite(pin[3],LOW);
  digitalWrite(pin[4],LOW);
  digitalWrite(pin[5],HIGH);
  digitalWrite(pin[6],LOW);
  digitalWrite(pin[7],HIGH);
}

void right(){
  digitalWrite(pin[0],LOW);
  digitalWrite(pin[1],LOW);
  digitalWrite(pin[2],LOW);
  digitalWrite(pin[3],LOW);
  digitalWrite(pin[4],HIGH);
  digitalWrite(pin[5],LOW);
  digitalWrite(pin[6],HIGH);
  digitalWrite(pin[7],LOW);
}

void hardRight(){
  digitalWrite(pin[0],LOW);
  digitalWrite(pin[1],HIGH);
  digitalWrite(pin[2],LOW);
  digitalWrite(pin[3],HIGH);
  digitalWrite(pin[4],HIGH);
  digitalWrite(pin[5],LOW);
  digitalWrite(pin[6],HIGH);
  digitalWrite(pin[7],LOW);
}

void stop(){
  digitalWrite(pin[0],LOW);
  digitalWrite(pin[1],LOW);
  digitalWrite(pin[2],LOW);
  digitalWrite(pin[3],LOW);
  digitalWrite(pin[4],LOW);
  digitalWrite(pin[5],LOW);
  digitalWrite(pin[6],LOW);
  digitalWrite(pin[7],LOW);
}

void test(){
  forward();
  delay(1000);
  backward();
  delay(1000);
  left();
  delay(1000);
  hardLeft();
  delay(1000);
  right();  
  delay(1000);
  hardRight();
  delay(1000);

}
  
