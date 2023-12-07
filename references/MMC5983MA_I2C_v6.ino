// ---------------------------------------------------------------------------------------------------- //
// Robert's Smorgasbord 2023                                                                            //
// https://robertssmorgasbord.net                                                                       //
// https://www.youtube.com/@robertssmorgasbord                                                          //
// MEMSIC MMC5983MA 3-Axis Digital Compass & Arduino MCU – The Details (6) https://youtu.be/HEKQfgcHaXQ //
// ---------------------------------------------------------------------------------------------------- //

#include <Wire.h>
#include "MMC5983MA_I2C.h"

MMC5983MA_I2C mmc5983ma(&Wire);

void setup()
{
   Serial.begin(1000000);
   while(!Serial);
   
   Wire.begin();
   Wire.setClock(400000); // 400kHz I2C clock

   if (!mmc5983ma.begin()) serialPrintError(mmc5983ma, "begin()");

   Serial.println("Offset Fix"); 
}

/*
void loop()
{
   delay(90);

   if (!mmc5983ma.takeMagneticFieldMeasurement()) serialPrintError(mmc5983ma, "takeMagneticFieldMeasurement()");

   delay(10);

   while (!mmc5983ma.readAxesOutput()) 
   {
      //serialPrintError(mmc5983ma, "readAxesOutput()");

      delay(1);
   }

   Serial.print(mmc5983ma.rawAxisOutput18BitGauss(0) * 1000.0, 4);
   Serial.print("\t");
   Serial.print(((float)mmc5983ma.rawAxisOutput18Bit(0) - 131071.5) * 0.0000625 * 1000.0, 4);
   Serial.print("\t");
   Serial.print((float)mmc5983ma.rawAxisOutput18BitSigned(0) * 0.0000625 * 1000.0, 4);
   Serial.println("");
}
*/

void loop()
{
   int32_t raw_x_set;
   int32_t raw_x_reset;
   int32_t offset;
   
   delay(82);

   if (!mmc5983ma.setSensorMagnetization()) serialPrintError(mmc5983ma, "setSensorMagnetization()");

   delay(1);
   
   if (!mmc5983ma.takeMagneticFieldMeasurement()) serialPrintError(mmc5983ma, "takeMagneticFieldMeasurement()");

   delay(10);
   
   while (!mmc5983ma.readAxesOutput()) 
   {
      //serialPrintError(mmc5983ma, "readAxesOutput()");

      delay(1);
   }

   raw_x_set = (int32_t)mmc5983ma.rawAxisOutput18Bit(0);

   if (!mmc5983ma.resetSensorMagnetization()) serialPrintError(mmc5983ma, "setSensorMagnetization()");

   delay(1);
   
   if (!mmc5983ma.takeMagneticFieldMeasurement()) serialPrintError(mmc5983ma, "takeMagneticFieldMeasurement()");

   delay(10);
   
   while (!mmc5983ma.readAxesOutput()) 
   {
      //serialPrintError(mmc5983ma, "readAxesOutput()");

      delay(1);
   }

   raw_x_reset = (int32_t)mmc5983ma.rawAxisOutput18Bit(0);

   offset = (raw_x_set + raw_x_reset) / 2;
   
   Serial.print(offset);
   Serial.print("\t");
   Serial.print(131072);
   Serial.println("");
}

void serialPrintError(MMC5983MA_I2C &mmc5583ma, char *message)
{
   Serial.print("Error ");
   Serial.print(message);
   Serial.print(": ");
   Serial.println(mmc5983ma.lastError());
}
