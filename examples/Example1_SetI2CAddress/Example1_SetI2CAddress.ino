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

#define NEW_ADDRESS 0x60 //Set this to the address we are changing _to_

ACS37800 mySensor; //Create an object of the ACS37800 class

void setup()
{
  Serial.begin(115200);
  Serial.println(F("ACS37800 Example"));

  emptySerial(); // Make sure the serial buffer is empty

  Serial.println(F("Press any key to continue..."));

  while (Serial.available() == 0) // Wait for the user to press a key
  {
    ;
  }

  Wire.begin();

  Wire.setClock(100000); //During detection, go slow

  for (uint8_t address = 1 ; address <= 127 ; address++) // Scan all I2C addresses
  {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) // Did we detect something?
    {
      if (address == NEW_ADDRESS) // Check if the address already matches NEW_ADDRESS
      {
        Serial.print(F("A device is already using address 0x"));
        Serial.print(NEW_ADDRESS, HEX);
        Serial.println(F(". Freezing..."));
        while(1)
          ; // Do nothing more
      }
      else
      {
        emptySerial(); // Make sure the serial buffer is empty
        
        Serial.print(F("Found something at address 0x"));
        if (address < 16) Serial.print(F("0")); // Pad with zero if required
        Serial.print(address, HEX);
        Serial.print(F(". Do you want to change the address to 0x"));
        if (address < 16) Serial.print(F("0")); // Pad with zero if required
        Serial.print(NEW_ADDRESS, HEX);
        Serial.println(F("? (Y/n)"));

        while (Serial.available() == 0) // Wait for the user to press a key
        {
          ;
        }
        
        uint8_t yesNo = Serial.read(); // Get the keypress

        if ((yesNo == 'Y') || (yesNo == 'y') || (yesNo == '\r') || (yesNo == '\n')) // Check for Y or y or newline
        {

          // Change the address
          
          //mySensor.enableDebugging(); // Uncomment this line to print useful debug messages to Serial
        
          //Initialize sensor
          if (mySensor.begin(address) == false)
          {
            Serial.print(F("Could not .begin ACS37800 at address 0x"));
            if (address < 16) Serial.print(F("0")); // Pad with zero if required
            Serial.print(address, HEX);
            Serial.println(F(". Check connections. Freezing..."));
            while (1)
              ; // Do nothing more
          }
        
          Serial.print(F("Changing the address to 0x"));
          if (address < NEW_ADDRESS) Serial.print(F("0")); // Pad with zero if required
          Serial.println(NEW_ADDRESS, HEX);
        
          if (mySensor.setI2Caddress(NEW_ADDRESS) == ACS37800_SUCCESS)
          {
            Serial.println(F("Address changed! Please power cycle the sensor to use the new address. Freezing..."));
          }
          else
          {
            Serial.println(F("Changing the address failed! Please try again. Freezing..."));    
          }
          
          while (1)
            ; // Do nothing more
        }
      }
    }
  }
  Serial.println(F("All done. Freezing..."));    
}

void loop()
{
  //Nothing to do here
}

void emptySerial()
{
  delay(100);
  while (Serial.available()) // Make sure the serial buffer is empty
  {
    Serial.read();
  }
}
