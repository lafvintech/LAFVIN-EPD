#ifndef EPD_1IN54G_H
#define EPD_1IN54G_H

#include "DEV_Config.h"

#define EPD_1IN54G_WIDTH 200U
#define EPD_1IN54G_HEIGHT 200U
#define EPD_1IN54G_FRAME_BYTES ((EPD_1IN54G_WIDTH / 4U) * EPD_1IN54G_HEIGHT)

#define EPD_1IN54G_BLACK 0x0U
#define EPD_1IN54G_WHITE 0x1U
#define EPD_1IN54G_YELLOW 0x2U
#define EPD_1IN54G_RED 0x3U

void EPD_1IN54G_Init(void);
void EPD_1IN54G_Init_Fast(void);
void EPD_1IN54G_Clear(UBYTE color);
void EPD_1IN54G_Display(const UBYTE *image);
void EPD_1IN54G_Sleep(void);
void EPD_Shutdown(void);

#endif
