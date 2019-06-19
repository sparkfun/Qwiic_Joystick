/*
  An I2C based Thumb Joystick
  By: Nathan Seidle
  Modified by: Wes Furuya
  SparkFun Electronics
  Date: February 5, 2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Qwiic Joystick is an I2C based thumb joystick that reports the joystick position and if the button is pressed.
  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15168
  To install support for ATtiny84 in Arduino IDE: https://github.com/SpenceKonde/ATTinyCore/blob/master/Installation.md
  This core is installed from the Board Manager menu
  This core has built in support for I2C S/M and serial
  If you have Dave Mellis' ATtiny installed you may need to remove it from \Users\xx\AppData\Local\Arduino15\Packages
  To support 400kHz I2C communication reliably ATtiny84 needs to run at 8MHz. This requires user to
  click on 'Burn Bootloader' before code is loaded.
*/

#include <Wire.h>
#include <EEPROM.h>
#include "nvm.h"

#if defined(__AVR_ATtiny85__)
#include "PinChangeInterrupt.h" //Nico Hood's library: https://github.com/NicoHood/PinChangeInterrupt/
//Used for pin change interrupts on ATtinys (joystick button causes interrupt)
//Note: To make this code work with Nico's library you have to comment out https://github.com/NicoHood/PinChangeInterrupt/blob/master/src/PinChangeInterruptSettings.h#L228
#endif

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

#if defined(__AVR_ATmega328P__)
//Hardware connections while developing with an Uno
const byte Button_Pin = 2;
const byte Vertical_Pin = A1;
const byte Horizontal_Pin = A0;

#elif defined(__AVR_ATtiny85__)
////For Debug Only
//#include <SoftwareSerial.h>
//#define TX    3   //P3, Pin 2
//#define RX    4   //P4, Pin 3
//SoftwareSerial TinySerial(RX, TX);

//Hardware connections for the final design
const byte Button_Pin = 1;
const byte Vertical_Pin = 3;
const byte Horizontal_Pin = 2;
#endif

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//Variables used in the I2C interrupt so we use volatile

//This is the pseudo register map of the product. If user asks for 0x02 then get the 3rd
//byte inside the register map.
//5602/118 on ATtiny84 prior to conversion
//Approximately 4276/156 on ATtiny84 after conversion
struct memoryMap {
  byte id;              // Reg: 0x00 - Default I2C Address
  byte firmwareMajor;   // Reg: 0x01 - Firmware Number
  byte firmwareMinor;   // Reg: 0x02 - Firmware Number
  byte X_MSB;           // Reg: 0x03 - Current Joystick Horizontal Position (MSB)
  byte X_LSB;           // Reg: 0x04 - Current Joystick Horizontal Position (LSB)
  byte Y_MSB;           // Reg: 0x05 - Current Joystick Vertical Position (MSB)
  byte Y_LSB;           // Reg: 0x06 - Current Joystick Vertical Position (LSB)
  byte Button_State;    // Reg: 0x07 - Current Button State (clears Reg 0x08)
  byte Button_Status;   // Reg: 0x08 - Indicator for if button was pressed since last
                        //             read of button state (Reg 0x07). Clears after read.
  byte i2cLock;         // Reg: 0x09 - Must be changed to 0x13 before I2C address can be changed.
  byte i2cAddress;      // Reg: 0x0A - Set I2C New Address (re-writable). Clears i2cLock.
};

//These are the default values for all settings
volatile memoryMap registerMap = {
  .id = I2C_ADDRESS_DEFAULT, //Default I2C Address (0x20)
  .firmwareMajor = 0x02, //Firmware version. Helpful for tech support.
  .firmwareMinor = 0x06,
  .X_MSB = 0,
  .X_LSB = 0,
  .Y_MSB = 0,
  .Y_LSB = 0,
  .Button_State = 0x00,
  .Button_Status = 0x00,
  .i2cLock = 0x00,
  .i2cAddress = I2C_ADDRESS_DEFAULT,
};

//This defines which of the registers are read-only (0) vs read-write (1)
memoryMap protectionMap = {
  .id = 0x00,
  .firmwareMajor = 0x00,
  .firmwareMinor = 0x00,
  .X_MSB = 0x00,
  .X_LSB = 0x00,
  .Y_MSB = 0x00,
  .Y_LSB = 0x00,
  .Button_State = 0x00,
  .Button_Status = 0x01,
  .i2cLock = 0xFF,
  .i2cAddress = 0xFF,
};

//Cast 32bit address of the object registerMap with uint8_t so we can increment the pointer
uint8_t *registerPointer = (uint8_t *)&registerMap;
uint8_t *protectionPointer = (uint8_t *)&protectionMap;

volatile byte registerNumber; //Gets set when user writes an address. We then serve the spot the user requested.

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


void setup(void)
{
  //Pin Definitions
  pinMode(Button_Pin, INPUT_PULLUP);
  pinMode(Vertical_Pin, INPUT); //No pull-up. External 10k
  pinMode(Horizontal_Pin, INPUT); //No pull-up. External 10k

  turnOffExtraBits(); //Turn off all unused peripherals
  readSystemSettings(); //Load all system settings from EEPROM
  setupInterrupts(); //Enable pin change interrupts for I2C and button
  startI2C(); //Determine the I2C address we should be using and begin listening on I2C bus

#if defined(__AVR_ATmega328P__)
  Serial.begin(9600);
  Serial.println("Qwiic Joystick");
  Serial.print("Address: 0x");
  Serial.print(registerMap.i2cAddress, HEX);
  Serial.println();
//// For Debug
//#else defined(__AVR_ATtiny85__)
//  TinySerial.begin(9600);
//  TinySerial.println("Qwiic Joystick");
//  TinySerial.print("Address: 0x");
//  TinySerial.print(registerMap.i2cAddress, HEX);
//  TinySerial.println();
#endif
}

void loop(void)
{

// Debug and Testing
#if defined(__AVR_ATmega328P__)

  updateJoystick();
  
  uint16_t X_Pos =  ((registerMap.X_MSB<<8) | registerMap.X_LSB)>>6;
  uint16_t Y_Pos =  ((registerMap.Y_MSB<<8) | registerMap.Y_LSB)>>6;

  Serial.print("X: ");
  Serial.print(X_Pos);

  Serial.print(" Y: ");
  Serial.print(Y_Pos);

  Serial.print(" Reg: ");
  Serial.print(registerMap.i2cAddress);
  
  Serial.print(" ID: ");
  Serial.print(registerMap.i2cLock);

  if (registerMap.Button_Status == 1)
    Serial.print(" Click");


//  Serial.print(registerMap.id, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.firmwareMajor, HEX);
//  Serial.print(".");
//  Serial.print(registerMap.firmwareMinor, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.X_MSB);
//  Serial.print(registerMap.X_LSB, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.Y_MSB);
//  Serial.print(registerMap.Y_LSB, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.Button_State, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.Button_Status, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.i2cLock, HEX);
//  Serial.print(" ");
//  Serial.print(registerMap.i2cAddress, HEX);
  Serial.println();
  delay(200);
//#else defined(__AVR_ATtiny85__)
//  TinySerial.print(" Addr: ");
//  TinySerial.print(registerMap.i2cAddress);
//  
//  TinySerial.print(" Lock: ");
//  TinySerial.print(registerMap.i2cLock);
//
//  TinySerial.println();
//  delay(200);
#endif

  //Sleep until interrupt
  //set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode(); //Stop everything and go to sleep. Wake up from Button interrupts.
}

//If the current setting is different from that in EEPROM, update EEPROM
void recordSystemSettings(void)
{
  //I2C address is byte
  byte i2cAddr;

  //TinySerial.println("Set Registers"); //Debug message

  //Error check the current I2C address
  if (registerMap.i2cAddress < 0x08 || registerMap.i2cAddress > 0x77)
  {
    //User has set the address out of range
    //Go back to defaults
    registerMap.i2cAddress = I2C_ADDRESS_DEFAULT;
  }

  //TinySerial.println("  Check value is in range"); //Debug message

  
  //Read the value currently in EEPROM. If it's different from the memory map then record the memory map value to EEPROM.
  EEPROM.get(LOCATION_I2C_ADDRESS, i2cAddr);
  if (i2cAddr != registerMap.i2cAddress && registerMap.i2cLock == 0x13)
  {    
    //TinySerial.print(registerMap.i2cLock, HEX); //Debug message
    registerMap.i2cLock = 0x00;
    //TinySerial.println(registerMap.i2cLock, HEX); //Debug message
    EEPROM.write(LOCATION_I2C_ADDRESS, registerMap.i2cAddress);
    startI2C(); //Determine the I2C address we should be using and begin listening on I2C bus
    //TinySerial.print("New Address: 0x"); //Debug message
    //TinySerial.println(registerMap.i2cAddress, HEX); //Debug message
  }
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void readSystemSettings(void)
{
  //Read what I2C address we should use
  registerMap.i2cAddress = EEPROM.read(LOCATION_I2C_ADDRESS);

  //Error check I2C address we read from EEPROM
  if (registerMap.i2cAddress < 0x08 || registerMap.i2cAddress > 0x77)
  {
    //User has set the address out of range
    //Go back to defaults
    registerMap.i2cAddress = I2C_ADDRESS_DEFAULT;
    EEPROM.write(LOCATION_I2C_ADDRESS, registerMap.i2cAddress);
  }

}

//Turn off anything we aren't going to use
void turnOffExtraBits()
{
//  //Disable Brown-Out Detect
//  MCUCR = bit (BODS) | bit (BODSE);
//  MCUCR = bit (BODS);

  //Power down various bits of hardware to lower power usage
  //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
}


//Begin listening on I2C bus as I2C slave using the global variable setting_i2c_address
void startI2C()
{
  Wire.end(); //Before we can change addresses we need to stop

  Wire.begin(registerMap.i2cAddress); //Start I2C and answer calls using address from EEPROM

  //The connections to the interrupts are severed when a Wire.begin occurs. So re-declare them.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}
