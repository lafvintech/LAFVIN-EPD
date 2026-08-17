#ifndef LAFVIN_EPD_BOARD_CONFIG_H
#define LAFVIN_EPD_BOARD_CONFIG_H

#if !defined(ARDUINO_ARCH_ESP32)
#error "This example requires an ESP32 Arduino board package"
#endif

/**
 * Select the wiring profile used by the carrier board. Individual EPD_*_PIN
 * macros may still be overridden from the build configuration.
 */
#define EPD_BOARD_ESP32_S3 1
#define EPD_BOARD_ESP32_CLASSIC 2

#ifndef EPD_BOARD_PROFILE
#define EPD_BOARD_PROFILE EPD_BOARD_ESP32_S3
#endif

#if EPD_BOARD_PROFILE == EPD_BOARD_ESP32_S3
#ifndef EPD_SCK_PIN
#define EPD_SCK_PIN 42
#endif
#ifndef EPD_MOSI_PIN
#define EPD_MOSI_PIN 41
#endif
#ifndef EPD_CS_PIN
#define EPD_CS_PIN 4
#endif
#ifndef EPD_DC_PIN
#define EPD_DC_PIN 5
#endif
#ifndef EPD_RST_PIN
#define EPD_RST_PIN 35
#endif
#ifndef EPD_BUSY_PIN
#define EPD_BUSY_PIN 36
#endif
#elif EPD_BOARD_PROFILE == EPD_BOARD_ESP32_CLASSIC
#ifndef EPD_SCK_PIN
#define EPD_SCK_PIN 4
#endif
#ifndef EPD_MOSI_PIN
#define EPD_MOSI_PIN 2
#endif
#ifndef EPD_CS_PIN
#define EPD_CS_PIN 5
#endif
#ifndef EPD_DC_PIN
#define EPD_DC_PIN 18
#endif
#ifndef EPD_RST_PIN
#define EPD_RST_PIN 19
#endif
#ifndef EPD_BUSY_PIN
#define EPD_BUSY_PIN 21
#endif
#else
#error "Unsupported EPD_BOARD_PROFILE"
#endif

#ifndef EPD_SPI_FREQUENCY
#define EPD_SPI_FREQUENCY 4000000UL
#endif

#endif
