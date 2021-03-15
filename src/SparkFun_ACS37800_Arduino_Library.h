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

#ifndef SparkFun_ACS37800_Arduino_Library_h
#define SparkFun_ACS37800_Arduino_Library_h

#include "Arduino.h"
#include <Wire.h>

// The default I2C Address is 0x60 when DIO_0 and DIO_1 are 0V on start-up
// (There is a typo in the datasheet that suggests it is 0x61. It isn't...!)
// The address can be configured in EEPROM too using setI2Caddress
const uint8_t ACS37800_DEFAULT_I2C_ADDRESS = 0x60;

//Customer Access Code - stored in volatile register 0x2F
const uint32_t ACS37800_CUSTOMER_ACCESS_CODE = 0x4F70656E;

//Default sense resistance for voltage measurement (Ohms)
const int ACS37800_DEFAULT_SENSE_RES = 8200;

//Error result
typedef enum {
  ACS37800_SUCCESS = 0,
  ACS37800_ERR_I2C_ERROR,
  ACS37800_ERR_REGISTER_READ_MODIFY_WRITE_FAILURE
} ACS37800ERR;

//EEPROM Registers
const uint8_t ACS37800_REGISTER_EEPROM_0B = 0x0B;
const uint8_t ACS37800_REGISTER_EEPROM_0C = 0x0C;
const uint8_t ACS37800_REGISTER_EEPROM_0D = 0x0D;
const uint8_t ACS37800_REGISTER_EEPROM_0E = 0x0E;
const uint8_t ACS37800_REGISTER_EEPROM_0F = 0x0F;

//Shadow Registers
//At power up, all shadow registers are loaded from EEPROM, including all configuration parameters.
//The shadow registers can be written to in order to change the device behavior without having to
//perform an EEPROM write. Any changes made in shadow memory are volatile and do not persist through a reset event.
const uint8_t ACS37800_REGISTER_SHADOW_1B = 0x1B;
const uint8_t ACS37800_REGISTER_SHADOW_1C = 0x1C;
const uint8_t ACS37800_REGISTER_SHADOW_1D = 0x1D;
const uint8_t ACS37800_REGISTER_SHADOW_1E = 0x1E;
const uint8_t ACS37800_REGISTER_SHADOW_1F = 0x1F;

//Volatile Registers
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

//EEPROM Registers : Bit Field definitions

typedef struct
{
  union
  {
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

typedef struct
{
  union
  {
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

typedef struct
{
  union
  {
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

typedef struct
{
  union
  {
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

typedef struct
{
  union
  {
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

//Shadow Registers : Bit Field definitions

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t qvo_fine : 9;
      uint32_t sns_fine : 9;
      uint32_t crs_sns : 3;
      uint32_t iavgselen : 1;
      //uint32_t pavgselen : 1; // Not present in shadow register?!
    } bits;
  } data;
} ACS37800_REGISTER_1B_t;

//Volatile Registers : Bit Field definitions

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t vrms : 16;
      uint32_t irms : 16;
    } bits;
  } data;
} ACS37800_REGISTER_20_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t pactive : 16;
      uint32_t pimag : 16;
    } bits;
  } data;
} ACS37800_REGISTER_21_t;

typedef struct
{
  union
  {
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

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t numptsout : 10;
    } bits;
  } data;
} ACS37800_REGISTER_25_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t vrmsavgonesec : 16;
      uint32_t irmsavgonesec : 16;
    } bits;
  } data;
} ACS37800_REGISTER_26_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t vrmsavgonemin : 16;
      uint32_t irmsavgonemin : 16;
    } bits;
  } data;
} ACS37800_REGISTER_27_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t pactavgonesec : 16;
    } bits;
  } data;
} ACS37800_REGISTER_28_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t pactavgonemin : 16;
    } bits;
  } data;
} ACS37800_REGISTER_29_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t vcodes : 16;
      uint32_t icodes : 16;
    } bits;
  } data;
} ACS37800_REGISTER_2A_t;

typedef struct
{
  union
  {
    uint32_t all;
    struct
    {
      uint32_t pinstant : 16;
    } bits;
  } data;
} ACS37800_REGISTER_2C_t;

typedef struct
{
  union
  {
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

//Register Field Enums

typedef enum
{
  ACS37800_CRS_SNS_1X = 0,
  ACS37800_CRS_SNS_2X,
  ACS37800_CRS_SNS_3X,
  ACS37800_CRS_SNS_3POINT5X,
  ACS37800_CRS_SNS_4X,
  ACS37800_CRS_SNS_4POINT5X,
  ACS37800_CRS_SNS_5POINT5X,
  ACS37800_CRS_SNS_8X
} ACS37800_CRS_SNS_e; //Coarse gain for the current channel

typedef enum
{
  ACS37800_FLTDLY_0000 = 0,
  ACS37800_FLTDLY_0475 = 2, //4.75 microseconds
  ACS37800_FLTDLY_0925, //9.25 microseconds
  ACS37800_FLTDLY_1375,
  ACS37800_FLTDLY_1850,
  ACS37800_FLTDLY_2325,
  ACS37800_FLTDLY_2775 //27.75 microseconds
} ACS37800_FLTDLY_e; //Fault Delay

typedef enum
{
  ACS37800_DIO0_FUNC_ZERO_CROSSING = 0,
  ACS37800_DIO0_FUNC_OVERVOLTAGE,
  ACS37800_DIO0_FUNC_UNDERVOLTAGE,
  ACS37800_DIO0_FUNC_OV_OR_UV
} ACS37800_DIO0_FUNC_e; //DIO_0 Function

typedef enum
{
  ACS37800_DIO1_FUNC_OVERCURRENT = 0,
  ACS37800_DIO1_FUNC_UNDERVOLTAGE,
  ACS37800_DIO1_FUNC_OVERVOLTAGE,
  ACS37800_DIO1_FUNC_OV_OR_UV_OR_OCF_LAT
} ACS37800_DIO1_FUNC_e; //DIO_1 Function

typedef enum
{
  ACS37800_EEPROM_ECC_NO_ERROR = 0,
  ACS37800_EEPROM_ECC_ERROR_CORRECTED,
  ACS37800_EEPROM_ECC_ERROR_UNCORRECTABLE,
  ACS37800_EEPROM_ECC_NO_MEANING
} ACS37800_EEPROM_ECC_e; //EEPROM ECC Errors

class ACS37800
{
  // User-accessible "public" interface
  public:

    //Default constructor
    ACS37800();

    //Start I2C communication using specified address and port
    boolean begin(uint8_t address = ACS37800_DEFAULT_I2C_ADDRESS, TwoWire &wirePort = Wire); //If user doesn't specify then Wire will be used

    //Debugging
    void enableDebugging(Stream &debugPort = Serial); //Turn on debug printing. If user doesn't specify then Serial will be used.

    //Basic methods for accessing registers
    ACS37800ERR readRegister(uint32_t *data, uint8_t address);
    ACS37800ERR writeRegister(uint32_t data, uint8_t address);

    //Configurable Settings
    //By default, settings are written to the shadow registers only. Set _eeprom to true to write to EEPROM too.
    ACS37800ERR setCurrentCoarseGain(ACS37800_CRS_SNS_e gain, boolean _eeprom = false);

    //Change the I2C address in EEPROM (i2c_slv_addr)
    //This also sets the i2c_dis_slv_addr flag so the DIO pins will no longer define the I2C address
    ACS37800ERR setI2Caddress(uint8_t newAddress);

    //Basic methods for accessing the volatile registers
    ACS37800ERR readRMS(float *vRMS, float *iRMS); // Read volatile register 0x20. Return the vRMS and iRMS.
    ACS37800ERR readInstantaneous(float *vInst, float *iInst); // Read volatile register 0x2A. Return the vInst and iInst.
    ACS37800ERR readErrorFlags(ACS37800_REGISTER_2D_t *errorFlags); // Read volatile register 0x2D. Return its contents in errorFlags.

    //Change the value of the sense resistor
    void setSensRes(int newRes); // Change the value of _senseResistance (Ohms)

  private:

    //This stores the requested i2c port
    TwoWire * _i2cPort;

    //Debug
    Stream *_debugPort; //The stream to send debug messages to if enabled. Usually Serial.
  	boolean _printDebug = false; //Flag to print debugging variables

    //ACS37800's I2C address
    uint8_t _ACS37800Address = ACS37800_DEFAULT_I2C_ADDRESS;

    //The value of the sense resistor for voltage measurement in Ohms
    int _senseResistance = ACS37800_DEFAULT_SENSE_RES;

};

#endif
