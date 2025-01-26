// MMC5983MA_IO_WindowsQwiic_MCP2221 - Windows Qwiic IO to MMC5983, using MCP2221 USB adapter.

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

#include <assert.h>
#include <thread>

#include "MCP2221.hpp"
#include "MMC5983MA_IO_WindowsQwiic_MCP2221.hpp"


void MMC5983MA_IO_WindowsQwiic_MCP2221_C::Init() {
    if(!mcp2221.IsOpen()) {
        mcp2221.Init();
    };
}
void  MMC5983MA_IO_WindowsQwiic_MCP2221_C::read(uint8_t registerAddress, uint8_t(&read_data)[], uint32_t len) {
    assert(mcp2221.IsOpen());
    // 4,5) write start-bit/slave address, then register address (should wait for ACK)
    int ret1 = mcp2221.Mcp2221_I2cWrite(1, slave7bitAddress, true, &registerAddress);
    assert(ret1 == 0);
    // 6,7) another start bit, sensor address with 'read' bit set, then start reading
    int ret2 = mcp2221.Mcp2221_I2cRead(len, slave7bitAddress, true, read_data);
    assert(ret2 == 0);
}
void MMC5983MA_IO_WindowsQwiic_MCP2221_C::write(uint8_t registerAddress, const uint8_t(&write_data)[], uint32_t len) {
    assert(mcp2221.IsOpen());
    // Note: MMC5983MA DOES auto-increment write address
    // Anyway, API only ever writes 1 byte here.
    assert(len == 1);
    uint8_t buf[2] = { registerAddress, write_data[0] };
    int ret1 = mcp2221.Mcp2221_I2cWrite(2, slave7bitAddress, true, buf);
    assert(ret1 == 0);
}
void MMC5983MA_IO_WindowsQwiic_MCP2221_C::delay_us(uint32_t uSecs) {
    std::this_thread::sleep_for(std::chrono::microseconds(uSecs));
}
