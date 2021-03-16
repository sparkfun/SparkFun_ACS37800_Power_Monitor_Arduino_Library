/*
  This is a library written for the I2C versions of the Allegro MicroSystems ACS37800 power monitor IC
  By Paul Clark @ SparkFun Electronics, January 21st, 2021


  https://github.com/sparkfun/SparkFun_ACS37800_Power_Monitor_Arduino_Library

  Development environment specifics:
  Arduino IDE 1.8.13

  SparkFun labored with love to create this code. Feel like supporting open
  source hardware? Buy a board from SparkFun!
  https://www.sparkfun.com/products/17873

*/

#include "SparkFun_ACS37800_Arduino_Library.h"

//Constructor
ACS37800::ACS37800()
{
}

//Start I2C communication using the specified port
//Returns true if successful or false if no sensor detected
bool ACS37800::begin(uint8_t address, TwoWire &wirePort, float currentSensingRange)
{
  _ACS37800Address = address; //Grab which i2c address the user wants us to use
  _i2cPort = &wirePort; //Grab which port the user wants us to use
  _currentSensingRange = currentSensingRange; // Define the current sensing range - usually 30.0 Amps but could be 90.0 Amps

  Wire.beginTransmission(address);
  if (Wire.endTransmission() != 0) // Did we detect something?
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("ACS37800::begin: failed to detect device with address 0x"));
      _debugPort->println(address, HEX);
    }
    return (false);
  }

  return (true);
}

//Calling this function with nothing sets the debug port to Serial
//You can also call it with other streams like Serial1, SerialUSB, etc.
void ACS37800::enableDebugging(Stream &debugPort)
{
	_debugPort = &debugPort;
	_printDebug = true;
}

//Read a register's contents. Contents are returned in data.
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
    return (ACS37800_ERR_I2C_ERROR); // Bail
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
  ACS37800ERR error = writeRegister(ACS37800_CUSTOMER_ACCESS_CODE, ACS37800_REGISTER_VOLATILE_2F); // Set the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: writeRegister (1) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: readRegister (1) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  store.data.bits.i2c_slv_addr = newAddress & 0x7F; //Update the address
  store.data.bits.i2c_dis_slv_addr = 1; //Disable setting the address via the DIO pins

  error = writeRegister(store.data.all, ACS37800_REGISTER_EEPROM_0F); // Write register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: writeRegister (2) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  error = writeRegister(0, ACS37800_REGISTER_VOLATILE_2F); // Clear the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: writeRegister (3) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  // We can't do this... Reading register 0F - after a successful write with a new address - returns zero...
  // error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F
  //
  // if (error != ACS37800_SUCCESS)
  // {
  //   if (_printDebug == true)
  //   {
  //     _debugPort->print(F("setI2Caddress: readRegister (2) returned: "));
  //     _debugPort->println(error);
  //   }
  //   return (error); // Bail
  // }
  //
  // if ((store.data.bits.i2c_slv_addr == newAddress) && (store.data.bits.ECC == ACS37800_EEPROM_ECC_NO_ERROR))
  // {
  //   return (ACS37800_SUCCESS);
  // }
  // else
  // {
  //   if (_printDebug == true)
  //   {
  //     _debugPort->print(F("setI2Caddress: i2c_slv_addr is 0x"));
  //     _debugPort->println(store.data.bits.i2c_slv_addr, HEX);
  //     _debugPort->print(F("setI2Caddress: ECC is "));
  //     _debugPort->println(store.data.bits.ECC);
  //   }
  //   return (ACS37800_ERR_REGISTER_READ_MODIFY_WRITE_FAILURE);
  // }

  return (error);
}

//Set the coarse gain adjustment for the current channel
//According to the datasheet, the bit fields for EEPROM (0B) and Shadow (1B) are different. Not sure why...?
ACS37800ERR ACS37800::setCurrentCoarseGain(ACS37800_CRS_SNS_e gain, boolean _eeprom)
{
  ACS37800_REGISTER_1B_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_SHADOW_1B); // Read register 1B

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setCurrentCoarseGain: readRegister (1B) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  store.data.bits.crs_sns = (uint32_t)gain; //Adjust the gain

  error = writeRegister(store.data.all, ACS37800_REGISTER_SHADOW_1B); // Write register 1B

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setCurrentCoarseGain: writeRegister (1B) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (!_eeprom) // Return now if _eeprom is false
    return (error);

  ACS37800_REGISTER_0B_t estore;
  error = readRegister(&estore.data.all, ACS37800_REGISTER_EEPROM_0B); // Read register 0B

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setCurrentCoarseGain: readRegister (0B) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  estore.data.bits.crs_sns = (uint32_t)gain; //Adjust the gain

  error = writeRegister(estore.data.all, ACS37800_REGISTER_EEPROM_0B); // Write register 0B

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setCurrentCoarseGain: writeRegister (0B) returned: "));
      _debugPort->println(error);
    }
  }
  return (error);
}

// Read volatile register 0x20. Return the vInst (Volts) and iInst (Amps).
// Note: there is much ambiguity in the datasheet:
//   vrms is in the range 0 to 1 and is unsigned. But the text mentions ΔVIN(MAX) = 0.84, and ΔVIN(min) = –0.84 ??
//   irms is in the range 0 to 1 but the text says it is a signed 16-bit fixed point number with 15 fractional bits, where IIP(MAX) = 0.84, and IIP(MIN)= -0.84 ??
ACS37800ERR ACS37800::readRMS(float *vRMS, float *iRMS)
{
  ACS37800_REGISTER_20_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_VOLATILE_20); // Read register 20

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readRMS: readRegister (20) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  //Extract vrms. Convert to voltage in Volts.
  float volts = (float)store.data.bits.vrms;
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: vrms: 0x"));
    _debugPort->println(store.data.bits.vrms, HEX);
    _debugPort->print(F("readRMS: volts (mV, before correction) is "));
    _debugPort->println(volts);
  }
  volts /= 65535.0; //Convert to 0 to 1
  volts /= 1.19; //Convert to mV
  volts /= 1000; //Convert to Volts
  //Correct for the voltage divider: (RISO1 + RISO2 +RSENSE) / RSENSE
  //Assumes RISO1/2 are both 1MOhm
  float resistorMultiplier = (2000000.0 + ((float)_senseResistance)) / ((float)_senseResistance);
  volts *= resistorMultiplier;
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: volts (V, after correction) is "));
    _debugPort->println(volts);
  }
  *vRMS = volts;

  //Extract the irms. Convert to current in Amps.
  //TO DO: figure out of we need to adjust for crs_sns coarse gain adjustment
  float amps = (float)(store.data.bits.irms & 0x7FFF); // Mask the MSbit ??
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: irms: 0x"));
    _debugPort->println(store.data.bits.irms, HEX);
    _debugPort->print(F("readRMS: amps (A, before correction) is "));
    _debugPort->println(amps);
  }
  amps /= 32768.0; //Convert to 0 to 1
  amps /= 1.19; //Convert to Amps
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: amps (A, after correction) is "));
    _debugPort->println(amps);
  }
  *iRMS = amps;

  return (error);
}

// Read volatile register 0x2A. Return the vInst (Volts) and iInst (Amps).
ACS37800ERR ACS37800::readInstantaneous(float *vInst, float *iInst)
{
  ACS37800_REGISTER_2A_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_VOLATILE_2A); // Read register 2A

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readInstantaneous: readRegister (2A) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  //Extract the vcodes. Convert to voltage in Volts.
  union
  {
    int16_t Signed;
    uint16_t unSigned;
  } signedUnsigned; // Avoid any ambiguity when casting to signed int

  //Extract vcodes as signed int
  //vcodes as actually int16_t but is stored in a uint32_t as a 16-bit bitfield
  signedUnsigned.unSigned = store.data.bits.vcodes;
  float volts = (float)signedUnsigned.Signed;
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: vcodes: 0x"));
    _debugPort->println(signedUnsigned.unSigned, HEX);
    _debugPort->print(F("readInstantaneous: volts (mV, before correction) is "));
    _debugPort->println(volts);
  }
  volts /= 27500.0; //Convert from codes to the fraction of full scale
  volts *= 250; //Convert to mV
  volts /= 1000; //Convert to Volts
  //Correct for the voltage divider: (RISO1 + RISO2 +RSENSE) / RSENSE
  //Assumes RISO1/2 are both 1MOhm
  float resistorMultiplier = (2000000.0 + ((float)_senseResistance)) / ((float)_senseResistance);
  volts *= resistorMultiplier;
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: volts (V, after correction) is "));
    _debugPort->println(volts);
  }
  *vInst = volts;

  //Extract the icodes. Convert to current in Amps.
  //TO DO: figure out of we need to adjust for crs_sns coarse gain adjustment
  signedUnsigned.unSigned = store.data.bits.icodes; //Extract icodes as signed int
  float amps = (float)signedUnsigned.Signed;
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: icodes: 0x"));
    _debugPort->println(signedUnsigned.unSigned, HEX);
    _debugPort->print(F("readInstantaneous: amps (A, before correction) is "));
    _debugPort->println(amps);
  }
  amps /= 27500.0; //Convert from codes to the fraction of full scale
  amps *= _currentSensingRange; //Convert to Amps
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: amps (A, after correction) is "));
    _debugPort->println(amps);
  }
  *iInst = amps;

  return (error);
}

//Read volatile register 0x2D. Return the error flags.
ACS37800ERR ACS37800::readErrorFlags(ACS37800_REGISTER_2D_t *errorFlags)
{
  ACS37800ERR error = readRegister(&errorFlags->data.all, ACS37800_REGISTER_VOLATILE_2D); // Read register 2D

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readErrorFlags: readRegister (2D) returned: "));
      _debugPort->println(error);
    }
  }

  return (error);
}

//Change the value of the sense resistor (Ohms)
void ACS37800::setSensRes(int newRes)
{
  _senseResistance = newRes;
}
