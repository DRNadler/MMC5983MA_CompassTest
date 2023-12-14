/// MMC5983MA.hpp - MMC5983MA_C class - driver for MEMSIC MMC5983MA 3-axis magnetometer.
// DRNadler 18-November-2023

/*
MIT License

Copyright (c) 2023 Dave Nadler

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

// ToDo MMC5983MA: Reliable compass reading...
// ToDo MMC5983MA: Fix haphazard return values from API, maybe factor out measurement result class.

// ToDo MMC5983MA: Implement continuous mode? Currently 4ms delay per measurement.
// ToDo MMC5983MA: Complete MMC5983MA_CONTINUOUS_MODE - no readings as coded
// #define MMC5983MA_CONTINUOUS_MODE // default as coded is one-shot
#define MMC5983MA_PRINT_DETAILED_LOG

/*
Datasheet Questions (MEMSIC MMC5983MA Rev A -- Formal release date: 4/3/2019)
The datasheet is seriously unclear on many points,
and sometimes conflicts with MEMSIC's sample code...

1) Continuous mode vs. one-shot mode aren't well documented.
   a) Is it necessary to write TM_M to start measurements in continuous mode?
   b) In continuous mode, is Meas_M_Done ever set?
   c) Are TM_M and Meas_M_Done only for one-shot measurements?
   d) Does temperature measurement require continuous pressure mode to be turned off?
      ANSWERED SORT-OF: Temperature sensor does not really work, don't use it.
2) Can SET and RESET used while continuous mode is enabled?
3) The use and behavior of auto-SET-RESET is not documented:
   a) What is the PURPOSE of auto-SET-RESET?
	  Unless a large field saturates the sensor, why is this helpful?
	  How can this improve performance?
   b) The Auto_SR_en bit is labeled "Automatic Set-Reset".
	  What exactly does the sensor do if this bit is set?
	  Does it operate in both continuous and one-shot mode?
   c) The sample code provided by MEMSIC says "continuous mode with auto set and reset" - is that correct?
	  The datasheet page 16 says "The device will perform a SET automatically...";
	  should that be "performs a periodic RESET/SET sequence"?
	  Which is the correct explanation of what the chip does?
	  The order of SET and RESET determines the sensing polarity, so this needs to be clear, right?
4) Explain saturation detection - what are the steps?
   What is the additional applied saturation test field in Gauss when a saturation-enable bit is set?
   Is it necessary to set the enable bits back to 0, or do they reset automatically like SET/RESET?
5) The datasheet says SET and RESET operations take 500 nsec.
   MEMSIC sample code waits 500 usec. Which is correct?
6) The decimation filter is mentioned but not described.
   Given BW and CM settings, What *exactly* is the sampling and filter?
7) OTP read controls are discussed, but OTP is never defined.
   What is it? Why do we care? ANSWERED: OTP indicates one-time programming
   read successfully by firmware after reset. This bit should always be set.

*/

#ifndef MMC5983A_HPP_INCLUDED
#define MMC5983A_HPP_INCLUDED


#include <stdint.h>
#include <stddef.h> // size_t
#include <assert.h>
#include <memory.h> // memcpy

/// Driver for MMC5983MA 3-axis magnetometer sensor
/// See MMC5983MA_IO.hpp for example TDEVICE class (provides platform-specific IO)
template <typename TDEVICE>
class MMC5983MA_C {
	// ==============================  Definitions	==============================
  public:
	/// Bandwidth selection adjusts the length of the decimation filter,
	/// and controls the measurement duration. Note: X/Y/Z channel
	/// measurements are taken in parallel.
	enum class Bandwidth_T : uint8_t {
		Bandwidth_00_100Hz = 0x00, // 8msec
		Bandwidth_01_200Hz = 0x01, // 4msec
		Bandwidth_10_400Hz = 0x02, // 2msec
		Bandwidth_11_800Hz = 0x03, // .5msec
	};
	/// Given a bandwidth setting, how much time does a measurement take?
	static int uSecPerMeasurement(Bandwidth_T bw) {
		static const int usec[4] = { 8000, 4000, 2000, 500 };
		return usec[(int)bw];
	};

  protected:
	enum class Register : uint8_t {
		X_out_0	   = 0x00, // Xout [17:10]		   read-only
		X_out_1	   = 0x01, // Xout [9:2]		   read-only
		Y_out_0	   = 0x02, // Yout [17:10]		   read-only
		Y_out_1	   = 0x03, // Yout [9:2]		   read-only
		Z_out_0	   = 0x04, // Zout [17:10]		   read-only
		Z_out_1	   = 0x05, // Zout [9:2]		   read-only
		XYZ_out_2  = 0x06, // Xout[1:0],Yout[1:0],Zout[1:0] (for 18-bit mode) read-only
		T_out	   = 0x07, // Temperature output   read-only
		Status	   = 0x08, // Device status		   read-write (write to reset interrupt flags)
		// To prevent accidents, control registers are in separate type below
		//Control_0	 = 0x09, // Control register 0	 write-only
		//Control_1	 = 0x0a, // Control register 1	 write-only
		//Control_2	 = 0x0b, // Control register 2	 write-only
		//Control_3	 = 0x0c, // Control register 3	 write-only
		Product_ID = 0x2f  // Product ID
	};
	enum class ControlRegister : uint8_t {
		Control_0  = 0x09, ///< Control register 0	 write-only
		Control_1  = 0x0a, ///< Control register 1	 write-only
		Control_2  = 0x0b, ///< Control register 2	 write-only
		Control_3  = 0x0c, ///< Control register 3	 write-only
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
		OTP_read_done = 0x10, ///< The chip successfully read its OTP memory; should always be set.
		Meas_T_Done	  = 0x02, ///< A temperature measurement completed.
							  ///< After a new measurement command, turns to 0.
							  ///< When measurement is finished, remains 1 till next measurement.
							  ///< Writing 1 into this bit will clear the corresponding interrupt.
		Meas_M_Done	  = 0x01, ///< A magnetic measurement completed (check before reading output).
							  ///< After a new measurement command, turns to 0.
							  ///< When measurement is finished, remains 1 till next measurement.
							  ///< Writing 1 into this bit will clear the corresponding interrupt.
	};
	enum class Control_0_Mask : uint8_t {
		Action_TM_M = 0x01, ///< Take magnetic field measurement (set 1 to initiate measurement).
							///< Automatically reset to 0 when the measurement is complete.
		Action_TM_T = 0x02, ///< Take Temperature measurement (set 1 to initiate measurement).
							///< Automatically reset to 0 when the measurement is complete.
							///< This bit and TM_M cannot be high at the same time.
		Setting_Enable_MeasurementDoneINT = 0x04, ///< Set 1 to enable the completed measurement interrupt.
							///< When a magnetic or temperature measurement finishes, an
							///< interrupt will be signaled.
		Action_SET = 0x08,	///< Writing 1 starts the SET operation, which sends a large current
							///< through the sensor coils for 500ns.
							///< Automatically reset to 0 at the end of the SET operation.
		Action_REVERSE_SET = 0x10,///< Writing 1 starts the REVERSE_SET operation (MEMSIC calls this RESET),
							///< which sends a large reverse current (in the opposite direction of SET)
							///< through the sensor coils for 500ns.
							///< Automatically reset to 0 at the end of the REVERSE_SET operation.
		Setting_Auto_SR_en = 0x20,///< Set 1 to enable automatic SET/REVERSE_SET.
		Action_OTP_Read = 0x40,///< Writing 1 commands the device to read the OTP data again.
							///< Automatically reset to 0 after the shadow registers for OTP are refreshed.
							///< MEMSIC support: Should never be required.
	};
	const static uint8_t Product_ID_Assigned = 0x30; // MMC5983MA product ID value
	enum class Control_1_Mask : uint8_t {
		Setting_Bandwidth = 0x03, ///< see Bandwidth_T enum below
		Setting_X_inhibit = 0x04, ///< Writing 1 will disable X channel.
		Setting_YZ_inhibit= 0x18, ///< Writing 1 to the two bits will disable Y and Z channel.
		Action_SW_RST	  = 0x80, ///< Writing 1 commands the part to reset, similar to power-up.
								  ///< Clears all registers and also rereads OTP
								  ///< Power/reset on time is 10mS.
	};
	enum class Control_2_Mask : uint8_t {
		Setting_ContinuousModeRate = 0x07,	///< 0 is Off. See table in datasheet for nominal values.
		Setting_ContinuousModeEnable = 0x80, ///< 1 to enable, rate above must not be 0 if enabled.
		Setting_AutoSETrate = 0x70, ///< Controls number of measurements between automatic SET if enabled
		Setting_AutoSETenable = 0x80,  ///< 1 enables automatic periodic SET
	};
	enum class Control_3_Mask : uint8_t {
		Action_SaturationTestEnablePlus = 0x02, // ToDo MMC5983MA: Is saturation function a setting or action?
		Action_SaturationTestEnableMinus = 0x04,// "" "" ""
		Setting_SPI_3wire = 0x40,
	};

	// ==============================	 Members	==============================
  public:
	MMC5983MA_C(TDEVICE &dev_) : initialized(false), dev(dev_) {};

	/// Initialize the sensor. Call this before using any other API.
	/// On success, sets 'initialized' member true.
	int8_t Init();

	// State and measurement information
	bool initialized; ///< false until Init() is called and succeeds

	/// Read the magnetic field, including the reverse/normal degauss operation
	/// to compute offset. Place results in field and offset members.
	int8_t Measure_XYZ_With_Degauss();

	int32_t field[3] = {0}; ///< Last magnetic field reading set (X,Y,Z), signed values already adjusted with offsets.
	const static int32_t CountsPerGauss = 16384; // 2^17 / 8G full-scale when using full 18-bit resolution as we do here.

	/// Each sensor's offset is the unsigned integer value the sensor will read
	/// with zero external field. Should be about mid-range for 18-bit value,
	/// ie 2^17 = 0x20000 = 131072.
	uint32_t offset[3] = {0};  ///< Last measured offset (X,Y,Z) - always included in lastReading

  protected:

	TDEVICE &dev; ///< platform-specific hardware IO instance provided by user

	typedef uint8_t uint8_array_t[]; ///< assist internal type conversions
	inline uint8_array_t &GetArrayRefFromSingle(uint8_t &s) {
		return	*reinterpret_cast<uint8_array_t*>(&s);
	};
	inline const uint8_array_t &GetConstArrayRefFromSingle(const uint8_t &s) {
		return	*reinterpret_cast<const uint8_array_t*>(&s);
	};

	// The control registers contain two different flavors of bits:
	// 1) "Action" Write-a-1 to initiate a process, and
	// 2) "Settings" such as bandwidth.
	// The control registers are WRITE-ONLY, so we have to track all settings,
	// because we must re-write some settings when for example starting a measurement.
	// This is a HORRIBLE design. The designer should have:
	// a) Segregated registers containing "Settings" vs. "Actions", and/or
	// b) made the registers readable.
	// Its like MEMSIC deliberately tried to make this part hard to support...

	/// Saved control settings in all 4 control registers ("action" bits will never be set here)
	uint8_t control_settings[4] = {0}; // Reset value of all control registers is 0
	/// Get a reference to the control setting copy for the given control register
	uint8_t inline &GetSettingRef(ControlRegister controlReg) {
		int controlRegIdx = (int)controlReg - (int)ControlRegister::Control_0; // 0-3
		assert(controlRegIdx>=0 && (size_t)controlRegIdx<sizeof(control_settings));
		// Return local copy of the control register to access settings
		return control_settings[controlRegIdx];
	}
	/// Update a setting (or settings if or'd together) in one control register
	void WriteControlSetting(ControlRegister controlReg, uint8_t settingMask, uint8_t settingValue) {
		// Update in-memory copy of the control register settings per arguments
		uint8_t &setting = GetSettingRef(controlReg);
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
		WriteControlSetting(ControlRegister::Control_1, (uint8_t)Control_1_Mask::Setting_Bandwidth, (uint8_t)bw);
	}
	Bandwidth_T GetBandwidth(void) const {
		return (Bandwidth_T)(control_settings[1] & (int)Control_1_Mask::Setting_Bandwidth);
	};
	int GetuSecPerMeasurement(void) { return uSecPerMeasurement(GetBandwidth()); };

	/// Set Continuous mode (0 off, 1-7 per datasheet)
	void SetContinuousMode(uint8_t cm) {
		// continuous rate setting is in low-order 3 bits; no shifting required
		if(cm) cm |= (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable;
		WriteControlSetting(ControlRegister::Control_2,
			(uint8_t)Control_2_Mask::Setting_ContinuousModeEnable | (uint8_t)Control_2_Mask::Setting_ContinuousModeRate, cm);
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

	/// Perform normal degauss pulse (SET) including required wait.
	inline void DegaussPulse_Normal(void)
	{
		// ToDo MMC5983MA: assert not in continuous mode for SET ???
		WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_SET);
		dev.delay_us(1); // 500ns required
	}
	/// Perform reverse degauss pulse (REVERSE_SET/RESET) including required wait.
	inline void DegaussPulse_Reverse(void)
	{
		// ToDo MMC5983MA: assert not in continuous mode for REVERSE_SET ???
		WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_REVERSE_SET);
		dev.delay_us(1); // 500ns required
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
			// ToDo MMC5983MA: assert not in continuous mode
			// Initiate Magnetic Measurement
			WriteControlAction(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Action_TM_M);
			// Wait for measurement complete
			int usec = GetuSecPerMeasurement()+4000; // ToDo MMC5983MA: Why is single-shot delay inadequate?? +4000, 10x work...
			dev.delay_us(usec);
			uint8_t status;
			get_reg(Register::Status,status);
			assert(status & (uint8_t)StatusMask::Meas_M_Done);
			Fetch_XYZ(result);
	}
};

template <typename TDEVICE>
int8_t MMC5983MA_C<TDEVICE>::Init()
{
	int8_t rslt;
	uint8_t chip_id_read;
	do {
		// Get chip into known state (needed when not immediately following a power-cycle) - SW reset
		WriteControlAction(ControlRegister::Control_1, (uint8_t)Control_1_Mask::Action_SW_RST);
		memset(control_settings,0,sizeof(control_settings)); // set local copy of control registers to default
		#ifdef PRINT_DETAILED_LOG
			dev.DiagPrintf("Wait >10mSec after reset\n");
		#endif
		dev.delay_us(20000); // Minimum 10msec required after reset
		// Read and validate chip ID
		rslt = get_reg(Register::Product_ID, chip_id_read);
		if (rslt != 0) break;
		if (chip_id_read != Product_ID_Assigned) return -1;
		SetBandwidth(Bandwidth_T::Bandwidth_00_100Hz/* Bandwidth_01_200Hz*/);
		#ifdef MMC5983MA_CONTINUOUS_MODE
			// Example from MEMSIC uses Auto_SR, BW00, CM_FREQ_50HZ:
			SetBandwidth(Bandwidth_T::Bandwidth_00_100Hz);
			WriteControlSetting(ControlRegister::Control_0, (uint8_t)Control_0_Mask::Setting_Auto_SR_en, (uint8_t)Control_0_Mask::Setting_Auto_SR_en);
			SetContinuousMode(4/*nominal 50Hz*/);
		  #if 0 // Initial try
			WriteControlSetting(ControlRegister::Control_2,
				(uint8_t)Control_2_Mask::Setting_ContinuousModeRate | (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable,
				(uint8_t)0x04/*50Hz @ BW=00*/						| (uint8_t)Control_2_Mask::Setting_ContinuousModeEnable);
			// ToDo MMC5983MA: Not required for continuous mode? Initiate Magnetic Measurement
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
int8_t MMC5983MA_C<TDEVICE>::Measure_XYZ_With_Degauss()
{
	#ifndef MMC5983MA_CONTINUOUS_MODE // REVERSE_SET-SET don't make sense in continuous mode
		uint32_t resultAfterNormalDegauss[3] = {0}, resultAfterReverseDegauss[3] = {0};
		DegaussPulse_Reverse(); // now reading ::= -H + Offset
		const int delayUsec = 50000; // ToDo MMC5983MA: remove extra delays?
		#ifdef MMC5983MA_PRINT_DETAILED_LOG
			dev.DiagPrintf("delay %d usecs\n", delayUsec);
		#endif
		dev.delay_us(delayUsec);
		MeasureOneTime(resultAfterReverseDegauss);
		DegaussPulse_Normal(); // now reading ::= +H + Offset
		#ifdef MMC5983MA_PRINT_DETAILED_LOG
			dev.DiagPrintf("delay %d usecs\n", delayUsec);
		#endif
		dev.delay_us(delayUsec);
		MeasureOneTime(resultAfterNormalDegauss);
		// Compute offset (zero field value) and signed result for each sensor
		for(int i=0; i<3; i++) {
			offset[i] = (         resultAfterNormalDegauss[i] +          resultAfterReverseDegauss[i])/2;
			field [i] = ((int32_t)resultAfterNormalDegauss[i] - (int32_t)resultAfterReverseDegauss[i])/2;
		}
	#else
		uint32_t result[3] = {0};
		MeasureOneTime(result);
		// Compute offset (zero field value) and signed result for each sensor
		for(int i=0; i<3; i++) {
			offset[i] = 0x20000; // nominal value
			field [i] = (int32_t)result[i] - (int32_t)offset[i];
		}
	#endif
	return 0;
}

#endif // MMC5983A_HPP_INCLUDED
