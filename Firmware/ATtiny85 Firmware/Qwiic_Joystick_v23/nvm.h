//Location in EEPROM for each thing we want to store between power cycles
enum eepromLocations {
  LOCATION_I2C_ADDRESS = 0, //Device's address
  LOCATION_FIRMWARE_MAJOR,
  LOCATION_FIRMWARE_MINOR,
  //LOCATION_X_POS,
  //LOCATION_Y_POS,
};

//Defaults for the I2C address
const byte I2C_ADDRESS_DEFAULT = 0x20;
