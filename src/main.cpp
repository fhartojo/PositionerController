#include <Arduino.h>
#include <DualG2HighPowerMotorShield.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include "version.h"
#include "config.h"

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
bool startMotor = false;

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

void adjustDesiredSpeed() {
  if (desiredSpeed > MOTOR_SPEED_MAX) {
    desiredSpeed = MOTOR_SPEED_MAX;
  } else if (desiredSpeed < -MOTOR_SPEED_MAX) {
    desiredSpeed = -MOTOR_SPEED_MAX;
  }
}

void setM1Speed(int i) {
  md.setM1Speed(i);
  stopOnFault();
  delay(2);
}

void setMotorSpeed() {
  if (currentSpeed == 0) {
    //If the motor is off now, check if we're to start it.
    if (startMotor) {
      md.enableM1Driver();
      delay(1);
    } else {
      return;
    }
  } else {
    //If the motor is running, check if we're to stop it.
    if (!startMotor) {
      if (currentSpeed > 0) {
        for (int i = currentSpeed - 1; i >= 0; i--) {
          setM1Speed(i);
        }
      } else {
        for (int i = currentSpeed + 1; i <= 0; i++) {
          setM1Speed(i);
        }
      }

      md.disableM1Driver();
      currentSpeed = 0;

      return;
    }
  }

  if (desiredSpeed > currentSpeed) {
    for (int i = currentSpeed + 1; i <= desiredSpeed; i++) {
      setM1Speed(i);
    }
  } else if (desiredSpeed < currentSpeed) {
    for (int i = currentSpeed - 1; i >= desiredSpeed; i--) {
      setM1Speed(i);
    }
  }

  if (desiredSpeed == 0) {
    //We're turning the motor off.
    md.disableM1Driver();
  }

  currentSpeed = desiredSpeed;
}

void displayWelcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Rotary Table");
  lcd.setCursor(0, 1);
  lcd.print("v");
  lcd.print(RELEASE_VERSION);
}

void displayForm() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Speed:");
  lcd.setCursor(0, 1);
  lcd.print("SELECT to ");
}

void displayMotorSpeed() {
  lcd.setCursor(6, 0);
  lcd.print("         ");
  lcd.setCursor(6, 0);

  if (currentSpeed == 0) {
    lcd.print("off");
  } else {
    lcd.print(currentSpeed);
  }

  lcd.print("/");

  if (desiredSpeed == 0) {
    lcd.print("off");
  } else {
    lcd.print(desiredSpeed);
  }
}

void displayAction() {
  lcd.setCursor(10, 1);
  lcd.print("     ");
  lcd.setCursor(10, 1);
  lcd.print(startMotor ? "stop" : "run");
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

  displayWelcome();
  delay(2000);

  displayForm();
  displayMotorSpeed();
  displayAction();
}

void loop() {
  uint8_t buttons = lcd.readButtons();

  if (buttons) {
    if (buttons & BUTTON_RIGHT) {
      desiredSpeed += MOTOR_SPEED_STEP_COARSE;
    } else if (buttons & BUTTON_LEFT) {
      desiredSpeed -= MOTOR_SPEED_STEP_COARSE;
    } else if (buttons & BUTTON_UP) {
      if (desiredSpeed > 0) {
        desiredSpeed += MOTOR_SPEED_STEP_FINE;
      } else if (desiredSpeed < 0) {
        desiredSpeed -= MOTOR_SPEED_STEP_FINE;
      }
    } else if (buttons & BUTTON_DOWN) {
      if (desiredSpeed > 0) {
        desiredSpeed -= MOTOR_SPEED_STEP_FINE;
      } else if (desiredSpeed < 0) {
        desiredSpeed += MOTOR_SPEED_STEP_FINE;
      }
    } else if (buttons & BUTTON_SELECT) {
      startMotor = !startMotor;
    }

    adjustDesiredSpeed();
    setMotorSpeed();
    displayMotorSpeed();
    displayAction();

    //Debounce.
    delay(200);
  }
}
