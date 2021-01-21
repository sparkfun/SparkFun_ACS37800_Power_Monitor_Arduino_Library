/*
  This is a library written for the I2C versions of the Allegro MicroSystems ACS37800 power monitor IC
  By Paul Clark @ SparkFun Electronics, January 21st, 2021


  https://github.com/sparkfun/SparkFun_ACS37800_Power_Monitor_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.13

  SparkFun labored with love to create this code. Feel like supporting open
  source hardware? Buy a board from SparkFun!
  https://www.sparkfun.com/products/nnnnn

*/

#include "SparkFun_ACS37800_Arduino_Library.h"

//Constructor
ACS37800::ACS37800()
{
}

//Start I2C communication using specified port
//Returns true if successful or false if no sensor detected
bool ACS37800::begin(uint8_t address, TwoWire &wirePort)
{
  _ACS37800Address = address; //Grab which i2c address the user wants us to use
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  uint32_t accessCode; //Read and validate the access code
  ACS37800ERR error = readRegister(&accessCode, ACS37800_REGISTER_VOLATILE_2F);

  if (_printDebug == true)
  {
    _debugPort->print(F("begin: readRegister error: "));
    _debugPort->println(error);
    _debugPort->print(F("begin: accessCode: 0x"));
    _debugPort->println(accessCode, HEX);
  }

  return ((error == ACS37800_SUCCESS) && (accessCode == ACS37800_CUSTOMER_ACCESS_CODE));
}

//Calling this function with nothing sets the debug port to Serial
//You can also call it with other streams like Serial1, SerialUSB, etc.
void ACS37800::enableDebugging(Stream &debugPort)
{
	_debugPort = &debugPort;
	_printDebug = true;
}

//Read a register's contents. Data is returned in data.
ACS37800ERR ACS37800::readRegister(uint32_t *data, uint8_t address)
{
  _i2cPort->beginTransmission(_ACS37800Address);
  _i2cPort->write(address); //Write the register address
  uint8_t i2cResult = _i2cPort->endTransmission(false); //Send restart. Don't release the bus.

  if (i2cResult != 0)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readRegister: endTransmission returned: "));
      _debugPort->println(i2cResult);
    }
    return (ACS37800_ERR_I2C_ERROR); // Bail
  }

  //Read 4 bytes (32 bits)
  uint8_t toRead = _i2cPort->requestFrom(_ACS37800Address, (uint8_t)4);
  if (toRead != 4)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readRegister: requestFrom returned: "));
      _debugPort->println(toRead);
    }
    return (ACS37800_ERR_I2C_ERROR); //Error out
  }

  //Data is returned LSB first (little endian)
  uint32_t readData = _i2cPort->read(); //store LSB
  readData |= ((uint32_t)_i2cPort->read()) << 8;
  readData |= ((uint32_t)_i2cPort->read()) << 16;
  readData |= ((uint32_t)_i2cPort->read()) << 24; //store MSB

  *data = readData; //Return the data
  return (ACS37800_SUCCESS);
}

//Write data to the selected register
ACS37800ERR ACS37800::writeRegister(uint32_t data, uint8_t address)
{
  _i2cPort->beginTransmission(_ACS37800Address);
  _i2cPort->write(address); //Write the register address
  _i2cPort->write(data & 0xFF); //Write the data LSB first (little endian)
  _i2cPort->write((data >> 8) & 0xFF);
  _i2cPort->write((data >> 16) & 0xFF);
  _i2cPort->write((data >> 24) & 0xFF);
  uint8_t i2cResult = _i2cPort->endTransmission(); //Release the bus.

  if (i2cResult != 0)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("writeRegister: endTransmission returned: "));
      _debugPort->println(i2cResult);
    }
    return (ACS37800_ERR_I2C_ERROR); // Bail
  }

  return (ACS37800_SUCCESS);
}

//Change the I2C address
ACS37800ERR ACS37800::setI2Caddress(uint8_t newAddress)
{
  ACS37800_REGISTER_0F_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: readRegister (1) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  store.data.bits.i2c_slv_addr = newAddress; //Update the address
  store.data.bits.i2c_dis_slv_addr = 1; //Disable setting the address via the DIO pins

  error = writeRegister(store.data.all, ACS37800_REGISTER_EEPROM_0F); // Write register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: writeRegister returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: readRegister (2) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (store.data.bits.i2c_slv_addr == newAddress) //Check the address was updated correctly
  {
    return (ACS37800_SUCCESS);
  }
  else
  {
    return (ACS37800_ERR_REGISTER_READ_MODIFY_WRITE_FAILURE);
  }
}
