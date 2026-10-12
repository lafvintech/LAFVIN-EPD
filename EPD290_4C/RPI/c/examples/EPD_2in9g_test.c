#include "EPD_Test.h"
#include "EPD_2in9g.h"

int EPD_2in9g_test(void)
{
    printf("EPD_2IN9G_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return -1;
    }

    printf("e-Paper Init and Clear...\r\n");
    EPD_2IN9G_Init();
    EPD_2IN9G_Clear(EPD_2IN9G_WHITE);
    DEV_Delay_ms(2000);

    // Create a new image cache.
    UBYTE *BlackImage = (UBYTE *)malloc(EPD_2IN9G_FRAME_BYTES);
    if (BlackImage == NULL) {
        printf("Failed to apply for black memory...\r\n");
        DEV_Module_Exit();
        return -1;
    }

#if 1   // show bmp
    EPD_2IN9G_Init_Fast();
    printf("show BMP-----------------\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9G_WIDTH, EPD_2IN9G_HEIGHT, 90, EPD_2IN9G_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(BlackImage);
    GUI_ReadBmp_RGB_4Color("./pic/img1.bmp", 0, 0);
    EPD_2IN9G_Display(BlackImage);
    DEV_Delay_ms(5000);
#endif

#if 1   // show bmp
    EPD_2IN9G_Init_Fast();
    printf("show BMP-----------------\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9G_WIDTH, EPD_2IN9G_HEIGHT, 90, EPD_2IN9G_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(BlackImage);
    GUI_ReadBmp_RGB_4Color("./pic/img2.bmp", 0, 0);
    EPD_2IN9G_Display(BlackImage);
    DEV_Delay_ms(5000);
#endif

#if 1   // Drawing on the image
    // 1. Select image.
    EPD_2IN9G_Init(); // Full-refresh initialization.
    // EPD_2IN9G_Init_Fast(); // Fast-refresh initialization.
    Debug("SelectImage:BlackImage\r\n");
    Paint_NewImage(BlackImage, EPD_2IN9G_WIDTH, EPD_2IN9G_HEIGHT, 90, EPD_2IN9G_WHITE);
    Paint_SetScale(4);
    Paint_SelectImage(BlackImage);
    Paint_Clear(EPD_2IN9G_WHITE);

    // 2. Draw four color blocks and labels.
    Debug("Drawing:BlackImage\r\n");
    Paint_DrawRectangle(10, 30, 50, 70, EPD_2IN9G_RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(25, 45, "R", &Font16, EPD_2IN9G_RED, EPD_2IN9G_YELLOW);
    Paint_DrawRectangle(50, 30, 90, 70, EPD_2IN9G_YELLOW, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(65, 45, "Y", &Font16, EPD_2IN9G_YELLOW, EPD_2IN9G_RED);
    Paint_DrawRectangle(10, 70, 50, 110, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawString_EN(25, 85, "B", &Font16, EPD_2IN9G_BLACK, EPD_2IN9G_WHITE);
    Paint_DrawRectangle(50, 70, 90, 110, EPD_2IN9G_WHITE, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawString_EN(65, 85, "W", &Font16, EPD_2IN9G_WHITE, EPD_2IN9G_BLACK);
    // Draw a rectangle.
    Paint_DrawRectangle(10, 110, 90, 127, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);

    // 3. Draw points, lines, a circle, and a triangle.
    Paint_DrawPoint(15, 120, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(45, 120, EPD_2IN9G_BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(75, 120, EPD_2IN9G_BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawCircle(120, 100, 20, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawLine(120, 84, 106, 108, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(106, 108, 134, 108, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(134, 108, 120, 84, EPD_2IN9G_BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    // 4. Draw text and numbers.
    Paint_DrawString_EN(95, 0, "Four color e-Paper", &Font16, EPD_2IN9G_RED, EPD_2IN9G_YELLOW);
    Paint_DrawString_EN(180, 20, "2.9 Inch e-Paper", &Font12, EPD_2IN9G_YELLOW, EPD_2IN9G_BLACK);
    Paint_DrawString_CN(160, 40, "你好世界", &Font24CN, EPD_2IN9G_RED, EPD_2IN9G_WHITE);
    Paint_DrawString_EN(210, 80, "Hello World!", &Font12, EPD_2IN9G_YELLOW, EPD_2IN9G_BLACK);
    Paint_DrawNum(250, 35, 123456, &Font12, EPD_2IN9G_RED, EPD_2IN9G_WHITE);

    Debug("EPD_Display\r\n");
    EPD_2IN9G_Display(BlackImage);
    EPD_2IN9G_Sleep();
    DEV_Delay_ms(5000);
#endif

    printf("Clear...\r\n");
    EPD_2IN9G_Init();
    EPD_2IN9G_Clear(EPD_2IN9G_WHITE);

    printf("Goto Sleep...\r\n");
    EPD_2IN9G_Sleep();
    free(BlackImage);
    BlackImage = NULL;
    DEV_Delay_ms(2000);//important, at least 2s
    DEV_Module_Exit();

    return 0;
}
