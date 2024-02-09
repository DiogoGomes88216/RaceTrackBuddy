/* 
   MPU9250_Master_SPI.cpp: SPI support for MPU9250 master class

   Copyright (C) 2018 Simon D. Levy

   This file is part of MPU.

   MPU is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   MPU is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with MPU.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MPU9250_Master_SPI.h"

#include "bcm2835.h"

// One ifdef needed to support delay() cross-platform
#if defined(ARDUINO)
#include <Arduino.h>
#elif defined(__arm__)
#include <wiringPi.h>
#else
extern void delay(uint32_t msec);
#endif

MPU9250_Master_SPI::MPU9250_Master_SPI(Ascale_t ascale, Gscale_t gscale, Mscale_t mscale, Mmode_t mmode, uint8_t sampleRateDivisor) :
    MPU9250_Master(ascale, gscale, mscale, mmode, sampleRateDivisor)
{
}

MPUIMU::Error_t MPU9250_Master_SPI::begin(void)
{
    return runTests();
}

void MPU9250_Master_SPI::readRegisters(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t * data)
{
    (void)address;
    cpspi_readRegisters(subAddress, count, data);
    if (count == 0) {
      return;
    }
    //Serial.print(F("Reading ")); 	Serial.print(count); Serial.println(F(" bytes from register."));
    byte address = 0x80 | (reg & 0x7E);		// MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
    byte index = 0;							// Index in values array.
    count--;								// One read is performed outside of the loop
    bcm2835_spi_transfer(address);
    while (index < count) {
      if (index == 0 && rxAlign) {		// Only update bit positions rxAlign..7 in values[0]
        // Create bit mask for bit positions rxAlign..7
        byte mask = 0;
        for (byte i = rxAlign; i <= 7; i++) {
      mask |= (1 << i);
        }
        // Read value and tell that we want to read the same address again.
        byte value = bcm2835_spi_transfer(address);
        // Apply mask to both current value of values[0] and the new data in value.
        values[0] = (values[index] & ~mask) | (value & mask);
      }
      else { // Normal case
        values[index] = bcm2835_spi_transfer(address);
      }
      index++;
    }			// Read the final byte. Send 0 to stop reading.
}


void MPU9250_Master_SPI::writeRegister(uint8_t address, uint8_t subAddress, uint8_t data)
{
    (void)address;
    cpspi_writeRegister(subAddress, data);
}

void MPU9250_Master_SPI::writeMPURegister(uint8_t subAddress, uint8_t data)
{
    cpspi_writeRegister(subAddress, data);
}

void MPU9250_Master_SPI::readMPURegisters(uint8_t subAddress, uint8_t count, uint8_t * dest)
{
    cpspi_readRegisters(subAddress, count, dest);
}
