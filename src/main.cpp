#include <Arduino.h>
#include <stdio.h>
#include <LiquidCrystal.h>
#include "components.h"
#include "helpers.h"

// one hour in milliseconds
#define HOUR 36000000

// declare variables
int stages = 3;
int peopleCounter = 0;
long long int lastMillisRead = 0;
bool configuring = false;
bool stageMode = true;

// declare pins
PIR activitySensor(A0, 1000);
PIR insideSensor(A1, 1500);
PIR outsideSensor(A2, 1500);
LED awaitingStatusPin(2);
LED outputLED(3, &stageMode, &stages, &peopleCounter);
LED calibratingStatusPin(4);
const int rightButton = 5;
const int leftButton = 6;

// create LCD instance
LiquidCrystal lcd(7, 13, 8, 9, 10, 12);

// create main page instance
MainProfile lcdMain(&lcd, &stageMode, &stages, &peopleCounter, &lastMillisRead);

// create configure page instance
ConfigureProfile lcdPrompt(&lcd, &stages);

void setup() {
  // set pin mode
  pinMode(insideSensor.getPin(), INPUT);
  pinMode(outsideSensor.getPin(), INPUT);
  pinMode(activitySensor.getPin(), INPUT);
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);
  pinMode(calibratingStatusPin.getPin(), OUTPUT);
  pinMode(awaitingStatusPin.getPin(), OUTPUT);
  pinMode(outputLED.getPin(), OUTPUT);
  
  // set data transfer to 9600 bits/second (comment out if using physical LCD)
  Serial.begin(9600);

  // begin lcd
  lcd.begin(16,2);

  // calibrate sensor
  calibrateSensor(60, calibratingStatusPin, lcd);
  
  // main display
  lcdMain.initial();
}

void loop() {
  // update idle time
  lcdMain.updateIdleTime();

  // check idle time
  if(isOverIdleTime(lastMillisRead, HOUR) && peopleCounter > 0){
    // reset
    peopleCounter = 0;

    // reset lcd, timer
    lcdMain.initial();

    // turn off lamp
    outputLED.toggle();
  }

  // start configuring mode
  if(digitalRead(rightButton) && configuring == false){
    configuring = true;
    lcdPrompt.initial();
  }

  // configuring mode
  while(configuring){
    // wait for button press
    
    // increment / reset to 1
    if(digitalRead(rightButton) == HIGH){
      if(stages < 8)
        stages++; 
      else 
        stages = 1;
      lcdPrompt.updateStage();
      delay(300);
    }

    // exit configure mode
    if(digitalRead(leftButton) == HIGH && stages > 1){
      lcdMain.initial();
      configuring = false;
      outputLED.toggle();
      delay(200);
    }
  }

  // toggle stage mode
  if(digitalRead(leftButton) == HIGH){
    if(stageMode)
      stageMode = false;
    else 
      stageMode = true;
    
    // update lamp
    outputLED.toggle();

    // update display
    lcdMain.updateMode();

    delay(300);
  }

  // wait for outside sensor activation
  if(!outsideSensor.awaiting){
    if(outsideSensor.isActivated()){
      Serial.println("Outside sensor awaiting...");
      outsideSensor.awaiting = true;
    }
  }
  // awaiting for opposite
  else {
    awaitingStatusPin.keepHigh();                           // turn on status led

    if(!outsideSensor.isOverAwait()){
      // still under await time
      if(insideSensor.isActivated()){
        // enter motion detected
        Serial.println("Enter motion detected.");
        peopleCounter++;                                    // increment
        if(peopleCounter == 1) lastMillisRead = millis();   // get millis last read 
        awaitingStatusPin.toggle();                         // turn off status led
        outputLED.toggle();                                 // update lamp
        lcdMain.updatePeople();                             // update display
        outsideSensor.awaiting = false;                     // exit await mode
        delay(1500);         
      }
      insideSensor.awaiting = false;
    }
    else {
      // over await time
      Serial.println("Exiting await mode");
      awaitingStatusPin.toggle();                           // turn off status led
      delay(500);
    }
  }

  // wait for inside sensor activation
  if(!insideSensor.awaiting){
    if(insideSensor.isActivated()){
      Serial.println("Inside sensor awaiting...");
      insideSensor.awaiting = true;
    } 
  }
  // awaiting for opposite 
  else {
    awaitingStatusPin.keepHigh();                 // turn on status led

    if(!insideSensor.isOverAwait()){
      // still under await time
      if(outsideSensor.isActivated()){
        // exit motion detected
        Serial.println("Exit motion detected");
        peopleCounter--;                          // decrement
        awaitingStatusPin.toggle();               // turn off status led
        insideSensor.awaiting = false;            // exit awaiting mode
        lcdMain.updatePeople();                   // update display
        outputLED.toggle();                       // upodate lamp
        delay(1500);
      }
      outsideSensor.awaiting = false;             // don't let opposite wait
    }
    else {
      // over await time
      Serial.println("Exiting await mode");
      awaitingStatusPin.toggle();                 // turn off status led
      delay(500);
    }
  }

  // detected motion in room
  if(!activitySensor.awaiting){                             // if unlocked
    if(activitySensor.isActivated()){                       // if activity detected
      Serial.println("Activity Detected");
      if(peopleCounter > 0) lastMillisRead = millis();
      activitySensor.awaiting = true;                       // lock sensor 
    }
  }
  else {
      activitySensor.isOverAwait();                         // unlock if lock time is over
  }
}