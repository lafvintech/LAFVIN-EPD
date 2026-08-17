#ifndef LAFVIN_EPD_BOARD_CONFIG_H
#define LAFVIN_EPD_BOARD_CONFIG_H

/**
 * Board wiring. Override these macros from the build configuration when the
 * e-paper module is connected to different Arduino-compatible pins.
 */
#ifndef EPD_SCK_PIN
#define EPD_SCK_PIN 13
#endif
#ifndef EPD_MOSI_PIN
#define EPD_MOSI_PIN 11
#endif
#ifndef EPD_CS_PIN
#define EPD_CS_PIN 7
#endif
#ifndef EPD_DC_PIN
#define EPD_DC_PIN 6
#endif
#ifndef EPD_RST_PIN
#define EPD_RST_PIN 5
#endif
#ifndef EPD_BUSY_PIN
#define EPD_BUSY_PIN 4
#endif

#ifndef EPD_SPI_FREQUENCY
#define EPD_SPI_FREQUENCY 4000000UL
#endif

#endif
