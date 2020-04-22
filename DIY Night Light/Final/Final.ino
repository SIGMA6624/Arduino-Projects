

int reading;

unsigned long currentMillis;        
unsigned long previousMillis = 0;        

const long interval = 100;           

int test = A0;
int led = 12;

int ldr[5];

int j = 0;

void setup(){
  Serial.begin(9600);
  
  
  pinMode(test,INPUT);
  pinMode(led, OUTPUT);
  
  
  for(int i = 0; i<5; i++){
    ldr[i]=0;
  }
}

void loop(){
  reading = analogRead(test);
  if(reading >= 950){
    Serial.println(reading);
    digitalWrite(led,HIGH);
  }
  else{
    Serial.println(reading);
    digitalWrite(led,LOW);
  }
}
