#include "EPD_Panel.h"
#include "Debug.h"
#include "time.h"


/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(200);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void EPD_ReadBusy(void)
{
    Debug("e-Paper busy\r\n");
    DEV_Delay_ms(100);
    const unsigned long start = millis();
    while(DEV_Digital_Read(EPD_BUSY_PIN) == 1) {      //LOW: idle, HIGH: busy
        if (millis() - start >= 120000UL) {
            Debug("ERROR: BUSY stayed HIGH for 120 seconds\r\n");
            while (1) {
                DEV_Delay_ms(1000);
            }
        }
        DEV_Delay_ms(100);
    }
    Debug("e-Paper busy release\r\n");
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
static void EPD_ReadBusyH(void)
{
    Debug("e-Paper busy H\r\n");
    DEV_Delay_ms(100);
    const unsigned long start = millis();
    while(!DEV_Digital_Read(EPD_BUSY_PIN)) {      //LOW: idle, HIGH: busy
        if (millis() - start >= 120000UL) {
            Debug("ERROR: BUSY stayed LOW for 120 seconds\r\n");
            while (1) {
                DEV_Delay_ms(1000);
            }
        }
        DEV_Delay_ms(5);
    }
    Debug("e-Paper busy H release\r\n");
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_TurnOnDisplay(void)
{
    EPD_SendCommand(0x12); // DISPLAY_REFRESH
    EPD_SendData(0x00);
    EPD_ReadBusyH();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_Init(void)
{
    EPD_Reset();

    // Good Display GDEM0154F61H full-refresh initialization sequence.
    EPD_SendCommand(0xE9);
    EPD_SendData(0x01);

    EPD_SendCommand(0x04);
    EPD_ReadBusyH();
}

void EPD_Init_Fast(void)
{
    EPD_Reset();

    // Good Display GDEM0154F61H 12-second fast-refresh sequence.
    EPD_SendCommand(0xE9);
    EPD_SendData(0x01);

    EPD_SendCommand(0xEF);
    EPD_SendData(0x01);

    EPD_SendCommand(0xF6);
    EPD_SendData(0x24);

    EPD_SendCommand(0xEF);
    EPD_SendData(0x00);

    EPD_SendCommand(0xE0);
    EPD_SendData(0x02);

    EPD_SendCommand(0xE6);
    EPD_SendData(92);

    EPD_SendCommand(0xA5);
    EPD_ReadBusyH();

    EPD_SendCommand(0x04);
    EPD_ReadBusyH();
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_Clear(UBYTE color)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 4 == 0)? (EPD_WIDTH / 4 ): (EPD_WIDTH / 4 + 1);
    Height = EPD_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData((color << 6) | (color << 4) | (color << 2) | color);
        }
    }

    EPD_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
******************************************************************************/
void EPD_Display(const UBYTE *Image)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 4 == 0)? (EPD_WIDTH / 4 ): (EPD_WIDTH / 4 + 1);
    Height = EPD_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_SendData(Image[i + j * Width]);
        }
    }

    EPD_TurnOnDisplay();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_Sleep(void)
{
    EPD_SendCommand(0x02); // POWER_OFF
    EPD_SendData(0X00);
    EPD_ReadBusyH();
    EPD_SendCommand(0x07); // DEEP_SLEEP
    EPD_SendData(0XA5);
}
