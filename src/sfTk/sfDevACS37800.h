/*!
 * @file sfDevACS37800.h
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

#pragma once

#include <stdint.h>

// include the sparkfun toolkit headers
#include <sfTk/sfToolkit.h>

// Bus interfaces
#include <sfTk/sfTkIBus.h>

/**
 * @brief The default I2C Address is 0x60 when DIO_0 and DIO_1 are 0V on start-up
 *
 * (There is a typo in the datasheet that suggests it is 0x61. It isn't...!)
 * The address can be configured in EEPROM too using setI2Caddress
 */
const uint8_t ACS37800_DEFAULT_I2C_ADDRESS = 0x60;

/** Customer Access Code - stored in volatile register 0x2F */
const uint32_t ACS37800_CUSTOMER_ACCESS_CODE = 0x4F70656E;

/** Default sense resistance for voltage measurement (Ohms) */
const float ACS37800_DEFAULT_SENSE_RES = 8200;

/** Default voltage-divider resistance for voltage measurement (Ohms) */
const float ACS37800_DEFAULT_DIVIDER_RES = 2000000;

/**
 * @brief Default current-sensing range
 *
 * ACS37800KMACTR-030B3-I2C is a 30.0 Amp part - as used on the SparkFun Qwiic Power Meter
 * ACS37800KMACTR-090B3-I2C is a 90.0 Amp part
 */
const float ACS37800_DEFAULT_CURRENT_RANGE = 30;

/**
 * @brief Error code returned when a read-modify-write of a register could not be verified
 */
const sfTkError_t ksfTkErrACS37800ReadModifyWriteFailure = ksfTkErrFail * 0x2001;

// EEPROM Registers
const uint8_t ACS37800_REGISTER_EEPROM_0B = 0x0B;
const uint8_t ACS37800_REGISTER_EEPROM_0C = 0x0C;
const uint8_t ACS37800_REGISTER_EEPROM_0D = 0x0D;
const uint8_t ACS37800_REGISTER_EEPROM_0E = 0x0E;
const uint8_t ACS37800_REGISTER_EEPROM_0F = 0x0F;

// Shadow Registers
// At power up, all shadow registers are loaded from EEPROM, including all configuration parameters.
// The shadow registers can be written to in order to change the device behavior without having to
// perform an EEPROM write. Any changes made in shadow memory are volatile and do not persist through a reset event.
const uint8_t ACS37800_REGISTER_SHADOW_1B = 0x1B;
const uint8_t ACS37800_REGISTER_SHADOW_1C = 0x1C;
const uint8_t ACS37800_REGISTER_SHADOW_1D = 0x1D;
const uint8_t ACS37800_REGISTER_SHADOW_1E = 0x1E;
const uint8_t ACS37800_REGISTER_SHADOW_1F = 0x1F;

// Volatile Registers
const uint8_t ACS37800_REGISTER_VOLATILE_20 = 0x20;
const uint8_t ACS37800_REGISTER_VOLATILE_21 = 0x21;
const uint8_t ACS37800_REGISTER_VOLATILE_22 = 0x22;
const uint8_t ACS37800_REGISTER_VOLATILE_25 = 0x25;
const uint8_t ACS37800_REGISTER_VOLATILE_26 = 0x26;
const uint8_t ACS37800_REGISTER_VOLATILE_27 = 0x27;
const uint8_t ACS37800_REGISTER_VOLATILE_28 = 0x28;
const uint8_t ACS37800_REGISTER_VOLATILE_29 = 0x29;
const uint8_t ACS37800_REGISTER_VOLATILE_2A = 0x2A;
const uint8_t ACS37800_REGISTER_VOLATILE_2C = 0x2C;
const uint8_t ACS37800_REGISTER_VOLATILE_2D = 0x2D;
const uint8_t ACS37800_REGISTER_VOLATILE_2F = 0x2F;
const uint8_t ACS37800_REGISTER_VOLATILE_30 = 0x30;

// EEPROM Registers : Bit Field definitions

/** EEPROM register 0x0B / shadow register 0x1B */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t qvo_fine : 9;
            uint32_t sns_fine : 10;
            uint32_t crs_sns : 3;
            uint32_t iavgselen : 1;
            uint32_t pavgselen : 1;
            uint32_t reserved : 2;
            uint32_t ECC : 6;
        } bits;
    } data;
} ACS37800_REGISTER_0B_t;

/** EEPROM register 0x0C / shadow register 0x1C */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t rms_avg_1 : 7;
            uint32_t rms_avg_2 : 10;
            uint32_t vchan_offset_code : 8;
            uint32_t reserved : 1;
            uint32_t ECC : 6;
        } bits;
    } data;
} ACS37800_REGISTER_0C_t;

/** EEPROM register 0x0D / shadow register 0x1D */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t reserved1 : 7;
            uint32_t ichan_del_en : 1;
            uint32_t reserved2 : 1;
            uint32_t chan_del_sel : 3;
            uint32_t reserved3 : 1;
            uint32_t fault : 8;
            uint32_t fltdly : 3;
            uint32_t reserved4 : 2;
            uint32_t ECC : 6;
        } bits;
    } data;
} ACS37800_REGISTER_0D_t;

/** EEPROM register 0x0E / shadow register 0x1E */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t vevent_cycs : 6;
            uint32_t reserved1 : 2;
            uint32_t overvreg : 6;
            uint32_t undervreg : 6;
            uint32_t delaycnt_sel : 1;
            uint32_t halfcycle_en : 1;
            uint32_t squarewave_en : 1;
            uint32_t zerocrosschansel : 1;
            uint32_t zerocrossedgesel : 1;
            uint32_t reserved2 : 1;
            uint32_t ECC : 6;
        } bits;
    } data;
} ACS37800_REGISTER_0E_t;

/** EEPROM register 0x0F / shadow register 0x1F */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t reserved1 : 2;
            uint32_t i2c_slv_addr : 7;
            uint32_t i2c_dis_slv_addr : 1;
            uint32_t dio_0_sel : 2;
            uint32_t dio_1_sel : 2;
            uint32_t n : 10;
            uint32_t bypass_n_en : 1;
            uint32_t reserved2 : 1;
            uint32_t ECC : 6;
        } bits;
    } data;
} ACS37800_REGISTER_0F_t;

// Volatile Registers : Bit Field definitions

/** Volatile register 0x20 - RMS voltage and current */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t vrms : 16;
            uint32_t irms : 16;
        } bits;
    } data;
} ACS37800_REGISTER_20_t;

/** Volatile register 0x21 - active and reactive power */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t pactive : 16;
            uint32_t pimag : 16;
        } bits;
    } data;
} ACS37800_REGISTER_21_t;

/** Volatile register 0x22 - apparent power and power factor */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t papparent : 16;
            uint32_t pfactor : 11;
            uint32_t posangle : 1;
            uint32_t pospf : 1;
        } bits;
    } data;
} ACS37800_REGISTER_22_t;

/** Volatile register 0x25 - number of samples */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t numptsout : 10;
        } bits;
    } data;
} ACS37800_REGISTER_25_t;

/** Volatile register 0x26 - one second RMS averages */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t vrmsavgonesec : 16;
            uint32_t irmsavgonesec : 16;
        } bits;
    } data;
} ACS37800_REGISTER_26_t;

/** Volatile register 0x27 - one minute RMS averages */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t vrmsavgonemin : 16;
            uint32_t irmsavgonemin : 16;
        } bits;
    } data;
} ACS37800_REGISTER_27_t;

/** Volatile register 0x28 - one second active power average */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t pactavgonesec : 16;
        } bits;
    } data;
} ACS37800_REGISTER_28_t;

/** Volatile register 0x29 - one minute active power average */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t pactavgonemin : 16;
        } bits;
    } data;
} ACS37800_REGISTER_29_t;

/** Volatile register 0x2A - instantaneous voltage and current codes */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t vcodes : 16;
            uint32_t icodes : 16;
        } bits;
    } data;
} ACS37800_REGISTER_2A_t;

/** Volatile register 0x2C - instantaneous power */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t pinstant : 16;
        } bits;
    } data;
} ACS37800_REGISTER_2C_t;

/** Volatile register 0x2D - status / error flags */
typedef struct
{
    union {
        uint32_t all;
        struct
        {
            uint32_t vzerocrossout : 1;
            uint32_t faultout : 1;
            uint32_t faultlatched : 1;
            uint32_t overvoltage : 1;
            uint32_t undervoltage : 1;
        } bits;
    } data;
} ACS37800_REGISTER_2D_t;

/**
 * @brief Shadow register selection for readShadowRegister() and writeShadowRegister()
 *
 * Each shadow register (0x1B - 0x1F) is paired with an EEPROM register (0x0B - 0x0F).
 */
typedef enum
{
    ACS37800_SHADOW_REGISTER_1B = ACS37800_REGISTER_SHADOW_1B,
    ACS37800_SHADOW_REGISTER_1C = ACS37800_REGISTER_SHADOW_1C,
    ACS37800_SHADOW_REGISTER_1D = ACS37800_REGISTER_SHADOW_1D,
    ACS37800_SHADOW_REGISTER_1E = ACS37800_REGISTER_SHADOW_1E,
    ACS37800_SHADOW_REGISTER_1F = ACS37800_REGISTER_SHADOW_1F
} ACS37800_shadow_register_t;

// Register Field Enums

/** Coarse gain for the current channel */
typedef enum
{
    ACS37800_CRS_SNS_1X = 0,
    ACS37800_CRS_SNS_2X,
    ACS37800_CRS_SNS_3X,
    ACS37800_CRS_SNS_3POINT5X,
    ACS37800_CRS_SNS_4X,
    ACS37800_CRS_SNS_4POINT5X, // This appears to be the default
    ACS37800_CRS_SNS_5POINT5X,
    ACS37800_CRS_SNS_8X
} ACS37800_CRS_SNS_e;

/** Gain values, indexed by ACS37800_CRS_SNS_e */
const float ACS37800_CRS_SNS_GAINS[8] = {1.0, 2.0, 3.0, 3.5, 4.0, 4.5, 5.5, 8.0};

/** Fault Delay */
typedef enum
{
    ACS37800_FLTDLY_0000 = 0,
    ACS37800_FLTDLY_0475 = 2, // 4.75 microseconds
    ACS37800_FLTDLY_0925,     // 9.25 microseconds
    ACS37800_FLTDLY_1375,
    ACS37800_FLTDLY_1850,
    ACS37800_FLTDLY_2325,
    ACS37800_FLTDLY_2775 // 27.75 microseconds
} ACS37800_FLTDLY_e;

/** DIO_0 Function */
typedef enum
{
    ACS37800_DIO0_FUNC_ZERO_CROSSING = 0,
    ACS37800_DIO0_FUNC_OVERVOLTAGE,
    ACS37800_DIO0_FUNC_UNDERVOLTAGE,
    ACS37800_DIO0_FUNC_OV_OR_UV
} ACS37800_DIO0_FUNC_e;

/** DIO_1 Function */
typedef enum
{
    ACS37800_DIO1_FUNC_OVERCURRENT = 0,
    ACS37800_DIO1_FUNC_UNDERVOLTAGE,
    ACS37800_DIO1_FUNC_OVERVOLTAGE,
    ACS37800_DIO1_FUNC_OV_OR_UV_OR_OCF_LAT
} ACS37800_DIO1_FUNC_e;

/** EEPROM ECC Errors */
typedef enum
{
    ACS37800_EEPROM_ECC_NO_ERROR = 0,
    ACS37800_EEPROM_ECC_ERROR_CORRECTED,
    ACS37800_EEPROM_ECC_ERROR_UNCORRECTABLE,
    ACS37800_EEPROM_ECC_NO_MEANING
} ACS37800_EEPROM_ECC_e;

/**
 * @class sfDevACS37800
 * @brief Driver class for the ACS37800 power monitor IC.
 *
 * This class provides a platform independent interface to configure and read data from the ACS37800.
 * Communication is performed through a SparkFun Toolkit bus object (sfTkIBus).
 *
 * Usage:
 * - Call begin() with an initialized bus object.
 * - Use setSenseRes(), setDividerRes() and setCurrentRange() to match the hardware configuration.
 * - Use the read methods to obtain voltage, current and power values.
 */
class sfDevACS37800
{
  public:
    sfDevACS37800()
        : _theBus{nullptr}, _senseResistance{ACS37800_DEFAULT_SENSE_RES},
          _dividerResistance{ACS37800_DEFAULT_DIVIDER_RES}, _currentSensingRange{ACS37800_DEFAULT_CURRENT_RANGE},
          _currentCoarseGain{0.0}
    {
    }

    /**
     * @brief Initializes the ACS37800 device on the specified bus.
     *
     * The bus is set to little endian byte order and the current coarse gain is read from shadow memory.
     *
     * @param theBus Pointer to the bus interface (sfTkIBus) to use for communication.
     * @return sfTkError_t ksfTkErrOk on success, ksfTkErrBusNotInit if no bus is provided, or a bus error code.
     */
    sfTkError_t begin(sfTkIBus *theBus = nullptr);

    /**
     * @brief Checks if the ACS37800 is connected and responding.
     *
     * @return true if the device responds to a register read, false otherwise.
     */
    bool isConnected(void);

    /**
     * @brief Read the contents of a shadow register
     *
     * @param reg The shadow register to read
     * @param[out] value The register contents
     * @return sfTkError_t ksfTkErrOk on success, ksfTkErrInvalidParam if reg is not a shadow register
     */
    sfTkError_t readShadowRegister(ACS37800_shadow_register_t reg, uint32_t *value);

    /**
     * @brief Write the contents of a shadow register
     *
     * The customer access code is written before, and cleared after, the register is written.
     * Shadow register changes do not persist through a reset event - set _eeprom to true to also write
     * the value to the matching EEPROM register.
     *
     * @param reg The shadow register to write
     * @param value The new register contents
     * @param _eeprom If true, the value is written to EEPROM as well as shadow memory
     * @return sfTkError_t ksfTkErrOk on success, ksfTkErrInvalidParam if reg is not a shadow register
     */
    sfTkError_t writeShadowRegister(ACS37800_shadow_register_t reg, uint32_t value, bool _eeprom = false);

    /**
     * @brief Change the I2C address in EEPROM (i2c_slv_addr)
     *
     * This also sets the i2c_dis_slv_addr flag so the DIO pins will no longer define the I2C address.
     * The new address takes effect after the device is power cycled.
     *
     * @param newAddress The new 7-bit I2C address
     * @return sfTkError_t ksfTkErrOk on success, ksfTkErrInvalidParam if the address is not 7-bit,
     *         ksfTkErrACS37800ReadModifyWriteFailure if the change could not be verified
     */
    sfTkError_t setI2Caddress(uint8_t newAddress);

    /**
     * @brief Set the number of samples for RMS calculations.
     *
     * Bypass_N_Enable must be set/true for this to have effect.
     *
     * @param numberOfSamples The number of samples (10 bits)
     * @param _eeprom If true, the value is written to EEPROM as well as shadow memory
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t setNumberOfSamples(uint32_t numberOfSamples, bool _eeprom = false);

    /**
     * @brief Read the number of samples from shadow memory
     *
     * @param[out] numberOfSamples The number of samples
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t getNumberOfSamples(uint32_t *numberOfSamples);

    /**
     * @brief Set/Clear the Bypass_N_Enable flag
     *
     * @param bypass The new flag value
     * @param _eeprom If true, the value is written to EEPROM as well as shadow memory
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t setBypassNenable(bool bypass, bool _eeprom = false);

    /**
     * @brief Read the bypass_n_en flag from shadow memory
     *
     * @param[out] bypass The flag value
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t getBypassNenable(bool *bypass);

    /**
     * @brief Read the current channel coarse gain from shadow memory
     *
     * @param[out] currentCoarseGain The gain
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t getCurrentCoarseGain(float *currentCoarseGain);

    /**
     * @brief Read volatile register 0x20. Return the vRMS (Volts) and iRMS (Amps).
     *
     * @param[out] vRMS RMS voltage
     * @param[out] iRMS RMS current
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t readRMS(float *vRMS, float *iRMS);

    /**
     * @brief Read volatile register 0x21. Return the active power (W) and reactive power (VAR)
     *
     * @param[out] pActive Active power
     * @param[out] pReactive Reactive power
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t readPowerActiveReactive(float *pActive, float *pReactive);

    /**
     * @brief Read volatile register 0x22. Return the apparent power, power factor, leading / lagging,
     * generated / consumed
     *
     * @param[out] pApparent Apparent power (VA)
     * @param[out] pFactor Power factor (-1 to ~1)
     * @param[out] posangle true if leading, false if lagging
     * @param[out] pospf true if consumed, false if generated
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t readPowerFactor(float *pApparent, float *pFactor, bool *posangle, bool *pospf);

    /**
     * @brief Read volatile registers 0x2A and 0x2C. Return the vInst (Volts), iInst (Amps) and pInst (Watts).
     *
     * @param[out] vInst Instantaneous voltage
     * @param[out] iInst Instantaneous current
     * @param[out] pInst Instantaneous power
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t readInstantaneous(float *vInst, float *iInst, float *pInst);

    /**
     * @brief Read volatile register 0x2D. Return its contents in errorFlags.
     *
     * @param[out] errorFlags The register contents
     * @return sfTkError_t ksfTkErrOk on success
     */
    sfTkError_t readErrorFlags(ACS37800_REGISTER_2D_t *errorFlags);

    /**
     * @brief Change the value of the sense resistor used for voltage measurement (Ohms)
     */
    void setSenseRes(float newRes)
    {
        _senseResistance = newRes;
    }

    /**
     * @brief Change the value of the voltage divider resistance (Ohms)
     */
    void setDividerRes(float newRes)
    {
        _dividerResistance = newRes;
    }

    /**
     * @brief Change the current-sensing range (Amps)
     *
     * ACS37800KMACTR-030B3-I2C is a 30.0 Amp part - as used on the SparkFun Qwiic Power Meter
     * ACS37800KMACTR-090B3-I2C is a 90.0 Amp part
     */
    void setCurrentRange(float newCurrent)
    {
        _currentSensingRange = newCurrent;
    }

  private:
    /**
     * @brief Update bits in an EEPROM register and/or its shadow register (eepromReg + 0x10).
     *
     * Writes the customer access code, performs a read-modify-write on the selected registers,
     * clears the customer access code and waits for the memory to update.
     *
     * @param eepromReg The EEPROM register address (0x0B - 0x0F)
     * @param mask Bits to change
     * @param value New values for the masked bits
     * @param shadow Update the shadow register
     * @param eeprom Update the EEPROM register
     * @return sfTkError_t ksfTkErrOk on success, otherwise the first error encountered
     */
    sfTkError_t updateConfigBits(uint8_t eepromReg, uint32_t mask, uint32_t value, bool shadow, bool eeprom);

    /** Multiplier to correct for the voltage divider: (RISO1 + RISO2 + RSENSE) / RSENSE */
    float resistorMultiplier(void)
    {
        return (_dividerResistance + _senseResistance) / _senseResistance;
    }

    /** Pointer to the bus interface used for communication with the ACS37800 */
    sfTkIBus *_theBus;

    /** The value of the sense resistor for voltage measurement in Ohms */
    float _senseResistance;

    /** The value of the divider resistance for voltage measurement in Ohms */
    float _dividerResistance;

    /** The ACS37800's current sensing range */
    float _currentSensingRange;

    /** The ACS37800's coarse current gain */
    float _currentCoarseGain;
};
