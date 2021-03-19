/*
  Library for the Allegro MicroSystems ACS37800 power monitor IC
  By: Paul Clark
  SparkFun Electronics
  Date: January 21st, 2021
  License: please see LICENSE.md for details

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/17873

  This example allows you to read and modify the ACS37800 registers (both eeprom and shadow)

  Note:
    The registers are locked by default. To unlock, write the customer access code 0x4F70656E to volatile regsiter 2F.
    Register 2F is write-only. You cannot read the access code back again.
    Register 30 will change from 0 to 1 if the code has been accepted.
    You need to cycle the power to disable customer write access. Writing zero to register 2F does not disable write access.

  Typical defaults for the ACS37800KMACTR-030B3-I2C (+/- 30A) are:

  00: Register 1B : qvo_fine          :   19 0x013 0b000010011   ** NOTE: this is device-specific and varies from chip to chip
  01: Register 1B : sns_fine          :  517 0x205 0b1000000101  ** NOTE: this is device-specific and varies from chip to chip
  02: Register 1B : crs_sns           :    5 0x5   0b101         ** NOTE: this will be different for the +/- 90A version
  03: Register 1B : iavgselen         :    0 0x0   0b0
  04: Register 1B : pavgselen         :    0 0x0   0b0
  05: Register 1C : rms_avg_1         :    0 0x00  0b0000000
  06: Register 1C : rms_avg_2         :    0 0x000 0b0000000000
  07: Register 1C : vchan_offset_code :    0 0x00  0b00000000    ** NOTE: datasheet says this is device-specific
  08: Register 1D : ichan_del_en      :    0 0x0   0b0
  09: Register 1D : chan_del_sel      :    0 0x0   0b000
  10: Register 1D : fault             :   70 0x46  0b01000110
  11: Register 1D : fltdly            :    0 0x0   0b000
  12: Register 1E : vevent_cycs       :    0 0x00  0b000000
  13: Register 1E : overvreg          :   32 0x20  0b100000
  14: Register 1E : undervreg         :   32 0x20  0b100000
  15: Register 1E : delaycnt_sel      :    0 0x0   0b0
  16: Register 1E : halfcycle_en      :    0 0x0   0b0
  17: Register 1E : squarewave_en     :    0 0x0   0b0
  18: Register 1E : zerocrosschansel  :    0 0x0   0b0
  19: Register 1E : zerocrossedgesel  :    0 0x0   0b0
  20: Register 1F : i2c_slv_addr      :  127 0x7F  0b1111111
  21: Register 1F : i2c_dis_slv_addr  :    0 0x0   0b0
  22: Register 1F : dio_0_sel         :    0 0x0   0b00
  23: Register 1F : dio_1_sel         :    0 0x0   0b00
  24: Register 1F : n                 :    0 0x000 0b0000000000
  25: Register 1F : bypass_n_en       :    0 0x0   0b0

*/

#include "SparkFun_ACS37800_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ACS37800
#include <Wire.h>

ACS37800 mySensor; //Create an object of the ACS37800 class

// Global copies of the register contents
// EEPROM registers are 0B-0F. Shadow registers are 1B-1F. Volatile registers are 20-30.
ACS37800_REGISTER_0B_t reg1B;
ACS37800_REGISTER_0C_t reg1C;
ACS37800_REGISTER_0D_t reg1D;
ACS37800_REGISTER_0E_t reg1E;
ACS37800_REGISTER_0F_t reg1F;
uint32_t reg2F; // Customer access code
uint32_t reg30; // Customer mode (write access)

void setup()
{
  Serial.begin(115200);
  Serial.println(F("ACS37800 Example"));

  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to print useful debug messages to Serial

  if (mySensor.begin(0x60) == false) //Initialize sensor using I2C address 0x60
  {
    Serial.print(F("ACS37800 not detected. Check connections and I2C address. Freezing..."));
    while (1)
      ; // Do nothing more
  }

}

void loop()
{
  // Read the shadow registers
  mySensor.readRegister(&reg1B.data.all, ACS37800_REGISTER_SHADOW_1B);
  mySensor.readRegister(&reg1C.data.all, ACS37800_REGISTER_SHADOW_1C);
  mySensor.readRegister(&reg1D.data.all, ACS37800_REGISTER_SHADOW_1D);
  mySensor.readRegister(&reg1E.data.all, ACS37800_REGISTER_SHADOW_1E);
  mySensor.readRegister(&reg1F.data.all, ACS37800_REGISTER_SHADOW_1F);
  // Read the volatile registers
  mySensor.readRegister(&reg2F, ACS37800_REGISTER_VOLATILE_2F);
  mySensor.readRegister(&reg30, ACS37800_REGISTER_VOLATILE_30);

  // Pretty-print the register contents
  Serial.println();
  Serial.println();
  Serial.print(F("00: Register 1B : qvo_fine          : ")); printDecHexBin(reg1B.data.bits.qvo_fine, 9);
  Serial.print(F("01: Register 1B : sns_fine          : ")); printDecHexBin(reg1B.data.bits.sns_fine, 10);
  Serial.print(F("02: Register 1B : crs_sns           : ")); printDecHexBin(reg1B.data.bits.crs_sns, 3);
  Serial.print(F("03: Register 1B : iavgselen         : ")); printDecHexBin(reg1B.data.bits.iavgselen, 1);
  Serial.print(F("04: Register 1B : pavgselen         : ")); printDecHexBin(reg1B.data.bits.pavgselen, 1);

  Serial.print(F("05: Register 1C : rms_avg_1         : ")); printDecHexBin(reg1C.data.bits.rms_avg_1, 7);
  Serial.print(F("06: Register 1C : rms_avg_2         : ")); printDecHexBin(reg1C.data.bits.rms_avg_2, 10);
  Serial.print(F("07: Register 1C : vchan_offset_code : ")); printDecHexBin(reg1C.data.bits.vchan_offset_code, 8);

  Serial.print(F("08: Register 1D : ichan_del_en      : ")); printDecHexBin(reg1D.data.bits.ichan_del_en, 1);
  Serial.print(F("09: Register 1D : chan_del_sel      : ")); printDecHexBin(reg1D.data.bits.chan_del_sel, 3);
  Serial.print(F("10: Register 1D : fault             : ")); printDecHexBin(reg1D.data.bits.fault, 8);
  Serial.print(F("11: Register 1D : fltdly            : ")); printDecHexBin(reg1D.data.bits.fltdly, 3);

  Serial.print(F("12: Register 1E : vevent_cycs       : ")); printDecHexBin(reg1E.data.bits.vevent_cycs, 6);
  Serial.print(F("13: Register 1E : overvreg          : ")); printDecHexBin(reg1E.data.bits.overvreg, 6);
  Serial.print(F("14: Register 1E : undervreg         : ")); printDecHexBin(reg1E.data.bits.undervreg, 6);
  Serial.print(F("15: Register 1E : delaycnt_sel      : ")); printDecHexBin(reg1E.data.bits.delaycnt_sel, 1);
  Serial.print(F("16: Register 1E : halfcycle_en      : ")); printDecHexBin(reg1E.data.bits.halfcycle_en, 1);
  Serial.print(F("17: Register 1E : squarewave_en     : ")); printDecHexBin(reg1E.data.bits.squarewave_en, 1);
  Serial.print(F("18: Register 1E : zerocrosschansel  : ")); printDecHexBin(reg1E.data.bits.zerocrosschansel, 1);
  Serial.print(F("19: Register 1E : zerocrossedgesel  : ")); printDecHexBin(reg1E.data.bits.zerocrossedgesel, 1);

  Serial.print(F("20: Register 1F : i2c_slv_addr      : ")); printDecHexBin(reg1F.data.bits.i2c_slv_addr, 7);
  Serial.print(F("21: Register 1F : i2c_dis_slv_addr  : ")); printDecHexBin(reg1F.data.bits.i2c_dis_slv_addr, 1);
  Serial.print(F("22: Register 1F : dio_0_sel         : ")); printDecHexBin(reg1F.data.bits.dio_0_sel, 2);
  Serial.print(F("23: Register 1F : dio_1_sel         : ")); printDecHexBin(reg1F.data.bits.dio_1_sel, 2);
  Serial.print(F("24: Register 1F : n                 : ")); printDecHexBin(reg1F.data.bits.n, 10);
  Serial.print(F("25: Register 1F : bypass_n_en       : ")); printDecHexBin(reg1F.data.bits.bypass_n_en, 1);

  Serial.print(F("26: Register 2F (customer access code)")); Serial.println();

  Serial.println();
  Serial.print(F("Customer write access is "));
  if (reg30 == 0)
  {
    Serial.println(F("disabled"));
    Serial.println(F("To enable write access: set register 2F to x4F70656E"));
  }
  else
  {
    Serial.println(F("enabled!"));
  }
  
  Serial.println();
  Serial.println(F("To change a value, enter the two digit code followed by b (binary) | x (hex) | d (decimal) followed by the new value and CR or LF"));
  Serial.println(F("E.g.: 26x4F70656E\\r or 22b01\\r or 23d2\\r"));
  Serial.println();

  clearSerial();

  uint8_t character = getChar(); // Get first digit of code
  if ((character < '0') || (character > '2')) return; // Check it is valid
  int param = (character - '0') * 10;
  character = getChar(); // Get second digit of code
  if ((character < '0') || (character > '9')) return; // Check it is valid
  param += character - '0';
  if (param > 26) return; // Check the two byte code is valid
  
  character = getChar(); // Get the 'base' b, x or d
  if ((character != 'b') && (character != 'd') && (character != 'x')) return; // Check it is valid
  uint32_t newValue = 0; // Store the new value
  if (character == 'b') // Binary
  {
    character = '0';
    while ((character == '0') || (character == '1'))
    {
      character = getChar(); // Get 0 or 1
      if (character == '0')
      {
        newValue <<= 1; // Shift newValue along by 1 bit (LS bit will be zero)
      }
      if (character == '1')
      {
        newValue <<= 1; // Shift newValue along by 1 bit (LS bit will be zero)
        newValue |= 1; // Set the LS bit
      }
    }
  }
  else if (character == 'x') // Hex
  {
    character = '0';
    while (((character >= '0') && (character <= '9')) || ((character >= 'a') && (character <= 'f')) || ((character >= 'A') && (character <= 'F')))
    {
      character = getChar(); // Get 0 to F
      if ((character >= '0') && (character <= '9'))
      {
        newValue <<= 4; // Shift newValue along by 4 bits (LS bits will be zero)
        newValue |= (character - '0') & 0xF; // Convert to 4 bits and or into newValue
      }
      if ((character >= 'a') && (character <= 'f'))
      {
        newValue <<= 4; // Shift newValue along by 4 bits (LS bits will be zero)
        newValue |= ((character - 'a') + 0x0a) & 0xF; // Convert to 4 bits and or into newValue
      }
      if ((character >= 'A') && (character <= 'F'))
      {
        newValue <<= 4; // Shift newValue along by 4 bits (LS bits will be zero)
        newValue |= ((character - 'A') + 0x0a) & 0xF; // Convert to 4 bits and or into newValue
      }
    }
  }
  else if (character == 'd') // decimal
  {
    character = '0';
    uint32_t decVal = 0; // decimal value
    while ((character >= '0') && (character <= '9'))
    {
      character = getChar(); // Get 0 to 9
      if ((character >= '0') && (character <= '9'))
      {
        decVal *= 10; // Multiply the existing value by 10
        decVal += character - '0'; // Add to decimal value
      }
    }
    newValue = decVal; // Copy into newValue
  }
  else
  {
    return; // base was not b, x, or d
  }

  if ((character != '\r') && (character != '\n')) return; // Check value was terminated by \r or \n

  // Update the value in both eeprom and shadow registers
  switch (param)
  {
    case 0:
      newValue &= 0b111111111; // For safety, mask off any invalid bits
      reg1B.data.bits.qvo_fine = newValue;
      mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_EEPROM_0B); delay(100); mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_SHADOW_1B); delay(100);
      break;
    case 1:
      newValue &= 0b1111111111; // For safety, mask off any invalid bits
      reg1B.data.bits.sns_fine = newValue;
      mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_EEPROM_0B); delay(100); mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_SHADOW_1B); delay(100);
      break;
    case 2:
      newValue &= 0b111; // For safety, mask off any invalid bits
      reg1B.data.bits.crs_sns = newValue;
      mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_EEPROM_0B); delay(100); mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_SHADOW_1B); delay(100);
      break;
    case 3:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1B.data.bits.iavgselen = newValue;
      mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_EEPROM_0B); delay(100); mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_SHADOW_1B); delay(100);
      break;
    case 4:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1B.data.bits.pavgselen = newValue;
      mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_EEPROM_0B); delay(100); mySensor.writeRegister(reg1B.data.all, ACS37800_REGISTER_SHADOW_1B); delay(100);
      break;
    case 5:
      newValue &= 0b1111111; // For safety, mask off any invalid bits
      reg1C.data.bits.rms_avg_1 = newValue;
      mySensor.writeRegister(reg1C.data.all, ACS37800_REGISTER_EEPROM_0C); delay(100); mySensor.writeRegister(reg1C.data.all, ACS37800_REGISTER_SHADOW_1C); delay(100);
      break;
    case 6:
      newValue &= 0b1111111111; // For safety, mask off any invalid bits
      reg1C.data.bits.rms_avg_2 = newValue;
      mySensor.writeRegister(reg1C.data.all, ACS37800_REGISTER_EEPROM_0C); delay(100); mySensor.writeRegister(reg1C.data.all, ACS37800_REGISTER_SHADOW_1C); delay(100);
      break;
    case 7:
      newValue &= 0b11111111; // For safety, mask off any invalid bits
      reg1C.data.bits.vchan_offset_code = newValue;
      mySensor.writeRegister(reg1C.data.all, ACS37800_REGISTER_EEPROM_0C); delay(100); mySensor.writeRegister(reg1C.data.all, ACS37800_REGISTER_SHADOW_1C); delay(100);
      break;
    case 8:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1D.data.bits.ichan_del_en = newValue;
      mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_EEPROM_0D); delay(100); mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_SHADOW_1D); delay(100);
      break;
    case 9:
      newValue &= 0b111; // For safety, mask off any invalid bits
      reg1D.data.bits.chan_del_sel = newValue;
      mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_EEPROM_0D); delay(100); mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_SHADOW_1D); delay(100);
      break;
    case 10:
      newValue &= 0b11111111; // For safety, mask off any invalid bits
      reg1D.data.bits.fault = newValue;
      mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_EEPROM_0D); delay(100); mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_SHADOW_1D); delay(100);
      break;
    case 11:
      newValue &= 0b111; // For safety, mask off any invalid bits
      reg1D.data.bits.fltdly = newValue;
      mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_EEPROM_0D); delay(100); mySensor.writeRegister(reg1D.data.all, ACS37800_REGISTER_SHADOW_1D); delay(100);
      break;
    case 12:
      newValue &= 0b111111; // For safety, mask off any invalid bits
      reg1E.data.bits.vevent_cycs = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 13:
      newValue &= 0b111111; // For safety, mask off any invalid bits
      reg1E.data.bits.overvreg = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 14:
      newValue &= 0b111111; // For safety, mask off any invalid bits
      reg1E.data.bits.undervreg = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 15:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1E.data.bits.delaycnt_sel = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 16:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1E.data.bits.halfcycle_en = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 17:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1E.data.bits.squarewave_en = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 18:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1E.data.bits.zerocrosschansel = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 19:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1E.data.bits.zerocrossedgesel = newValue;
      mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_EEPROM_0E); delay(100); mySensor.writeRegister(reg1E.data.all, ACS37800_REGISTER_SHADOW_1E); delay(100);
      break;
    case 20:
      newValue &= 0b1111111; // For safety, mask off any invalid bits
      reg1F.data.bits.i2c_slv_addr = newValue;
      mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_EEPROM_0F); delay(100); mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_SHADOW_1F); delay(100);
      break;
    case 21:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1F.data.bits.i2c_dis_slv_addr = newValue;
      mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_EEPROM_0F); delay(100); mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_SHADOW_1F); delay(100);
      break;
    case 22:
      newValue &= 0b11; // For safety, mask off any invalid bits
      reg1F.data.bits.dio_0_sel = newValue;
      mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_EEPROM_0F); delay(100); mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_SHADOW_1F); delay(100);
      break;
    case 23:
      newValue &= 0b11; // For safety, mask off any invalid bits
      reg1F.data.bits.dio_1_sel = newValue;
      mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_EEPROM_0F); delay(100); mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_SHADOW_1F); delay(100);
      break;
    case 24:
      newValue &= 0b1111111111; // For safety, mask off any invalid bits
      reg1F.data.bits.n = newValue;
      mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_EEPROM_0F); delay(100); mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_SHADOW_1F); delay(100);
      break;
    case 25:
      newValue &= 0b1; // For safety, mask off any invalid bits
      reg1F.data.bits.bypass_n_en = newValue;
      mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_EEPROM_0F); delay(100); mySensor.writeRegister(reg1F.data.all, ACS37800_REGISTER_SHADOW_1F);
      break;
    case 26:
      reg2F = newValue;
      mySensor.writeRegister(reg2F, ACS37800_REGISTER_VOLATILE_2F); delay(100);
      break;
  }
}

void clearSerial()
{
  delay(100); // Wait for any extra data to arrive
  while (Serial.available()) Serial.read(); // Clear the serial buffer
}

uint8_t getChar()
{
  while (Serial.available() == 0) // Wait for a character to arrive
    ; 
  uint8_t character = Serial.read();
  Serial.write(character); // Echo it
  return (character); // Return it
}

void printDecHexBin(uint32_t val, uint8_t width) // Pretty=print the value in decimal, hex and binary
{
  // Decimal. Max width is 10 bits = 1023. So pad to four digits
  if (val < 1000) Serial.print(F(" "));
  if (val < 100) Serial.print(F(" "));
  if (val < 10) Serial.print(F(" "));
  Serial.print(val);
  // Hex. Pad with zeros according to width.
  Serial.print(F(" 0x"));
  if ((width >= 9) && (val < 0x100)) Serial.print(F("0"));
  if ((width >= 5) && (val < 0x10)) Serial.print(F("0"));
  Serial.print(val, HEX);
  if (width < 9)  Serial.print(F(" ")); // Pad with spaces
  if (width < 5)  Serial.print(F(" ")); // Pad with spaces
  // Binary.
  Serial.print(F(" 0b"));
  for (int bitPos = width; bitPos > 0; bitPos--) // For each bit
  {
    if ((val & (1 << (bitPos - 1))) > 0) // Check if bit is set. Print '1' if it is
      Serial.print(F("1"));
    else
      Serial.print(F("0")); // Else print '0'
  }
  Serial.println();
}
