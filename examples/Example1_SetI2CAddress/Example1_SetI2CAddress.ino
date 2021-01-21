/*
  Library for the Allegro MicroSystems ACS37800 power monitor IC
  By: Paul Clark
  SparkFun Electronics
  Date: January 21st, 2021
  License: please see LICENSE.md for details

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/nnnnn
*/

#include "SparkFun_ACS37800_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ACS37800
#include <Wire.h>

#define OLD_ADDRESS 0x61 //This is the address the ACS37800 is currently using
#define NEW_ADDRESS 0x62 //This is the address we are changing to

ACS37800 mySensor; //Create an object of the ACS37800 class

void setup()
{
  Serial.begin(115200);
  Serial.println(F("ACS37800 Example"));

  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to print useful debug messages to Serial

  //Initialize sensor
  if (mySensor.begin(OLD_ADDRESS) == false)
  {
    Serial.print(F("ACS37800 not detected at 0x"));
    Serial.print(OLD_ADDRESS);
    Serial.println(F(". Check connections. Freezing..."));
    while (1)
      ; // Do nothing more
  }

  Serial.print(F("Changing the address to 0x"));
  Serial.println(NEW_ADDRESS);

  //Initialize sensor
  if (mySensor.setI2Caddress(NEW_ADDRESS) == ACS37800_SUCCESS)
  {
    Serial.println(F("Address changed! Please power cycle the sensor to use the new address. Freezing..."));
  }
  else
  {
    Serial.println(F("Changing the address failed! Please try again. Freezing..."));    
  }
}

void loop()
{
  //Nothing to do here
}
