/// MMC5983MA_IO.hpp - MMC5983MA_IO_Base_C class  <BR>
/// IO for MEMSIC MMC5983MA 3-axis magnetometer driver template class.

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

#ifndef MMC5983A_IO_HPP_INCLUDED
#define MMC5983A_IO_HPP_INCLUDED

/// You must provide a class TDEVICE implementing platform-specific device IO
/// to the MMC5983MA_C template. Either:
/// - Implement MMC5983MA_IO_base_C members directly, or
/// - Implement a derived class, adding any required instance data (ie chip select used, error details).
/// Note: Using a template avoids all indirect call overhead (from virtual functions or C function pointers).  <BR>
class MMC5983MA_IO_base_C {
  public:
    typedef enum { SPI, I2C } InterfaceType_T;
    MMC5983MA_IO_base_C(InterfaceType_T interfaceType_) : interfaceType(interfaceType_) {};
    const InterfaceType_T interfaceType;
    bool UsesSPI() const { return interfaceType==SPI; };
    /// Platform-specific Initialization
    bool Init();
    /// Platform-specific bus read
    void read(uint8_t reg_addr, uint8_t (&read_data)[], uint32_t len);
    /// Platform-specific bus write
    void write(uint8_t reg_addr, const uint8_t (&write_data)[], uint32_t len);
    /// Platform-specific delay before return
    void delay_us(uint32_t uSecs);
    /// Did last IO operation succeed?
    bool IO_OK();
    /// Application must implement printf-analog if MMC5983MA_PRINT_DETAILED_LOG is defined in MMC5983MA_C
    static int DiagPrintf(const char* format, ...)
    #ifdef __GNUG__
        __attribute__((format(printf, 1, 2))); // help GCC do DiagPrintf format string checking
    #else
        ;
    #endif
};

#endif // MMC5983A_IO_HPP_INCLUDED
