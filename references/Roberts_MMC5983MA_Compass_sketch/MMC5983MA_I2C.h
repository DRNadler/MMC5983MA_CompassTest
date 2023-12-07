// ---------------------------------------------------------------------------------------------------- //
// Robert's Smorgasbord 2023                                                                            //
// https://robertssmorgasbord.net                                                                       //
// https://www.youtube.com/@robertssmorgasbord                                                          //
// MEMSIC MMC5983MA 3-Axis Digital Compass & Arduino MCU – The Details (6) https://youtu.be/HEKQfgcHaXQ //
// ---------------------------------------------------------------------------------------------------- //

#ifndef MMC5983MA_I2C_H
#define MMC5983MA_I2C_H

#include <Arduino.h>
#include <Wire.h>

class MMC5983MA_I2C
{
   public:

   // Internal Control 1 Bandwidth [BW] bits
   // --------------------------------------
   // 
   typedef enum Bandwidth : uint8_t
   {
      bw_100Hz = 0b00000000, // Default
      bw_200Hz = 0b00000001,
      bw_400Hz = 0b00000010,
      bw_800Hz = 0b00000011
   };

   // Automatic Set (of AMR sensors' magnetization)
   // ---------------------------------------------
   // The AMR sensors' magnetization is automatically restored to the default
   // direction before a single magnetic field measurement is commanded.
   // Use periodic set to achive the same in continous measurement mode. 
   // See MMC5983MA datasheet Internal Control 0 register AUTO_SR_EN bit.
   typedef enum AutomaticSet : uint8_t
   {
      as_disabled = 0b00000000, // Default
      as_enabled  = 0b00100000
   };

   // Periodic Set (of AMR sensors' magnetization) every Nth measurement
   // ------------------------------------------------------------------
   // The AMR sensors' magnetization is restored to the default direction
   // every Nth measurement in continuous measurement mode.
   // Automatic set and continuous measurement have to be enabled! 
   // See MMC5983MA datasheet Internal Control 2 register EN_PRD_SET and PRD_SET[2:0] bits.

   typedef enum PeriodicSet : uint8_t
   {
      ps_disabled = 0b00000000, // Default
      ps_1        = 0b10000000,
      ps_25       = 0b10010000,
      ps_75       = 0b10100000,
      ps_100      = 0b10110000,
      ps_250      = 0b11000000,
      ps_500      = 0b11010000,
      ps_1000     = 0b11100000, 
      ps_2000     = 0b11110000  
   };

   // Continuous Measurement
   // ----------------------
   // In continuous measurement mode magnetic field measurements
   // will be taken continuosly at a specified frequency.
   // See MMC5983MA datasheet Internal Control 2 register CMM_EN and CM_FREQ[2:0] bits. 
   typedef enum ContinuousMeasurement : uint8_t
   {
      cm_disabled = 0b00000000, // Default
      cm_1Hz      = 0b00001001,
      cm_10Hz     = 0b00001010,
      cm_20Hz     = 0b00001011,
      cm_50Hz     = 0b00001100,
      cm_100Hz    = 0b00001101,
      cm_0200Hz   = 0b00001110, // Bandwidth >= 200Hz only
      cm_1000Hz   = 0b00001111  // Bandwidth =  800Hz only
   };

   // Measurement Done Interrupt [INT_MEAS_DONE_EN] bits
   // --------------------------------------------------
   typedef enum Interrupt : uint8_t
   {
      int_disabled = 0b00000000, // Default
      int_enabled  = 0b00000100
   };

   // Disable X, Y and Z channel [X_INHIBIT] and [YZ_INHIBIT] bits
   // ------------------------------------------------------------
   typedef enum Inhibit : uint8_t
   {
      no_channel = 0b00000000, // Default
      x_channel  = 0b00000100,
      yz_channel = 0b00011000,
   };

   // Enable positive ([P]lus) / negative ([M]inus) current through a coil [ST_ENP]/[ST_ENM] bits
   // --------------------------------------------------------------------------------------------
   // This will result in a positive [ST_ENM] / negative [ST_ENP] magentic field.
   // X-Axis: +/-1000mG
   // Y-Axis: +/- 300mG
   // Z-Axis: +/- 200mG
   typedef enum MagneticField : uint8_t
   {
      mf_off      = 0b00000000, // Default
      mf_negative = 0b00000010,
      mf_positive = 0b00000100,
   };
      
   typedef enum Error : uint8_t // Type of error, returned by lastError().
   {
      i2c_buffer_overflow  = 01, // Data too long to fit in transmit buffer
      i2c_address_nack     = 02, // Received NACK on transmit of address
      i2c_data_nack        = 03, // Received NACK on transmit of data
      i2c_other            = 04, // Other error
      i2c_request_partial  = 05, // Received less than expected bytes on request
      otp_data_load_failed = 06  // Loading OTP memory into registers failed
   };

   // Constructors/Destructors
   // ========================
   
   // Constructor
   // -----------
   // TwoWire* wire: TwoWire object to be used for communication,
   //                wire.begin() has to be called before using the QMC5883L object.    
   MMC5983MA_I2C(TwoWire* wire);
   MMC5983MA_I2C() = delete;     // Default constructor is not supported     
            
   // Destructor
   // ----------
   ~MMC5983MA_I2C();

   // General Library Methods
   // =======================
   
   // begin()
   // -------
   // Call this method before using a MMC5983MA_I2C object instance. This method will bring the MMC5983MA
   // into a defined state (softwareReset()) and check if it is operational (otpDataLoadSuccessful()).  
   // Return true if successful, otherwise false (use lastError() to retrive fault condition).
   bool begin();

   // lastError()
   // -----------
   // Returns the last encountered error.
   Error lastError();
   
   // MMC5983MA Command Methods
   // =========================
   // Causing the MMC5983MA to perform an action by writing 1 to a register "write only" bit.
   // Those "write only" bits will automatically revert to 0 after the action has been performed.
   // These commands are all non-blocking. 
   // The methods return true if successful, otherwise false (use lastError() to retrive fault condition). 

   // softwareReset()
   // ---------------
   // Causes the MMC5983MA to do a software reset, which includes loading the OTP (one time programmable)
   // memory into the registers (all OTP memory / register bits are by default 0).
   bool softwareReset();

   // reloadOTPData()
   // ---------------
   // Causes the MMC5983MA to load the OTP (one time programmable) memory into the registers (all OTP
   // memory / register bits are by default 0).
   bool reloadOTPData();
     
   // takeMagneticFieldMeasurement()
   // ------------------------------
   // Causes the MMC5983MA to take a single magnetic field measurement.
   // This can take between 0.5ms and 8ms, depending on the bandwidth selected.
   bool takeMagneticFieldMeasurement();

   // takeTemperatureMeasurement()
   // ----------------------------
   // Causes the MMC5983MA to take a single temperature measurement.
   bool takeTemperatureMeasurement();

   // setSensorMagnetization(), resetSensorMagnetization()
   // ----------------------------------------------------
   // Causes the MMC5983MA to send a large current through coils, magnetizing the anisotropic
   // magentoresitive (AMR) sensors in the default (set) or a reverse (reset) orientation.
   bool setSensorMagnetization();
   bool resetSensorMagnetization();

   // clearInterruptMagneticField(), clearInterruptTemperature()
   // ----------------------------------------------------------
   // Causes the MMC5983MA to clear the measurement done interrupt for magentic field, respectively,
   // temperatur.
   bool clearInterruptMagneticField();
   bool clearInterruptTemperature();

   // Read Methods
   // ============
   // Reading data from the MMC5983MA registers into the object's registers[] array.
   // All methods return true if successful, otherwise false (use lastError() to retrive fault condition).
   
   // readAxesOutput()
   // ----------------
   // Reads the axes output registers.
   bool readAxesOutput();

   // readTemperatureOutput()
   // -----------------------
   // Reads the temperature output register.
   bool readTemperatureOutput();

   // readStatus()
   // ------------
   // Reads the status register.
   bool readStatus();

   // Write Methods
   // =============
   // Writing data from the the object's registers[] array into MMC5983MA registers.
   // All methods return true if successful, otherwise false (use lastError() to retrive fault condition).

   // writeConfig()
   // -------------
   // Writes all configuration data to the internal control registers.
   bool writeConfig();

   // Get Methods
   // ============
   // Getting data from the object's registers[] array.

   // rawAxisOutputXXBit()
   // --------------------
   // Get 16 bit or 18 bit axis output from last readAxesOutput() call, if it was successful (returned true).
   // If the last readAxesOutput() call failed (returned false), this data might be corrupted. 
   uint16_t rawAxisOutput16Bit(uint8_t axis); // axis = 0 (X) or 1 (Y) or 2 (Z)
   uint32_t rawAxisOutput18Bit(uint8_t axis); // axis = 0 (X) or 1 (Y) or 2 (Z)

   // rawAxisOutputXXBitSigned(), rawAxisOutputXXBitGauss()
   // -----------------------------------------------------
   // Get axis output as signed 16 or 18 bit or in Gauss from last succesful (returned true) readAxesOutput() call.
   // If the last readAxesOutput() call failed (returned false), this data might be corrupted. 
   int16_t rawAxisOutput16BitSigned(uint8_t axis); // axis = 0 (X) or 1 (Y) or 2 (Z)
   int32_t rawAxisOutput18BitSigned(uint8_t axis); // axis = 0 (X) or 1 (Y) or 2 (Z)
   float rawAxisOutput16BitGauss(uint8_t axis); // axis = 0 (X) or 1 (Y) or 2 (Z)
   float rawAxisOutput18BitGauss(uint8_t axis); // axis = 0 (X) or 1 (Y) or 2 (Z)

   // rawTemperatureOutput(), temperatureCelsius()
   // --------------------------------------------
   // Get 8 bit (raw) or float (°C) temperature output from last readtemperatureOutput() call, if it was
   // successful (returned true). If the last readTemperatureOutput() call failed (returned false),
   // this data might be corrupted. 
   uint8_t rawTemperatureOutput();
   float   temperatureCelsius();

   // otpDataLoadSuccessful()
   // -----------------------
   // Returns true if OTP data (PROM) has been successfully read into registers, otherwise false.
   // Retrieved by last readStatus() call, if it was successful (returned true).
   // If the last readStatus() call failed (returned false), this data might be corrupted. 
   bool otpDataLoadSuccessful();

   // temperatureMeasurementDone()
   // ---------------------------
   // Returns true if temperature measurement is done and data is available, otherwise false.
   // Retrieved by last readStatus() call, if it was successful (returned true).
   // If the last readStatus() call failed (returned false), this data might be corrupted. 
   bool temperatureMeasurementDone();

   // magneticMeasurementDone()
   // -------------------------
   // Returns true if magnetic measurement is done and data is available, otherwise false.
   // Retrieved by last readStatus() call, if it was successful (returned true).
   // If the last readStatus() call failed (returned false), this data might be corrupted. 
   bool magneticMeasurementDone();
   
   // Set Methods
   // ===========
   // Setting configuration data in the object's registers[] array.
   // Use writeConfig() to write all configuration data to the MMC5983MA internal control 0 to 3 registers.

   // setBandwidth()
   // --------------
   // Sets the bandwidth configuration data. Use writeConfig() to write all configuration data.
   void setBandwidth(Bandwidth bandwidth);

   // setContinuousMeasurement()
   // --------------------------
   // Sets the continuous measurement mode configuration data. Use writeConfig() to write all configuration data. 
   void setContinuousMeasurement(ContinuousMeasurement continuous_measurement);

   // setMagneticField()
   // ------------------
   // Sets configuration data for current through coils creating a magnetic field.
   void setMagneticField(MagneticField magentic_field);

   // setInterrupt()
   // --------------
   // Enables/disables the measurement done interrupt (pin 15 INT, active high).
   void setInterrupt(Interrupt interrupt);

   // setInhibit()
   // ------------
   // Inhibits (disables) the X, Y and/or Z channel.
   void setInhibit(Inhibit inhibit);

   // setAutomaticSet()
   // -----------------
   // Enables/disables the automatic set feature.
   void setAutomaticSet(AutomaticSet automatic_set);

   // setPeriodicSet()
   // ----------------
   // Sets periodic set to be carried out every Nth measurement or disables it.
   void setPeriodicSet(PeriodicSet periodic_set);

   private:

   // I2C address
   static constexpr uint8_t device_address = 0b0110000; // I2C address of MMC5983MA

   // Register addresses
   static constexpr uint8_t magnetic_output_registers   = 0x00; // First magnetic output register
   static constexpr uint8_t temperature_output_register = 0x07; // Temperature outpu register
   static constexpr uint8_t status_register             = 0x08; // Status register
   static constexpr uint8_t control_0_register          = 0x09; // Internal control 0 register
   static constexpr uint8_t control_1_register          = 0x0A; // Internal control 1 register
   static constexpr uint8_t control_2_register          = 0x0B; // Internal control 2 register
   static constexpr uint8_t control_3_register          = 0x0C; // Internal control 3 register

   // Register bits                                            76543210
   static constexpr uint8_t status_meas_m_done_bit         = 0b00000001; // Magnetic measurement done bit
   static constexpr uint8_t status_meas_t_done_bit         = 0b00000010; // Temperature measurement done bit   
   static constexpr uint8_t status_otp_rd_done_bit         = 0b00010000; // OTP data read successfull bit   
   static constexpr uint8_t control_0_tm_m_bit             = 0b00000001; // Take magnetic measurement bit
   static constexpr uint8_t control_0_tm_t_bit             = 0b00000010; // Take temperature measurement bit
   static constexpr uint8_t control_0_int_meas_done_en_bit = 0b00000100; // Enable measurement done interrupt 
   static constexpr uint8_t control_0_set_bit              = 0b00001000; // Set sensor magnetization bit
   static constexpr uint8_t control_0_reset_bit            = 0b00010000; // Reset sensor magnetization bit
   static constexpr uint8_t control_0_auto_sr_en_bit       = 0b00100000; // Enable automatic set/reset feature
   static constexpr uint8_t control_0_otp_read_bit         = 0b01000000; // Reload OTP memory bit
   static constexpr uint8_t control_1_bw_bits              = 0b00000011; // Bandwidth selection bits
   static constexpr uint8_t control_1_inhibit_bits         = 0b00011100; // Inhibit channel bits
   static constexpr uint8_t control_1_sw_rst_bit           = 0b10000000; // Software reset bit
   static constexpr uint8_t control_2_cm_bits              = 0b00001111; // Continuous measurement mode bits
   static constexpr uint8_t control_2_prd_set_bits         = 0b11110000; // Periodic set bits
   static constexpr uint8_t control_3_st_enx_bits          = 0b00000110; // Enable positive/negative field bits
    
   // Private variables
   TwoWire*     wire;                              // I2C object  
   uint8_t      registers[control_3_register + 1]; // Copies of all MMC5983MA registers
   Error        last_error;                        // Holds the last error encountered

   // Read registers of MMC5983MA and store the data in register[]
   // Write data in register[] to registers of MMC5983MA
   bool readRegisters(uint8_t register_first,     // Address of first register
                      size_t  register_count );   // Number of registers
   bool writeRegister(uint8_t register_address,   // Address of register
                      uint8_t register_value   ); // Value of register
   bool writeRegister(uint8_t register_address);  // Address of register
   bool writeRegisters(uint8_t register_first,    // Address of first register
                       size_t  register_count );  // Number of registers
};

#endif
