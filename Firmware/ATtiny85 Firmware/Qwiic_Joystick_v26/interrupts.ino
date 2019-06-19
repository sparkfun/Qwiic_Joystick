//Turn on interrupt for the button
void setupInterrupts()
{
  //Attach interrupt to button
  #if defined(__AVR_ATmega328P__)
  attachInterrupt(digitalPinToInterrupt(Button_Pin), buttonInterrupt, CHANGE);
  
  #elif defined(__AVR_ATtiny85__)
  attachPCINT(digitalPinToPCINT(Button_Pin), buttonInterrupt, CHANGE);
  #endif
}


//Called any time the pin changes state
void buttonInterrupt()
{
  if (digitalRead(Button_Pin) == LOW) //User has released the button, we have completed a click cycle
  {
    registerMap.Button_Status = 1; //Set the clicked bit
  }
}


//Called every time the joystick is moved
void updateJoystick() {
  //Bring up ADC
  ADCSRA |= (1 << ADEN);

  int X_Pot = analogRead(Horizontal_Pin);
  int Y_Pot = analogRead(Vertical_Pin);

  //Convert int value to a 16-bit integer by bitshitting it left by 6.
  //Example-
  //DEC:           630
  //BIN:           1001110110
  //Bitshift by 6: 1001110110000000
  uint16_t X_unit16 = X_Pot<<6;
  uint16_t Y_unit16 = Y_Pot<<6;
  
  //Split the bitshifted word into the MSB and LSB parts
  //Continuing from the example above...
  //DEC:            630
  //BIN:            10011101   10
  //Bitshift by 6:  10011101   10000000
  //Split:          10011101 | 10000000

  byte * bytePointer;

  bytePointer = (byte*)&X_unit16;
  registerMap.X_MSB = bytePointer[1];
  registerMap.X_LSB = bytePointer[0];
  
  bytePointer = (byte*)&Y_unit16;
  registerMap.Y_MSB = bytePointer[1];
  registerMap.Y_LSB = bytePointer[0];

  //Shut off ADC
  ADCSRA &= ~(1 << ADEN);
}


//When Qwiic Joystick receives data bytes from Master, this function is called as an interrupt
//(Serves rewritable I2C address)
void receiveEvent(int numberOfBytesReceived)
{
  //TinySerial.println("Start RX Event"); //Debug message
  registerNumber = Wire.read(); //Get the memory map offset from the user
  //TinySerial.println("  Read 4 Register"); //Debug message

  

  //Begin recording the following incoming bytes to the temp memory map
  //starting at the registerNumber (the first byte received)
  for (byte x = 0 ; x < numberOfBytesReceived - 1 ; x++)
  {
    byte temp = Wire.read(); //We might record it, we might throw it away

    if ( (x + registerNumber) < sizeof(memoryMap))
    {
      //Clense the incoming byte against the read only protected bits
      //Store the result into the register map
      *(registerPointer + registerNumber + x) &= ~*(protectionPointer + registerNumber + x); //Clear this register if needed
      *(registerPointer + registerNumber + x) |= temp & *(protectionPointer + registerNumber + x); //Or in the user's request (clensed against protection bits)
    }
  }
  //TinySerial.println("  Read 4 Data"); //Debug message
  recordSystemSettings();
  //TinySerial.println("  Data Saved"); //Debug message
}


//Respond to GET commands
//When Qwiic Joystick gets a request for data from the user, this function is called as an interrupt
//The interrupt will respond with bytes starting from the last byte the user sent to us
//While we are sending bytes we may have to do some calculations
void requestEvent()
{
  updateJoystick();
  registerMap.Button_State = digitalRead(Button_Pin);

  //This will write the entire contents of the register map struct starting from
  //the register the user requested, and when it reaches the end the master
  //will read 0xFFs.
  Wire.write((registerPointer + registerNumber), sizeof(memoryMap) - registerNumber);
}
