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
    while(DEV_Digital_Read(EPD_BUSY_PIN)) {      //LOW: idle, HIGH: busy
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
    while(!DEV_Digital_Read(EPD_BUSY_PIN)) {      //LOW: idle, HIGH: busy
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
    EPD_ReadBusyH();

    EPD_SendCommand(0x4D);
    EPD_SendData(0x78);

    EPD_SendCommand(0x00);	//0x00
    EPD_SendData(0x0F);
    EPD_SendData(0x29);

    EPD_SendCommand(0x01);	//0x01
    EPD_SendData(0x07);
    EPD_SendData(0x00);

    EPD_SendCommand(0x03);	//0x03
    EPD_SendData(0x10);
    EPD_SendData(0x54);
    EPD_SendData(0x44);

    EPD_SendCommand(0x06);	//0x06
    EPD_SendData(0x0F);
    EPD_SendData(0x0A);
    EPD_SendData(0x2F);
    EPD_SendData(0x25);
    EPD_SendData(0x22);
    EPD_SendData(0x2E);
    EPD_SendData(0x21);

    EPD_SendCommand(0x41);	//TSE
    EPD_SendData(0x00);

    EPD_SendCommand(0x50);	//0x50
    EPD_SendData(0x37);

    EPD_SendCommand(0x60);	//0x60
    EPD_SendData(0x02);
    EPD_SendData(0x02);

    EPD_SendCommand(0x61); //0x61
    EPD_SendData(EPD_WIDTH/256);
    EPD_SendData(EPD_WIDTH%256);
    EPD_SendData(EPD_HEIGHT/256);
    EPD_SendData(EPD_HEIGHT%256);

    EPD_SendCommand(0x65);	//GSST
    EPD_SendData(0x00);
    EPD_SendData(0x00);
    EPD_SendData(0x00);
    EPD_SendData(0x00);

    EPD_SendCommand(0xE7);
    EPD_SendData(0x1C);

    EPD_SendCommand(0xE3);	//0xE3
    EPD_SendData(0x22);

    EPD_SendCommand(0xB4);
    EPD_SendData(0xD0);
    EPD_SendCommand(0xB5);
    EPD_SendData(0x03);

    EPD_SendCommand(0xE9);
    EPD_SendData(0x01);

    EPD_SendCommand(0x30);
    EPD_SendData(0x08);

    EPD_SendCommand(0x04);
    DEV_Delay_ms(500);
    EPD_ReadBusyH();
}

void EPD_Init_Fast(void)
{
    EPD_Reset();
    EPD_ReadBusyH();

    EPD_SendCommand(0x4D);
    EPD_SendData(0x78);

    EPD_SendCommand(0x00);	//0x00
    EPD_SendData(0x0F);
    EPD_SendData(0x29);

    EPD_SendCommand(0x01);	//0x01
    EPD_SendData(0x07);
    EPD_SendData(0x00);

    EPD_SendCommand(0x03);	//0x03
    EPD_SendData(0x10);
    EPD_SendData(0x54);
    EPD_SendData(0x44);

    EPD_SendCommand(0x06);	//0x06
    EPD_SendData(0x0F);
    EPD_SendData(0x0A);
    EPD_SendData(0x2F);
    EPD_SendData(0x25);
    EPD_SendData(0x22);
    EPD_SendData(0x2E);
    EPD_SendData(0x21);

    EPD_SendCommand(0x41);	//TSE
    EPD_SendData(0x00);

    EPD_SendCommand(0x50);	//0x50
    EPD_SendData(0x37);

    EPD_SendCommand(0x60);	//0x60
    EPD_SendData(0x02);
    EPD_SendData(0x02);

    EPD_SendCommand(0x61); //0x61
    EPD_SendData(EPD_WIDTH/256);
    EPD_SendData(EPD_WIDTH%256);
    EPD_SendData(EPD_HEIGHT/256);
    EPD_SendData(EPD_HEIGHT%256);

    EPD_SendCommand(0x65);	//GSST
    EPD_SendData(0x00);
    EPD_SendData(0x00);
    EPD_SendData(0x00);
    EPD_SendData(0x00);

    EPD_SendCommand(0xE7);
    EPD_SendData(0x1C);

    EPD_SendCommand(0xE3);	//0xE3
    EPD_SendData(0x22);

    EPD_SendCommand(0xB4);
    EPD_SendData(0xD0);
    EPD_SendCommand(0xB5);
    EPD_SendData(0x03);

    EPD_SendCommand(0xE9);
    EPD_SendData(0x01);

    EPD_SendCommand(0x30);
    EPD_SendData(0x08);

    //Fast
    EPD_SendCommand(0xe0);
    EPD_SendData(0x02);
    EPD_SendCommand(0xe6);
    EPD_SendData(90);

    EPD_SendCommand(0xa5);
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
function :	Display an image2lcd-exported buffer, remapping its palette
parameter:
    image2lcd's 4-gray-level export numbers its palette by brightness
    (white=0, yellow=1, red=2, black=3), while this panel's color codes are
    black=0, white=1, yellow=2, red=3 -- a constant +1 mod 4 offset. Each
    2-bit field is remapped before being sent.

    This matches Good Display's own Color_get() table in Display_EPD_W21.cpp,
    which applies the same 0->white, 1->yellow, 2->red, 3->black mapping to
    their converted image data.
******************************************************************************/
void EPD_Display_Image2LCD(const UBYTE *Image)
{
    UWORD Width, Height;
    Width = (EPD_WIDTH % 4 == 0)? (EPD_WIDTH / 4 ): (EPD_WIDTH / 4 + 1);
    Height = EPD_HEIGHT;

    EPD_SendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            UBYTE src = Image[i + j * Width];
            UBYTE dst = 0;
            for (UBYTE shift = 0; shift < 8; shift += 2) {
                UBYTE color = (src >> shift) & 0x03;
                color = (color + 1) & 0x03;
                dst |= color << shift;
            }
            EPD_SendData(dst);
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

void EPD_Shutdown(void)
{
    EPD_Sleep();
    DEV_Delay_ms(2000);
    DEV_Digital_Write(EPD_RST_PIN, 0);
}
