#ifndef GUI_PAINT_H
#define GUI_PAINT_H

#include <stddef.h>

#include "DEV_Config.h"
#include "../Fonts/fonts.h"

#define ROTATE_0 0U
#define ROTATE_90 90U
#define ROTATE_180 180U
#define ROTATE_270 270U

#define BLACK 0U
#define WHITE 1U
#define YELLOW 2U
#define RED 3U

#define IMAGE_BACKGROUND WHITE
#define FONT_FOREGROUND BLACK
#define FONT_BACKGROUND 0xFFU /* Transparent background sentinel. */

typedef enum {
    MIRROR_NONE = 0,
    MIRROR_HORIZONTAL = 1,
    MIRROR_VERTICAL = 2,
    MIRROR_ORIGIN = 3,
} MIRROR_IMAGE;
#define MIRROR_IMAGE_DFT MIRROR_NONE

typedef enum {
    DOT_PIXEL_1X1 = 1,
    DOT_PIXEL_2X2,
    DOT_PIXEL_3X3,
    DOT_PIXEL_4X4,
    DOT_PIXEL_5X5,
    DOT_PIXEL_6X6,
    DOT_PIXEL_7X7,
    DOT_PIXEL_8X8,
} DOT_PIXEL;
#define DOT_PIXEL_DFT DOT_PIXEL_1X1

typedef enum {
    DOT_FILL_AROUND = 1,  /* Anchor at center; expand in every direction. */
    DOT_FILL_RIGHTUP,      /* Anchor at lower left; expand toward upper right. */
} DOT_STYLE;
#define DOT_STYLE_DFT DOT_FILL_AROUND

typedef enum {
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DOTTED,
} LINE_STYLE;

typedef enum {
    DRAW_FILL_EMPTY = 0,
    DRAW_FILL_FULL,
} DRAW_FILL;

typedef struct {
    UWORD Year;
    UBYTE Month;
    UBYTE Day;
    UBYTE Hour;
    UBYTE Min;
    UBYTE Sec;
} PAINT_TIME;

typedef struct {
    UBYTE *Image;
    size_t ImageSize;
    UWORD Width;
    UWORD Height;
    UWORD WidthMemory;
    UWORD HeightMemory;
    UWORD Color;
    UWORD Rotate;
    UWORD Mirror;
    UWORD WidthByte;
    UWORD HeightByte;
    UWORD Scale;
} PAINT;

extern PAINT Paint;
extern PAINT_TIME sPaint_time;

void Paint_NewImage(UBYTE *image, UWORD width, UWORD height, UWORD rotate, UWORD color);
void Paint_SelectImage(UBYTE *image);
void Paint_SetRotate(UWORD rotate);
void Paint_SetMirroring(UBYTE mirror);
void Paint_SetScale(UBYTE scale);
void Paint_SetPixel(UWORD x, UWORD y, UWORD color);

void Paint_Clear(UWORD color);
void Paint_ClearWindows(UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end, UWORD color);

void Paint_DrawPoint(UWORD x, UWORD y, UWORD color, DOT_PIXEL size, DOT_STYLE style);
void Paint_DrawLine(UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end,
                    UWORD color, DOT_PIXEL width, LINE_STYLE style);
void Paint_DrawRectangle(UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end,
                         UWORD color, DOT_PIXEL width, DRAW_FILL fill);
void Paint_DrawCircle(UWORD x, UWORD y, UWORD radius,
                      UWORD color, DOT_PIXEL width, DRAW_FILL fill);

void Paint_DrawChar(UWORD x, UWORD y, char character, sFONT *font,
                    UWORD foreground, UWORD background);
void Paint_DrawString_EN(UWORD x, UWORD y, const char *text, sFONT *font,
                         UWORD foreground, UWORD background);
void Paint_DrawString_CN(UWORD x, UWORD y, const char *text, cFONT *font,
                         UWORD foreground, UWORD background);
void Paint_DrawNum(UWORD x, UWORD y, int32_t number, sFONT *font,
                   UWORD foreground, UWORD background);
void Paint_DrawNumDecimals(UWORD x, UWORD y, double number, sFONT *font, UWORD digits,
                           UWORD foreground, UWORD background);
void Paint_DrawTime(UWORD x, UWORD y, PAINT_TIME *time, sFONT *font,
                    UWORD foreground, UWORD background);

void Paint_DrawBitMap(const unsigned char *image_buffer);

#endif
