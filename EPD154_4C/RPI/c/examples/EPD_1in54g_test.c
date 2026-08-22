#include "EPD_Test.h"
#include "EPD_1in54g.h"

int EPD_1in54g_test(void)
{
    printf("EPD_1IN54G_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_1IN54G_Init();
    EPD_1IN54G_Clear(EPD_1IN54G_WHITE);
    DEV_Delay_ms(2000);

    // Create a new image cache.
    UBYTE *BlackImage = (UBYTE *)malloc(EPD_1IN54G_FRAME_BYTES);
    if (BlackImage == NULL) {
        printf("Failed to apply for black memory...\r\n");
        EPD_Shutdown();
        DEV_Module_Exit();
        return -1;
    }

#if 1   // show bmp
    EPD_1IN54G_Init_Fast();
    printf("show BMP-----------------\r\n");
    Paint_NewImage(BlackImage, EPD_1IN54G_WIDTH, EPD_1IN54G_HEIGHT, 90, EPD_1IN54G_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(BlackImage);
    GUI_ReadBmp_RGB_4Color("./pic/img1.bmp", 0, 0);
    EPD_1IN54G_Display(BlackImage);
    DEV_Delay_ms(5000);
#endif

#if 1   // Drawing on the image
    // 1. Select image.
    EPD_1IN54G_Init(); // Full-refresh initialization.
    // EPD_1IN54G_Init_Fast(); // Fast-refresh initialization.
    Debug("SelectImage:BlackImage\r\n");
    Paint_NewImage(BlackImage, EPD_1IN54G_WIDTH, EPD_1IN54G_HEIGHT, 90, EPD_1IN54G_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_1IN54G_WHITE);

    // 2. Draw four color blocks and labels.
    Debug("Drawing:BlackImage\r\n");
    Paint_DrawRectangle(0, 120, 40, 160, EPD_1IN54G_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(15, 135, "R", &Font16, EPD_1IN54G_RED, EPD_1IN54G_YELLOW);
    Paint_DrawRectangle(40, 120, 80, 160, EPD_1IN54G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(55, 135, "Y", &Font16, EPD_1IN54G_YELLOW, EPD_1IN54G_RED);
    Paint_DrawRectangle(0, 160, 40, 199, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(15, 175, "B", &Font16, EPD_1IN54G_BLACK, EPD_1IN54G_WHITE);
    Paint_DrawRectangle(40, 160, 80, 199, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawString_EN(55, 175, "W", &Font16, EPD_1IN54G_WHITE, EPD_1IN54G_BLACK);

    // 3. Draw points, small squares, a circle, and a triangle.
    Paint_DrawPoint(15, 110, EPD_1IN54G_RED, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawRectangle(43, 108, 47, 112, EPD_1IN54G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawRectangle(72, 107, 78, 113, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(40, 80, 20, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawLine(40, 60, 23, 90, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(23, 90, 57, 90, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(57, 90, 40, 60, EPD_1IN54G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // 4. Draw text and numbers.
    Paint_DrawString_EN(5, 0, "Four color e-Paper", &Font16, EPD_1IN54G_YELLOW, EPD_1IN54G_RED);
    Paint_DrawString_EN(15, 20, "1.54 Inch e-Paper", &Font16, EPD_1IN54G_BLACK, EPD_1IN54G_YELLOW);
    Paint_DrawString_CN(70, 35, "你好世界", &Font24CN, EPD_1IN54G_RED, EPD_1IN54G_WHITE);
    Paint_DrawString_EN(70, 80, "Hello World!", &Font16, EPD_1IN54G_BLACK, EPD_1IN54G_YELLOW);
    Paint_DrawNum(155, 100, 123456, &Font12, EPD_1IN54G_WHITE, EPD_1IN54G_RED);

    Debug("EPD_Display\r\n");
    EPD_1IN54G_Display(BlackImage);
    EPD_1IN54G_Sleep();
    DEV_Delay_ms(5000);
#endif

    printf("Clear...\r\n");
    EPD_1IN54G_Init();
    EPD_1IN54G_Clear(EPD_1IN54G_WHITE);

    printf("Goto Sleep...\r\n");
    EPD_Shutdown();
    free(BlackImage);
    BlackImage = NULL;
    DEV_Module_Exit();

    return 0;
}
