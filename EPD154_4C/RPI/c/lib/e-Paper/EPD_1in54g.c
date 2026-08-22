#include "EPD_1in54g.h"

#include <stddef.h>
#include <string.h>

#include "Debug.h"

#define CMD_POWER_ON 0x04U
#define CMD_DATA_START 0x10U
#define CMD_DISPLAY_REFRESH 0x12U
#define CMD_POWER_OFF 0x02U
#define CMD_DEEP_SLEEP 0x07U

#define BUSY_TIMEOUT_MS 120000U
#define BUSY_POLL_MS 5U

/*
 *
 * SPI carries both commands and data. DC selects the transfer type:
 * DC = 0 for a command byte and DC = 1 for a data byte.
 */
typedef struct {
    UBYTE command; /* Register command. */
    UBYTE length;  /* Number of data bytes that follow: 0 or 1. */
    UBYTE data[1]; /* This panel uses at most one data byte per init command. */
} RegisterWrite;

/* This register is written immediately after reset in both refresh modes. */
static const RegisterWrite shared_init[] = {
    {0xE9, 1, {0x01}},
};

/* Fast-refresh-only registers select the waveform and refresh duration. */
static const RegisterWrite fast_init[] = {
    {0xEF, 1, {0x01}},
    {0xF6, 1, {0x24}},
    {0xEF, 1, {0x00}},
    {0xE0, 1, {0x02}},
    {0xE6, 1, {92}},
    {0xA5, 0, {0}},
};

static void reset_panel(void)
{
    /* Generate the high-low-high reset pulse required by the controller. */
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
}

static int write_bytes(UBYTE data_mode, const UBYTE *data, size_t length)
{
    /* data_mode selects command (0) or data (1); hardware SPI controls CS. */
    DEV_Digital_Write(EPD_DC_PIN, data_mode);
    return DEV_SPI_Write(data, length);
}

static int write_command(UBYTE command)
{
    return write_bytes(0, &command, 1);
}

static int write_data(const UBYTE *data, size_t length)
{
    return write_bytes(1, data, length);
}

static int write_register(const RegisterWrite *entry)
{
    if (write_command(entry->command) < 0) {
        return -1;
    }
    return entry->length == 0 ? 0 : write_data(entry->data, entry->length);
}

static int write_register_table(const RegisterWrite *table, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        if (write_register(&table[i]) < 0) {
            Debug("EPD register write failed: 0x%02X\n", table[i].command);
            return -1;
        }
    }
    return 0;
}

static int wait_busy_level(UBYTE ready_level, const char *label)
{
    /*
     * BUSY reports whether the controller has completed its internal work.
     * This panel is ready when BUSY is high at these wait points.
     * The timeout prevents wiring faults from blocking the program forever.
     */
    Debug("e-Paper busy %s\n", label);
    DEV_Delay_ms(100);

    UDOUBLE elapsed = 100;
    while (DEV_Digital_Read(EPD_BUSY_PIN) != ready_level) {
        if (elapsed >= BUSY_TIMEOUT_MS) {
            Debug("e-Paper busy timeout (%s, %u ms)\n", label, BUSY_TIMEOUT_MS);
            return -1;
        }
        DEV_Delay_ms(BUSY_POLL_MS);
        elapsed += BUSY_POLL_MS;
    }

    Debug("e-Paper busy %s release\n", label);
    return 0;
}

static int initialize_panel(int fast)
{
    reset_panel();
    /* Write registers immediately after reset; no initial BUSY wait is needed. */
    if (write_register_table(shared_init, sizeof(shared_init) / sizeof(shared_init[0])) < 0) {
        return -1;
    }

    if (fast) {
        if (write_register_table(fast_init, sizeof(fast_init) / sizeof(fast_init[0])) < 0) {
            return -1;
        }
        if (wait_busy_level(1, "H") < 0) {
            return -1;
        }
    }

    if (write_command(CMD_POWER_ON) < 0) {
        return -1;
    }
    /* Match the Python driver: full init adds no 500 ms delay after power-on. */
    return wait_busy_level(1, "H");
}

static int refresh_panel(void)
{
    /* Refresh only after the image has been written to controller RAM. */
    const UBYTE value = 0x00;
    if (write_command(CMD_DISPLAY_REFRESH) < 0 || write_data(&value, 1) < 0) {
        return -1;
    }
    return wait_busy_level(1, "H");
}

void EPD_1IN54G_Init(void)
{
    if (initialize_panel(0) < 0) {
        Debug("EPD full initialization failed\n");
    }
}

void EPD_1IN54G_Init_Fast(void)
{
    if (initialize_panel(1) < 0) {
        Debug("EPD fast initialization failed\n");
    }
}

void EPD_1IN54G_Clear(UBYTE color)
{
    /*
     * Each four-color pixel uses 2 bits, so one byte stores four pixels.
     * A white color code of 1 packs as 01 01 01 01, or 0x55.
     */
    UBYTE packed = (UBYTE)((color & 0x03U) * 0x55U);
    UBYTE chunk[256];
    memset(chunk, packed, sizeof(chunk));

    if (write_command(CMD_DATA_START) < 0) {
        return;
    }

    size_t remaining = EPD_1IN54G_FRAME_BYTES;
    while (remaining != 0) {
        size_t length = remaining < sizeof(chunk) ? remaining : sizeof(chunk);
        if (write_data(chunk, length) < 0) {
            return;
        }
        remaining -= length;
    }
    (void)refresh_panel();
}

void EPD_1IN54G_Display(const UBYTE *image)
{
    if (image == NULL) {
        Debug("EPD display skipped: image is NULL\n");
        return;
    }
    /*
     * One frame is 200 x 200 x 2 bits = 10,000 bytes.
     * Write the complete frame before calling refresh_panel().
     */
    if (write_command(CMD_DATA_START) < 0 ||
        write_data(image, EPD_1IN54G_FRAME_BYTES) < 0) {
        return;
    }
    (void)refresh_panel();
}

void EPD_1IN54G_Sleep(void)
{
    /* Power off, wait for completion, and then enter deep sleep. */
    const UBYTE zero = 0x00;
    const UBYTE sleep_key = 0xA5;

    if (write_command(CMD_POWER_OFF) < 0 || write_data(&zero, 1) < 0) {
        return;
    }
    if (wait_busy_level(1, "H") < 0) {
        return;
    }
    if (write_command(CMD_DEEP_SLEEP) < 0 || write_data(&sleep_key, 1) < 0) {
        Debug("EPD deep sleep command failed\n");
    }
}

void EPD_Shutdown(void)
{
    EPD_1IN54G_Sleep();
    DEV_Delay_ms(2000);
    DEV_Digital_Write(EPD_RST_PIN, 0);
}
