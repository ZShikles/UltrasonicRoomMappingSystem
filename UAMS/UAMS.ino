#include <Stepper.h>

#include <SD.h>
#include <SPI.h>



long areas[3]={0,0,0};
long distances[4]={0,0,0,0};
long pos = 0;

const int pingTRIG = 12;
const int pingECHO=13;
int messageFlag=0;
const int button=6;
const int button2=7;
const int button3=8;

#define STEPS 64

int mode=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //pin initialization;
  pinMode(button, INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);
  
  //initialize SD
  if (!SD.begin(10)) {
    Serial.println(F("Card Failed, or Not Present"));
    //exit;
  }
  else{
    Serial.println("Card Initialized.");
  }

  pinMode(10,OUTPUT);
  

  systems_clear();
}

void loop() {
  if(messageFlag==0){
     Serial.println("Awaiting Mode Selection");
     Serial.println("Button 1: Standard Operation");
     Serial.println("Button 2: Sensor Test Protocol");
     Serial.println("Button 3: Motor Test Protocol");
     messageFlag=1;
  }
  
  if(digitalRead(button) == HIGH){//wait for button push to select system mode
    mode = 1;
  }
  else if(digitalRead(button2) == HIGH){
    mode = 2;
  }
  else if(digitalRead(button3) == HIGH){
    mode = 3;
  }
  else{
    mode = 0;
  }
  
  switch(mode){
    case 1:
      Serial.println("Standard Protocol Engaged");
      standard();
      break;
    case 2:
      Serial.println("Sensor Test Protocol Engaged");
      test1();
      break;
    case 3:
      Serial.println("Motor Test Protocol Engaged");
      test2();
      break;
    default:
      break;
  }
}

void standard(){          //Normal functionality
  //delay(7000000);       //delays 7 seconds
  for(int i=0; i<4; i++){
      get_distance(i);   //gets distance to object(wall)
      if (i<3){         //rotates motor once per cycle
          motor_rotate(i); 
      }
      //delay(7000000);   //waits 7 seconds  
    }
    
    calc_area();        //calculates area
    sd_write();         //writes data to SD card
    systems_clear();    //empties arrays
    motor_reset();      //returns motor to starting position
}

void test1(){           //test procedure for sensor
  for(int i=0; i<4; i++){
      get_distance(i);
      Serial.print("Distance ");
      Serial.print(i+1);
      Serial.print(" is ");
      Serial.println(distances[i]);
      if(i<3){
        motor_rotate(i);
        Serial.println("Awaiting Button Push for Next State");
        while(digitalRead(button2) == HIGH){
        }     
      }   
  }
  systems_clear();
  motor_reset();
}

void test2(){           //test procedure for motor
  long time1, time2, elapsedTime;
  for(int i=0; i<3; i++){
    delay(7000000);
    motor_rotate(i);
    elapsedTime=time2-time1;
    Serial.print("Elapsed Time for rotation cycle ");
    Serial.print(i+1);
    Serial.print(" is ");
    Serial.print(elapsedTime);
    Serial.println(" Milliseconds");
    Serial.println("Awaiting Button Push for Next Cycle");
    while(digitalRead(button3)==HIGH){
    }
  }
  systems_clear();
  motor_reset();
}

void systems_clear(){ //empties arrays
  for(int i=0; i<3; i++){
    areas[i]=0;
    distances[i]=0;
  }
  distances[3]=0;

  messageFlag=0;
  mode=0;
  
  Serial.println("Arrays emptied, Flags Reset");
}

void motor_reset(){
  //YET TO IMPLEMENT
  Stepper stepper(STEPS, 2,3,4,5);
  stepper.setSpeed(2);
  stepper.step(-64);
  Serial.println("Motor position reset to position 1");
}

void motor_rotate(int i){
  //YET TO IMPLEMENT
  Stepper stepper(STEPS, 2,3,4,5);
  stepper.setSpeed(2);
  stepper.step(16);
  Serial.print("Motor Rotated to position ");
  Serial.println(i+1);
}

void get_distance(int i){  //gets distance to object in cm
  long duration;
  
  //Sends request to sensor
  pinMode(pingTRIG, OUTPUT);
  digitalWrite(pingTRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(pingTRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingTRIG, LOW);

  pinMode(pingECHO,INPUT); 
  duration = pulseIn(pingECHO,HIGH); //recieves return signal, calculates elapsed time
  distances[i] = duration / 29 / 2;  //converts elapsed time to cm

  Serial.print("Distance ");
  Serial.print(i+1);
  Serial.print(" is ");
  Serial.println(distances[i]);
}

void calc_area(){
    areas[0]=distances[0]+distances[2]; //calculates length
    areas[1]=distances[1]+distances[3]; //calculates width
    areas[2]=areas[0]*areas[1];         //calculates area
    Serial.println("Areas Calculated");
}

void sd_write(){
  File myFile = SD.open("DataLog.csv", FILE_WRITE); //opens file on card

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to DataLog.csv...");

    //writes areas to file
    for(int i=0; i<3; i++){
      myFile.print(areas[i]);
      myFile.print(",");        //seperates data
    }

    //writes distances to file
    for(int i=0; i<4; i++){  
      myFile.print(distances[i]);
      myFile.print(",");       //seperates data
    }
    
    myFile.println(); //bumps to next line in file;
    
    // close the file:
    myFile.close();
    Serial.println("done.");
    
   } else {
    // if the file didn't open, print an error:
    Serial.println("error opening DataLog.csv");
  }
}
