#ifndef EPD_DEV_CONFIG_H
#define EPD_DEV_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#include "Debug.h"

typedef uint8_t UBYTE;
typedef uint16_t UWORD;
typedef uint32_t UDOUBLE;

/* BCM GPIO numbering used by the Raspberry Pi header. */
extern int EPD_RST_PIN;
extern int EPD_DC_PIN;
extern int EPD_CS_PIN;
extern int EPD_BUSY_PIN;
extern int EPD_MOSI_PIN;
extern int EPD_SCLK_PIN;

void DEV_Digital_Write(UWORD pin, UBYTE value);
UBYTE DEV_Digital_Read(UWORD pin);
void DEV_Delay_ms(UDOUBLE milliseconds);

/* Returns 0 on success and a negative lgpio error code on failure. */
int DEV_SPI_Write(const UBYTE *data, size_t length);

/* Compatibility helpers used by existing display drivers. */
void DEV_SPI_WriteByte(UBYTE value);
void DEV_SPI_Write_nByte(const UBYTE *data, uint32_t length);

UBYTE DEV_Module_Init(void);
void DEV_Module_Exit(void);

#endif
