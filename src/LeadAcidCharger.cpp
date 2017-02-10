#include "LeadAcidCharger.h"

LeadAcidCharger::LeadAcidCharger(Psu *_psu, LiquidCrystal_I2C* _lcd):
    psu(_psu),
    lcd(_lcd)
{

}

void LeadAcidCharger::run() {
  if(isControlTime()){
    psu->servo();
    avgCurrent = (((int32_t) psu->getCurrent() - avgCurrent) >> 4) + avgCurrent;
    avgVoltage = (((int32_t) psu->getVoltage() - avgVoltage) >> 4) + avgVoltage;
    
    if(MODE_BULK == mode) {
      if(millis() - stageStarted > 8000 && avgVoltage > absorptionVoltage + voltsPerDegreeCompensation()) {
        startAbsorptionStage();
      }
    } else if(MODE_ABSORPTION == mode) {
	  if(millis() - stageStarted > 3000 && avgCurrent > bulkCurrent){
		  setBulkCurrent(bulkCurrent-100);
		  startBulkStage();
	  }	
      else if(millis() - stageStarted > 3000 && avgCurrent < bulkEndCurrent) {
        startFloatStage();
      }
    } else if(MODE_FLOAT == mode) {
    } else if(MODE_PRECHARGE == mode) {
      if(millis() - stageStarted > 3000 && avgVoltage > criticallyDischargedVoltage + voltsPerDegreeCompensation()) {
        startBulkStage();
      }
    }
    
    checkBatteryConnected();

//    Serial.print(psu->getVoltage());
//    Serial.print(", ");
//    Serial.print(psu->getCurrent());
//    Serial.print(", ");
//    Serial.print(psu->getControllSignal());
//    Serial.print(", ");
    Serial.print(avgVoltage);
    Serial.print(", ");
    Serial.print(avgCurrent);
    Serial.println();
    
    if(millis() - lcdOutTimestamp > 1000) {
        lcd->setCursor(0,1);
        lcd->print(avgVoltage);        
//        lcd->print(avgVoltage / 1000);
//        lcd->print(".");
//        lcd->print(avgVoltage % 1000);
        lcd->print("mV ");
        lcd->print(avgCurrent);
//        lcd->print(avgCurrent / 1000);
//        lcd->print(".");
//        lcd->print(avgCurrent % 1000);
        lcd->print("mA ");
        lcdOutTimestamp = millis();
    }

  }
}

void LeadAcidCharger::printState() {
	lcd->clear();
//	lcd->setCursor(0,0);
	switch(mode) {
		case MODE_PRECHARGE : lcd->print("PRECHARGE");
				 break;
		case MODE_BULK : lcd->print("BULK CHARGE");
				 break;
		case MODE_ABSORPTION : lcd->print("ABSORPTION");
				 break;
		case MODE_FLOAT : lcd->print("FLOAT");
				 break;
	}
	
}

void LeadAcidCharger::startPrechargeState() {
  Serial.println("Starting PRECHARGE");
  mode = MODE_PRECHARGE;
  printState();
  psu->setConstantCurrent(prechargeCurrent);
  stageStarted = millis();
}

void LeadAcidCharger::startBulkStage() {
  Serial.println("Starting BULK CHARGE");
  mode = MODE_BULK; 
  printState();
  psu->setConstantCurrent(bulkCurrent);
  stageStarted = millis();
}

void LeadAcidCharger::startAbsorptionStage() {
  Serial.println("Starting ABSORPTION CHARGE");
  mode = MODE_ABSORPTION; 
  printState();
  psu->setConstantVoltage(absorptionVoltage-200);		// check leter. PID are not fast enougth
  stageStarted = millis();
}

void LeadAcidCharger::startFloatStage() {
  Serial.println("Starting FLOATING CHARGE");
  mode = MODE_FLOAT; 
  printState();
  psu->setConstantVoltage(floatVoltage);
  stageStarted = millis();
}

void LeadAcidCharger::updateMode() {
	switch(mode) {
		case MODE_PRECHARGE : startPrechargeState();
				 break;
		case MODE_BULK : startBulkStage();
				 break;
		case MODE_ABSORPTION : startAbsorptionStage();
				 break;
		case MODE_FLOAT : startFloatStage();
				 break;
	}
}

void LeadAcidCharger::setBulkCurrent(uint16_t c) {
  bulkCurrent = c;
  prechargeCurrent = bulkCurrent / 10;
  bulkEndCurrent = bulkCurrent / 10;
}

uint16_t LeadAcidCharger::getBulkCurrent() {
  return bulkCurrent;
}

void LeadAcidCharger::checkBatteryConnected() {
  if(millis() - stageStarted > 20000){
    if(psu->getCurrent() < 50) {
      psu->off();
      Serial.println("Connect the battery!");
      lcd->setCursor(0,0);
      lcd->print("CONNECT BATTERY"); 
      while(1);
    }
  }
}


int8_t LeadAcidCharger::getBatteryTemperature() {				//  #I asume it is not correct way of getting temp :D
  return 20;
}

//battery temp coeff -3.9 mV/C°/cell = -0.0234 V/C°                  #dude wrong more like 5mV/C/Cell
int16_t LeadAcidCharger::voltsPerDegreeCompensation() {
  return (int16_t)(getBatteryTemperature() - 20) * -234 / 10;
}

uint8_t LeadAcidCharger::isControlTime() {
  if(millis() - lastControlTimestamp >= VOLTAGE_CTRL_INTERVAL){
    lastControlTimestamp = millis();
    return 1;
  }
  return 0;
}

