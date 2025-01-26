MPSSE for Windows
-----------------

FTDI developed libmpsse to extend D2XX to facilitate I2C and SPI using the
MPSSE interface on various FTDI USB devices.  We intend the APIs to behave the 
same on Windows and Linux so if you notice any differences, please contact us 
(see http://www.ftdichip.com/FTSupport.htm).

FTDI do release the source code for libmpsse but not D2XX. An up-to-date
copy of ftd2xx.dll is required to use the mibmpsse library. This will be 
installed with the driver for an FTDI device. It is necessary for the library
to be able to find and use the ftd2xx.dll file.

The library is released as a DLL and a LIB file, and accompanying source code.

Installing the MPSSE library and DLL
------------------------------------

1.  Unzip libmpsse-windows-1.0.4.zip

This unpacks the archive, creating the following directory structure:

    build
        libmpsse.lib    (statically linkable library)
        libmpsse.dll    (dynamic link library)
    include
        ftdi_spi.h      (header file for SPI API)
        ftdi_i2c.h      (header file for I2C API)
    libftd2xx
        ftd2xx.h        (header file for D2XX API)
        WinTypes.h      (standard Windows C data types)
    source              (source code to build library and dll)
        ftdi_spi.c
        ftdi_i2c.c
        ftdi_infra.c
        ftdi_mid.c
        ftdi_infra.h
        ftdi_mid.h
        ftdi_common.h
    test
        test.c          (test program for library and dll)

The pre-built lib and dll files can be used directly in a project. They are not
signed by FTDI. The source code is included to allow the libraries to be 
rebuilt and signed or integrated in another project.

Building the static library examples.
------------------------------------

1.  Compile the libmpsse.lib project in Visual Studio

2.  Compile the testlib.exe project in Visual Studio. The test.c program is
    used as the source of the executable.

This will link the testlib.exe code to the libmpsse.lib library. To use the
static library with the test.c program undefine the macro DYNAMIC_TEST macro.

Building the dynamic library examples.
-------------------------------------

1.  Compile the libmpsse.dll project in Visual Studio

2.  Compile the testdll.exe project in Visual Studio. The test.c program is
    used as the source of the executable.

This will compile the testdll.exe code. When it is run it will search for the
libmpsse.dll dynamic link library and use that. To use the dynamic link library 
with the test.c program define the macro DYNAMIC_TEST macro. This will enable
the DLL portion of the code.

