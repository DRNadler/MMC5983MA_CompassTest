// ---------------------------------------------------------------------------------------------------- //
// Robert's Smorgasbord 2023                                                                            //
// https://robertssmorgasbord.net                                                                       //
// https://www.youtube.com/@robertssmorgasbord                                                          //
// MEMSIC MMC5983MA 3-Axis Digital Compass & Arduino MCU – The Details (6) https://youtu.be/HEKQfgcHaXQ //
// ---------------------------------------------------------------------------------------------------- //

#include "MMC5983MA_I2C.h"

// Constructors/Destructors
// ========================

MMC5983MA_I2C::MMC5983MA_I2C(TwoWire* wire)
: wire(wire),
  registers({0})
{    
}

MMC5983MA_I2C::~MMC5983MA_I2C()
{
}

// General Library Methods
// =======================

bool MMC5983MA_I2C::begin()
{ 
   if (!softwareReset()) return false;

   delay(10); // 10ms until software reset is done according to datasheet
   
   if (!readStatus()) return false;
   
   if (!otpDataLoadSuccessful())
   {
      last_error = Error::otp_data_load_failed;

      return false;
   }

   if (!writeConfig()) return false;

   return true;
}

MMC5983MA_I2C::Error MMC5983MA_I2C::lastError()
{
   return last_error;
}

// MMC5983MA Command Methods
// =========================

bool MMC5983MA_I2C::softwareReset()
{
   return writeRegister(control_1_register, registers[control_1_register] | control_1_sw_rst_bit);
}

bool MMC5983MA_I2C::reloadOTPData()
{
   return writeRegister(control_0_register, registers[control_0_register] | control_0_otp_read_bit);
}
   
bool MMC5983MA_I2C::takeMagneticFieldMeasurement()
{
   return writeRegister(control_0_register, registers[control_0_register] | control_0_tm_m_bit);
}

bool MMC5983MA_I2C::takeTemperatureMeasurement()
{
   return writeRegister(control_0_register, registers[control_0_register] | control_0_tm_t_bit);
}

bool MMC5983MA_I2C::setSensorMagnetization()
{
   return writeRegister(control_0_register, registers[control_0_register] | control_0_set_bit);
}

bool MMC5983MA_I2C::resetSensorMagnetization()
{
   return writeRegister(control_0_register, registers[control_0_register] | control_0_reset_bit);
}

bool MMC5983MA_I2C::clearInterruptMagneticField()
{
   return writeRegister(status_register, status_meas_m_done_bit);
}

bool MMC5983MA_I2C::clearInterruptTemperature()
{
   return writeRegister(status_register, status_meas_t_done_bit);
}
   
// Read Methods
// ============

bool MMC5983MA_I2C::readAxesOutput()
{
   return readRegisters(magnetic_output_registers, 7);
}

bool MMC5983MA_I2C::readTemperatureOutput()
{
   return readRegisters(temperature_output_register, 1);
}

bool MMC5983MA_I2C::readStatus()
{
   return readRegisters(status_register, 1);
}

// Write Methods
// ==============

bool MMC5983MA_I2C::writeConfig()
{
   return writeRegisters(control_0_register, 4);
}
 
// Get Methods
// ===========

uint16_t MMC5983MA_I2C::rawAxisOutput16Bit(uint8_t axis)
{
   return (((uint16_t)(registers[magnetic_output_registers + (2 * axis)    ])) << 8) |
          (((uint16_t)(registers[magnetic_output_registers + (2 * axis) + 1]))     ); 
}

uint32_t MMC5983MA_I2C::rawAxisOutput18Bit(uint8_t axis)
{
   return ((uint32_t)rawAxisOutput16Bit(axis) << 2) | 
          (uint32_t)((registers[magnetic_output_registers + 6] >> (2 * (3 - axis))) & 0b00000011);
}

int16_t MMC5983MA_I2C::rawAxisOutput16BitSigned(uint8_t axis)
{
   // Full scale range is +/- 8 Gauss (G) according to datasheet
   // Resolution in 16 bit mode is 0.25mG per LSB according to datasheet
   // Max raw unsigned 16 bit value is 65535 (2^16 - 1); times 0.25mG is 16.38375G?!?
   // LSB is 16G (8G - -8G) / 65535 (2^16 - 1) = 0.24414435034714274814984359502556mG?!?
   // Center (0G) unsigned 16 bit value is 65535 / 2 = 32767.5 (32768 = 2^15)
   // int16_t range is -32768 (-2^15) to +32767 (+2^15-1)

   return (int16_t)rawAxisOutput16Bit(axis) - 32768; // Rounding error 1/2 LSB
}

int32_t MMC5983MA_I2C::rawAxisOutput18BitSigned(uint8_t axis)
{
   // Full scale range is +/- 8 Gauss (G) according to datasheet
   // Resolution in 18 bit mode is 0.0625mG per LSB according to datasheet
   // Max raw unsigned 18 bit value is 262143 (2^18 - 1); times 0.0625mG is 16.3839375G?!?
   // LSB is 16G (8G - -8G) / 262143 (2^18 - 1) = 0.06103538908153183567747374524592mG?!?
   // Center (0G) unsigned 18 bit value is 262143 / 2 = 131071.5 (131072 = 2^17)

   return (int32_t)rawAxisOutput18Bit(axis) - 131072; // Rounding error 1/2 LSB
}

float MMC5983MA_I2C::rawAxisOutput16BitGauss(uint8_t axis)
{
   // Arduino float (32 bit) has 23 bit fraction and 8 bit exponent + 1 bit sign
   
   return ((float)rawAxisOutput16Bit(axis) - 32767.5) * 0.00025; 
}

float MMC5983MA_I2C::rawAxisOutput18BitGauss(uint8_t axis)
{
   // Arduino float (32 bit) has 23 bit fraction and 8 bit exponent + 1 bit sign
   
   return ((float)rawAxisOutput18Bit(axis) - 131071.5) * 0.0000625; 
}

uint8_t MMC5983MA_I2C::rawTemperatureOutput()
{
   return registers[temperature_output_register]; 
}

float MMC5983MA_I2C::temperatureCelsius()
{
   // Range is -75°C (0) to 125°C (255) according to datasheet
   // Offset is -75
   // 1 LSB is 200 (125°C - -75°C) / 255 = 0.784313... (about 0.8°C according to datasheet)
     
   return registers[temperature_output_register] * 200.0 / 255.0 - 75; 
}

bool MMC5983MA_I2C::otpDataLoadSuccessful()
{
   return registers[status_register] & status_otp_rd_done_bit;
}

bool MMC5983MA_I2C::temperatureMeasurementDone()
{
   return registers[status_register] & status_meas_t_done_bit;
}

bool MMC5983MA_I2C::magneticMeasurementDone()
{
   return registers[status_register] & status_meas_m_done_bit;
}

// Set Methods
// ===========

void MMC5983MA_I2C::setBandwidth(Bandwidth bandwidth)
{
   registers[control_1_register] = (registers[control_1_register] & ~control_1_bw_bits) | bandwidth;
}

void MMC5983MA_I2C::setContinuousMeasurement(ContinuousMeasurement continuous_measurement)
{
   registers[control_2_register] = (registers[control_2_register] & ~control_2_cm_bits) | continuous_measurement;
}

void MMC5983MA_I2C::setMagneticField(MagneticField magnetic_field)
{
   registers[control_3_register] = (registers[control_3_register] & ~control_3_st_enx_bits) | magnetic_field;
}

void MMC5983MA_I2C::setInterrupt(Interrupt interrupt)
{
   registers[control_0_register] = (registers[control_0_register] & ~control_0_int_meas_done_en_bit) | interrupt;
}

void MMC5983MA_I2C::setInhibit(Inhibit inhibit)
{
   registers[control_1_register] = (registers[control_1_register] & ~control_1_inhibit_bits) | inhibit;
}

void MMC5983MA_I2C::setAutomaticSet(AutomaticSet automatic_set)
{
   registers[control_0_register] = (registers[control_0_register] & ~control_0_auto_sr_en_bit) | automatic_set;
}

void MMC5983MA_I2C::setPeriodicSet(PeriodicSet periodic_set)
{
   registers[control_2_register] = (registers[control_2_register] & ~control_2_prd_set_bits) | periodic_set;
}

// Private Methods
// ===============

bool MMC5983MA_I2C::readRegisters(uint8_t register_first,  // Address of first register
                                  size_t  register_count ) // Number of registers
{
   uint8_t status;

   wire->beginTransmission(device_address); // Queue slave address with read bit 0 into buffer 
   wire->write(register_first);             // Queue register address into output buffer 

   // Claim bus (START), send data in buffer, but don't release bus (STOP = false)
   // NOTE: If the slave doesn't acknowledge the transmission (pulling SDA low),
   //       the endTransmission() method will hang indefinitely!  
   if ((status = wire->endTransmission(false)) != 0) 
   {
      last_error = (Error)status;
      
      return false;
   }

   // Address the slave with read bit 1, read data into buffer and release the bus (STOP = true)
   if (wire->requestFrom(device_address, register_count, true) != register_count)
   {
      last_error = Error::i2c_request_partial;
      
      return false;
   }

   // Check if there is really the correct number of bytes in the input buffer
   if (wire->available() != register_count)
   {
      last_error = Error::i2c_request_partial;
      
      return false;
   }

   // Read data from input buffer
   if (wire->readBytes(registers + register_first, register_count) != register_count) 
   {
      last_error = Error::i2c_request_partial;

      return false;
   }
   
   return true;
}

bool MMC5983MA_I2C::writeRegister(uint8_t register_address,  // Address of register
                                  uint8_t register_value   ) // Value of register
{
   uint8_t status;

   wire->beginTransmission(device_address);  // Queue slave address with read bit 0 into buffer
   wire->write(register_address);            // Queue register address into buffer
   wire->write(register_value);              // Queue register value into output buffer 

   // Claim bus (START), send data in buffer and release bus (STOP = true)
   // NOTE: If the slave doesn't acknowledge the transmission (pulling SDA low),
   //       the endTransmission() method will hang indefinitely!  
   if ((status = wire->endTransmission(true)) != 0) 
   {
      last_error = (Error)status;

      return false;
   }
   
   return true;  
}

bool MMC5983MA_I2C::writeRegister(uint8_t register_address) // Address of register
{
   return writeRegister(register_address, registers[register_address]);
}

bool MMC5983MA_I2C::writeRegisters(uint8_t register_first,  // Address of first register
                                   size_t  register_count ) // Number of registers
{
   uint8_t register_address;

   for (register_address = register_first; 
        register_address < register_first + register_count;
        register_address++                                 )
   {
      if (!writeRegister(register_address)) return false;
   }
   
   return true;  
}
