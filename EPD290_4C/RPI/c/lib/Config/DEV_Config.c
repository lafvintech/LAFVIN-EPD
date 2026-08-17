#include "DEV_Config.h"

#include <errno.h>
#include <limits.h>
#include <lgpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GPIO_FLAGS 0
#define SPI_BUS 0
#define SPI_CHANNEL 0
#define SPI_SPEED_HZ 4000000
#define SPI_WRITE_CHUNK 4096U

int EPD_RST_PIN = 17;
int EPD_DC_PIN = 25;
int EPD_CS_PIN = 8;
int EPD_BUSY_PIN = 24;
int EPD_MOSI_PIN = 10;
int EPD_SCLK_PIN = 11;

static int gpio_handle = -1;
static int spi_handle = -1;

/* Raspberry Pi 5 normally uses gpiochip4; earlier models use gpiochip0.
 * Read the device-tree model string to select the most likely chip first. */
static int model_is_pi5(void)
{
    char model[128] = {0};
    FILE *file = fopen("/proc/device-tree/model", "rb");
    if (file == NULL) {
        return 0;
    }

    size_t length = fread(model, 1, sizeof(model) - 1, file);
    fclose(file);
    model[length] = '\0';
    return strstr(model, "Raspberry Pi 5") != NULL;
}

static int requested_gpiochip(void)
{
    const char *value = getenv("EPD_GPIO_CHIP");
    if (value == NULL || *value == '\0') {
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long chip = strtol(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || chip < 0 || chip > INT_MAX) {
        Debug("Ignoring invalid EPD_GPIO_CHIP=%s\n", value);
        return -1;
    }
    return (int)chip;
}

static int open_gpiochip(void)
{
    int requested = requested_gpiochip();
    if (requested >= 0) {
        int handle = lgGpiochipOpen(requested);
        if (handle >= 0) {
            Debug("Using gpiochip%d from EPD_GPIO_CHIP\n", requested);
        }
        return handle;
    }

    /* Try the most likely chip first, then fall back to the other candidate. */
    int candidates[2] = {0, 4};
    if (model_is_pi5()) {
        candidates[0] = 4;
        candidates[1] = 0;
    }

    for (size_t i = 0; i < 2; ++i) {
        int handle = lgGpiochipOpen(candidates[i]);
        if (handle >= 0) {
            Debug("Using gpiochip%d\n", candidates[i]);
            return handle;
        }
    }
    return -1;
}

static int claim_gpio(void)
{
    int result = lgGpioClaimInput(gpio_handle, GPIO_FLAGS, EPD_BUSY_PIN);
    if (result < 0) {
        Debug("Failed to claim BUSY GPIO%d: %d\n", EPD_BUSY_PIN, result);
        return result;
    }

    result = lgGpioClaimOutput(gpio_handle, GPIO_FLAGS, EPD_RST_PIN, LG_HIGH);
    if (result < 0) {
        Debug("Failed to claim RST GPIO%d: %d\n", EPD_RST_PIN, result);
        return result;
    }

    result = lgGpioClaimOutput(gpio_handle, GPIO_FLAGS, EPD_DC_PIN, LG_LOW);
    if (result < 0) {
        Debug("Failed to claim DC GPIO%d: %d\n", EPD_DC_PIN, result);
        return result;
    }

    /* CE0/GPIO8 is controlled by the kernel SPI driver. */
    return 0;
}

void DEV_Digital_Write(UWORD pin, UBYTE value)
{
    if (gpio_handle < 0) {
        return;
    }
    int result = lgGpioWrite(gpio_handle, pin, value ? LG_HIGH : LG_LOW);
    if (result < 0) {
        Debug("GPIO write failed (GPIO%u): %d\n", pin, result);
    }
}

UBYTE DEV_Digital_Read(UWORD pin)
{
    if (gpio_handle < 0) {
        return 0;
    }
    int result = lgGpioRead(gpio_handle, pin);
    if (result < 0) {
        Debug("GPIO read failed (GPIO%u): %d\n", pin, result);
        return 0;
    }
    return (UBYTE)result;
}

void DEV_Delay_ms(UDOUBLE milliseconds)
{
    lguSleep((double)milliseconds / 1000.0);
}

int DEV_SPI_Write(const UBYTE *data, size_t length)
{
    if (spi_handle < 0 || (data == NULL && length != 0)) {
        return -1;
    }

    /* Split transfers to stay below the kernel spidev transaction limit. */
    size_t offset = 0;
    while (offset < length) {
        size_t remaining = length - offset;
        int chunk = remaining > SPI_WRITE_CHUNK ? (int)SPI_WRITE_CHUNK : (int)remaining;
        int result = lgSpiWrite(spi_handle, (char *)(data + offset), chunk);
        if (result < 0) {
            Debug("SPI write failed: %d\n", result);
            return result;
        }
        if (result == 0) {
            Debug("SPI write stopped before the buffer was complete\n");
            return -1;
        }
        offset += (size_t)result;
    }
    return 0;
}

void DEV_SPI_WriteByte(UBYTE value)
{
    (void)DEV_SPI_Write(&value, 1);
}

void DEV_SPI_Write_nByte(const UBYTE *data, uint32_t length)
{
    (void)DEV_SPI_Write(data, length);
}

UBYTE DEV_Module_Init(void)
{
    /* Reuse existing GPIO/SPI handles when init() is called again. */
    if (gpio_handle >= 0 && spi_handle >= 0) {
        return 0;
    }

    gpio_handle = open_gpiochip();
    if (gpio_handle < 0) {
        Debug("Unable to open a Raspberry Pi GPIO chip\n");
        DEV_Module_Exit();
        return 1;
    }

    if (claim_gpio() < 0) {
        DEV_Module_Exit();
        return 1;
    }

    spi_handle = lgSpiOpen(SPI_BUS, SPI_CHANNEL, SPI_SPEED_HZ, 0);
    if (spi_handle < 0) {
        Debug("Unable to open SPI%d.%d: %d\n", SPI_BUS, SPI_CHANNEL, spi_handle);
        DEV_Module_Exit();
        return 1;
    }

    Debug("Raspberry Pi lgpio initialized (SPI %d Hz)\n", SPI_SPEED_HZ);
    return 0;
}

void DEV_Module_Exit(void)
{
    if (gpio_handle >= 0) {
        DEV_Digital_Write(EPD_DC_PIN, 0);
        DEV_Digital_Write(EPD_RST_PIN, 0);
    }

    if (spi_handle >= 0) {
        lgSpiClose(spi_handle);
        spi_handle = -1;
    }
    if (gpio_handle >= 0) {
        lgGpiochipClose(gpio_handle);
        gpio_handle = -1;
    }
}
