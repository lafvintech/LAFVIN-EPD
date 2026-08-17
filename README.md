# LAFVIN Four-Color E-Paper Examples

This repository contains examples for the LAFVIN 1.54-inch (200 x 200) and
2.9-inch (128 x 296) black, white, yellow, and red e-paper modules.

Supported platforms:

- Arduino
- ESP32 and ESP32-S3
- Raspberry Pi with C and Python examples

The two display sizes use the same wiring on each platform. Their panel
dimensions, initialization sequences, refresh timing, and image data remain
model-specific.

## Before wiring

Disconnect power before connecting or disconnecting the display. Connect VCC
only to the voltage specified on the module label or product page. Raspberry
Pi and ESP32 GPIO use 3.3 V logic; never apply 5 V directly to those GPIO pins.

## Arduino wiring

The default Arduino configuration is shared by both display sizes.

| E-paper pin | Arduino pin |
| --- | --- |
| VCC | Module-rated supply |
| GND | GND |
| DIN / MOSI | D11 |
| CLK / SCK | D13 |
| CS | D7 |
| DC | D6 |
| RST | D5 |
| BUSY | D4 |

Change the pin macros or `EPD_SPI_FREQUENCY` in the matching file:

- `EPD154_4C/Arduino/EPD154_4C/BoardConfig.h`
- `EPD290_4C/Arduino/EPD290_4C/BoardConfig.h`

The default SPI frequency is 4 MHz. On boards with fixed hardware SPI pins,
`SPI.begin()` still uses the board's hardware SCK and MOSI pins; changing only
the SCK or MOSI macros does not reroute that hardware peripheral.

## ESP32-S3 wiring

`EPD_BOARD_ESP32_S3` is the default profile.

| E-paper pin | ESP32-S3 GPIO |
| --- | --- |
| VCC | Module-rated supply |
| GND | GND |
| DIN / MOSI | GPIO41 |
| CLK / SCK | GPIO42 |
| CS | GPIO4 |
| DC | GPIO5 |
| RST | GPIO35 |
| BUSY | GPIO36 |

## Classic ESP32 wiring

| E-paper pin | ESP32 GPIO |
| --- | --- |
| VCC | Module-rated supply |
| GND | GND |
| DIN / MOSI | GPIO2 |
| CLK / SCK | GPIO4 |
| CS | GPIO5 |
| DC | GPIO18 |
| RST | GPIO19 |
| BUSY | GPIO21 |

To select the classic ESP32 profile, change:

```cpp
#define EPD_BOARD_PROFILE EPD_BOARD_ESP32_S3
```

to:

```cpp
#define EPD_BOARD_PROFILE EPD_BOARD_ESP32_CLASSIC
```

The ESP32 pin profiles and 4 MHz SPI frequency are defined in:

- `EPD154_4C/ESP32/EPD154_4C/BoardConfig.h`
- `EPD290_4C/ESP32/EPD290_4C/BoardConfig.h`

Individual `EPD_*_PIN` macros may also be supplied by the build configuration.

## Raspberry Pi wiring

GPIO numbers use BCM numbering. Both the C and Python examples use SPI0 CE0.

| E-paper pin | Raspberry Pi signal | Physical pin |
| --- | --- | --- |
| VCC | 3.3 V | Pin 1 or 17 |
| GND | GND | Pin 6 |
| DIN / MOSI | GPIO10 / SPI0 MOSI | Pin 19 |
| CLK / SCK | GPIO11 / SPI0 SCLK | Pin 23 |
| CS | GPIO8 / SPI0 CE0 | Pin 24 |
| DC | GPIO25 | Pin 22 |
| RST | GPIO17 | Pin 11 |
| BUSY | GPIO24 | Pin 18 |

The default SPI frequency is 4 MHz.

For the C examples, edit the pin constants, `SPI_BUS`, `SPI_CHANNEL`, or
`SPI_SPEED_HZ` in:

- `EPD154_4C/RPI/c/lib/Config/DEV_Config.c`
- `EPD290_4C/RPI/c/lib/Config/DEV_Config.c`

For the Python examples, edit the corresponding values in:

- `EPD154_4C/RPI/python/lib/lafvin_epd/epdconfig.py`
- `EPD290_4C/RPI/python/lib/lafvin_epd/epdconfig.py`

MOSI, SCLK, and hardware chip select are assigned by the selected Raspberry Pi
SPI bus and channel. To move from CE0 to CE1, update `SPI_CHANNEL` as well as
the documented CS pin value. DC, RST, and BUSY are regular GPIO assignments and
may be changed directly.

## SPI frequency summary

| Platform | 1.54-inch | 2.9-inch | Setting |
| --- | --- | --- | --- |
| Arduino | 4 MHz | 4 MHz | `EPD_SPI_FREQUENCY` |
| ESP32 / ESP32-S3 | 4 MHz | 4 MHz | `EPD_SPI_FREQUENCY` |
| Raspberry Pi C | 4 MHz | 4 MHz | `SPI_SPEED_HZ` |
| Raspberry Pi Python | 4 MHz | 4 MHz | `SPI_SPEED_HZ` |
