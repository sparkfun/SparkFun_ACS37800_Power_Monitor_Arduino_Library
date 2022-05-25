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
bool ACS37800::begin(uint8_t address, TwoWire &wirePort)
{
  _ACS37800Address = address; //Grab which i2c address the user wants us to use
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  // Wire.beginTransmission(address);
  // if (Wire.endTransmission() != 0) // Did we detect something?
  // {
  //   if (_printDebug == true)
  //   {
  //     _debugPort->print(F("ACS37800::begin: failed to detect device with address 0x"));
  //     _debugPort->println(address, HEX);
  //   }
  //   return (false);
  // }

  ACS37800ERR error = getCurrentCoarseGain(&_currentCoarseGain); // Get the current gain from shadow memory

  if (_printDebug == true)
  {
    if  (error != ACS37800_SUCCESS)
    {
      _debugPort->print(F("ACS37800::begin: failed! getCurrentCoarseGain returned: "));
      _debugPort->println(error);
    }
    else
    {
      _debugPort->print(F("ACS37800::begin: success! _currentCoarseGain is: "));
      _debugPort->println(_currentCoarseGain, 1);
    }
  }

  return (error == ACS37800_SUCCESS);
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
  // if (_printDebug == true)
  // {
  //   _debugPort->print(F("writeRegister: writing 0x"));
  //   _debugPort->print(data, HEX);
  //   _debugPort->print(F(" to address 0x"));
  //   _debugPort->println(address, HEX);
  // }

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
  ACS37800ERR error = writeRegister(ACS37800_CUSTOMER_ACCESS_CODE, ACS37800_REGISTER_VOLATILE_2F); // Set the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: writeRegister (2F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  ACS37800_REGISTER_0F_t store;
  error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: readRegister (0F) returned: "));
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
      _debugPort->print(F("setI2Caddress: writeRegister (0F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  error = writeRegister(0, ACS37800_REGISTER_VOLATILE_2F); // Clear the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: writeRegister (2F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  delay(100); // Allow time for the shadow/eeprom memory to be updated - otherwise the next readRegister will return zero...

  // Verify that the address was written correctly
  error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: readRegister (0F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if ((store.data.bits.i2c_slv_addr == newAddress) && (store.data.bits.ECC == ACS37800_EEPROM_ECC_NO_ERROR))
  {
    return (ACS37800_SUCCESS);
  }
  else
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setI2Caddress: i2c_slv_addr is 0x"));
      _debugPort->println(store.data.bits.i2c_slv_addr, HEX);
      _debugPort->print(F("setI2Caddress: ECC is "));
      _debugPort->println(store.data.bits.ECC);
    }
    return (ACS37800_ERR_REGISTER_READ_MODIFY_WRITE_FAILURE);
  }

  return (error);
}

//Set the number of samples for RMS calculations. Bypass_N_Enable must be set/true for this to have effect.
ACS37800ERR ACS37800::setNumberOfSamples(uint32_t numberOfSamples, bool _eeprom)
{
  ACS37800ERR error = writeRegister(ACS37800_CUSTOMER_ACCESS_CODE, ACS37800_REGISTER_VOLATILE_2F); // Set the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setNumberOfSamples: writeRegister (2F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  ACS37800_REGISTER_0F_t store;
  error = readRegister(&store.data.all, ACS37800_REGISTER_SHADOW_1F); // Read register 1F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setNumberOfSamples: readRegister (1F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (_printDebug == true)
  {
    _debugPort->print(F("setNumberOfSamples: number of samples is currently: "));
    _debugPort->println(store.data.bits.n);
  }

  store.data.bits.n = numberOfSamples & 0x3FF; //Adjust the number of samples (limit to 10 bits)

  error = writeRegister(store.data.all, ACS37800_REGISTER_SHADOW_1F); // Write register 1F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setNumberOfSamples: writeRegister (1F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (_eeprom) // Check if user wants to set eeprom too
  {
    error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

    if (error != ACS37800_SUCCESS)
    {
      if (_printDebug == true)
      {
        _debugPort->print(F("setNumberOfSamples: readRegister (0F) returned: "));
        _debugPort->println(error);
      }
      return (error); // Bail
    }

    if (_printDebug == true)
    {
      _debugPort->print(F("setNumberOfSamples: eeprom number of samples is currently: "));
      _debugPort->println(store.data.bits.n);
    }

    store.data.bits.n = numberOfSamples & 0x3FF; //Adjust the number of samples (limit to 10 bits)

    error = writeRegister(store.data.all, ACS37800_REGISTER_EEPROM_0F); // Write register 0F

    if (error != ACS37800_SUCCESS)
    {
      if (_printDebug == true)
      {
        _debugPort->print(F("setNumberOfSamples: writeRegister (0F) returned: "));
        _debugPort->println(error);
      }
    }
  }

  error = writeRegister(0, ACS37800_REGISTER_VOLATILE_2F); // Clear the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setNumberOfSamples: writeRegister (2F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  delay(100); // Allow time for the shadow/eeprom memory to be updated - otherwise the next readRegister will return zero...

  return (error);
}

//Read and return the number of samples from shadow memory
ACS37800ERR ACS37800::getNumberOfSamples(uint32_t *numberOfSamples)
{
  ACS37800_REGISTER_0F_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_SHADOW_1F); // Read register 1F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("getNumberOfSamples: readRegister (1F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (_printDebug == true)
  {
    _debugPort->print(F("getNumberOfSamples: number of samples is currently: "));
    _debugPort->println(store.data.bits.n);
  }

  *numberOfSamples = store.data.bits.n; //Return the number of samples

  return (error);
}

//Set/Clear the Bypass_N_Enable flag
ACS37800ERR ACS37800::setBypassNenable(bool bypass, bool _eeprom)
{
  ACS37800ERR error = writeRegister(ACS37800_CUSTOMER_ACCESS_CODE, ACS37800_REGISTER_VOLATILE_2F); // Set the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setBypassNenable: writeRegister (2F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  ACS37800_REGISTER_0F_t store;
  error = readRegister(&store.data.all, ACS37800_REGISTER_SHADOW_1F); // Read register 1F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setBypassNenable: readRegister (1F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (_printDebug == true)
  {
    _debugPort->print(F("setBypassNenable: bypass_n_en is currently: "));
    _debugPort->println(store.data.bits.bypass_n_en);
  }

  if (bypass) //Adjust bypass_n_en
  {
    store.data.bits.bypass_n_en = 1;
  }
  else
  {
    store.data.bits.bypass_n_en = 0;
  }

  error = writeRegister(store.data.all, ACS37800_REGISTER_SHADOW_1F); // Write register 1F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setBypassNenable: writeRegister (1F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (_eeprom) // Check if user wants to set eeprom too
  {
    error = readRegister(&store.data.all, ACS37800_REGISTER_EEPROM_0F); // Read register 0F

    if (error != ACS37800_SUCCESS)
    {
      if (_printDebug == true)
      {
        _debugPort->print(F("setBypassNenable: readRegister (0F) returned: "));
        _debugPort->println(error);
      }
      return (error); // Bail
    }

    if (_printDebug == true)
    {
      _debugPort->print(F("setBypassNenable: eeprom bypass_n_en is currently: "));
      _debugPort->println(store.data.bits.bypass_n_en);
    }

    if (bypass) //Adjust bypass_n_en
    {
      store.data.bits.bypass_n_en = 1;
    }
    else
    {
      store.data.bits.bypass_n_en = 0;
    }

    error = writeRegister(store.data.all, ACS37800_REGISTER_EEPROM_0F); // Write register 0F

    if (error != ACS37800_SUCCESS)
    {
      if (_printDebug == true)
      {
        _debugPort->print(F("setBypassNenable: writeRegister (0F) returned: "));
        _debugPort->println(error);
      }
    }
  }

  error = writeRegister(0, ACS37800_REGISTER_VOLATILE_2F); // Clear the customer access code

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("setBypassNenable: writeRegister (2F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  delay(100); // Allow time for the shadow/eeprom memory to be updated - otherwise the next readRegister will return zero...

  return (error);
}

//// Read and return the bypass_n_en flag from shadow memory
ACS37800ERR ACS37800::getBypassNenable(bool *bypass)
{
  ACS37800_REGISTER_0F_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_SHADOW_1F); // Read register 1F

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("getBypassNenable: readRegister (1F) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  if (_printDebug == true)
  {
    _debugPort->print(F("getBypassNenable: bypass_n_en is currently: "));
    _debugPort->println(store.data.bits.bypass_n_en);
  }

  *bypass = (bool)store.data.bits.bypass_n_en; //Return bypass_n_en

  return (error);
}

//Get the coarse current gain from shadow memory
ACS37800ERR ACS37800::getCurrentCoarseGain(float *currentCoarseGain)
{
  ACS37800_REGISTER_0B_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_SHADOW_1B); // Read register 1B

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("getCurrentCoarseGain: readRegister (1B) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  float gain = ACS37800_CRS_SNS_GAINS[store.data.bits.crs_sns];

  if (_printDebug == true)
  {
    _debugPort->print(F("getCurrentCoarseGain: shadow gain setting is currently: "));
    _debugPort->println(gain, 1);
  }

  *currentCoarseGain = gain; //Return the gain

  return (error);
}

// Read volatile register 0x20. Return the vInst (Volts) and iInst (Amps).
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
  // Note: datasheet says "RMS voltage output. This field is an unsigned 16-bit fixed point number with 16 fractional bits"
  // Datasheet also says "Voltage Channel ADC Sensitivity: 110 LSB/mV"
  float volts = (float)store.data.bits.vrms;
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: vrms: 0x"));
    _debugPort->println(store.data.bits.vrms, HEX);
    _debugPort->print(F("readRMS: volts (LSB, before correction) is "));
    _debugPort->println(volts);
  }
  volts /= 55000.0; //Convert from codes to the fraction of ADC Full Scale (16-bit)
  volts *= 250; //Convert to mV (Differential Input Range is +/- 250mV)
  volts /= 1000; //Convert to Volts
  //Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
  //Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
  float resistorMultiplier = (_dividerResistance + _senseResistance) / _senseResistance;
  volts *= resistorMultiplier;
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: volts (V, after correction) is "));
    _debugPort->println(volts);
  }
  *vRMS = volts;

  //Extract the irms. Convert to current in Amps.
  //Datasheet says: "RMS current output. This field is a signed 16-bit fixed point number with 15 fractional bits"
  union
  {
    int16_t Signed;
    uint16_t unSigned;
  } signedUnsigned; // Avoid any ambiguity when casting to signed int

  signedUnsigned.unSigned = store.data.bits.irms; //Extract irms as signed int
  float amps = (float)signedUnsigned.Signed;
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: irms: 0x"));
    _debugPort->println(store.data.bits.irms, HEX);
    _debugPort->print(F("readRMS: amps (LSB, before correction) is "));
    _debugPort->println(amps);
  }
  amps /= 55000.0; //Convert from codes to the fraction of ADC Full Scale (16-bit)
  amps *= _currentSensingRange; //Convert to Amps
  if (_printDebug == true)
  {
    _debugPort->print(F("readRMS: amps (A, after correction) is "));
    _debugPort->println(amps);
  }
  *iRMS = amps;

  return (error);
}

// Read volatile register 0x21. Return the pactive and pimag.
ACS37800ERR ACS37800::readPowerActiveReactive(float *pActive, float *pReactive)
{
  ACS37800_REGISTER_21_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_VOLATILE_21); // Read register 21

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readPowerActiveReactive: readRegister (21) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  // Extract pactive. Convert to Watts
  // Note: datasheet says:
  // "Active power output. This field is a signed 16-bit fixed point
  //  number with 15 fractional bits, where positive MaxPow = 0.704,
  //  and negative MaxPow = –0.704. To convert the value (input
  //  power) to line power, divide the input power by the RSENSE and
  //  RISO voltage divider ratio using actual resistor values."
  // Datasheet also says:
  //  "3.08 LSB/mW for the 30A version and 1.03 LSB/mW for the 90A version"

  union
  {
    int16_t Signed;
    uint16_t unSigned;
  } signedUnsigned; // Avoid any ambiguity when casting to signed int

  signedUnsigned.unSigned = store.data.bits.pactive;

  float power = (float)signedUnsigned.Signed;
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerActiveReactive: pactive: 0x"));
    _debugPort->println(signedUnsigned.unSigned, HEX);
    _debugPort->print(F("readPowerActiveReactive: pactive (LSB, before correction) is "));
    _debugPort->println(power);
  }
  float LSBpermW = 3.08; // LSB per mW
  LSBpermW *= 30.0 / _currentSensingRange; // Correct for sensor version
  power /= LSBpermW; //Convert from codes to mW
  //Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
  //Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
  float resistorMultiplier = (_dividerResistance + _senseResistance) / _senseResistance;
  power *= resistorMultiplier;
  power /= 1000; // Convert from mW to W
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerActiveReactive: pactive (W, after correction) is "));
    _debugPort->println(power);
  }
  *pActive = power;

  // Extract pimag. Convert to VAR
  // Note: datasheet says:
  // "Reactive power output. This field is an unsigned 16-bit fixed
  //  point number with 16 fractional bits, where MaxPow = 0.704. To
  //  convert the value (input power) to line power, divide the input
  //  power by the RSENSE and RISO voltage divider ratio using actual
  //  resistor values."
  // Datasheet also says:
  //  "6.15 LSB/mVAR for the 30A version and 2.05 LSB/mVAR for the 90A version"

  power = (float)store.data.bits.pimag;
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerActiveReactive: pimag: 0x"));
    _debugPort->println(store.data.bits.pimag, HEX);
    _debugPort->print(F("readPowerActiveReactive: pimag (LSB, before correction) is "));
    _debugPort->println(power);
  }
  float LSBpermVAR = 6.15; // LSB per mVAR
  LSBpermVAR *= 30.0 / _currentSensingRange; // Correct for sensor version
  power /= LSBpermVAR; //Convert from codes to mVAR
  //Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
  //Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
  power *= resistorMultiplier;
  power /= 1000; // Convert from mVAR to VAR
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerActiveReactive: pimag (VAR, after correction) is "));
    _debugPort->println(power);
  }
  *pReactive = power;

  return (error);
}

// Read volatile register 0x22. Return the apparent power, power factor, leading / lagging, generated / consumed
ACS37800ERR ACS37800::readPowerFactor(float *pApparent, float *pFactor, bool *posangle, bool *pospf)
{
  ACS37800_REGISTER_22_t store;
  ACS37800ERR error = readRegister(&store.data.all, ACS37800_REGISTER_VOLATILE_22); // Read register 22

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readPowerFactor: readRegister (22) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  // Extract papparent. Convert to VA
  // Note: datasheet says:
  // "Apparent power output magnitude. This field is an unsigned
  //  16-bit fixed point number with 16 fractional bits, where MaxPow
  //  = 0.704. To convert the value (input power) to line power, divide
  //  the input power by the RSENSE and RISO voltage divider ratio
  //  using actual resistor values."
  // Datasheet also says:
  //  "6.15 LSB/mVA for the 30A version and 2.05 LSB/mVA for the 90A version"

  float power = (float)store.data.bits.papparent;
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerFactor: papparent: 0x"));
    _debugPort->println(store.data.bits.papparent, HEX);
    _debugPort->print(F("readPowerFactor: papparent (LSB, before correction) is "));
    _debugPort->println(power);
  }
  float LSBpermVA = 6.15; // LSB per mVA
  LSBpermVA *= 30.0 / _currentSensingRange; // Correct for sensor version
  power /= LSBpermVA; //Convert from codes to mVA
  //Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
  //Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
  float resistorMultiplier = (_dividerResistance + _senseResistance) / _senseResistance;
  power *= resistorMultiplier;
  power /= 1000; // Convert from mVAR to VAR
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerFactor: papparent (VA, after correction) is "));
    _debugPort->println(power);
  }
  *pApparent = power;

  // Extract power factor
  // Datasheet says:
  // "Power factor output. This field is a signed 11-bit fixed point number
  //  with 10 fractional bits. It ranges from –1 to ~1 with a step
  //  size of 2^-10."

  union
  {
    int16_t Signed;
    uint16_t unSigned;
  } signedUnsigned; // Avoid any ambiguity when casting to signed int

  signedUnsigned.unSigned = store.data.bits.pfactor << 5; // Move 11-bit number into 16-bits (signed)

  float pfactor = (float)signedUnsigned.Signed / 32768.0; // Convert to +/- 1
  if (_printDebug == true)
  {
    _debugPort->print(F("readPowerFactor: pfactor: 0x"));
    _debugPort->println(store.data.bits.pfactor, HEX);
    _debugPort->print(F("readPowerFactor: pfactor is "));
    _debugPort->println(pfactor);
  }
  *pFactor = pfactor;

  // Extract posangle and pospf
  *posangle = store.data.bits.posangle & 0x1;
  *pospf = store.data.bits.pospf & 0x1;

  return (error);
}

// Read volatile registers 0x2A and 0x2C. Return the vInst (Volts), iInst (Amps) and pInst (VAR).
ACS37800ERR ACS37800::readInstantaneous(float *vInst, float *iInst, float *pInst)
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
    _debugPort->print(F("readInstantaneous: volts (LSB, before correction) is "));
    _debugPort->println(volts);
  }
  // Datasheet says "Voltage Channel ADC Sensitivity: 110 LSB/mV"
  volts /= 27500.0; //Convert from codes to the fraction of ADC Full Scale
  volts *= 250; //Convert to mV (Differential Input Range is +/- 250mV)
  volts /= 1000; //Convert to Volts
  //Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
  //Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
  float resistorMultiplier = (_dividerResistance + _senseResistance) / _senseResistance;
  volts *= resistorMultiplier;
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: volts (V, after correction) is "));
    _debugPort->println(volts);
  }
  *vInst = volts;

  //Extract the icodes. Convert to current in Amps.
  signedUnsigned.unSigned = store.data.bits.icodes; //Extract icodes as signed int
  float amps = (float)signedUnsigned.Signed;
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: icodes: 0x"));
    _debugPort->println(signedUnsigned.unSigned, HEX);
    _debugPort->print(F("readInstantaneous: amps (LSB, before correction) is "));
    _debugPort->println(amps);
  }
  amps /= 27500.0; //Convert from codes to the fraction of ADC Full Scale
  amps *= _currentSensingRange; //Convert to Amps
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: amps (A, after correction) is "));
    _debugPort->println(amps);
  }
  *iInst = amps;

  ACS37800_REGISTER_2C_t pstore;
  error = readRegister(&pstore.data.all, ACS37800_REGISTER_VOLATILE_2C); // Read register 2C

  if (error != ACS37800_SUCCESS)
  {
    if (_printDebug == true)
    {
      _debugPort->print(F("readInstantaneous: readRegister (2C) returned: "));
      _debugPort->println(error);
    }
    return (error); // Bail
  }

  //Extract pinstant as signed int. Convert to W
  //pinstant as actually int16_t but is stored in a uint32_t as a 16-bit bitfield
  signedUnsigned.unSigned = pstore.data.bits.pinstant;
  float power = (float)signedUnsigned.Signed;
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: pinstant: 0x"));
    _debugPort->println(signedUnsigned.unSigned, HEX);
    _debugPort->print(F("readInstantaneous: power (LSB, before correction) is "));
    _debugPort->println(power);
  }
  //Datasheet says: 3.08 LSB/mW for the 30A version and 1.03 LSB/mW for the 90A version
  float LSBpermW = 3.08; // LSB per mW
  LSBpermW *= 30.0 / _currentSensingRange; // Correct for sensor version
  power /= LSBpermW; //Convert from codes to mW
  //Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
  //Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
  power *= resistorMultiplier;
  power /= 1000; // Convert from mW to W
  if (_printDebug == true)
  {
    _debugPort->print(F("readInstantaneous: power (W, after correction) is "));
    _debugPort->println(power);
  }
  *pInst = power;

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
void ACS37800::setSenseRes(float newRes)
{
  _senseResistance = newRes;
}

//Change the value of the voltage divider resistance (Ohms)
void ACS37800::setDividerRes(float newRes)
{
  _dividerResistance = newRes;
}

//Change the current-sensing range (Amps)
//ACS37800KMACTR-030B3-I2C is a 30.0 Amp part - as used on the SparkFun Qwiic Power Meter
//ACS37800KMACTR-090B3-I2C is a 90.0 Amp part
void ACS37800::setCurrentRange(float newCurrent)
{
  _currentSensingRange = newCurrent;
}
