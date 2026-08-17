#include <stdlib.h>

#include "Debug.h"
#include "EPD_Panel.h"
#include "GUI_Paint.h"
#include "ImageData.h"
#include "fonts.h"

static void display_sample_images(void)
{
    Debug("Fast init and display sample image(s)\r\n");
    EPD_Init_Fast();
    EPD_Display(gImage_img1);
    DEV_Delay_ms(5000);

#if EPD_SAMPLE_IMAGE_COUNT > 1
    EPD_Display(gImage_img2);
    DEV_Delay_ms(5000);
#endif

    EPD_Sleep();
}

static void draw_demo(UBYTE *image_buffer)
{
    Debug("Fast init and draw image buffer\r\n");
    EPD_Init_Fast();
    Paint_NewImage(image_buffer, EPD_WIDTH, EPD_HEIGHT, 90, EPD_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(image_buffer);
    Paint_Clear(EPD_WHITE);

    // 1. Draw four color blocks and labels.
    Paint_DrawRectangle(0, 120, 40, 160, EPD_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(15, 135, "R", &Font16, EPD_RED, EPD_YELLOW);
    Paint_DrawRectangle(40, 120, 80, 160, EPD_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(55, 135, "Y", &Font16, EPD_YELLOW, EPD_RED);
    Paint_DrawRectangle(0, 160, 40, 199, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(15, 175, "B", &Font16, EPD_BLACK, EPD_WHITE);
    Paint_DrawRectangle(40, 160, 80, 199, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawString_EN(55, 175, "W", &Font16, EPD_WHITE, EPD_BLACK);

    // 2. Draw points, a circle, and a triangle.
    Paint_DrawPoint(15, 110, EPD_RED, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(45, 110, EPD_YELLOW, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(75, 110, EPD_BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawCircle(40, 80, 20, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawLine(40, 60, 23, 90, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(23, 90, 57, 90, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(57, 90, 40, 60, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // 3. Draw text and numbers.
    Paint_DrawString_EN(90, 0, "Four color", &Font16, EPD_RED, EPD_YELLOW);
    Paint_DrawString_EN(80, 20, "1.54 Inch e-Paper", &Font12, EPD_YELLOW, EPD_BLACK);
    Paint_DrawString_CN(70, 40, "你好世界!", &Font24CN, EPD_RED, EPD_WHITE);
    Paint_DrawString_EN(110, 80, "Hello World!", &Font12, EPD_YELLOW, EPD_BLACK);
    Paint_DrawNum(150, 35, 123456, &Font12, EPD_RED, EPD_WHITE);

    EPD_Display(image_buffer);
    EPD_Sleep();
    DEV_Delay_ms(5000);
}

void setup(void)
{
    if (DEV_Module_Init() != 0) {
        Debug("Module initialization failed\r\n");
        return;
    }

    Debug(EPD_MODEL_NAME " demo\r\n");
    Debug("Full init and clear\r\n");
    EPD_Init();
    EPD_Clear(EPD_WHITE);
    EPD_Sleep();
    DEV_Delay_ms(2000);

    UBYTE *image_buffer = (UBYTE *)malloc(EPD_FRAME_BYTES);
    if (image_buffer == NULL) {
        Debug("Failed to allocate image buffer\r\n");
        return;
    }

    display_sample_images();
    draw_demo(image_buffer);

    Debug("Final full init and clear\r\n");
    EPD_Init();
    EPD_Clear(EPD_WHITE);
    EPD_Sleep();

    free(image_buffer);
    DEV_Delay_ms(2000);
    Debug("Demo complete\r\n");
}

void loop(void)
{
}
