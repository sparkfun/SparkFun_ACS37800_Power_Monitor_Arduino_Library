/*
  Library for the Allegro MicroSystems ACS37800 power monitor IC
  By: Paul Clark
  SparkFun Electronics
  Date: December 4th, 2021
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
  // CONFIGURING THE DEVICE FOR AC APPLICATIONS : DYNAMIC CALCULATION OF N
  // Set bypass_n_en = 0 (default). This setting enables the device to
  // dynamically calculate N based off the voltage zero crossings.
  mySensor.setBypassNenable(false, true); // Disable bypass_n in shadow memory and eeprom

  // We need to connect the LO pin to the 'low' side of the AC source.
  // So we need to set the divider resistance to 4M Ohms (instead of 2M).
  mySensor.setDividerRes(4000000); // Comment this line if you are using GND to measure the 'low' side of the AC voltage
}

void loop()
{
  float volts = 0.0;
  float amps = 0.0;

  mySensor.readRMS(&volts, &amps); // Read the RMS voltage and current
  Serial.print(F("Volts: "));
  Serial.print(volts, 2);
  Serial.print(F(" Amps: "));
  Serial.println(amps, 2);

  float pactive = 0.0;
  float preactive = 0.0;
  
  mySensor.readPowerActiveReactive(&pactive, &preactive); // Read the active and reactive power
  Serial.print(F("Power: Active (W): "));
  Serial.print(pactive, 2);
  Serial.print(F(" Reactive (VAR): "));
  Serial.println(preactive, 2);
  
  float papparent = 0.0;
  float pfactor = 0.0;
  bool posangle = 0;
  bool pospf = 0;
  
  mySensor.readPowerFactor(&papparent, &pfactor, &posangle, &pospf); // Read the apparent power and the power factor
  Serial.print(F("Power: Apparent (VA): "));
  Serial.print(papparent, 2);
  Serial.print(F(" Power Factor: "));
  Serial.print(pfactor, 2);
  if (posangle)
    Serial.print(F(" Lagging"));
  else
    Serial.print(F(" Leading"));
  if (pospf)
    Serial.println(F(" Consumed"));
  else
    Serial.println(F(" Generated"));
  
  delay(250);
}
