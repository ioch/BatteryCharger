#include <Arduino.h>
#include "hardware.h"
#include "smpsCharger.h"
#include "Psu.h"
#include "LeadAcidCharger.h"
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include <Encoder.h>


PID voltagePid = {8, 1.50, 0};
PID currentPid = {2.1, 0.05, 0}; 

#define OVERCURRENT_INHIBIT 6
#define OVERCURRENT_SENSE 7

Encoder encoder(ENCODER1, ENCODER2);

Psu psu(DAC_OUT, ISENSE, VSENSE, &currentPid, &voltagePid);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

LeadAcidCharger charger(&psu, &lcd);


void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight

  analogReference(INTERNAL);
  
  pinMode(DAC_OUT, OUTPUT);
  pinMode(OVERCURRENT_SENSE, INPUT);
  pinMode(OVERCURRENT_INHIBIT, OUTPUT);
  pinMode(PUSH_BUTTON, INPUT);
  pinMode(ENCODER1, INPUT);
  pinMode(ENCODER2, INPUT);
  
  psu.off();
  digitalWrite(OVERCURRENT_INHIBIT, LOW);
  
  while(HIGH == digitalRead(OVERCURRENT_SENSE)){
    psu.off();
    digitalWrite(OVERCURRENT_INHIBIT, HIGH);
    Serial.println("Overcurrent situation!");
  }
  digitalWrite(OVERCURRENT_INHIBIT, LOW);

  lcd.backlight(); // finish with backlight on  

  charger.setBulkCurrent(charger.getBulkCurrent());
  charger.startPrechargeState();
} 

boolean isEncoderActive = false;
int lastButtonState = HIGH;

void loop()  { 
  if(HIGH == digitalRead(OVERCURRENT_SENSE)){
    psu.off();
    Serial.println("Overcurrent situation!");
    while(1);
  }

  charger.run();
  if (true == isEncoderActive){
//      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("I(max)=");
      lcd.print(charger.getBulkCurrent()+ encoder.read()*25);
      lcd.print("mA       ");
  }
      
  int currentButtonState = digitalRead(PUSH_BUTTON);
      
  if((LOW == currentButtonState) && (HIGH == lastButtonState)){
      Serial.println("debug");
      if(isEncoderActive){
          int newCurrent = charger.getBulkCurrent()+ encoder.read()*25;
          if(newCurrent > 10000){
              newCurrent = 10000;
          }
          if(newCurrent <500){
              newCurrent = 500;
          }
          charger.setBulkCurrent(newCurrent);
          charger.updateMode();
          charger.printState();
          isEncoderActive= !isEncoderActive;
      } else {
          encoder.write(0);
          isEncoderActive= !isEncoderActive;
      }
  }
  lastButtonState = currentButtonState;
   
}

