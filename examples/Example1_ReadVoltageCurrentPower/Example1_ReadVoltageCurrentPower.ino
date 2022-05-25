/*
  Library for the Allegro MicroSystems ACS37800 power monitor IC
  By: Paul Clark
  SparkFun Electronics
  Date: January 21st, 2021
  License: please see LICENSE.md for details

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/17873
*/

#include "SparkFun_ACS37800_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ACS37800
#include <Wire.h>

ACS37800 mySensor; //Create an object of the ACS37800 class

void setup()
{
  Serial.begin(115200);
  Serial.println(F("ACS37800 Example"));

  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to print useful debug messages to Serial

  //Initialize sensor using default I2C address
  if (mySensor.begin() == false)
  {
    Serial.print(F("ACS37800 not detected. Check connections and I2C address. Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // From the ACS37800 datasheet:
  // CONFIGURING THE DEVICE FOR DC APPLICATIONS : FIXED SETTING OF N
  // Set bypass_n_en = 1. This setting disables the dynamic calculation of n based off voltage zero crossings
  // and sets n to a fixed value, which is set using EERPOM field n
  //
  // Sample rate is 32kHz. Maximum number of samples is 1023 (0x3FF) (10-bit)
  mySensor.setNumberOfSamples(1023, true); // Set the number of samples in shadow memory and eeprom
  mySensor.setBypassNenable(true, true); // Enable bypass_n in shadow memory and eeprom
}

void loop()
{
  float volts = 0.0;
  float amps = 0.0;
  float watts = 0.0;

  mySensor.readInstantaneous(&volts, &amps, &watts); // Read the instantaneous voltage, current and power
  Serial.print(F("Volts: "));
  Serial.print(volts, 2);
  Serial.print(F(" Amps: "));
  Serial.print(amps, 2);
  Serial.print(F(" Watts: "));
  Serial.println(watts, 2);

  delay(250);
}
