#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef _WIN32
#include <unistd.h>
#include <dlfcn.h>	/*for dlopen() & dlsym()*/
#endif

#include "ftd2xx.h"
#include "libmpsse_i2c.h"

#define BME280_REGISTER_DEVICE_ID 0xd0
#define BME280_REGISTER_DEVICE_ID_EXPECT 0x60

#define BME280_REGISTER_DEVICE_RESET 0xe0
#define BME280_REGISTER_DEVICE_RESET_VALUE 0xb6

#define BME280_REGISTER_DEVICE_CONFIG 0xf5
#define BME280_REGISTER_DEVICE_CONFIG_TIMEOUT_MIN 0x00 // 0.5 ms
#define BME280_REGISTER_DEVICE_CONFIG_TIMEOUT_MAX 0x05 // 1000 ms
#define BME280_REGISTER_DEVICE_CONFIG_FILTER_OFF 0x00 // filter off
#define BME280_REGISTER_DEVICE_CONFIG_FILTER_MIN 0x01 // filter coefficient 2
#define BME280_REGISTER_DEVICE_CONFIG_FILTER_MAX 0x04 // filter coefficient 16
#define BME280_REGISTER_DEVICE_CONFIG_VALUE \
	((BME280_REGISTER_DEVICE_CONFIG_TIMEOUT_MIN) \
	|BME280_REGISTER_DEVICE_CONFIG_FILTER_MAX) 

#define BME280_REGISTER_DEVICE_CTRL_MEAS 0xf4
#define BME280_REGISTER_DEVICE_CTRL_MEAS_MODE_SLEEP 0x00
#define BME280_REGISTER_DEVICE_CTRL_MEAS_MODE_FORCED 0x01
#define BME280_REGISTER_DEVICE_CTRL_MEAS_MODE_NORMAL 0x03
#define BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_TEMP_SKIP 0x00
#define BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_TEMP_MIN 0x20
#define BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_TEMP_MAX 0xa0
#define BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_PRESSURE_SKIP 0x00
#define BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_PRESSURE_MIN 0x04
#define BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_PRESSURE_MAX 0x14
#define BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_OFF \
	(BME280_REGISTER_DEVICE_CTRL_MEAS_MODE_SLEEP \
	|BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_TEMP_SKIP \
	|BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_PRESSURE_SKIP)
#define BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_ON \
	(BME280_REGISTER_DEVICE_CTRL_MEAS_MODE_SLEEP \
	|BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_TEMP_MAX \
	|BME280_REGISTER_DEVICE_CTRL_MEAS_OVERSAMPLE_PRESSURE_MAX)
#define BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_NORMAL \
	(BME280_REGISTER_DEVICE_CTRL_MEAS_MODE_NORMAL \
	|BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_ON)

#define BME280_REGISTER_DEVICE_CTRL_HUM 0xf2
#define BME280_REGISTER_DEVICE_CTRL_HUM_OVERSAMPLE_SKIP 0x00
#define BME280_REGISTER_DEVICE_CTRL_HUM_OVERSAMPLE_MIN 0x01
#define BME280_REGISTER_DEVICE_CTRL_HUM_OVERSAMPLE_MAX 0x5

#define BME280_REGISTER_DEVICE_PRESSURE_MSB 0xf7
#define BME280_REGISTER_DEVICE_PRESSURE_LSB 0xf8
#define BME280_REGISTER_DEVICE_PRESSURE_XLSB 0xf9
#define BME280_REGISTER_DEVICE_TEMP_MSB 0xfa
#define BME280_REGISTER_DEVICE_TEMP_LSB 0xfb
#define BME280_REGISTER_DEVICE_TEMP_XLSB 0xfc
#define BME280_REGISTER_DEVICE_HUM_MSB 0xfd
#define BME280_REGISTER_DEVICE_HUM_LSB 0xfe

#define BME280_REGISTER_DIG_T1_LSB 0x88
#define BME280_REGISTER_DIG_T1_MSB 0x89
#define BME280_REGISTER_DIG_T2_LSB 0x8a
#define BME280_REGISTER_DIG_T2_MSB 0x8b
#define BME280_REGISTER_DIG_T3_LSB 0x8c
#define BME280_REGISTER_DIG_T3_MSB 0x8d

#define BME280_REGISTER_DIG_P1_LSB 0x8e
#define BME280_REGISTER_DIG_P1_MSB 0x8f
#define BME280_REGISTER_DIG_P2_LSB 0x90
#define BME280_REGISTER_DIG_P2_MSB 0x91
#define BME280_REGISTER_DIG_P3_LSB 0x92
#define BME280_REGISTER_DIG_P3_MSB 0x93
#define BME280_REGISTER_DIG_P4_LSB 0x94
#define BME280_REGISTER_DIG_P4_MSB 0x95
#define BME280_REGISTER_DIG_P5_LSB 0x96
#define BME280_REGISTER_DIG_P5_MSB 0x97
#define BME280_REGISTER_DIG_P6_LSB 0x98
#define BME280_REGISTER_DIG_P6_MSB 0x99
#define BME280_REGISTER_DIG_P7_LSB 0x9a
#define BME280_REGISTER_DIG_P7_MSB 0x9b
#define BME280_REGISTER_DIG_P8_LSB 0x9c
#define BME280_REGISTER_DIG_P8_MSB 0x9d
#define BME280_REGISTER_DIG_P9_LSB 0x9e
#define BME280_REGISTER_DIG_P9_MSB 0x9f

#define BME280_REGISTER_DIG_H1 0xa1
#define BME280_REGISTER_DIG_H2_LSB 0xe1
#define BME280_REGISTER_DIG_H2_MSB 0xe2
#define BME280_REGISTER_DIG_H3 0xe3
#define BME280_REGISTER_DIG_H4_LSB 0xe4
#define BME280_REGISTER_DIG_H4_MSB 0xe5
#define BME280_REGISTER_DIG_H5_LSB 0xe5
#define BME280_REGISTER_DIG_H5_MSB 0xe6

#define APP_CHECK_STATUS(exp) {if (exp!=FT_OK){printf(" status(0x%x) != FT_OK\n", exp);}else{;}};

/* Globals */

/* Handle to MPSSE driver */
#ifdef _WIN32
HANDLE hdll_mpsse;
#else
void *hdll_mpsse;
#endif

unsigned short digT1;
signed short digT2, digT3;
unsigned short digP1;
signed short digP2, digP3, digP4, digP5, digP6, digP7, digP8, digP9;
unsigned char digH1, digH3;
signed short digH2, digH4, digH5;
signed char digH6;

FT_STATUS i2c_read(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value)
{
	FT_STATUS status;
	DWORD xfer = 0;

	/* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Read. */
	status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
		I2C_TRANSFER_OPTIONS_START_BIT |
		I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);

	if (status == FT_OK)
	{
		/* Repeated Start condition generated. */
		status = I2C_DeviceRead(ftHandle, address, 1, value, &xfer,
			I2C_TRANSFER_OPTIONS_START_BIT |
			I2C_TRANSFER_OPTIONS_STOP_BIT |
			I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
	}
	APP_CHECK_STATUS(status);

	return status;
}

FT_STATUS i2c_read_multi(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value, UCHAR length)
{
	FT_STATUS status;
	DWORD xfer = 0;

	/* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Read. */
	status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
		I2C_TRANSFER_OPTIONS_START_BIT |
		I2C_TRANSFER_OPTIONS_STOP_BIT |
		I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);

	if (status == FT_OK)
	{
		/* Repeated Start condition generated. */
		status = I2C_DeviceRead(ftHandle, address, length, value, &xfer,
			I2C_TRANSFER_OPTIONS_START_BIT |
			I2C_TRANSFER_OPTIONS_STOP_BIT |
			I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE);
	}
	APP_CHECK_STATUS(status);

	return status;
}

FT_STATUS i2c_write(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, UCHAR value)
{
	FT_STATUS status;
	DWORD xfer = 0;

	/* As per Bosch BME280 Datasheet Figure 9: I2C Multiple Byte Write (not autoincremented) */
	status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
		I2C_TRANSFER_OPTIONS_START_BIT |
		I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
	APP_CHECK_STATUS(status);

	if (status == FT_OK)
	{
		/* Register address not sent on register write. */
		status = I2C_DeviceWrite(ftHandle, address, 1, &value, &xfer,
			I2C_TRANSFER_OPTIONS_NO_ADDRESS |
			I2C_TRANSFER_OPTIONS_STOP_BIT |
			I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
		APP_CHECK_STATUS(status);
	}

	return status;
}

int t_fine;
int compensate_T(int adc)
{
	int var1, var2, temperature;

	var1 = ((((adc >> 3) - ((int)digT1 << 1))) * ((int)digT2)) >> 11;
	var2 = (((((adc >> 4) - ((int)digT1)) * ((adc >> 4) - ((int)digT1))) >> 12) * ((int)digT3)) >> 14;

	t_fine = var1 + var2;
	temperature = ((t_fine * 5) + 128) >> 8;

	return temperature;
}

unsigned int compensate_P(int adc)
{
	int var1, var2;
	unsigned int p_acc;
	int pressure;

	var1 = (((int)t_fine) >> 1) - ((int)64000);
	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int)digP6);
	var2 = var2 + ((var1 * ((int)digP5)) << 1);
	var2 = (var2 >> 2) + (((int)digP4) << 16);
	var1 = (((digP3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int)digP2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int)digP1)) >> 15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p_acc = (((unsigned int)(((signed int)1048576) - adc) - (var2 >> 12))) * 3125;
	if (p_acc < 0x8000000)
	{
		p_acc = (p_acc << 1) / ((unsigned int)var1);
	}
	else
	{
		p_acc = (p_acc / ((unsigned int)var1)) * 2;
	}
	var1 = (((int)digP9) * ((int)(((p_acc >> 3)  * (p_acc >> 3)) >> 13))) >> 12;
	var2 = (((int)(p_acc >> 2)) * ((int)digP8)) >> 13;

	p_acc = (signed int)p_acc + ((var1 + var2 + digP7) >> 4);
	pressure = (unsigned int)p_acc;

	return pressure;
}

int compensate_H(int adc)
{
	int32_t var1;
	int humidity;

	var1 = (t_fine - ((int32_t)76800));
	var1 = (((((adc << 14) - (((int32_t)digH4) << 20) - (((int32_t)digH5) * var1)) +
		((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)digH6)) >> 10) *
		(((var1 * ((int32_t)digH3)) >> 11) + ((int32_t)32768))) >> 10) +
		((int32_t)2097152)) * ((int32_t)digH2) + 8192) >> 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)digH1)) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);
	humidity = var1 >> 12;

	return humidity;
}

int main(void)
{
	DWORD channel;
	DWORD channels;
	FT_HANDLE ftHandle;
	ChannelConfig channelConf;
	FT_STATUS status;
	int i;
	int count;
	UCHAR address = 0x76;
	UCHAR rev;
	UCHAR reg;
	UCHAR regbuf[32];

	FT_DEVICE_LIST_INFO_NODE devList;

	channelConf.ClockRate = I2C_CLOCK_STANDARD_MODE;
	channelConf.LatencyTimer = 10;
	channelConf.Options = 0;

	Init_libMPSSE();

	printf("\nTest case 1 - I2C_GetNumChannels\n");
	status = I2C_GetNumChannels(&channels);
	printf("		I2C_GetNumChannels returned %d; channels=%d\n", status, channels);

	printf("\nTest case 2 - I2C_GetChannelInfo\n");
	for (channel = 0; channel < channels; channel++)
	{
		status = I2C_GetChannelInfo(channel, &devList);
		printf("		I2C_GetNumChannels returned %d for channel =%d\n", status, channel);
		/*print the dev info*/
		printf("		Flags=0x%x\n", devList.Flags);
		printf("		Type=0x%x\n", devList.Type);
		printf("		ID=0x%x\n", devList.ID);
		printf("		LocId=0x%x\n", devList.LocId);
		printf("		SerialNumber=%s\n", devList.SerialNumber);
		printf("		Description=%s\n", devList.Description);
		printf("		ftHandle=%p (should be zero)\n", devList.ftHandle);
	}

	if (channels > 1)
	{
		channel = 1;

		printf("\nTest case 3 - Read Register 0x%02x\n", BME280_REGISTER_DEVICE_ID);
		// test: read ID register
		status = I2C_OpenChannel(channel, &ftHandle);
		APP_CHECK_STATUS(status);
		printf("Channel %d open status=%d\n", channel, status);

		status = I2C_InitChannel(ftHandle, &channelConf);

		// Read it 10 times
		for (count = 0; count < 10; count++)
		{
			printf("%d ", count);
			status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_ID, &rev);
			APP_CHECK_STATUS(status);
			if (status == FT_OK)
			{
				if (rev == BME280_REGISTER_DEVICE_ID_EXPECT)
				{
					printf("Success. Device ID register 0x%02x is 0x%02x\n", BME280_REGISTER_DEVICE_ID, rev);
				}
				else
				{
					printf("Wrong. Device ID register 0x%02x is 0x%02x but should be 0x%02x\n",
						BME280_REGISTER_DEVICE_ID, rev, BME280_REGISTER_DEVICE_ID_EXPECT);
				}
			}
		}

		printf("\nTest case 4 - Reset Register 0x%02x\n", BME280_REGISTER_DEVICE_RESET);
		// test: write reset register

		status = i2c_write(ftHandle, address, BME280_REGISTER_DEVICE_RESET, BME280_REGISTER_DEVICE_RESET_VALUE);
		APP_CHECK_STATUS(status);
#ifndef _WIN32
		usleep(2 * 1000);
#else // _WIN32
		Sleep(2);
#endif // _WIN32
		status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_RESET, &reg);
		if (status == FT_OK)
		{
			printf("Success. Reset register 0x%02x returned 0x%02x\n", BME280_REGISTER_DEVICE_RESET, reg);
		}

		printf("\nTest case 5a - Read Temperature Compensation Registers 0x%02x\n", BME280_REGISTER_DIG_T1_LSB);

		for (i = 0; i < 6; i++)
		{
			status = i2c_read(ftHandle, address, BME280_REGISTER_DIG_T1_LSB + i, &regbuf[i]);
			APP_CHECK_STATUS(status);
			if (status == FT_OK)
			{
				printf("Register 0x%02x is 0x%02x\n", BME280_REGISTER_DIG_T1_LSB + i, regbuf[i]);
			}
		}

		// calculate compensation values
		digT1 = (regbuf[1] << 8) + regbuf[0];
		digT2 = (regbuf[3] << 8) + regbuf[2];
		digT3 = (regbuf[5] << 8) + regbuf[4];

		printf("Compensation values are T1 %u, T2 %d, T3 %d\n", digT1, digT2, digT3);

		printf("\nTest case 5b - Read Pressure Compensation Registers 0x%02x\n", BME280_REGISTER_DIG_P1_LSB);

		for (i = 0; i < 18; i++)
		{
			status = i2c_read(ftHandle, address, BME280_REGISTER_DIG_P1_LSB + i, &regbuf[i]);
			APP_CHECK_STATUS(status);
			if (status == FT_OK)
			{
				printf("Register 0x%02x is 0x%02x\n", BME280_REGISTER_DIG_P1_LSB + i, regbuf[i]);
			}
		}

		// calculate compensation values
		digP1 = (regbuf[1] << 8) + regbuf[0];
		digP2 = (regbuf[3] << 8) + regbuf[2];
		digP3 = (regbuf[5] << 8) + regbuf[4];
		digP4 = (regbuf[7] << 8) + regbuf[6];
		digP5 = (regbuf[9] << 8) + regbuf[8];
		digP6 = (regbuf[11] << 8) + regbuf[10];
		digP7 = (regbuf[13] << 8) + regbuf[12];
		digP8 = (regbuf[15] << 8) + regbuf[14];
		digP9 = (regbuf[17] << 8) + regbuf[16];

		printf("Compensation values are P1 %u, P2 %d, P3 %d, P4 %d, P5 %d, P6 %d, P7 %d, P8 %d, P9 %d\n",
			digP1, digP2, digP3, digP4, digP5, digP6, digP7, digP8, digP9);

		printf("\nTest case 5c - Read Humidity Compensation Registers 0x%02x/0x%02x\n", BME280_REGISTER_DIG_H1, BME280_REGISTER_DIG_H2_LSB);

		status = i2c_read(ftHandle, address, BME280_REGISTER_DIG_H1, &regbuf[0]);
		APP_CHECK_STATUS(status);
		if (status == FT_OK)
		{
			printf("Register 0x%02x is 0x%02x\n", BME280_REGISTER_DIG_H1, regbuf[0]);
		}
		digH1 = regbuf[0];

		for (i = 0; i < 7; i++)
		{
			status = i2c_read(ftHandle, address, BME280_REGISTER_DIG_H2_LSB + i, &regbuf[i]);
			APP_CHECK_STATUS(status);
			if (status == FT_OK)
			{
				printf("Register 0x%02x is 0x%02x\n", BME280_REGISTER_DIG_H2_LSB + i, regbuf[i]);
			}
		}

		// calculate compensation values
		digH2 = (regbuf[1] << 8) + regbuf[0];
		digH3 = regbuf[2];
		digH4 = (regbuf[3] << 4) + (regbuf[4] & 0x0f);
		digH5 = (regbuf[4] >> 4) + (regbuf[5] << 4);
		digH6 = regbuf[6];

		printf("Compensation values are H1 %u, H2 %d, H3 %u, H4 %d, H5 %d, H6 %d\n", digH1, digH2, digH3, digH4, digH5, digH6);

		printf("\nTest case 6 - Start Measurement\n");

		// test: Start Measurements

		// Write config Register 		
		status = i2c_write(ftHandle, address, BME280_REGISTER_DEVICE_CONFIG, BME280_REGISTER_DEVICE_CONFIG_VALUE);
		APP_CHECK_STATUS(status);

		// Write Mode bits in ctrl_meas Register
		// MODE_SLEEP and temperature and pressure skipped
		status = i2c_write(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_MEAS, BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_OFF);
		APP_CHECK_STATUS(status);

		// Read back value from ctrl_meas register
		status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_MEAS, &reg);
		APP_CHECK_STATUS(status);
		if (status == FT_OK)
		{
			printf("Success. Control and Measurement register 0x%02x returned 0x%02x cf 0x%02x\n", BME280_REGISTER_DEVICE_CTRL_MEAS, reg, BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_OFF);
		}

		// Turn on humidity sampling in ctrl_hum Register
		// Humidity oversampling on
		status = i2c_write(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_HUM, BME280_REGISTER_DEVICE_CTRL_HUM_OVERSAMPLE_MAX);
		APP_CHECK_STATUS(status);

		// Read back value from ctrl_meas register
		status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_HUM, &reg);
		APP_CHECK_STATUS(status);
		if (status == FT_OK)
		{
			printf("Success. Control and Measurement register 0x%02x returned 0x%02x cf 0x%02x\n", BME280_REGISTER_DEVICE_CTRL_HUM, reg, BME280_REGISTER_DEVICE_CTRL_HUM_OVERSAMPLE_MAX);
		}

		// MODE_SLEEP and temperature and pressure oversampling
		status = i2c_write(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_MEAS, BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_ON);
		APP_CHECK_STATUS(status);

		// Read back value from ctrl_meas register
		status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_MEAS, &reg);
		APP_CHECK_STATUS(status);
		if (status == FT_OK)
		{
			printf("Success. Control and Measurement register 0x%02x returned 0x%02x cf 0x%02x\n", BME280_REGISTER_DEVICE_CTRL_MEAS, reg, BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_ON);
		}

		// MODE_NORMAL and temperature and pressure oversampling
		status = i2c_write(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_MEAS, BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_NORMAL);
		APP_CHECK_STATUS(status);

		// Read back value from ctrl_meas register
		status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_CTRL_MEAS, &reg);
		APP_CHECK_STATUS(status);
		if (status == FT_OK)
		{
			printf("Success. Control and Measurement register 0x%02x returned 0x%02x cf 0x%02x\n", BME280_REGISTER_DEVICE_CTRL_MEAS, reg, BME280_REGISTER_DEVICE_CTRL_MEAS_VALUE_NORMAL);
		}

		status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_ID, &rev);
		APP_CHECK_STATUS(status);
		if (status == FT_OK)
		{
			printf("Success. Device ID register 0x%02x returned 0x%02x\n", BME280_REGISTER_DEVICE_ID, rev);
		}

		printf("\nTest case 6 - Read Temperature, Pressure and Humidity\n");
		// test: read temperature registers

		// Sleep for 1 second to allow oversampling to occur
#ifndef _WIN32
		usleep(1000 * 1000);
#else // _WIN32
		Sleep(1000);
#endif // _WIN32

		for (count = 0; count < 100; count++)
		{
			int temperature_raw;
			int temperature;
			int pressure_raw;
			int pressure;
			int humidity_raw;
			int humidity;

			for (i = 0; i < 8; i++)
			{
				status = i2c_read(ftHandle, address, BME280_REGISTER_DEVICE_PRESSURE_MSB + i, &regbuf[i]);
				APP_CHECK_STATUS(status);
				if (status == FT_OK)
				{
					if (count == 0)
					{
						printf("Register 0x%02x is 0x%02x\n", BME280_REGISTER_DEVICE_PRESSURE_MSB + i, regbuf[i]);
					}
				}
			}

			APP_CHECK_STATUS(status);
			/* Temperature is MSB in bits 19:12, LSB in bits 11:4, XLSB in bits 3:0 */
			/* The XLSB is not calculated as filtering is turned off and resolution is 16 bits. */
			temperature_raw = ((regbuf[3] << 12) + (regbuf[4] << 4));
			pressure_raw = ((regbuf[0] << 12) + (regbuf[1] << 4));
			humidity_raw = ((regbuf[6] << 8) + (regbuf[7]));

			if (count == 0)
			{
				printf("Raw temperature is %d, pressure is %d, humidity is %d\n", temperature_raw, pressure_raw, humidity_raw);
			}

			temperature = compensate_T(temperature_raw);
			pressure = compensate_P(pressure_raw);
			humidity = compensate_H(humidity_raw);
			printf("Temperature is %d.%d C, pressure is %d Pa, humidity is %d.%d %% rH\r",
				temperature / 100, temperature % 100, pressure, humidity / 1024, ((humidity % 1024) * 100) / 1024);

#ifndef _WIN32
			usleep(1000 * 1000);
#else // _WIN32
			Sleep(1000);
#endif // _WIN32
			printf("\n");
		}

		status = I2C_CloseChannel(ftHandle);
		APP_CHECK_STATUS(status);
	}

	Cleanup_libMPSSE();

	return 0;
}
