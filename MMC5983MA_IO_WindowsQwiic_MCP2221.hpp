// MMC5983MA_IO_WindowsQwiic_MCP2221.hpp - IO class for accessing MMC5983MA via MCP2221

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

#ifndef MMC5983MA_IO_WindowsQwiic_MCP2221_HPP_INCLUDED
#define MMC5983MA_IO_WindowsQwiic_MCP2221_HPP_INCLUDED

#include "MMC5983MA_IO.hpp"
#include "MCP2221.hpp"

// Provide IO primitives for MMC5983MA API
class MMC5983MA_IO_WindowsQwiic_MCP2221_C : public MMC5983MA_IO_base_C {
public:
    MCP2221 mcp2221; // IO device
    MMC5983MA_IO_WindowsQwiic_MCP2221_C() : MMC5983MA_IO_base_C(I2C) {}; // Warning: no communications initialization in ctor
    // Implement the base class IO function suggestions in this derived class
    void Init(); // not invoked by ctors; do this before using MMC5983MA_IO_WindowsQwiic_C!
    void read(uint8_t reg_addr, uint8_t(&read_data)[], uint32_t len);
    void write(uint8_t reg_addr, const uint8_t(&write_data)[], uint32_t len);
    void delay_us(uint32_t period);
    int last_IO_status = 0;
    bool IO_OK(void) { return last_IO_status == 0; };
    // const uint8_t slave7bitAddress = 0x77; // kludge try DSP310
    const static uint8_t slave7bitAddress = (0b0110000); /// The MEMSIC device 7 - bit device WRITE address is[0110000] (left-shifted, then optional OR'd with read-bit 1)
    /// Application must implement printf-analog if MMC5983MA_PRINT_DETAILED_LOG is defined in MMC5983MA_C
    int DiagPrintf(const char* format, ...);
    #ifdef __GNUG__
        __attribute__((format(printf, 2, 3))); // help GCC check DiagPrintf format against provided arguments
    #endif
};

#endif // MMC5983MA_IO_WindowsQwiic_MCP2221_HPP_INCLUDED
