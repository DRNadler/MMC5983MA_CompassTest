/// MMC5983MA.hpp - MMC5983MA_C class - driver for MEMSIC MMC5983MA 3-axis magnetometer.

#ifndef MMC5983A_HPP_INCLUDED
#define MMC5983A_HPP_INCLUDED

/*
MIT License

Copyright (c) 2023-2025 Dave Nadler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// ToDo MMC5983MA: Fix haphazard return values from API, maybe factor out measurement result class.
// ToDo MMC5983MA: Implement continuous mode? Currently 8ms delay per measurement (17ms using AutoSR).
// #define MMC5983MA_CONTINUOUS_MODE // default as coded is one-shot

// Register-IO trace is enabled with the following #define:
// #define MMC5983MA_PRINT_DETAILED_LOG

// See Ten(!) YouTube videos about this part by Robert.Drees@gmx.net here:
// https://www.youtube.com/@robertssmorgasbord/search?query=mmc


/*
Datasheet Questions (MEMSIC MMC5983MA Rev A -- Formal release date: 4/3/2019)
Above is the latest release on MEMSIC.com as of 11-Jan-2025.
The datasheet is seriously unclear on many points...
The following questions are still outstanding with MEMSIC:

1) Continuous mode vs. one-shot mode aren't well documented.
   a) Is it necessary to write TM_M to start measurements in continuous mode?
   b) In continuous mode, is Meas_M_Done ever set?
   c) Are TM_M and Meas_T_Done only for one-shot measurements?
   d) Can MMC5983MA measure temperature in continuous mode?
      ANSWERED SORT-OF: Temperature sensor does not really work, don't use it.

2) Can SET and RESET used while continuous mode is enabled?

3) The use and behavior of auto-SET-RESET (AutoSR) is NOT DOCUMENTED:
   a) What is the PURPOSE of auto-SET-RESET?
      Not what does it do! What problem does this solve?
   b) What exactly does MMC5983MA do if Auto_SR_en bit is set?
   c) Does Auto_SR_en work in both continuous and one-shot mode?
   d) What is the 0-field output value when using AutoSR?
   e) How long do AutoSR measurements take?

   f) MEMSIC sample code says "continuous mode with auto set and reset" - is that correct?
      The datasheet page 16 says "The device will perform a SET automatically...";
      should that be "performs a periodic RESET/SET sequence"?
      Which is the correct explanation of what the chip does?
      The order of SET and RESET determines the sensing polarity, so this needs to be clear, right?

6) The decimation filter is mentioned but not described.
   Given BW and CM settings, What *exactly* is the sampling and filter?

*/

/*
======================  Things MISSING from MEMSIC datasheet  ======================

Measurement offsets vary greatly with temperature, so a SET/RESET sequence
to remove the offset is a good idea (either explicit or using Auto-Set-Reset).
AutoSR is undocumented in the datasheet but recommended by MEMSIC tech support.
AutoSR internally does a sequence as follows:
   SET, Sample1, RESET, Sample2, Output=0x20000+(Sample1-Sample2)/2.
The observed delay until results are available is twice the datasheet-specified
measurement time plus an additional 1msec for SET and RESET.
AutoSR is available for commanded measurements or in continuous mode.

RESET/SET procedure:
- SET/RESET does not reverse polarity of YZ (only X works) when using SPI: bug in MMC5983MA!
  See: https://electronics.stackexchange.com/questions/736609/magnetometer-memsic-mmc5983ma-set-reset-only-works-on-x-channel-when-using-spi
- Any rotation between the two measurements (between SET and RESET measurements) creates an error!
  Sensor must be relatively still.
- Code must wait 500uSec after RESET or SET before making a reading (AutoSR waits internally)

Saturation Detection:
  An internal coil can add or subtract from the ambient field (enabled via [ST_ENP]/[ST_ENM] bits).
  If the sensor is saturated (field > ~8G), one of adding or subtracting this additional field will always cause no output change.
  The amount added/subtract varies by axis (measured by Robert):
     X-Axis: +/-1000mG
     Y-Axis: +/- 300mG
     Z-Axis: +/- 200mG

Chip power-on or reset does a 'SET', so chip always comes up with normal polarization.


*/


#include <stdint.h>
#include <stddef.h> // size_t
#include <assert.h>
#include <memory.h> // memcpy

/// Driver for MMC5983MA 3-axis magnetometer sensor <BR>
/// See MMC5983MA_IO.hpp for example TDEVICE class (provides platform-specific IO)
template <typename TDEVICE>
class MMC5983MA_C {
    // ==============================  Definitions  ==============================
  public:
    /// Bandwidth selection adjusts the length of the decimation filter,
    /// and controls the measurement duration.
    /// Note: X/Y/Z channel measurements are taken in parallel.
    enum class Bandwidth_T : uint8_t { // These delays assume auto-SR is not in use
        Bandwidth_00_100Hz = 0x00, // 8msec - default
        Bandwidth_01_200Hz = 0x01, // 4msec
        Bandwidth_10_400Hz = 0x02, // 2msec
        Bandwidth_11_800Hz = 0x03, // .5msec
    };

  protected:
    // Register addresses within MMC5983MA
    enum class Register : uint8_t {
        X_out_0    = 0x00, ///< Xout [17:10]         read-only
        X_out_1    = 0x01, ///< Xout [9:2]           read-only
        Y_out_0    = 0x02, ///< Yout [17:10]         read-only
        Y_out_1    = 0x03, ///< Yout [9:2]           read-only
        Z_out_0    = 0x04, ///< Zout [17:10]         read-only
        Z_out_1    = 0x05, ///< Zout [9:2]           read-only
        XYZ_out_2  = 0x06, ///< Xout[1:0],Yout[1:0],Zout[1:0] (for 18-bit mode) read-only
        T_out      = 0x07, ///< Temperature output   read-only
        Status     = 0x08, ///< Device status        read-write (write to reset interrupt flags)
        // To prevent accidents, control registers are in separate type below
        //Control_0  = 0x09, // Control register 0   write-only
        //Control_1  = 0x0a, // Control register 1   write-only
        //Control_2  = 0x0b, // Control register 2   write-only
        //Control_3  = 0x0c, // Control register 3   write-only
        Product_ID = 0x2f  // Product ID
    };
    enum class ControlRegister : uint8_t {
        Control_0  = 0x09, ///< Control register 0   write-only
        Control_1  = 0x0a, ///< Control register 1   write-only
        Control_2  = 0x0b, ///< Control register 2   write-only
        Control_3  = 0x0c, ///< Control register 3   write-only
    };
    const char* RegisterName(Register reg) const {
        if(reg==Register::Product_ID/*0x2f*/) return "Product ID";
        int regIdx = (int)reg;
        static const char* const names[] {
            /* 0x00 */ "Xout [17:10]",
            /* 0x01 */ "Xout [9:2]",
            /* 0x02 */ "Yout [17:10]",
            /* 0x03 */ "Yout [9:2]",
            /* 0x04 */ "Zout [17:10]",
            /* 0x05 */ "Zout [9:2]",
            /* 0x06 */ "Xout[1:0],Yout[1:0],Zout[1:0]",
            /* 0x07 */ "Temperature",
            /* 0x08 */ "Device status",
            /* 0x09 */ "Control register 0",
            /* 0x0a */ "Control register 1",
            /* 0x0b */ "Control register 2",
            /* 0x0c */ "Control register 3",
        };
        if(regIdx<=0x0c) return names[regIdx];
        return "Invalid Register";
    };
    enum class StatusMask : uint8_t {
        Meas_M_Done   = 0x01, ///< A magnetic measurement completed (check before reading output).
                              ///< A new measurement command resets to 0.
                              ///< Set 1 when measurement is finished, and remains 1 till next measurement.
                              ///< Writing 1 into this bit will clear the corresponding interrupt.
        Meas_T_Done   = 0x02, ///< A temperature measurement completed.
                              ///< A new measurement command resets to 0.
                              ///< Set 1 when measurement is finished, and remains 1 till next measurement.
                              ///< Writing 1 into this bit will clear the corresponding interrupt.
        OTP_read_done = 0x10, ///< The chip successfully read its OTP memory; should always be set.
    };
    enum class Control_0_Mask : uint8_t {
        Action_TM_M = 0x01, ///< Take magnetic field measurement (set 1 to initiate measurement).
                            ///< Automatically reset to 0 when the measurement is complete (but this register is write-only).
        Action_TM_T = 0x02, ///< Take Temperature measurement (set 1 to initiate measurement).
                            ///< Automatically reset to 0 when the measurement is complete (but this register is write-only).
                            ///< This bit and TM_M cannot be high at the same time.
        Setting_Enable_MeasurementDoneINT = 0x04, ///< Set 1 to enable the completed measurement interrupt.
                            ///< When a magnetic or temperature measurement finishes, an
                            ///< interrupt will be signaled.
        Action_SET = 0x08,  ///< Writing 1 starts the SET operation, which sends a large current
                            ///< through the sensor coils for 500ns.
                            ///< Automatically reset to 0 at the end of the SET operation (but this register is write-only).
        Action_REVERSE_SET = 0x10,///< Writing 1 starts the REVERSE_SET operation (MEMSIC calls this RESET),
                            ///< which sends a large reverse current (in the opposite direction of SET)
                            ///< through the sensor coils for 500ns.
                            ///< Automatically reset to 0 at the end of the REVERSE_SET operation (but this register is write-only).
        Setting_Auto_SR_en = 0x20,///< Set 1 to enable automatic SET/REVERSE_SET.                   <BR>
                            ///< Function/Purpose not documented in datasheet; does:                <BR>
                            ///<   SET, Sample1, RESET, Sample2, Output=0x2000+(Sample1-Sample2)/2. <BR>
                            ///< Works in either:
                            ///< - commanded measurement mode, or
                            ///< - continuous mode/Setting_ContinuousModeEnable and periodic set/Setting_AutoSETenable
        Action_OTP_Read = 0x40,///< Writing 1 commands the device to read the OTP data again.
                            ///< Automatically reset to 0 after the shadow registers for OTP are refreshed (but this register is write-only).
                            ///< MEMSIC support: Should NEVER be required.
    };
    const static uint8_t Product_ID_Assigned = 0x30; // MMC5983MA product ID value
    enum class Control_1_Mask : uint8_t {
        Setting_Bandwidth = 0x03, ///< see Bandwidth_T enum below
        Setting_X_inhibit = 0x04, ///< Writing 1 disables X channel.
        Setting_YZ_inhibit= 0x18, ///< Writing 1 to BOTH bits disables Y and Z channels (individual disable not supported).
        Action_SW_RST     = 0x80, ///< Writing 1 commands the part to reset, similar to power-up.
                                  ///< Clears all registers and also rereads OTP.
                                  ///< PowerOn/reset time is 10mS.
    };
    enum class Control_2_Mask : uint8_t {
        Setting_ContinuousModeRate = 0x07,  ///< 0 is Off. See table in datasheet for nominal values.
        Setting_ContinuousModeEnable = 0x80, ///< 1 to enable, rate above must not be 0 if enabled.
        Setting_AutoSETrate = 0x70, ///< Controls number of measurements between automatic SET if enabled
        Setting_AutoSETenable = 0x80,  ///< 1 enables automatic periodic SET
    };
    enum class Control_3_Mask : uint8_t {
        Setting_SaturationTestEnablePlus = 0x02, // Turn on supplemental coil + for saturation-detection
        Setting_SaturationTestEnableMinus = 0x04,// Turn on supplemental coil - for saturation-detection
        Setting_SPI_3wire = 0x40,
    };

    // ==============================    Members    ==============================
  public:
    MMC5983MA_C() : initialized(false) {};

    /// Initialize the sensor. Call this before using any other API.
    /// On success, sets 'initialized' member true.
    int8_t Init();

    // State and measurement information
    bool initialized; ///< false until Init() is called and succeeds

    /// Read the magnetic field, including a Reset/Set operation
    /// to compute offset. Place results in field and offset members.
    int8_t Measure_XYZ_Field_WithResetSet();

    /// Read the magnetic field using poorly-documented Auto-Set-Reset feature.
    int8_t Measure_XYZ_Field_WithAutoSR();

    int32_t field[3] = {0}; ///< Last magnetic field reading set (X,Y,Z), signed values already adjusted with offsets.
    const static int32_t CountsPerGauss = 16384; // 2^17 / 8G full-scale when using full 18-bit resolution as we do here.

    /// Each sensor's offset is the unsigned integer value the sensor will read
    /// with zero external field. Should be about mid-range for 18-bit value,
    /// ie 2^17 = 0x20000 = 131072.
    uint32_t offset[3] = {0};  ///< Last measured offset (X,Y,Z) - always included in field above.

  protected:

    TDEVICE dev; ///< platform-specific hardware IO instance

    typedef uint8_t uint8_array_t[]; ///< assist internal type conversions
    inline uint8_array_t &GetArrayRefFromSingle(uint8_t &s) {
        return  *reinterpret_cast<uint8_array_t*>(&s);
    };
    inline const uint8_array_t &GetConstArrayRefFromSingle(const uint8_t &s) {
        return  *reinterpret_cast<const uint8_array_t*>(&s);
    };

    // The control registers contain two different flavors of bits:
    // 1) "Action" Write-a-1 to initiate a process, and
    // 2) "Settings" such as bandwidth.
    // The control registers are WRITE-ONLY, so we have to track all settings,
    // because we must re-write some settings when for example starting a measurement.
    // This is a HORRIBLE design. The designer should have:
    // a) Segregated registers containing "Settings" vs. "Actions", and/or
    // b) Made the registers readable (action-only registers would not need reading).
    // Its like MEMSIC deliberately tried to make this part hard to support...

    /// Saved control settings of all 4 control registers ("action" bits will never be set here)
    uint8_t control_settings[4] = {0}; // Reset value of all 4 control registers is 0
    /// Get a reference to the control setting copy for the given control register
    uint8_t inline &GetSettingRef(ControlRegister controlReg) {
        int controlRegIdx = (int)controlReg - (int)ControlRegister::Control_0; // 0-3
        assert(controlRegIdx>=0 && (size_t)controlRegIdx<sizeof(control_settings));
        // Return reference to local copy of the control register to access settings
        return control_settings[controlRegIdx];
    }
    /// Update a setting (or settings if OR'd together) in one control register.
    /// Only writes if the setting changed
    void WriteControlSetting(ControlRegister controlReg, uint8_t settingMask, uint8_t settingValue) {
        uint8_t &setting = GetSettingRef(controlReg); // the control register's shadow...
        // Is the setting already in place?
        if( (setting&settingMask) == settingValue )
            return; // nothing to do, already set as requested.
        // Update in-memory copy of the control register settings per arguments
        setting &= ~settingMask; // mask out prior setting(s), and
        setting |= settingValue; // OR in new setting(s)
        // Write updated in-memory value to the sensor (any "Action" bits will be 0)
        set_reg((Register)controlReg, setting);
    }
    /// Command an action via a control register
    void WriteControlAction(ControlRegister controlReg, uint8_t actionMask) {
        // This control register may contain settings, so OR in saved settings
        uint8_t &setting = GetSettingRef(controlReg);
        // Write value to the sensor, presumably just one Action bit in the mask
        set_reg((Register)controlReg, actionMask | setting);
    }
    void SetBandwidth(Bandwidth_T bw) {
        WriteControlSetting(ControlRegister::Control_1, (uint8_t)Control_1_Mask::Setting_Bandwidth, (uint8_t)bw); // bw is low-order 2 bits
    }
    Bandwidth_T GetBandwidth(void) const {
        return (Bandwidth_T)(control_settings[1] & (int)Control_1_Mask::Setting_Bandwidth); // bw is low-order 2 bits
    };
    bool InAutoSRmode() const {
        return (control_settings[0] & (uint8_t)Control_0_Mask::Setting_Auto_SR_en) != 0;
    }
    int uSecPerMeasurement(void) const {
        // For current mode, how much time does a measurement take?
        static const int usecGivenBandwidth[4] = { 8000, 4000, 2000, 500 }; // times for a single measurement
        int usec = usecGivenBandwidth[(int)GetBandwidth()];
        if(InAutoSRmode()) {
            usec = usec*2 +1; // AutoSR mode makes two measurements, and needs additional 1msec for SET-RESET
        }
        return usec;
    };

    /// Set Continuous mode (0 off, 1-7 per datasheet)
    void SetContinuousMode(uint8_t cm) {
        // continuous rate setting is in low-order 3 bits; no shifting required
        if(cm) cm |= (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable;
        WriteControlSetting(ControlRegister::Control_2,
            (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable | (uint8_t)Control_2_Mask::Setting_ContinuousModeRate, cm);
    }
    bool InContinuousMode() const {
        return (control_settings[2] & (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable)!=0;
    }

    /// Read one or more sequential registers
    int8_t get_regs(Register reg, uint8_t (&data)[], uint32_t len);
    /// Write one or more sequential registers
    int8_t set_regs(Register reg, const uint8_t (&data)[], uint32_t len);

    /// Read a single register
    inline int8_t get_reg(Register reg, uint8_t &singleByte) {
        return get_regs(reg, GetArrayRefFromSingle(singleByte), 1);
    }
    /// Write a single register
    int8_t set_reg(Register reg, const uint8_t &singleByte) {
        return set_regs(reg, GetConstArrayRefFromSingle(singleByte), 1);
    }

    /// RESET or SET generate a 500ns pulse to magnetize ANR film,
    /// after which code must wait 500uSec before making a reading.
    /// 500uSec delay from MEMSIC tech support and sample code (not in datasheet).
    static const uint32_t RequiredWaitAfterMagnetizePulse_uSec = 500; // per MEMSIC
    /// Perform SET including required wait.
    inline void SET(void)
    {
        assert(!InContinuousMode());
        WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_SET);
        dev.delay_us(RequiredWaitAfterMagnetizePulse_uSec);
    }
    /// Perform RESET including required wait.
    inline void RESET(void)
    {
        assert(!InContinuousMode());
        WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_REVERSE_SET);
        dev.delay_us(RequiredWaitAfterMagnetizePulse_uSec);
    }

    /// Is measurement complete?
    inline bool MeasurementIsComplete() {
        uint8_t status;
        get_reg(Register::Status,status);
        bool complete = (status & (uint8_t)StatusMask::Meas_M_Done) != 0;
        return complete;
    }

    /// Fetch the XYZ results (3 raw unsigned 18-bit values)
    inline void Fetch_XYZ(uint32_t (&result)[3]) {
        uint8_t rawBytes[7];
        get_regs(Register::X_out_0, rawBytes, sizeof(rawBytes)); // 7 sequential field measurement bytes
        result[0] =  ((uint32_t)rawBytes[0] << 10) |
                     ((uint32_t)rawBytes[1] <<  2) |
                    (((uint32_t)rawBytes[6] & 0xC0u) >> 6) ;
        result[1] =  ((uint32_t)rawBytes[2] << 10) |
                     ((uint32_t)rawBytes[3] <<  2) |
                    (((uint32_t)rawBytes[6] & 0x30u) >> 4) ;
        result[2] =  ((uint32_t)rawBytes[4] << 10) |
                     ((uint32_t)rawBytes[5] <<  2) |
                    (((uint32_t)rawBytes[6] & 0x0Cu) >> 2) ;
    }

    /// Make one measurement, then read XYZ results (returns 3 unsigned 18-bit quantities)
    inline void MeasureOneTime(uint32_t (&result)[3])
    {
        #ifndef MMC5983MA_CONTINUOUS_MODE
            assert(!InContinuousMode());
            // Initiate Magnetic Measurement
            WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_TM_M);
            // Wait for measurement complete
            int usec = uSecPerMeasurement(); // 8msec for 100Hz bandwidth, rarely measurement not complete !
            int tries=0;
            for(; tries<5; tries++) {
                dev.delay_us(usec);
                if(MeasurementIsComplete()) break; // measurement finished =>
                usec = 1000; // wait another millisecond and try again...
            }
            assert(MeasurementIsComplete());
        #else
            #error MMC5983MA_CONTINUOUS_MODE not implemented in MeasureOneTime
        #endif // #ifndef MMC5983MA_CONTINUOUS_MODE
        Fetch_XYZ(result);
    }
};

template <typename TDEVICE>
int8_t MMC5983MA_C<TDEVICE>::Init()
{
    int8_t rslt;
    uint8_t chip_id_read;
    initialized = false;
    do {
        dev.Init(); // communication layer initialization
        // Get chip into known state (needed when not immediately following a power-cycle) - SW reset
        WriteControlAction(ControlRegister::Control_1, (uint8_t)Control_1_Mask::Action_SW_RST);
        memset(control_settings,0,sizeof(control_settings)); // set local copy of control registers to default
        #ifdef MMC5983MA_PRINT_DETAILED_LOG
            dev.DiagPrintf("Wait >10mSec after reset\n");
        #endif
        dev.delay_us(20000); // Minimum 10msec required after reset
        // Read and validate chip ID
        rslt = get_reg(Register::Product_ID, chip_id_read);
        if (rslt != 0) break;
        if (chip_id_read != Product_ID_Assigned) return -1;
        SetBandwidth(Bandwidth_T::Bandwidth_00_100Hz);
        #ifdef MMC5983MA_CONTINUOUS_MODE
            // Example from MEMSIC uses Auto_SR, BW00, CM_FREQ_50HZ:
            SetBandwidth(Bandwidth_T::Bandwidth_00_100Hz);
            WriteControlSetting(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Setting_Auto_SR_en, (uint8_t)Control_0_Mask::Setting_Auto_SR_en);
            SetContinuousMode(4/*nominal 50Hz*/);
          #if 0 // Initial try
            WriteControlSetting(ControlRegister::Control_2,
                (uint8_t)Control_2_Mask::Setting_ContinuousModeRate | (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable,
                (uint8_t)0x04/*50Hz @ BW=00*/                       | (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable);
            // ??? Not required for continuous mode? Initiate Magnetic Measurement
            WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_TM_M);
          #endif
        #endif
        initialized = true;
    } while(0);
    return rslt;
}

template <typename TDEVICE>
int8_t MMC5983MA_C<TDEVICE>::get_regs(Register reg, uint8_t (&reg_data)[], uint32_t len)
{
    int8_t rslt = 0; // BMP5_OK;
    dev.read((uint8_t)reg, reg_data, len);
    #ifdef MMC5983MA_PRINT_DETAILED_LOG
        for(uint8_t idx=0; idx<len; idx++) {
            uint8_t regn = (uint8_t)reg+idx;
            dev.DiagPrintf("get_reg %02x %s => %02x\n", regn, RegisterName((Register)regn), reg_data[idx]);
        };
    #endif
    if (!dev.IO_OK())
    {
        rslt = -1; // BMP5_E_COM_FAIL;
    }
    return rslt;
}

template <typename TDEVICE>
int8_t MMC5983MA_C<TDEVICE>::set_regs(Register reg, const uint8_t (&reg_data)[], uint32_t len)
{
    #ifdef MMC5983MA_PRINT_DETAILED_LOG
        for(uint8_t idx=0; idx<len; idx++) {
            uint8_t regn = (uint8_t)reg+idx;
            dev.DiagPrintf("set_reg %02x %s <= %02x\n", regn, RegisterName((Register)regn), reg_data[idx]);
        };
    #endif
    int8_t rslt = 0; // BMP5_OK;
    dev.write((uint8_t)reg, reg_data, len);
    if (!dev.IO_OK())
    {
        rslt = -1; // BMP5_E_COM_FAIL;
    }
    return rslt;
}

template <typename TDEVICE>
int8_t MMC5983MA_C<TDEVICE>::Measure_XYZ_Field_WithResetSet()
{
    #ifndef MMC5983MA_CONTINUOUS_MODE // RESET-SET don't make sense in continuous mode
        // Make sure we're not in AutoSR mode before trying explicit SET-RESET
        WriteControlSetting(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Setting_Auto_SR_en, 0);
        uint32_t resultAfter_SET[3] = {0}, resultAfter_RESET[3] = {0};
        RESET(); // includes required post-pulse delay (nominal 500us, implemented 1msec), now reading ::= -H + Offset
        MeasureOneTime(resultAfter_RESET);
        SET();   // includes required post-pulse delay (nominal 500us, implemented 1msec), now reading ::= +H + Offset
        MeasureOneTime(resultAfter_SET);
        // Compute offset (zero field value) and signed result for each sensor
        for(int chIdx=0; chIdx<3; chIdx++) {
            if(chIdx>0 && dev.UsesSPI()) {
                // Work-around MMC5983MA bug: With SPI interface, RESET only works on X channel
                offset[chIdx] = 0x20000; // With this bug, best we can do is use nominal 0 value...
                field [chIdx] = (int32_t)resultAfter_SET[chIdx] - 0x20000;
            } else {
                offset[chIdx] = (         resultAfter_SET[chIdx] +          resultAfter_RESET[chIdx])/2;
                field [chIdx] = ((int32_t)resultAfter_SET[chIdx] - (int32_t)resultAfter_RESET[chIdx])/2;
            }
        }
    #else
        uint32_t result[3] = {0};
        MeasureOneTime(result);
        // Compute offset (zero field value) and signed result for each sensor
        for(int i=0; i<3; i++) {
            offset[i] = 0x20000; // nominal center value 2^17
            field [i] = (int32_t)result[i] - (int32_t)offset[i];
        }
    #endif
    return 0;
}

template <typename TDEVICE>
int8_t MMC5983MA_C<TDEVICE>::Measure_XYZ_Field_WithAutoSR()
{
    WriteControlSetting(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Setting_Auto_SR_en, (uint8_t)Control_0_Mask::Setting_Auto_SR_en);
    uint32_t autoSR_result[3] = {0};
    MeasureOneTime(autoSR_result);
    for(int chIdx=0; chIdx<3; chIdx++) {
        offset[chIdx] = 0; // the offset value is not available when using Auto-SR
        // MEMSIC support re AutoSR mode function:
        //   SET, Sample1, RESET, Sample2, Output=(Sample1-Sample2)/2.
        // That is wrong: The 0-field value is 0x20000
        field [chIdx] = (int32_t)autoSR_result[chIdx] - 0x20000; // Auto-SR value is centered around 0x2000
    }
    return 0;
}

#endif // MMC5983A_HPP_INCLUDED
