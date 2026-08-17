#ifndef GUI_BMPFILE_H
#define GUI_BMPFILE_H

#include "DEV_Config.h"

/* Load an uncompressed 24-bit BMP and map it to black, white, yellow and red. */
UBYTE GUI_ReadBmp_RGB_4Color(const char *path, UWORD x, UWORD y);

#endif
