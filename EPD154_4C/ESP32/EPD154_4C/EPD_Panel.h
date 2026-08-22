#ifndef LAFVIN_EPD_PANEL_H
#define LAFVIN_EPD_PANEL_H

#include "DEV_Config.h"

#define EPD_MODEL_NAME "GDEM0154F61H"
#define EPD_WIDTH 200U
#define EPD_HEIGHT 200U
#define EPD_FRAME_BYTES ((EPD_WIDTH / 4U) * EPD_HEIGHT)

#define EPD_BLACK 0x0U
#define EPD_WHITE 0x1U
#define EPD_YELLOW 0x2U
#define EPD_RED 0x3U

void EPD_Init(void);
void EPD_Init_Fast(void);
void EPD_Clear(UBYTE color);
void EPD_Display(const UBYTE *image);
void EPD_Sleep(void);
void EPD_Shutdown(void);

#endif
