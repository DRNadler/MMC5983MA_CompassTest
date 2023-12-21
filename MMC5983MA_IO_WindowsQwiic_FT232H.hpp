// MMC5983MA_IO_WindowsQwiic_FT232H.hpp - IO class for accessing MMC5983MA via FT232H

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

#ifndef MMC5983MA_IO_WindowsQwiic_FT232H_HPP_INCLUDED
#define MMC5983MA_IO_WindowsQwiic_FT232H_HPP_INCLUDED

#include "MMC5983MA_IO.hpp"
extern "C" { // antique FTDI headers lack this
    #include "ftdi_infra.h"  /*Common portable infrastructure (datatypes, libraries, etc)*/
    #include "ftdi_common.h" /*Common across I2C, SPI, JTAG modules*/
    // Set up to use MPSSE library etc, cribbed from simple-static.c example
    #include "ftd2xx.h"
    #include "libMPSSE_i2c.h" // Bizarrely, redefines stuff from ftdi_i2c.h
}

// Provide IO primitives for MMC5983MA API
class MMC5983MA_IO_WindowsQwiic_FT232H_C : public MMC5983MA_IO_base_C {
public:
    MMC5983MA_IO_WindowsQwiic_FT232H_C() : MMC5983MA_IO_base_C(I2C) {}; // Warning: no communications initialization in ctor
    // Implement the base class IO function suggestions in this derived class
    void read(uint8_t registerAddress, uint8_t(&read_data)[], uint32_t len) {
        DWORD bytesTransferred;
        // 4,5) write start-bit/slave address, then register address (should wait for ACK)
        // int ret1 = mcp2221.Mcp2221_I2cWrite(1, slave7bitAddress, true, &registerAddress);
        bytesTransferred = 0;
        ftStatus = I2C_DeviceWrite(ftHandle, slave7bitAddress, 1, &registerAddress, \
            & bytesTransferred, I2C_TRANSFER_OPTIONS_START_BIT);
        if (ftStatus == FT_DEVICE_NOT_FOUND) {
            DiagPrintf("Ooops, device 0x%x not found!\n", slave7bitAddress);
            DiagPrintf("...failed to read an ACK after sending device address in I2C_Write8bitsAndGetAck\n");
        }
        assert(ftStatus == FT_OK);
        assert(bytesTransferred == 1);
        // 6,7) another start bit, sensor address with 'read' bit set, then start reading
        // int ret2 = mcp2221.Mcp2221_I2cRead(len, slave7bitAddress, true, read_data);
        bytesTransferred = 0;
        ftStatus = I2C_DeviceRead(ftHandle, slave7bitAddress, len, read_data, \
            & bytesTransferred, I2C_TRANSFER_OPTIONS_START_BIT | I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
        assert(ftStatus == FT_OK);
        assert(bytesTransferred == len);
    };
    void write(uint8_t registerAddress, const uint8_t(&write_data)[], uint32_t len) {
        assert(len == 1);
        UCHAR buf[2] = { registerAddress, write_data[0] };
        // int ret1 = mcp2221.Mcp2221_I2cWrite(2, slave7bitAddress, true, buf);
        DWORD bytesTransferred = 0;
        ftStatus = I2C_DeviceWrite(ftHandle, slave7bitAddress, 2, buf, \
            & bytesTransferred, I2C_TRANSFER_OPTIONS_START_BIT);
        if (ftStatus == FT_DEVICE_NOT_FOUND) {
            DiagPrintf("Ooops, device 0x%x not found!\n", slave7bitAddress);
            DiagPrintf("...failed to read an ACK after sending device address in I2C_Write8bitsAndGetAck\n");
        }
        assert(ftStatus == FT_OK);
    };
    void delay_us(uint32_t uSecs) {
        std::this_thread::sleep_for(std::chrono::microseconds(uSecs));
    };
    void init() { // not invoked by MMC5983MA_C; do this before using MMC5983MA_IO_WindowsQwiic_C!
        Init_libMPSSE(); // This application builds MPSSE components into EXE; so Init_lib is not automatically called on DLL load.
        DiagPrintf("ftd2xx.dll loaded OK!\n");
        DWORD numChannels;
        ftStatus = I2C_GetNumChannels(&numChannels);
        assert(numChannels >= 1);
        DiagPrintf("Found at least one channel on FT232H\n");
        for (uint32 i = 0; i < numChannels; i++)
        {
            FT_DEVICE_LIST_INFO_NODE devList;
            ftStatus = I2C_GetChannelInfo(i, &devList);
            assert(ftStatus == FT_OK);
            DiagPrintf("Information on channel number %d:\n", i);
            /*print the dev info*/
            DiagPrintf("		Flags=0x%x\n", devList.Flags);
            DiagPrintf("		Type=0x%x\n", devList.Type);
            DiagPrintf("		ID=0x%x\n", devList.ID);
            DiagPrintf("		LocId=0x%x\n", devList.LocId);
            DiagPrintf("		SerialNumber=%s\n", devList.SerialNumber);
            DiagPrintf("		Description=%s\n", devList.Description);
            DiagPrintf("		ftHandle=0x%x (0 unless channel is open)\n", (unsigned int)devList.ftHandle);
        }
        DiagPrintf("\nVersion Check\n");
        DWORD verMPSSE, verD2XX;
        ftStatus = Ver_libMPSSE(&verMPSSE, &verD2XX);
        // Never set for non-DLL build: DiagPrintf("libmpsse: %08x\n", verMPSSE);
        DiagPrintf("libftd2xx: %08x\n", verD2XX);
        /* Open the first available channel */
        ftStatus = I2C_OpenChannel(/*channel=*/0, &ftHandle);
        assert(ftStatus == FT_OK);
        ChannelConfig channelConf = {
            .ClockRate = I2C_CLOCK_STANDARD_MODE,
            .LatencyTimer = 100,
            .Options = 0
                | I2C_DISABLE_3PHASE_CLOCKING
                | I2C_ENABLE_DRIVE_ONLY_ZERO /* pull-up resistors are on SEN-19921 sensor Qwiic board. */
                ,
            .Pin = 0,
            .currentPinState = 0,
            };
        channelConf.ClockRate = I2C_CLOCK_STANDARD_MODE;
        channelConf.LatencyTimer = 100;
        channelConf.Options = 0
            | I2C_DISABLE_3PHASE_CLOCKING
            | I2C_ENABLE_DRIVE_ONLY_ZERO /* pull-up resistors are on SEN-19921 sensor Qwiic board. */
            ;
        channelConf.Pin = channelConf.currentPinState = 0; // DRN guess
        ftStatus = I2C_InitChannel(ftHandle, &channelConf);
        assert(ftStatus == FT_OK);
        DiagPrintf("MMC5983MA_IO_WindowsQwiic_C::init opened and initialized channel AOK\n");
    };
    bool IO_OK(void) { return ftStatus == 0; };
    const static uint8_t slave7bitAddress = (0b0110000); /// The MEMSIC device 7 - bit device WRITE address is[0110000] (left-shifted, then optional OR'd with read-bit 1)
    /// Application must implement printf-analog if MMC5983MA_PRINT_DETAILED_LOG is defined in MMC5983MA_C
    #ifdef __GNUG__
        int DiagPrintf(const char* format, ...); __attribute__((format(printf, 2, 3)));
    #else
        int DiagPrintf(const char* format, ...); // __attribute__((format(printf, 2, 3)));
    #endif
    // FTDI-specific stuff
    FT_HANDLE ftHandle = 0;
    FT_STATUS ftStatus = 0;
};

#endif // MMC5983MA_IO_WindowsQwiic_FT232H_HPP_INCLUDED
