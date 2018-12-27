#include <TinyWire.h> //https://github.com/lucullusTheOnly/TinyWire
#include <avr/sleep.h>
#include <EEPROM.h>

#define REG_MAP_SIZE         7
#define MAX_SENT_BYTES       3

#define ADC1_ADDRESS_MSB         0x00
#define ADC2_ADDRESS_MSB         0x02
#define BTN1_ADDRESS             0x04

byte registerMap[REG_MAP_SIZE];
byte registerMapTemp[REG_MAP_SIZE - 1];
byte receivedCommands[MAX_SENT_BYTES];

//"factory default" slave address
byte slave_addr = 0x20;

int adc1 = 0;
int adc2 = 0;
byte btn = 0b00000000;

void setup()
{

  //Check EEPROM for a slave address
  //If no address is stored, set to "factory default"

  byte eepromVal;
  EEPROM.get(0, eepromVal);

  if (eepromVal == 0xFF) {
    EEPROM.put(0, 0x20);
  } else {
    //If there is an address in EEPROM, set it
    slave_addr = eepromVal;
  }

  //Setup TinyWire
  TinyWire.begin(slave_addr);
  TinyWire.onRequest(requestEvent);
  TinyWire.onReceive(receiveEvent);

  //Set pullup on pin 1
  pinMode(1, INPUT);
  digitalWrite(1, 1);

  //Shut off ADC
  ADCSRA &= ~(1 << ADEN);

}



void loop() {

  //Sleep until interrupt
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();

}

void requestEvent()
{

  //Bring up ADC
  ADCSRA |= (1 << ADEN);

  //Read all the things
  adc1 = analogRead(A2);
  adc2 = analogRead(A3);
  if (digitalRead(1) == 0) {
    btn = 0b00000000;
  } else {
    btn = 0b00000001;
  }

  //Shut off ADC
  ADCSRA &= ~(1 << ADEN);

  //Jam our stuff into an array
  byte * bytePointer;
  byte arrayIndex = 0;

  bytePointer = (byte*)&adc1;
  for (int i = 1; i > -1; i--)
  {
    registerMap[arrayIndex] = bytePointer[i];  //increment pointer to store each byte
    arrayIndex++;
  }

  bytePointer = (byte*)&adc2;
  for (int i = 1; i > -1; i--)
  {
    registerMap[arrayIndex] = bytePointer[i];  //increment pointer to store each byte
    arrayIndex++;
  }

  registerMap[4] = btn;

  switch (receivedCommands[0]) {

    case 0x00:

      TinyWire.send(registerMap[0]);
      TinyWire.send(registerMap[1]);
      break;

    case 0x02:

      TinyWire.send(registerMap[2]);
      TinyWire.send(registerMap[3]);
      break;

    case 0x04:

      TinyWire.send(registerMap[4]);
      break;

    case 0x06:

      byte eepromVal;
      EEPROM.get(0, eepromVal);
      TinyWire.send(eepromVal);
      break;

    default:
      break;

  }

}

void receiveEvent(int bytesReceived)
{

  for (int a = 0; a < bytesReceived; a++)
  {
    if ( a < MAX_SENT_BYTES)
    {
      receivedCommands[a] = TinyWire.read(); //grab stuff and jam it into the command buffer
    }
    else
    {
      TinyWire.read(); //if we receive more data than allowed, chuck it
    }
  }

  if (bytesReceived == 1 && (receivedCommands[0] < REG_MAP_SIZE)) //if we got a byte within the register map range, keep it
  {

    return;
  }

  if (bytesReceived == 1 && (receivedCommands[0] >= REG_MAP_SIZE)) //if we got a byte outside the register map range, chuck it
  {
    receivedCommands[0] = 0x00;
    return;
  }

  if (bytesReceived > 1 && (receivedCommands[0] == 0x05)) {

    if (receivedCommands[1] < 0x80 && receivedCommands[1] > 0x00) {
      EEPROM.put(0, receivedCommands[1]);
    }
    receivedCommands[0] = 0x00;
    receivedCommands[1] = 0x00;
    return;
  }

  return; // ignore anything else and return

}


