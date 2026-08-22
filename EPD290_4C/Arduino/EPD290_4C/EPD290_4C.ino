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
    Paint_DrawRectangle(10, 30, 50, 70, EPD_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(25, 45, "R", &Font16, EPD_RED, EPD_YELLOW);
    Paint_DrawRectangle(50, 30, 90, 70, EPD_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(65, 45, "Y", &Font16, EPD_YELLOW, EPD_RED);
    Paint_DrawRectangle(10, 70, 50, 110, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(25, 85, "B", &Font16, EPD_BLACK, EPD_WHITE);
    Paint_DrawRectangle(50, 70, 90, 110, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawString_EN(65, 85, "W", &Font16, EPD_WHITE, EPD_BLACK);

    // 2. Draw points, a circle, and a triangle.
    Paint_DrawPoint(15, 120, EPD_BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(45, 120, EPD_BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(75, 120, EPD_BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawCircle(120, 100, 20, EPD_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawLine(120, 84, 106, 108, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(106, 108, 134, 108, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(134, 108, 120, 84, EPD_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // 3. Draw text and numbers.
    Paint_DrawString_EN(95, 0, "Four color e-Paper", &Font16, EPD_RED, EPD_YELLOW);
    Paint_DrawString_EN(180, 20, "2.9 Inch e-Paper", &Font12, EPD_YELLOW, EPD_BLACK);
    Paint_DrawString_CN(170, 40, "你好世界!", &Font24CN, EPD_RED, EPD_WHITE);
    Paint_DrawString_EN(210, 80, "Hello World!", &Font12, EPD_YELLOW, EPD_BLACK);
    Paint_DrawNum(250, 35, 123456, &Font12, EPD_RED, EPD_WHITE);

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
    EPD_Shutdown();

    free(image_buffer);
    Debug("Demo complete\r\n");
}

void loop(void)
{
}
