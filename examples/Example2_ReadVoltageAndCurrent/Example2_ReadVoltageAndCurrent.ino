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
    Serial.print(F("ACS37800 not detected. Check connections. Freezing..."));
    while (1)
      ; // Do nothing more
  }
}

void loop()
{
  float volts;
  float amps;

  mySensor.readInstantaneous(&volts, &amps); // Read the instantaneous voltage and current
  Serial.print(F("vInst: "));
  Serial.print(volts, 2);
  Serial.print(F(" iInst: "));
  Serial.print(amps, 2);

  mySensor.readRMS(&volts, &amps); // Read the RMS voltage and current
  Serial.print(F(" vRMS: "));
  Serial.print(volts, 2);
  Serial.print(F(" iRMS: "));
  Serial.print(amps, 2);

  ACS37800_REGISTER_2D_t errorFlags;
  mySensor.readErrorFlags(&errorFlags); // Read the error flags
  Serial.print(F(" vzerocrossout: "));
  Serial.print(errorFlags.data.bits.vzerocrossout);
  Serial.print(F(" faultout: "));
  Serial.print(errorFlags.data.bits.faultout);
  Serial.print(F(" faultlatched: "));
  Serial.print(errorFlags.data.bits.faultlatched);
  Serial.print(F(" overvoltage: "));
  Serial.print(errorFlags.data.bits.overvoltage);
  Serial.print(F(" undervoltage: "));
  Serial.println(errorFlags.data.bits.undervoltage);

  delay(1000);
}
