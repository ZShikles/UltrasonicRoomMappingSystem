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

int bluePin = 2;    //IN1 on the ULN2003 Board, BLUE end of the Blue/Yellow motor coil
int pinkPin = 3;    //IN2 on the ULN2003 Board, PINK end of the Pink/Orange motor coil
int yellowPin = 4;  //IN3 on the ULN2003 Board, YELLOW end of the Blue/Yellow motor coil
int orangePin = 5;  //IN4 on the ULN2003 Board, ORANGE end of the Pink/Orange motor coil

//Keeps track of the current step.
//We'll use a zero based index. 
int currentStep = 0;
int waveStepCount = 4;
bool clockwise = true;



#define STEPS 64

int mode=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //pin initialization;
  pinMode(button, INPUT);
  pinMode(button2,INPUT);
  pinMode(button3,INPUT);
  
   
  pinMode(bluePin, OUTPUT);
  pinMode(pinkPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(orangePin,OUTPUT);
  
  digitalWrite(bluePin, LOW);
  digitalWrite(pinkPin, LOW);
  digitalWrite(yellowPin, LOW);
  digitalWrite(orangePin, LOW);

  pinMode(pingECHO,INPUT); 
  pinMode(pingTRIG, OUTPUT);

 
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
      if(i<3){
        motor_rotate(i);
        Serial.println("Awaiting Button Push for Next State");
        while(digitalRead(button2) == LOW){
        }     
      }   
  }
  systems_clear();
  motor_reset();
}

void test2(){           //test procedure for motor
  long time1, time2, elapsedTime;
  for(int i=0; i<3; i++){
    //delay(7000000);
    motor_rotate(i);
    elapsedTime=time2-time1;
    Serial.print("Elapsed Time for rotation cycle ");
    Serial.print(i+1);
    Serial.print(" is ");
    Serial.print(elapsedTime);
    Serial.println(" Milliseconds");
    Serial.println("Awaiting Button Push for Next Cycle");
    while(digitalRead(button3)==LOW){
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
 int targetSteps=1536;
  while(1){
    
  int stepCount = waveStepCount;
  
  //Then we can figure out what our current step within the sequence from the overall current step
  //and the number of steps in the sequence
  int currentStepInSequence = currentStep % stepCount;
  
  //Figure out which step to use. If clock wise, it is the same is the current step
  //if not clockwise, we fire them in the reverse order...
  int directionStep = !clockwise ? currentStepInSequence : (stepCount-1) - currentStepInSequence;  
  
  switch(directionStep){
    case 0:
      digitalWrite(bluePin, HIGH);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);
      break;
    case 1:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, HIGH);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);
      break;
    case 2:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(orangePin, LOW);
      break;
    case 3:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, HIGH);
      break;
  }
  
    // Increment the program field tracking the current step we are on
  ++currentStep;
  
  // If targetSteps has been specified, and we have reached
  // that number of steps, reset the currentStep, and reverse directions
  if(targetSteps != 0 && currentStep == targetSteps){
    currentStep = 0;
    break;
  } else if(targetSteps == 0 && currentStep == stepCount) {
    // don't reverse direction, just reset the currentStep to 0
    // resetting this will prevent currentStep from 
    // eventually overflowing the int variable it is stored in.
    currentStep = 0;
  }
  
  delay(15);
  }
  
}

void motor_rotate(int i){
  int targetSteps=512;
  while(1){
    
  int stepCount = waveStepCount;
  
  //Then we can figure out what our current step within the sequence from the overall current step
  //and the number of steps in the sequence
  int currentStepInSequence = currentStep % stepCount;
  
  //Figure out which step to use. If clock wise, it is the same is the current step
  //if not clockwise, we fire them in the reverse order...
  int directionStep = clockwise ? currentStepInSequence : (stepCount-1) - currentStepInSequence;  
  
  switch(directionStep){
    case 0:
      digitalWrite(bluePin, HIGH);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);
      break;
    case 1:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, HIGH);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, LOW);
      break;
    case 2:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, HIGH);
      digitalWrite(orangePin, LOW);
      break;
    case 3:
      digitalWrite(bluePin, LOW);
      digitalWrite(pinkPin, LOW);
      digitalWrite(yellowPin, LOW);
      digitalWrite(orangePin, HIGH);
      break;
  }
  
    // Increment the program field tracking the current step we are on
  ++currentStep;
  
  // If targetSteps has been specified, and we have reached
  // that number of steps, reset the currentStep, and reverse directions
  if(targetSteps != 0 && currentStep == targetSteps){
    currentStep = 0;
    break;
  } else if(targetSteps == 0 && currentStep == stepCount) {
    // don't reverse direction, just reset the currentStep to 0
    // resetting this will prevent currentStep from 
    // eventually overflowing the int variable it is stored in.
    currentStep = 0;
  }
  
  delay(15);
  }
  
}

  



void get_distance(int i){  //gets distance to object in cm
  long duration;
  
  //Sends request to sensor
  digitalWrite(pingTRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(pingTRIG, HIGH);
  delayMicroseconds(15);
  digitalWrite(pingTRIG, LOW);

  duration = pulseIn(pingECHO,HIGH); //recieves return signal, calculates elapsed time
  distances[i] = duration;// / 29 / 2;  //converts elapsed time to cm
  delay(2000);
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
    myFile.println();
    myFile.println();
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
