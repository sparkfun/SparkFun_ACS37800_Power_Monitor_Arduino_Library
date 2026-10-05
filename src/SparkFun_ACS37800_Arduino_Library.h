/******************************************************************************
 * @file SparkFun_ACS37800_Arduino_Library.h
 * @brief SparkFun ACS37800 Power Monitor Library header file
 *
 * This file implements the SparkFunACS37800 class for use with the
 * I2C versions of the Allegro MicroSystems ACS37800 power monitor IC
 * https://www.sparkfun.com/products/17873
 *
 * @author SparkFun Electronics
 * @date 2021-2025
 * @version 2.0.0
 * @copyright (c) 2021-2025 SparkFun Electronics Inc. This project is released under the MIT License.
 *
 * SPDX-License-Identifier: MIT
 *
 ******************************************************************************/

#pragma once

// helps to keep the Toolkit header before the tk calls
// clang-format off
#include <SparkFun_Toolkit.h>
#include "sfTk/sfDevACS37800.h"
// clang-format on

// For backwards compatibility - with version 1.x of this library, remap error codes
// to the toolkit error codes
typedef enum : sfTkError_t
{
    ACS37800_SUCCESS = ksfTkErrOk,
    ACS37800_ERR_I2C_ERROR = ksfTkErrFail,
    ACS37800_ERR_REGISTER_READ_MODIFY_WRITE_FAILURE = ksfTkErrACS37800ReadModifyWriteFailure
} ACS37800ERR;

/**
 * @brief Class for interfacing with the ACS37800 power monitor using I2C communication
 *
 * This class provides methods to initialize and communicate with the ACS37800 over an
 * I2C bus. It inherits from the sfDevACS37800 class and uses the SparkFun Toolkit for
 * I2C communication.
 *
 * @see sfDevACS37800
 */
class SparkFunACS37800 : public sfDevACS37800
{
  public:
    /**
     * @brief Begins the Device with I2C as the communication bus
     *
     * This method initializes the I2C bus and sets up communication with the ACS37800.
     *
     * @param address I2C device address to use for the sensor
     * @param wirePort Wire port to use for I2C communication
     * @return True if successful, false otherwise
     */
    bool begin(const uint8_t address = ACS37800_DEFAULT_I2C_ADDRESS, TwoWire &wirePort = Wire)
    {
        // Setup Arduino I2C bus
        if (_theI2CBus.init(wirePort, address) != ksfTkErrOk)
            return false;

        _theI2CBus.setStop(false); // Send a restart between the register address write and the data read

        // Begin the sensor
        return sfDevACS37800::begin(&_theI2CBus) == ksfTkErrOk;
    }

    /**
     * @brief Debug printing is no longer supported.
     *
     * @deprecated since version 2.0.0. This method does nothing and is kept for backwards compatibility.
     */
    void enableDebugging(Stream &debugPort = Serial)
    {
        (void)debugPort;
    }

  private:
    sfTkArdI2C _theI2CBus;
};

// for backwards compatibility
/**
 * @brief Deprecated class for interfacing with the ACS37800 - supports version 1.x of this library
 *
 * @deprecated This class is deprecated for version 2.0 of this library. Use SparkFunACS37800 instead.
 */
class ACS37800 : public SparkFunACS37800
{
};
