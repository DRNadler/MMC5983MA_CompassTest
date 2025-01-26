#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <dlfcn.h>	/*for dlopen() & dlsym()*/
#endif

#include "ftd2xx.h"
#include "libmpsse_i2c.h"

#define APP_CHECK_STATUS(exp) {if (exp!=FT_OK){printf(" status(0x%x) != FT_OK\n", exp);}else{;}};

#define I2C_DEVICE_ADDRESS_RTC			0x50
#define I2C_DEVICE_ADDRESS_EEPROM		0x57
#define I2C_DEVICE_BUFFER_SIZE			256
#define I2C_WRITE_COMPLETION_RETRY		10

#define START_ADDRESS_EEPROM 	0xFA
#define END_ADDRESS_EEPROM		0xFC
#define RETRY_COUNT_EEPROM		10

#ifdef DYNAMIC_TEST

/* Handle to MPSSE driver */
#ifdef _WIN32
	HANDLE hdll_mpsse;
#else
	void *hdll_mpsse;
#endif

#ifndef _WIN32
	#define GET_FUNC(libHandle, symbol)	dlsym(libHandle, symbol)
		/* Macro to check if dlsym returned correctly */
	#define CHECK_SYMBOL(exp) {if (!exp)\
			fprintf(stderr, "dlsym failed: %s\n", dlerror());};
#else // _WIN32
	#define GET_FUNC(libHandle, symbol) GetProcAddress(libHandle, symbol)
	#define CHECK_SYMBOL(exp) {if (GetLastError())\
			fprintf(stderr, "GetProcAddress failed: 0x%x\n", GetLastError());};
#endif // _WIN32

typedef FT_STATUS (*pfunc_I2C_GetNumChannels)(DWORD *numChannels);
pfunc_I2C_GetNumChannels pI2C_GetNumChannels;
#define I2C_GetNumChannels(a) pI2C_GetNumChannels((a))

typedef FT_STATUS (*pfunc_I2C_GetChannelInfo)(DWORD index, FT_DEVICE_LIST_INFO_NODE *chanInfo);
pfunc_I2C_GetChannelInfo pI2C_GetChannelInfo;
#define I2C_GetChannelInfo(a, b) pI2C_GetChannelInfo((a), (b))

typedef FT_STATUS (*pfunc_I2C_OpenChannel)(DWORD index, FT_HANDLE *handle);
pfunc_I2C_OpenChannel pI2C_OpenChannel;
#define I2C_OpenChannel(a, b) pI2C_OpenChannel((a), (b))

typedef FT_STATUS (*pfunc_I2C_CloseChannel)(FT_HANDLE handle);
pfunc_I2C_CloseChannel pI2C_CloseChannel;
#define I2C_CloseChannel(a) pI2C_CloseChannel((a))

typedef FT_STATUS (*pfunc_I2C_InitChannel)(FT_HANDLE handle, ChannelConfig *config);
pfunc_I2C_InitChannel pI2C_InitChannel;
#define I2C_InitChannel(a, b) pI2C_InitChannel((a), (b))

typedef FT_STATUS (*pfunc_I2C_DeviceRead)(FT_HANDLE handle, DWORD deviceAddress, DWORD bytesToTransfer, UCHAR *buffer, DWORD *bytesTransfered, DWORD options);
pfunc_I2C_DeviceRead pI2C_DeviceRead;
#define I2C_DeviceRead(a, b, c, d, e, f) pI2C_DeviceRead((a), (b), (c), (d), (e), (f))

typedef FT_STATUS (*pfunc_I2C_DeviceWrite)(FT_HANDLE handle, DWORD deviceAddress, DWORD bytesToTransfer, UCHAR *buffer, DWORD *bytesTransfered, DWORD options);
pfunc_I2C_DeviceWrite pI2C_DeviceWrite;
#define I2C_DeviceWrite(a, b, c, d, e, f) pI2C_DeviceWrite((a), (b), (c), (d), (e), (f))

typedef FT_STATUS (*pfunc_I2C_GetDeviceID)(FT_HANDLE handle, UCHAR deviceAddress, UCHAR* deviceID);
pfunc_I2C_GetDeviceID pI2C_GetDeviceID;
#define I2C_GetDeviceID(a, b, c) pI2C_GetDeviceID((a), (b), (c))

typedef FT_STATUS (*pfunc_Ver_libMPSSE)(LPDWORD verMPSSE, LPDWORD verD2XX);
pfunc_Ver_libMPSSE pVer_libMPSSE;
#define Ver_libMPSSE(a, b) pVer_libMPSSE((a), (b))

#endif // DYNAMIC_TEST

/* Globals */

DWORD channels;
FT_HANDLE ftHandle;
ChannelConfig channelConf;
FT_STATUS status;

UCHAR buffer[I2C_DEVICE_BUFFER_SIZE];

DWORD write_byte(UCHAR slaveAddress, UCHAR registerAddress, UCHAR data)
{
	DWORD bytesToTransfer = 0;
	DWORD bytesTransfered;
	BOOL writeComplete = 0;
	DWORD retry = 0;
	
	bytesToTransfer = 0;
	bytesTransfered = 0;
	//buffer[bytesToTransfer++] = (slaveAddress<<1);//0xAE; //EEPROM's address
	buffer[bytesToTransfer++] = registerAddress; //Byte addressed inside EEPROM's memory
	buffer[bytesToTransfer++] = data;
	status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT|I2C_TRANSFER_OPTIONS_STOP_BIT);
	APP_CHECK_STATUS(status);

	//Sleep(1);

	while((writeComplete == 0) && (retry < I2C_WRITE_COMPLETION_RETRY))
	{
		bytesToTransfer = 0;
		bytesTransfered = 0;
		//buffer[bytesToTransfer++] = (slaveAddress<<1);//0xAE;	//EEPROM's address
		buffer[bytesToTransfer++] = registerAddress; //Byte addressed inside EEPROM's memory
		status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT|I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
		if (FT_OK != status)
		{
			printf("EEPROM write cycle isn't complete\n");
		}
		if (bytesToTransfer == bytesTransfered)
		{
			writeComplete = 1;
			printf("Write done\n");
		}
		else
		{
			printf("Retrying...\n");
		}
		retry++;
	}
	return 0;
}

FT_STATUS read_byte(UCHAR slaveAddress, UCHAR registerAddress, UCHAR *data)
{
	FT_STATUS status;
	DWORD bytesToTransfer = 0;
	DWORD bytesTransfered;
	
	bytesToTransfer = 0;
	bytesTransfered = 0;
	//buffer[bytesToTransfer++]= (slaveAddress<<1); //0xAE;	//EEPROM's address
	buffer[bytesToTransfer++] = registerAddress; //Byte addressed inside EEPROM's memory
	status = I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);
#if 0
	bytesToTransfer = 0;
	bytesTransfered = 0;
	buffer[bytesToTransfer++]= (slaveAddress<<1)|0x01; //0xAF;	//EEPROM's address
	status |= I2C_DeviceWrite(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);

	bytesToTransfer = 1;
	bytesTransfered = 0;
	status |= I2C_DeviceRead(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, 0);
#else
	bytesToTransfer = 1;
	bytesTransfered = 0;
	status |= I2C_DeviceRead(ftHandle, slaveAddress, bytesToTransfer, buffer, &bytesTransfered, I2C_TRANSFER_OPTIONS_START_BIT);
#endif	
	*data = buffer[0];
	return status;
}

int main(void)
{
	DWORD channel;
	FT_DEVICE_LIST_INFO_NODE devList;
	channelConf.ClockRate = 400000;
	channelConf.LatencyTimer = 0;//10000;//0;//latency can be 0 for HS chips
	channelConf.Options = 100;
	DWORD verMPSSE, verD2XX;
	
#ifdef DYNAMIC_TEST

#ifndef _WIN32
	hdll_mpsse = dlopen("libmpsse.so", RTLD_LAZY);
	if (!hdll_mpsse) 
    { 
		fprintf(stderr, "dlopen failed: %s\n", dlerror()); 
		exit(-1);
	}
#else // _WIN32
	hdll_mpsse = LoadLibrary(L"libMPSSE.dll");
	if (!hdll_mpsse) 
    { 
		fprintf(stderr, "LoadLibrary failed 0x%x\n", GetLastError()); 
		exit(-1);
	}
#endif // _WIN32
	
	/* Setup pointers to the functions we require in the library */
	pI2C_GetNumChannels = (pfunc_I2C_GetNumChannels)GET_FUNC(hdll_mpsse, "I2C_GetNumChannels");
	CHECK_SYMBOL(pI2C_GetNumChannels);
	pI2C_GetChannelInfo = (pfunc_I2C_GetChannelInfo)GET_FUNC(hdll_mpsse, "I2C_GetChannelInfo");
	CHECK_SYMBOL(pI2C_GetChannelInfo);
	pI2C_OpenChannel = (pfunc_I2C_OpenChannel)GET_FUNC(hdll_mpsse, "I2C_OpenChannel");
	CHECK_SYMBOL(pI2C_OpenChannel);
	pI2C_CloseChannel = (pfunc_I2C_CloseChannel)GET_FUNC(hdll_mpsse, "I2C_CloseChannel");
	CHECK_SYMBOL(pI2C_CloseChannel);
	pI2C_InitChannel = (pfunc_I2C_InitChannel)GET_FUNC(hdll_mpsse, "I2C_InitChannel");
	CHECK_SYMBOL(pI2C_InitChannel);
	pI2C_DeviceRead = (pfunc_I2C_DeviceRead)GET_FUNC(hdll_mpsse, "I2C_DeviceRead");
	CHECK_SYMBOL(pI2C_DeviceRead);
	pI2C_DeviceWrite = (pfunc_I2C_DeviceWrite)GET_FUNC(hdll_mpsse, "I2C_DeviceWrite");
	CHECK_SYMBOL(pI2C_DeviceWrite);
	pI2C_GetDeviceID = (pfunc_I2C_GetDeviceID)GET_FUNC(hdll_mpsse, "I2C_GetDeviceID");
	CHECK_SYMBOL(pI2C_GetDeviceID);
	pVer_libMPSSE = (pfunc_Ver_libMPSSE)GET_FUNC(hdll_mpsse, "Ver_libMPSSE");
	CHECK_SYMBOL(pVer_libMPSSE);
	
#else // DYNAMIC_TEST

	Init_libMPSSE();

#endif // DYNAMIC_TEST

	printf("\nVersion Check\n");
	status = Ver_libMPSSE(&verMPSSE, &verD2XX);
	printf("libmpsse: %08x\n", verMPSSE);
	printf("libftd2xx: %08x\n", verD2XX);
	
	printf("\nTest case 1 - I2C_GetNumChannels\n");
	status = I2C_GetNumChannels(&channels);
	printf("		I2C_GetNumChannels returned %d; channels=%d\n", status, channels);

	printf("\nTest case 2 - I2C_GetChannelInfo\n");
	for (channel = 0; channel < channels; channel++)
	{
		status = I2C_GetChannelInfo(channel,&devList);
		printf("		I2C_GetChannelInfo returned %d for channel =%d\n", status, channel);
		/*print the dev info*/
		printf("		Flags=0x%x\n", devList.Flags); 
		printf("		Type=0x%x\n", devList.Type); 
		printf("		ID=0x%x\n", devList.ID); 
		printf("		LocId=0x%x\n", devList.LocId); 
		printf("		SerialNumber=%s\n", devList.SerialNumber); 
		printf("		Description=%s\n", devList.Description); 
		printf("		ftHandle=%p (should be zero)\n", devList.ftHandle);
	}

	printf("\nTest case 3 - Read EEPROM\n");
	for (channel = 0; channel < channels; channel++)
	{
		// test: write to EEPROM & read back

		UCHAR address;
		UCHAR data;
		FT_STATUS status;
		int i;

		status = I2C_OpenChannel(channel, &ftHandle);
		APP_CHECK_STATUS(status);
		printf("\n---------------------handle=%p status=%d\n", ftHandle, status);
		status = I2C_InitChannel(ftHandle, &channelConf);
		
		for (address = START_ADDRESS_EEPROM; address < END_ADDRESS_EEPROM; address++)
		{
			printf("writing byte at address = %d\n", address);
			write_byte(I2C_DEVICE_ADDRESS_EEPROM, address, address+1);
		}

		for (address = START_ADDRESS_EEPROM; address < END_ADDRESS_EEPROM; address++)
		{
			printf("\nReading address %u \n",(unsigned)address);
			status = read_byte(I2C_DEVICE_ADDRESS_EEPROM, address, &data);
			for (i = 0; ((i < RETRY_COUNT_EEPROM) && (FT_OK !=status)); i++)
			{
				printf("read error... retrying \n");
				status = read_byte(I2C_DEVICE_ADDRESS_EEPROM, address, &data);
			}
			printf("address %d data read=%d\n", address, data);
		}
		status = I2C_CloseChannel(ftHandle);
		APP_CHECK_STATUS(status);
	}

#ifndef DYNAMIC_TEST

	Cleanup_libMPSSE();

#endif // DYNAMIC_TEST
		
	return 0;
}
