#ifndef LAFVIN_EPD_DEV_CONFIG_H
#define LAFVIN_EPD_DEV_CONFIG_H

#include <Arduino.h>
#include <SPI.h>
#include <stdint.h>

#include "BoardConfig.h"

#define UBYTE uint8_t
#define UWORD uint16_t
#define UDOUBLE uint32_t

#define DEV_Digital_Write(pin, value) digitalWrite((pin), (value) == 0 ? LOW : HIGH)
#define DEV_Digital_Read(pin) digitalRead(pin)
#define DEV_Delay_ms(milliseconds) delay(milliseconds)

UBYTE DEV_Module_Init(void);
void DEV_SPI_Init(void);
void DEV_SPI_WriteByte(UBYTE data);
void DEV_SPI_Write_nByte(const UBYTE *data, UDOUBLE length);

#endif
