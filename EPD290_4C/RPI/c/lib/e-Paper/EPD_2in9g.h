#ifndef EPD_2IN9G_H
#define EPD_2IN9G_H

#include "DEV_Config.h"

#define EPD_2IN9G_WIDTH 128U
#define EPD_2IN9G_HEIGHT 296U
#define EPD_2IN9G_FRAME_BYTES ((EPD_2IN9G_WIDTH / 4U) * EPD_2IN9G_HEIGHT)

#define EPD_2IN9G_BLACK 0x0U
#define EPD_2IN9G_WHITE 0x1U
#define EPD_2IN9G_YELLOW 0x2U
#define EPD_2IN9G_RED 0x3U

void EPD_2IN9G_Init(void);
void EPD_2IN9G_Init_Fast(void);
void EPD_2IN9G_Clear(UBYTE color);
void EPD_2IN9G_Display(const UBYTE *image);
void EPD_2IN9G_Sleep(void);
void EPD_Shutdown(void);

#endif
