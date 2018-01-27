
/*
  Author: Declan Herbertson
  Date: Jan 10, 2018
  Purpose: To control a servo powered grabbing arm with sonar depth sensing
*/


#include <NewPing.h>
#include <Servo.h>

//sonar
#define GroundPin 10
#define echoPin 11
#define trigPin 12
#define VccPin 13
#define MAX_DISTANCE 200//maximum distance set to 200 cm

//adjustable variables
#define ClawOpenPos 100    //open servo position
#define ClawClosePos 150 //close servo position
#define OpenDelay 5000  //time claw reamins closes (millis)

//CloseToClose variables
#define CloseDist 22   //dist before closing (cm)
#define PingsToClose1 25 //pings before closing
#define DelayBetweenPings1 100 //(millis)

//TimeDelayMode variables
#define InitiationDistance 25 //distance before algorithim starts
#define HeightVariation 2 //height descrepency +- at which the count to close will continue(cm)
#define PingsToClose2 50 //pings before closing
#define DelayBetweenPings2 100 //(millis)
#define ToggleSensorPingCount 50 //if sensor reads 0 100 times in a row (malfunction or unplug data cable)
#define NoSensorDelay 5000 //time to close if sensor is removed/malfunctioning (millis)

//Checks if sensor is disconnected or reconnected and toggles lift modes accordingly
#define CheckIfConnected true
#define CheckIfDisconnected false

void SensorConnectionCheck(boolean ConnectionState, int distance);

int BadPings = 0;
int GoodPings = 0;


//prints ping distance to serial
void PingtoSerial(int dist);

//closes claw to position specified by ClawClosePos
void CloseClaw();

//opens claw to position specified by ClawOpenPos
void OpenClaw();

//gets distance from ground (sonar sensor)(in cm)
int GetDist();

//standard close algorithim - Closes Claw when certain distance to the ground
void CloseWhenClose();

//time delay algorithim - closes when remaining at certain height for time, under intialization height
void CloseStayHeight();

//closes on time delay(if sensor isnt plugged in) PrimaryAlgorithim = true for CloseToClose, false for CloseStayHeight
void CloseTimeDelay();

boolean TimeDelayMode = false;

boolean PrimaryAlgorithim = false;


NewPing sonar(trigPin, echoPin, MAX_DISTANCE);  //initialize NewPing

Servo myservo;  // create servo object to control a servo




void setup() {
  //servo
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object

  //sonar
  Serial.begin(9600);          //set data transmission rate to communicate with computer
  pinMode(VccPin, OUTPUT) ;    //tell pin 13 it is going to be an output
  digitalWrite(VccPin, HIGH) ; //tell pin 13 to output HIGH (+5V)
  pinMode(GroundPin, OUTPUT) ; //tell pin 10 it is going to be an output
  digitalWrite(GroundPin, LOW) ; //tell pin 10 to output LOW (0V, or ground)

  //starts claw in open position as defined by the ClawOpenPos variable
  OpenClaw();
  delay(1500);
  //CloseStayHeight();
  CloseWhenClose();

}

void loop() {

  CloseTimeDelay();  

}


void PingtoSerial(int distance) {
  Serial.print("Ping: ");              //print "Ping:" on the computer display
  Serial.print(distance);        //print the value of the variable next
  Serial.println("cm");                //print "cm" after that, then go to next line

}

void CloseClaw() {
  myservo.write(ClawClosePos);
}

void OpenClaw() {
  myservo.write(ClawOpenPos);
}

int GetDist() {
  return sonar.ping_cm();
}

void SensorConnectionCheck(boolean ConnectionState,int distance){

  if(ConnectionState == CheckIfDisconnected){

    if(distance == 0){

      BadPings++;
      Serial.print("BadPing");
      Serial.print(BadPings);
      if (BadPings >= ToggleSensorPingCount) {
        TimeDelayMode = true;       
        loop();
      }     
    }
    else{      
      BadPings = 0;  
    }   
  }
  
  if(ConnectionState == CheckIfConnected){
  
    while (distance != 0) {
      GoodPings++;
      if (GoodPings >= ToggleSensorPingCount) {
        if (PrimaryAlgorithim) {
          GoodPings = 0;
          TimeDelayMode = false;
          CloseWhenClose();
        }
        else {
          GoodPings = 0;
          TimeDelayMode = false;
          CloseStayHeight();
        }
      }
    }
      GoodPings = 0;
    }
    
  }
  


void CloseWhenClose() {
  while(BadPings < 50){
  //this loop waits until a specified number of 'pings' are closer than the specified distance for closing
  int count = 0;
  BadPings = 0;
  while (count < PingsToClose1) {
    Serial.print("\nCloseWhenCloseLoop");

    int distance = GetDist();
    PingtoSerial(distance);

    if (distance <= CloseDist && distance != 0) {
      count++;      
    }
    
    SensorConnectionCheck(CheckIfDisconnected,distance);
    delay(DelayBetweenPings1);

  }
  CloseClaw();
  delay(OpenDelay);
  OpenClaw();

}
}

void CloseStayHeight() {
while(BadPings < 50){
  int PingsInRange = 0;
  int BadPings = 0;
  int ReferenceDistance = GetDist();
  int CurrentDistance = 0;

  while (PingsInRange < PingsToClose2) {

    CurrentDistance = GetDist();
    Serial.print("\nCloseStayHeightLoop");
    PingtoSerial(CurrentDistance);

    if (CurrentDistance <= InitiationDistance && CurrentDistance < ReferenceDistance + HeightVariation && CurrentDistance > ReferenceDistance - HeightVariation && CurrentDistance != 0) {
      PingsInRange++;
    }
    
    else {

      ReferenceDistance = CurrentDistance;
      PingsInRange = 0;
      
    }

    SensorConnectionCheck(CheckIfDisconnected,CurrentDistance);
  }

  CloseClaw();
  delay(OpenDelay);
  OpenClaw();

}
}
void CloseTimeDelay(){

 
  if (TimeDelayMode) {

     Serial.print("\nGood Pings ");
     Serial.print(GoodPings);
     Serial.print("\n distance: ");
    int dist = GetDist();
    Serial.print(dist);
  

    SensorConnectionCheck(CheckIfConnected,dist);
    
    delay(25);

   
    Serial.print("Time Delay: ClawClosed");
    CloseClaw();    
    delay(NoSensorDelay);
    Serial.print("Time Delay: ClawOpened");
    OpenClaw();
    delay(NoSensorDelay);
    
  }
}





