// MCP2221.cpp - Class for MCP2221 I2C access, using Microchip's MCP2221 library.

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

#include <stdexcept>
#ifndef MCP2221_LIB
  #define MCP2221_LIB // Specify we're importing Microchip LIB symbols
#endif
#include "MCP2221.hpp"

bool MCP2221::Init()
{
    const unsigned int MCP2221_VID = 0x4d8; //default VID
    const unsigned int MCP2221_PID = 0xDD; // default PID

    Mcp2221_GetConnectedDevices(MCP2221_VID, MCP2221_PID, &connectedDevices);
    if (connectedDevices <= 0)
        throw std::runtime_error("No MCP2221 connected to this PC");
    handle = Mcp2221_OpenByIndex(MCP2221_VID, MCP2221_PID, 0);
    if (handle == 0)
        throw std::runtime_error("Mcp2221_OpenByIndex failed");
    return false;
}