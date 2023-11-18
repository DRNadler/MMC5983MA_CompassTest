// MCP2221.hpp - Class for MCP2221 I2C access, using Microchip's MCP2221 library.

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

#pragma once
#include <stdint.h>
#include <assert.h>
#include "mcp2221_dll_um.h" // Microchip API

class MCP2221
{
  public:
	MCP2221() {}; // real work is done in Init(), not ctor
	bool Init();
	unsigned int connectedDevices = 0; // How many MCP2221 attached to this PC?
	void* handle = 0;
	bool IsOpen() { return handle != 0; };

	// wrappers for DLL I2C functions
	int Mcp2221_I2cCancelCurrentTransfer() {
		assert(handle);
		return ::Mcp2221_I2cCancelCurrentTransfer(handle);
	};
	int Mcp2221_I2cRead(unsigned int bytesToRead, uint8_t slaveAddress, bool use7bitAddress, uint8_t* i2cRxData) {
		assert(handle);
		return ::Mcp2221_I2cRead(handle, bytesToRead, slaveAddress, (unsigned char)use7bitAddress, i2cRxData);
	};
	int Mcp2221_I2cWrite(unsigned int bytesToWrite, uint8_t slaveAddress, bool use7bitAddress, uint8_t* i2cTxData) {
		assert(handle);
		return ::Mcp2221_I2cWrite(handle, bytesToWrite, slaveAddress, (unsigned char)use7bitAddress, i2cTxData);
	};
	int Mcp2221_SetAdvancedCommParams(uint8_t timeout, uint8_t maxRetries) {
		assert(handle);
		return ::Mcp2221_SetAdvancedCommParams(handle, timeout, maxRetries);
	};
	int Mcp2221_SetSpeed(unsigned int speed) {
		assert(handle);
		return ::Mcp2221_SetSpeed(handle, speed);
	};
	int Mcp2221_I2cWriteNoStop(unsigned int bytesToWrite, uint8_t slaveAddress, bool use7bitAddress, uint8_t* i2cTxData) {
		assert(handle);
		return ::Mcp2221_I2cWriteNoStop(handle, bytesToWrite, slaveAddress, (unsigned char)use7bitAddress, i2cTxData);
	};
	int Mcp2221_I2cReadRestart(unsigned int bytesToRead, uint8_t slaveAddress, bool use7bitAddress, uint8_t* i2cRxData) {
		assert(handle);
		return ::Mcp2221_I2cReadRestart(handle, bytesToRead, slaveAddress, (unsigned char)use7bitAddress, i2cRxData);
	};
	int Mcp2221_I2cWriteRestart(unsigned int bytesToWrite, uint8_t slaveAddress, bool use7bitAddress, uint8_t* i2cTxData) {
		assert(handle);
		return ::Mcp2221_I2cWriteRestart(handle, bytesToWrite, slaveAddress, (unsigned char)use7bitAddress, i2cTxData);
	};
};
