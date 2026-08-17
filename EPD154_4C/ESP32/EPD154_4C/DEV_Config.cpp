#include "DEV_Config.h"

static void configure_gpio(void)
{
    pinMode(EPD_BUSY_PIN, INPUT);
    pinMode(EPD_RST_PIN, OUTPUT);
    pinMode(EPD_DC_PIN, OUTPUT);
    pinMode(EPD_CS_PIN, OUTPUT);
    digitalWrite(EPD_CS_PIN, HIGH);
}

UBYTE DEV_Module_Init(void)
{
    configure_gpio();
    Serial.begin(115200);
    DEV_SPI_Init();
    return 0;
}

void DEV_SPI_Init(void)
{
    SPI.begin(EPD_SCK_PIN, -1, EPD_MOSI_PIN, EPD_CS_PIN);
    SPI.beginTransaction(SPISettings(EPD_SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
}

void DEV_SPI_WriteByte(UBYTE data)
{
    SPI.transfer(data);
}

void DEV_SPI_Write_nByte(const UBYTE *data, UDOUBLE length)
{
    for (UDOUBLE index = 0; index < length; ++index) {
        DEV_SPI_WriteByte(data[index]);
    }
}
