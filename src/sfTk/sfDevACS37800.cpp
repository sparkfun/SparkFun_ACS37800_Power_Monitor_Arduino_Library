/*!
 * @file sfDevACS37800.cpp
 *
 * SparkFun ACS37800 Power Monitor Library - platform independent device driver
 *
 * This library facilitates communication with the I2C versions of the Allegro MicroSystems ACS37800
 * power monitor IC.
 *
 * Want to support open source hardware? Buy a board from SparkFun!
 * https://www.sparkfun.com/products/17873
 *
 * This library was originally written by:
 * Paul Clark
 * SparkFun Electronics
 * January 21st, 2021
 *
 * @author SparkFun Electronics
 * @date 2021-2025
 * @copyright Copyright (c) 2021-2025, SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 */

#include "sfDevACS37800.h"

// Time to allow the shadow/eeprom memory to be updated - otherwise the next readRegister will return zero...
const uint32_t kACS37800MemoryUpdateDelay = 100;

//--------------------------------------------------------------------------------------------------
// Convert a 16-bit field to signed. Avoid any ambiguity when casting to signed int
//
static int16_t toSigned16(uint16_t value)
{
    union {
        int16_t Signed;
        uint16_t unSigned;
    } signedUnsigned;

    signedUnsigned.unSigned = value;
    return signedUnsigned.Signed;
}

//--------------------------------------------------------------------------------------------------
// Begin the ACS37800 device. Requires a bus object to communicate with the device.
//
sfTkError_t sfDevACS37800::begin(sfTkIBus *theBus)
{
    if (theBus == nullptr)
        return ksfTkErrBusNotInit;

    _theBus = theBus;

    // Data is transferred LSB first (little endian)
    _theBus->setByteOrder(sfTkByteOrder::LittleEndian);

    // Get the current gain from shadow memory
    return getCurrentCoarseGain(&_currentCoarseGain);
}

//--------------------------------------------------------------------------------------------------
// Check if the ACS37800 is connected by reading a shadow register
//
bool sfDevACS37800::isConnected(void)
{
    uint32_t data;
    return _theBus != nullptr && _theBus->readRegister(ACS37800_REGISTER_SHADOW_1B, data) == ksfTkErrOk;
}

//--------------------------------------------------------------------------------------------------
// Update bits in an EEPROM register and/or its shadow register, using the customer access code
//
sfTkError_t sfDevACS37800::updateConfigBits(uint8_t eepromReg, uint32_t mask, uint32_t value, bool shadow,
                                            bool eeprom)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    // Set the customer access code
    sfTkError_t rc = _theBus->writeRegister(ACS37800_REGISTER_VOLATILE_2F, ACS37800_CUSTOMER_ACCESS_CODE);
    if (rc != ksfTkErrOk)
        return rc;

    // The shadow register is first, then the EEPROM register
    uint8_t registers[] = {(uint8_t)(eepromReg + 0x10), eepromReg};
    bool enabled[] = {shadow, eeprom};

    for (int i = 0; i < 2 && rc == ksfTkErrOk; i++)
    {
        if (!enabled[i])
            continue;

        uint32_t data;
        rc = _theBus->readRegister(registers[i], data);
        if (rc == ksfTkErrOk)
            rc = _theBus->writeRegister(registers[i], (data & ~mask) | (value & mask));
    }

    // Always clear the customer access code
    sfTkError_t rcClear = _theBus->writeRegister(ACS37800_REGISTER_VOLATILE_2F, (uint32_t)0);

    sftk_delay_ms(kACS37800MemoryUpdateDelay);

    return rc != ksfTkErrOk ? rc : rcClear;
}

//--------------------------------------------------------------------------------------------------
// Is the register a shadow register?
//
static bool isShadowRegister(ACS37800_shadow_register_t reg)
{
    return reg >= ACS37800_SHADOW_REGISTER_1B && reg <= ACS37800_SHADOW_REGISTER_1F;
}

//--------------------------------------------------------------------------------------------------
// Read the contents of a shadow register
//
sfTkError_t sfDevACS37800::readShadowRegister(ACS37800_shadow_register_t reg, uint32_t *value)
{
    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    if (!isShadowRegister(reg) || value == nullptr)
        return ksfTkErrInvalidParam;

    return _theBus->readRegister((uint8_t)reg, *value);
}

//--------------------------------------------------------------------------------------------------
// Write the contents of a shadow register - and optionally the matching EEPROM register
//
sfTkError_t sfDevACS37800::writeShadowRegister(ACS37800_shadow_register_t reg, uint32_t value, bool _eeprom)
{
    if (!isShadowRegister(reg))
        return ksfTkErrInvalidParam;

    // The EEPROM registers are 0x10 below the shadow registers
    return updateConfigBits((uint8_t)(reg - 0x10), 0xFFFFFFFF, value, true, _eeprom);
}

//--------------------------------------------------------------------------------------------------
// Change the I2C address in EEPROM
//
sfTkError_t sfDevACS37800::setI2Caddress(uint8_t newAddress)
{
    if (newAddress > 0x7F)
        return ksfTkErrInvalidParam;

    ACS37800_REGISTER_0F_t mask;
    mask.data.all = 0;
    mask.data.bits.i2c_slv_addr = 0x7F;
    mask.data.bits.i2c_dis_slv_addr = 1;

    ACS37800_REGISTER_0F_t store;
    store.data.all = 0;
    store.data.bits.i2c_slv_addr = newAddress; // Update the address
    store.data.bits.i2c_dis_slv_addr = 1;      // Disable setting the address via the DIO pins

    sfTkError_t rc = updateConfigBits(ACS37800_REGISTER_EEPROM_0F, mask.data.all, store.data.all, false, true);
    if (rc != ksfTkErrOk)
        return rc;

    // Verify that the address was written correctly
    rc = _theBus->readRegister(ACS37800_REGISTER_EEPROM_0F, store.data.all);
    if (rc != ksfTkErrOk)
        return rc;

    if ((store.data.bits.i2c_slv_addr == newAddress) && (store.data.bits.ECC == ACS37800_EEPROM_ECC_NO_ERROR))
        return ksfTkErrOk;

    return ksfTkErrACS37800ReadModifyWriteFailure;
}

//--------------------------------------------------------------------------------------------------
// Set the number of samples for RMS calculations. Bypass_N_Enable must be set/true for this to have effect.
//
sfTkError_t sfDevACS37800::setNumberOfSamples(uint32_t numberOfSamples, bool _eeprom)
{
    ACS37800_REGISTER_0F_t mask;
    mask.data.all = 0;
    mask.data.bits.n = 0x3FF;

    ACS37800_REGISTER_0F_t store;
    store.data.all = 0;
    store.data.bits.n = numberOfSamples & 0x3FF; // Adjust the number of samples (limit to 10 bits)

    return updateConfigBits(ACS37800_REGISTER_EEPROM_0F, mask.data.all, store.data.all, true, _eeprom);
}

//--------------------------------------------------------------------------------------------------
// Read and return the number of samples from shadow memory
//
sfTkError_t sfDevACS37800::getNumberOfSamples(uint32_t *numberOfSamples)
{
    if (numberOfSamples == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_0F_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_SHADOW_1F, store.data.all);

    if (rc == ksfTkErrOk)
        *numberOfSamples = store.data.bits.n;

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Set/Clear the Bypass_N_Enable flag
//
sfTkError_t sfDevACS37800::setBypassNenable(bool bypass, bool _eeprom)
{
    ACS37800_REGISTER_0F_t mask;
    mask.data.all = 0;
    mask.data.bits.bypass_n_en = 1;

    ACS37800_REGISTER_0F_t store;
    store.data.all = 0;
    store.data.bits.bypass_n_en = bypass ? 1 : 0;

    return updateConfigBits(ACS37800_REGISTER_EEPROM_0F, mask.data.all, store.data.all, true, _eeprom);
}

//--------------------------------------------------------------------------------------------------
// Read and return the bypass_n_en flag from shadow memory
//
sfTkError_t sfDevACS37800::getBypassNenable(bool *bypass)
{
    if (bypass == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_0F_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_SHADOW_1F, store.data.all);

    if (rc == ksfTkErrOk)
        *bypass = (bool)store.data.bits.bypass_n_en;

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Get the coarse current gain from shadow memory
//
sfTkError_t sfDevACS37800::getCurrentCoarseGain(float *currentCoarseGain)
{
    if (currentCoarseGain == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_0B_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_SHADOW_1B, store.data.all);

    if (rc == ksfTkErrOk)
        *currentCoarseGain = ACS37800_CRS_SNS_GAINS[store.data.bits.crs_sns];

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Read volatile register 0x20. Return the vRMS (Volts) and iRMS (Amps).
//
sfTkError_t sfDevACS37800::readRMS(float *vRMS, float *iRMS)
{
    if (vRMS == nullptr || iRMS == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_20_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_VOLATILE_20, store.data.all);
    if (rc != ksfTkErrOk)
        return rc;

    // Extract vrms. Convert to voltage in Volts.
    // Note: datasheet says "RMS voltage output. This field is an unsigned 16-bit fixed point number with 16 fractional
    // bits" Datasheet also says "Voltage Channel ADC Sensitivity: 110 LSB/mV"
    float volts = (float)store.data.bits.vrms;
    volts /= 55000.0; // Convert from codes to the fraction of ADC Full Scale (16-bit)
    volts *= 250;     // Convert to mV (Differential Input Range is +/- 250mV)
    volts /= 1000;    // Convert to Volts
    // Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
    // Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
    volts *= resistorMultiplier();
    *vRMS = volts;

    // Extract the irms. Convert to current in Amps.
    // Datasheet says: "RMS current output. This field is a signed 16-bit fixed point number with 15 fractional bits"
    float amps = (float)toSigned16(store.data.bits.irms);
    amps /= 55000.0;              // Convert from codes to the fraction of ADC Full Scale (16-bit)
    amps *= _currentSensingRange; // Convert to Amps
    *iRMS = amps;

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Read volatile register 0x21. Return the pactive and pimag.
//
sfTkError_t sfDevACS37800::readPowerActiveReactive(float *pActive, float *pReactive)
{
    if (pActive == nullptr || pReactive == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_21_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_VOLATILE_21, store.data.all);
    if (rc != ksfTkErrOk)
        return rc;

    // Extract pactive. Convert to Watts
    // Note: datasheet says:
    // "Active power output. This field is a signed 16-bit fixed point
    //  number with 15 fractional bits, where positive MaxPow = 0.704,
    //  and negative MaxPow = –0.704. To convert the value (input
    //  power) to line power, divide the input power by the RSENSE and
    //  RISO voltage divider ratio using actual resistor values."
    // Datasheet also says:
    //  "3.08 LSB/mW for the 30A version and 1.03 LSB/mW for the 90A version"
    float power = (float)toSigned16(store.data.bits.pactive);
    float LSBpermW = 3.08;                   // LSB per mW
    LSBpermW *= 30.0 / _currentSensingRange; // Correct for sensor version
    power /= LSBpermW;                       // Convert from codes to mW
    // Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
    // Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
    power *= resistorMultiplier();
    power /= 1000; // Convert from mW to W
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
    float LSBpermVAR = 6.15;                   // LSB per mVAR
    LSBpermVAR *= 30.0 / _currentSensingRange; // Correct for sensor version
    power /= LSBpermVAR;                       // Convert from codes to mVAR
    power *= resistorMultiplier();
    power /= 1000; // Convert from mVAR to VAR
    *pReactive = power;

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Read volatile register 0x22. Return the apparent power, power factor, leading / lagging, generated / consumed
//
sfTkError_t sfDevACS37800::readPowerFactor(float *pApparent, float *pFactor, bool *posangle, bool *pospf)
{
    if (pApparent == nullptr || pFactor == nullptr || posangle == nullptr || pospf == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_22_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_VOLATILE_22, store.data.all);
    if (rc != ksfTkErrOk)
        return rc;

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
    float LSBpermVA = 6.15;                   // LSB per mVA
    LSBpermVA *= 30.0 / _currentSensingRange; // Correct for sensor version
    power /= LSBpermVA;                       // Convert from codes to mVA
    // Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
    // Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
    power *= resistorMultiplier();
    power /= 1000; // Convert from mVA to VA
    *pApparent = power;

    // Extract power factor
    // Datasheet says:
    // "Power factor output. This field is a signed 11-bit fixed point number
    //  with 10 fractional bits. It ranges from –1 to ~1 with a step
    //  size of 2^-10."
    // Move 11-bit number into 16-bits (signed), then convert to +/- 1
    *pFactor = (float)toSigned16(store.data.bits.pfactor << 5) / 32768.0;

    // Extract posangle and pospf
    *posangle = store.data.bits.posangle & 0x1;
    *pospf = store.data.bits.pospf & 0x1;

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Read volatile registers 0x2A and 0x2C. Return the vInst (Volts), iInst (Amps) and pInst (Watts).
//
sfTkError_t sfDevACS37800::readInstantaneous(float *vInst, float *iInst, float *pInst)
{
    if (vInst == nullptr || iInst == nullptr || pInst == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    ACS37800_REGISTER_2A_t store;
    sfTkError_t rc = _theBus->readRegister(ACS37800_REGISTER_VOLATILE_2A, store.data.all);
    if (rc != ksfTkErrOk)
        return rc;

    // Extract the vcodes. Convert to voltage in Volts.
    // vcodes is actually int16_t but is stored in a uint32_t as a 16-bit bitfield
    float volts = (float)toSigned16(store.data.bits.vcodes);
    // Datasheet says "Voltage Channel ADC Sensitivity: 110 LSB/mV"
    volts /= 27500.0; // Convert from codes to the fraction of ADC Full Scale
    volts *= 250;     // Convert to mV (Differential Input Range is +/- 250mV)
    volts /= 1000;    // Convert to Volts
    // Correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE
    // Or:  (RISO1 + RISO2 + RISO3 + RISO4 + RSENSE) / RSENSE
    volts *= resistorMultiplier();
    *vInst = volts;

    // Extract the icodes. Convert to current in Amps.
    float amps = (float)toSigned16(store.data.bits.icodes);
    amps /= 27500.0;              // Convert from codes to the fraction of ADC Full Scale
    amps *= _currentSensingRange; // Convert to Amps
    *iInst = amps;

    ACS37800_REGISTER_2C_t pstore;
    rc = _theBus->readRegister(ACS37800_REGISTER_VOLATILE_2C, pstore.data.all);
    if (rc != ksfTkErrOk)
        return rc;

    // Extract pinstant as signed int. Convert to W
    // pinstant is actually int16_t but is stored in a uint32_t as a 16-bit bitfield
    float power = (float)toSigned16(pstore.data.bits.pinstant);
    // Datasheet says: 3.08 LSB/mW for the 30A version and 1.03 LSB/mW for the 90A version
    float LSBpermW = 3.08;                   // LSB per mW
    LSBpermW *= 30.0 / _currentSensingRange; // Correct for sensor version
    power /= LSBpermW;                       // Convert from codes to mW
    power *= resistorMultiplier();
    power /= 1000; // Convert from mW to W
    *pInst = power;

    return rc;
}

//--------------------------------------------------------------------------------------------------
// Read volatile register 0x2D. Return the error flags.
//
sfTkError_t sfDevACS37800::readErrorFlags(ACS37800_REGISTER_2D_t *errorFlags)
{
    if (errorFlags == nullptr)
        return ksfTkErrInvalidParam;

    if (_theBus == nullptr)
        return ksfTkErrBusNotInit;

    return _theBus->readRegister(ACS37800_REGISTER_VOLATILE_2D, errorFlags->data.all);
}
