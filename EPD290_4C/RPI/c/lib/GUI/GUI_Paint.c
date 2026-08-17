#include "GUI_Paint.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

PAINT Paint;
PAINT_TIME sPaint_time;

/* Each pixel uses 2 bits, so one byte stores four pixels. Multiplying 0x55
 * (0b01010101) by color code 0..3 copies that code into all four slots. */
static UBYTE packed_color(UWORD color)
{
    UBYTE value = (UBYTE)(color & 0x03U);
    return (UBYTE)(value * 0x55U);
}

static int valid_rotation(UWORD rotate)
{
    return rotate == ROTATE_0 || rotate == ROTATE_90 ||
           rotate == ROTATE_180 || rotate == ROTATE_270;
}

static void update_logical_size(void)
{
    if (Paint.Rotate == ROTATE_0 || Paint.Rotate == ROTATE_180) {
        Paint.Width = Paint.WidthMemory;
        Paint.Height = Paint.HeightMemory;
    } else {
        Paint.Width = Paint.HeightMemory;
        Paint.Height = Paint.WidthMemory;
    }
}

void Paint_NewImage(UBYTE *image, UWORD width, UWORD height, UWORD rotate, UWORD color)
{
    memset(&Paint, 0, sizeof(Paint));
    Paint.Image = image;
    Paint.WidthMemory = width;
    Paint.HeightMemory = height;
    Paint.WidthByte = (UWORD)(((uint32_t)width + 3U) / 4U);
    Paint.HeightByte = height;
    Paint.ImageSize = (size_t)Paint.WidthByte * Paint.HeightByte;
    Paint.Color = color & 0x03U;
    Paint.Scale = 4;
    Paint.Mirror = MIRROR_NONE;
    Paint.Rotate = valid_rotation(rotate) ? rotate : ROTATE_0;
    update_logical_size();
}

void Paint_SelectImage(UBYTE *image)
{
    Paint.Image = image;
}

void Paint_SetRotate(UWORD rotate)
{
    if (!valid_rotation(rotate)) {
        Debug("Paint: rotation must be 0, 90, 180 or 270\n");
        return;
    }
    Paint.Rotate = rotate;
    update_logical_size();
}

void Paint_SetMirroring(UBYTE mirror)
{
    if (mirror > MIRROR_ORIGIN) {
        Debug("Paint: invalid mirror mode %u\n", mirror);
        return;
    }
    Paint.Mirror = mirror;
}

void Paint_SetScale(UBYTE scale)
{
    if (scale != 4) {
        Debug("Paint: this project supports only 4-color (2-bit) buffers\n");
        return;
    }
    Paint.Scale = 4;
    Paint.WidthByte = (UWORD)(((uint32_t)Paint.WidthMemory + 3U) / 4U);
    Paint.ImageSize = (size_t)Paint.WidthByte * Paint.HeightMemory;
}

/*
 * Convert logical canvas coordinates (Paint.Width/Height after rotation) into
 * physical buffer coordinates (WidthMemory/HeightMemory in panel orientation).
 * For ROTATE_90, logical x becomes physical y and logical y maps to mirrored x.
 */
static int map_to_memory(UWORD logical_x, UWORD logical_y, UWORD *memory_x, UWORD *memory_y)
{
    if (logical_x >= Paint.Width || logical_y >= Paint.Height) {
        return 0;
    }

    int32_t x = logical_x;
    int32_t y = logical_y;
    int32_t mapped_x = 0;
    int32_t mapped_y = 0;

    switch (Paint.Rotate) {
    case ROTATE_0:
        mapped_x = x;
        mapped_y = y;
        break;
    case ROTATE_90:
        mapped_x = (int32_t)Paint.WidthMemory - y - 1;
        mapped_y = x;
        break;
    case ROTATE_180:
        mapped_x = (int32_t)Paint.WidthMemory - x - 1;
        mapped_y = (int32_t)Paint.HeightMemory - y - 1;
        break;
    case ROTATE_270:
        mapped_x = y;
        mapped_y = (int32_t)Paint.HeightMemory - x - 1;
        break;
    default:
        return 0;
    }

    if ((Paint.Mirror & MIRROR_HORIZONTAL) != 0) {
        mapped_x = (int32_t)Paint.WidthMemory - mapped_x - 1;
    }
    if ((Paint.Mirror & MIRROR_VERTICAL) != 0) {
        mapped_y = (int32_t)Paint.HeightMemory - mapped_y - 1;
    }

    if (mapped_x < 0 || mapped_y < 0 ||
        mapped_x >= Paint.WidthMemory || mapped_y >= Paint.HeightMemory) {
        return 0;
    }

    *memory_x = (UWORD)mapped_x;
    *memory_y = (UWORD)mapped_y;
    return 1;
}

void Paint_SetPixel(UWORD x, UWORD y, UWORD color)
{
    if (Paint.Image == NULL || Paint.Scale != 4) {
        return;
    }

    UWORD memory_x = 0;
    UWORD memory_y = 0;
    if (!map_to_memory(x, y, &memory_x, &memory_y)) {
        return;
    }

    /*
     * Locate the byte for this pixel. Each row uses WidthByte bytes and each
     * byte stores four pixels from most to least significant bits.
     */
    size_t address = (size_t)memory_y * Paint.WidthByte + memory_x / 4U;
    if (address >= Paint.ImageSize) {
        Debug("Paint: blocked framebuffer overflow at byte %lu\n", (unsigned long)address);
        return;
    }

    unsigned shift = (unsigned)(3U - (memory_x % 4U)) * 2U;
    UBYTE mask = (UBYTE)(0x03U << shift);
    UBYTE value = (UBYTE)((color & 0x03U) << shift);
    Paint.Image[address] = (UBYTE)((Paint.Image[address] & (UBYTE)~mask) | value);
}

void Paint_Clear(UWORD color)
{
    if (Paint.Image != NULL) {
        memset(Paint.Image, packed_color(color), Paint.ImageSize);
    }
}

static void set_pixel_signed(int32_t x, int32_t y, UWORD color)
{
    if (x >= 0 && y >= 0 && x < Paint.Width && y < Paint.Height) {
        Paint_SetPixel((UWORD)x, (UWORD)y, color);
    }
}

void Paint_ClearWindows(UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end, UWORD color)
{
    UWORD right = x_end < Paint.Width ? x_end : Paint.Width;
    UWORD bottom = y_end < Paint.Height ? y_end : Paint.Height;
    for (UWORD y = y_start; y < bottom; ++y) {
        for (UWORD x = x_start; x < right; ++x) {
            Paint_SetPixel(x, y, color);
        }
    }
}

/*
 * DOT_FILL_AROUND expands around an anchor at the square center.
 * Other styles use the lower-left corner and expand toward the upper right.
 */
void Paint_DrawPoint(UWORD x, UWORD y, UWORD color, DOT_PIXEL size, DOT_STYLE style)
{
    int32_t width = (int32_t)size;
    int32_t left = x;
    int32_t top = y;

    if (style == DOT_FILL_AROUND) {
        left -= (width - 1) / 2;
        top -= (width - 1) / 2;
    } else {
        top -= width - 1;
    }

    for (int32_t dy = 0; dy < width; ++dy) {
        for (int32_t dx = 0; dx < width; ++dx) {
            set_pixel_signed(left + dx, top + dy, color);
        }
    }
}

/* Bresenham's algorithm selects each x/y step using integer error accumulation. */
void Paint_DrawLine(UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end,
                    UWORD color, DOT_PIXEL width, LINE_STYLE style)
{
    int32_t x0 = x_start;
    int32_t y0 = y_start;
    int32_t x1 = x_end;
    int32_t y1 = y_end;
    int32_t dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int32_t sx = x0 < x1 ? 1 : -1;
    int32_t dy_abs = y1 > y0 ? y1 - y0 : y0 - y1;
    int32_t dy = -dy_abs;
    int32_t sy = y0 < y1 ? 1 : -1;
    int32_t error = dx + dy;
    unsigned step = 0;

    for (;;) {
        if (style == LINE_STYLE_SOLID || (step / 3U) % 2U == 0U) {
            if (x0 >= 0 && y0 >= 0 && x0 <= UINT16_MAX && y0 <= UINT16_MAX) {
                Paint_DrawPoint((UWORD)x0, (UWORD)y0, color, width, DOT_FILL_AROUND);
            }
        }
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int32_t twice_error = 2 * error;
        if (twice_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y0 += sy;
        }
        ++step;
    }
}

static void ordered_bounds(UWORD a, UWORD b, UWORD *low, UWORD *high)
{
    if (a <= b) {
        *low = a;
        *high = b;
    } else {
        *low = b;
        *high = a;
    }
}

void Paint_DrawRectangle(UWORD x_start, UWORD y_start, UWORD x_end, UWORD y_end,
                         UWORD color, DOT_PIXEL width, DRAW_FILL fill)
{
    UWORD left, right, top, bottom;
    ordered_bounds(x_start, x_end, &left, &right);
    ordered_bounds(y_start, y_end, &top, &bottom);

    if (fill == DRAW_FILL_FULL) {
        for (uint32_t y = top; y <= bottom; ++y) {
            for (uint32_t x = left; x <= right; ++x) {
                set_pixel_signed((int32_t)x, (int32_t)y, color);
            }
        }
        return;
    }

    Paint_DrawLine(left, top, right, top, color, width, LINE_STYLE_SOLID);
    Paint_DrawLine(right, top, right, bottom, color, width, LINE_STYLE_SOLID);
    Paint_DrawLine(right, bottom, left, bottom, color, width, LINE_STYLE_SOLID);
    Paint_DrawLine(left, bottom, left, top, color, width, LINE_STYLE_SOLID);
}

/* The midpoint circle algorithm uses eight-way symmetry for each point. */
static void circle_points(int32_t cx, int32_t cy, int32_t x, int32_t y,
                          UWORD color, DOT_PIXEL width)
{
    const int32_t points[8][2] = {
        {cx + x, cy + y}, {cx - x, cy + y},
        {cx + x, cy - y}, {cx - x, cy - y},
        {cx + y, cy + x}, {cx - y, cy + x},
        {cx + y, cy - x}, {cx - y, cy - x},
    };
    for (size_t i = 0; i < 8; ++i) {
        if (points[i][0] >= 0 && points[i][1] >= 0 &&
            points[i][0] <= UINT16_MAX && points[i][1] <= UINT16_MAX) {
            Paint_DrawPoint((UWORD)points[i][0], (UWORD)points[i][1],
                            color, width, DOT_FILL_AROUND);
        }
    }
}

void Paint_DrawCircle(UWORD center_x, UWORD center_y, UWORD radius,
                      UWORD color, DOT_PIXEL width, DRAW_FILL fill)
{
    int32_t cx = center_x;
    int32_t cy = center_y;
    int32_t r = radius;

    if (fill == DRAW_FILL_FULL) {
        int64_t squared_radius = (int64_t)r * r;
        int32_t top = cy - r < 0 ? 0 : cy - r;
        int32_t bottom = cy + r >= Paint.Height ? (int32_t)Paint.Height - 1 : cy + r;
        int32_t left = cx - r < 0 ? 0 : cx - r;
        int32_t right = cx + r >= Paint.Width ? (int32_t)Paint.Width - 1 : cx + r;
        for (int32_t y = top; y <= bottom; ++y) {
            for (int32_t x = left; x <= right; ++x) {
                int64_t dx = x - cx;
                int64_t dy = y - cy;
                if (dx * dx + dy * dy <= squared_radius) {
                    set_pixel_signed(x, y, color);
                }
            }
        }
        return;
    }

    /* Outline circle: decision determines whether y moves inward. */
    int32_t x = 0;
    int32_t y = r;
    int32_t decision = 1 - r;
    while (x <= y) {
        circle_points(cx, cy, x, y, color, width);
        ++x;
        if (decision < 0) {
            decision += 2 * x + 1;
        } else {
            --y;
            decision += 2 * (x - y) + 1;
        }
    }
}

/*
 * Font glyphs are row-major 1-bit bitmaps with stride = ceil(width / 8).
 * A set bit draws the foreground; a clear bit draws the background unless
 * the background is FONT_BACKGROUND, which means transparent.
 */
static void draw_glyph(UWORD x, UWORD y, const UBYTE *bitmap,
                       UWORD width, UWORD height, UWORD foreground, UWORD background)
{
    if (bitmap == NULL) {
        return;
    }
    size_t stride = ((size_t)width + 7U) / 8U;
    for (UWORD row = 0; row < height; ++row) {
        for (UWORD column = 0; column < width; ++column) {
            UBYTE bits = bitmap[(size_t)row * stride + column / 8U];
            UWORD color = (bits & (UBYTE)(0x80U >> (column % 8U))) != 0
                              ? foreground
                              : background;
            if (color != FONT_BACKGROUND) {
                set_pixel_signed((int32_t)x + column, (int32_t)y + row, color);
            }
        }
    }
}

void Paint_DrawChar(UWORD x, UWORD y, char character, sFONT *font,
                    UWORD foreground, UWORD background)
{
    if (font == NULL || font->table == NULL) {
        return;
    }
    unsigned char code = (unsigned char)character;
    if (code < ' ' || code > '~') {
        return;
    }
    size_t stride = ((size_t)font->Width + 7U) / 8U;
    size_t glyph_size = stride * font->Height;
    const UBYTE *glyph = font->table + (size_t)(code - ' ') * glyph_size;
    draw_glyph(x, y, glyph, font->Width, font->Height, foreground, background);
}

void Paint_DrawString_EN(UWORD x, UWORD y, const char *text, sFONT *font,
                         UWORD foreground, UWORD background)
{
    if (text == NULL || font == NULL) {
        return;
    }
    UWORD origin_x = x;
    while (*text != '\0') {
        if (*text == '\n') {
            x = origin_x;
            y = (UWORD)(y + font->Height);
        } else {
            /* Keep the color order used by the original teaching example. */
            Paint_DrawChar(x, y, *text, font, background, foreground);
            x = (UWORD)(x + font->Width);
        }
        ++text;
    }
}

/* Determine the UTF-8 character length (1..4) from the leading byte. */
static size_t utf8_length(const UBYTE *text)
{
    if (text[0] <= 0x7FU) {
        return 1;
    }
    if ((text[0] & 0xE0U) == 0xC0U && text[1] != 0 && (text[1] & 0xC0U) == 0x80U) {
        return 2;
    }
    if ((text[0] & 0xF0U) == 0xE0U && text[1] != 0 && text[2] != 0 &&
        (text[1] & 0xC0U) == 0x80U && (text[2] & 0xC0U) == 0x80U) {
        return 3;
    }
    if ((text[0] & 0xF8U) == 0xF0U && text[1] != 0 && text[2] != 0 && text[3] != 0 &&
        (text[1] & 0xC0U) == 0x80U && (text[2] & 0xC0U) == 0x80U &&
        (text[3] & 0xC0U) == 0x80U) {
        return 4;
    }
    return 1;
}

/* Search the small UTF-8 glyph table linearly. */
static const CH_CN *find_glyph(const cFONT *font, const UBYTE *text, size_t length)
{
    if (length > sizeof(font->table[0].index)) {
        return NULL;
    }
    for (UWORD i = 0; i < font->size; ++i) {
        if (memcmp(font->table[i].index, text, length) == 0 &&
            (length == sizeof(font->table[i].index) || font->table[i].index[length] == 0)) {
            return &font->table[i];
        }
    }
    return NULL;
}

void Paint_DrawString_CN(UWORD x, UWORD y, const char *text, cFONT *font,
                         UWORD foreground, UWORD background)
{
    if (text == NULL || font == NULL || font->table == NULL) {
        return;
    }

    const UBYTE *cursor = (const UBYTE *)text;
    UWORD origin_x = x;
    while (*cursor != 0) {
        if (*cursor == '\n') {
            x = origin_x;
            y = (UWORD)(y + font->Height);
            ++cursor;
            continue;
        }

        size_t length = utf8_length(cursor);
        const CH_CN *glyph = find_glyph(font, cursor, length);
        if (glyph != NULL) {
            draw_glyph(x, y, glyph->matrix, font->Width, font->Height,
                       foreground, background);
        }
        x = (UWORD)(x + (length == 1 ? font->ASCII_Width : font->Width));
        cursor += length;
    }
}

void Paint_DrawNum(UWORD x, UWORD y, int32_t number, sFONT *font,
                   UWORD foreground, UWORD background)
{
    char text[16];
    (void)snprintf(text, sizeof(text), "%ld", (long)number);
    Paint_DrawString_EN(x, y, text, font, foreground, background);
}

void Paint_DrawNumDecimals(UWORD x, UWORD y, double number, sFONT *font, UWORD digits,
                           UWORD foreground, UWORD background)
{
    char text[48];
    int precision = digits > 9U ? 9 : (int)digits;
    (void)snprintf(text, sizeof(text), "%.*f", precision, number);
    Paint_DrawString_EN(x, y, text, font, foreground, background);
}

void Paint_DrawTime(UWORD x, UWORD y, PAINT_TIME *time, sFONT *font,
                    UWORD foreground, UWORD background)
{
    if (time == NULL) {
        return;
    }
    char text[16];
    (void)snprintf(text, sizeof(text), "%02u:%02u:%02u",
                   (unsigned)time->Hour, (unsigned)time->Min, (unsigned)time->Sec);
    Paint_DrawString_EN(x, y, text, font, foreground, background);
}

void Paint_DrawBitMap(const unsigned char *image_buffer)
{
    if (Paint.Image != NULL && image_buffer != NULL) {
        memcpy(Paint.Image, image_buffer, Paint.ImageSize);
    }
}
