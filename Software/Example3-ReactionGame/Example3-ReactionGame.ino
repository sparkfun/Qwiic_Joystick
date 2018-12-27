/******************************************************************************
  Qwiic Joystiic Example 3 - Reaction Game
  Nick Poole @ SparkX
  Feb 6, 2018
  https://github.com/sparkfunX/Qwiic_Joystiic

  This sketch communicates with the Qwiic Joystiic breakout using I2C and
  hosts a reaction time game over the serial port.

  If a connection is succesfully made to the Joystiic on startup, the device
  will also return its currently configured I2C Slave Address. For new units,
  this address will be 0x20, any other value on a new unit may be symptomatic
  of communication error.
******************************************************************************/

#include <Wire.h>

bool passing = 1;
byte DEV_ADDR = 0x20;
int reading = 0;
int trialTimes[15];
int rseed = 0;

void setup() {

  Wire.begin();        // join i2c bus
  Serial.begin(9600);  // start serial for output

  Serial.print("Slave Address in EEPROM: 0x");
  Serial.println(getAddr(), HEX);

  
  // The tiny85 doesn't have a spare ADC to float for randomSeed.
  // This tends to generate better seeds anyway.
  Serial.println("Please move the Joystick in a circle to seed the Random Number Generator");
  while (getHorizontal() < 800) {};
  for (int t = 0; t < 4; t++) {
    Serial.print("  . ");
    rseed += getHorizontal();
    rseed += getVertical();
    delay(1000);
  }
  randomSeed(rseed);

  // Talk to the player and explain the game
  Serial.println();
  Serial.println("Alright! Let's play...");
  Serial.println();

  Serial.println("Reaction Test!");
  Serial.println("As I call out directions, you try to follow along.");
  Serial.println("After 15 rounds I'll tell you your average reaction time!");

}

void loop() {

  // Waiting for player ready
  Serial.println("Click the Joystick Button to Begin...");
  Serial.println();

  while (getButton()) {}

  // Countdown
  Serial.print("OK! ");
  delay(500);
  Serial.print("3.. ");
  delay(1000);
  Serial.print("2.. ");
  delay(1000);
  Serial.print("1.. ");
  delay(1000);
  Serial.println("GO!");

  // Begin game logic
  int trialCount = 0;
  passing = 1;

  while (trialCount < 15 && passing) {

    if (randomRound(trialCount) == 0) {
      passing = 0;
    }
    trialCount++;

  }

  if (passing) {

    int score = 0;

    for (int i = 0; i < 15; i++) {

      score = score + trialTimes[i];

    }

    score = score / 15;

    Serial.print("Congrats! Your Score Was ");
    Serial.print(score);
    Serial.println(" milliseconds!");

    Serial.println();

    Serial.println("Let's play again!");

  } else {

    Serial.println("Want to try again?");

  }


}

int getHorizontal() {

  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(DEV_ADDR, 2);

  while (Wire.available()) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    reading = (uint16_t)msb << 8 | lsb;
  }

  Wire.endTransmission();
  return reading;

}

int getVertical() {

  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x02);
  Wire.endTransmission(false);
  Wire.requestFrom(DEV_ADDR, 2);

  while (Wire.available()) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    reading = (uint16_t)msb << 8 | lsb;
  }

  Wire.endTransmission();
  return reading;

}


byte getButton() {

  byte btnbyte = 0b00000000;

  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x04);
  Wire.endTransmission(false);
  Wire.requestFrom(DEV_ADDR, 1);    // request 1 byte

  while (Wire.available()) {
    btnbyte = Wire.read();
  }

  Wire.endTransmission();
  return btnbyte;

}

byte getAddr() {

  byte addr = 0x00;

  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x06);
  Wire.endTransmission(false);
  Wire.requestFrom(DEV_ADDR, 1);    // request 1 byte

  while (Wire.available()) {
    addr = Wire.read();
  }

  Wire.endTransmission();
  return addr;

}

void setAddr(byte addr) {

  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x05);
  Wire.write(addr);
  Wire.endTransmission();

}

bool randomRound(int trialNumber) {

  delay(random(1, 4) * 1000);

  bool pass = 0;
  int roundTime;
  int startTime;

  switch (random(0, 3)) {

    case 0:

      Serial.print("Right! ");
      startTime = millis();
      while (getHorizontal() > 300) {
        if (getHorizontal() > 800 || getVertical() > 800 || getVertical() < 300) {
          Serial.println("  Oops!! You pressed the wrong direction!");
          pass = 0;
          goto roundEnd;
        }
      }
      roundTime = millis() - startTime;
      pass = 1;
      Serial.print("...Nice, ");
      Serial.print(roundTime);
      Serial.println("ms!");
      break;

    case 1:

      Serial.print("Left! ");
      startTime = millis();
      while (getHorizontal() < 800) {
        if (getHorizontal() < 300 || getVertical() > 800 || getVertical() < 300) {
          Serial.println("  Oops!! You pressed the wrong direction!");
          pass = 0;
          goto roundEnd;
        }
      }
      roundTime = millis() - startTime;
      pass = 1;
      Serial.print("...Nice, ");
      Serial.print(roundTime);
      Serial.println("ms!");
      break;

    case 2:

      Serial.print("Down! ");
      startTime = millis();
      while (getVertical() > 300) {
        if (getVertical() > 800 || getHorizontal() > 800 || getHorizontal() < 300) {
          Serial.println("  Oops!! You pressed the wrong direction!");
          pass = 0;
          goto roundEnd;
        }
      }
      roundTime = millis() - startTime;
      pass = 1;
      Serial.print("...Nice, ");
      Serial.print(roundTime);
      Serial.println("ms!");
      break;

    case 3:

      Serial.print("Up! ");
      startTime = millis();
      while (getVertical() < 800) {
        if (getVertical() < 300 || getHorizontal() > 800 || getHorizontal() < 300) {
          Serial.println("  Oops!! You pressed the wrong direction!");
          pass = 0;
          goto roundEnd;
        }
      }
      roundTime = millis() - startTime;
      pass = 1;
      Serial.print("...Nice, ");
      Serial.print(roundTime);
      Serial.println("ms!");
      break;

  }


roundEnd:

  if (!pass) {
    return 0;
  } else {
    trialTimes[trialNumber] = roundTime;
    return 1;
  }

}
