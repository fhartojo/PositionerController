#include <Arduino.h>
#include <DualG2HighPowerMotorShield.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

DualG2HighPowerMotorShield24v14 md;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
int desiredSpeed;
int currentSpeed;

void stopOnFault() {
  if (md.getM1Fault()) {
    md.disableDrivers();
    delay(1);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Motor Fault");
    Serial.println("M1 Fault");
    while(1)
      ;
  }
}

void setMotorSpeed() {
  if (currentSpeed == 0) {
    //If the motor is off now, assume we're here to start it.
    md.enableM1Driver();
    delay(1);
  }

  md.setM1Speed(desiredSpeed);
  stopOnFault();
  delay(2);

  if (desiredSpeed == 0) {
    //We're turning the motor off.
    md.disableM1Driver();
  }

  currentSpeed = desiredSpeed;
}

void displayMotorSpeed() {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (currentSpeed == 0) {
    lcd.print("OFF");
  } else {
    lcd.print(currentSpeed);
  }
}

void setup() {
  Serial.begin(9600);

  desiredSpeed = 0;
  currentSpeed = 0;

  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);

  md.init();
  md.calibrateCurrentOffsets();

  delay(10);

  displayMotorSpeed();
}

void loop() {
  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    if (buttons & BUTTON_RIGHT) {
      desiredSpeed += 10;

      if (desiredSpeed > 400) {
        desiredSpeed = 400;
      }
    } else if (buttons & BUTTON_LEFT) {
      desiredSpeed -= 10;

      if (desiredSpeed < -400) {
        desiredSpeed = -400;
      }
    }

    setMotorSpeed();
    displayMotorSpeed();

    //Debounce, of sorts.
    delay(200);
  }
}
