/******************************************************************************
Qwiic Joystiic Example 2 - Directions Only
Nick Poole @ SparkX
Feb 6, 2018
https://github.com/sparkfunX/Qwiic_Joystiic

This sketch communicates with the Qwiic Joystiic breakout using I2C and 
reports the direction that the stick is being pushed (up, down, left or
right) using the serial port.

If a connection is succesfully made to the Joystiic on startup, the device
will also return its currently configured I2C Slave Address. For new units, 
this address will be 0x20, any other value on a new unit may be symptomatic 
of communication error.
******************************************************************************/

#include <Wire.h>

byte DEV_ADDR = 0x20;
int reading = 0;
bool dirFlags[5];

void setup() {

  Wire.begin();        // join i2c bus
  Serial.begin(9600);  // start serial for output

  Serial.print("Slave Address in EEPROM: 0x");
  Serial.println(getAddr(), HEX);

  Serial.println("Press the joystick in any direction");

}

void loop() {

for(int i = 0; i < 5 ; i++){
dirFlags[i] = 0;
}

while(getHorizontal()>800){
  if(!dirFlags[0]){
    Serial.println("Left");
  }
  dirFlags[0] = 1;  
}

while(getHorizontal()<200){
  if(!dirFlags[1]){
    Serial.println("Right");
  }
  dirFlags[1] = 1;  
}

while(getVertical()>800){
  if(!dirFlags[2]){
    Serial.println("Up");
  }
  dirFlags[2] = 1;  
}

while(getVertical()<200){
  if(!dirFlags[3]){
    Serial.println("Down");
  }
  dirFlags[3] = 1;  
}

while(getButton()==0){
  if(!dirFlags[4]){
    Serial.println("Click");
  }
  dirFlags[4] = 1;  
}

}

// getHorizontal() returns an int between 0 and 1023 
// representing the Joystiic's horizontal position
// (axis indicated by silkscreen on the board)
// centered roughly on 512

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

// getVertical() returns an int between 0 and 1023 
// representing the Joystiic's vertical position
// (axis indicated by silkscreen on the board)
// centered roughly on 512

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

// getButton() returns a byte indicating the 
// position of the button where a 0b1 is not
// pressed and 0b0 is pressed

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

// getAddr() returns a byte indicating the 
// 7-bit Slave Address that the Joystiic 
// currently has stored in EEPROM. Because
// address changes do not take effect until
// power reset, the value returned from 
// getAddr() may not match the address that 
// the device is currently using. As a 
// result, this call can be used to confirm
// that a setAddr() call was successful.

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

// setAddr() takes a 7-bit I2C Slave Address 
// and passes it to the Joystiic. The Joystiic
// will store this address in EEPROM and use
// it on power reset. If a reserved address is
// passed to this function, it will send it to
// the Joystiic, but the Joystiic will not set
// store it in EEPROM. 
// getAddr() can be used to confirm
// that a setAddr() call was successful.

void setAddr(byte addr) {

  Wire.beginTransmission(DEV_ADDR);
  Wire.write(0x05);
  Wire.write(addr);
  Wire.endTransmission();

}


