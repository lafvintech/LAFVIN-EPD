#include "GUI_BMPfile.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "GUI_Paint.h"

#define BMP_FILE_HEADER_SIZE 14U
#define BMP_INFO_HEADER_SIZE 40U
#define BMP_COMPRESSION_RGB 0U

static uint16_t read_le16(const UBYTE *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t read_le32(const UBYTE *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

static uint32_t color_distance(UBYTE blue, UBYTE green, UBYTE red,
                               UBYTE target_blue, UBYTE target_green, UBYTE target_red)
{
    int db = (int)blue - target_blue;
    int dg = (int)green - target_green;
    int dr = (int)red - target_red;
    return (uint32_t)(db * db + dg * dg + dr * dr);
}

static UBYTE nearest_epd_color(UBYTE blue, UBYTE green, UBYTE red)
{
    static const UBYTE palette[4][3] = {
        {0, 0, 0},       /* black: B, G, R */
        {255, 255, 255}, /* white */
        {0, 255, 255},   /* yellow */
        {0, 0, 255},     /* red */
    };

    UBYTE best = 0;
    uint32_t best_distance = UINT32_MAX;
    for (UBYTE color = 0; color < 4; ++color) {
        uint32_t distance = color_distance(
            blue, green, red,
            palette[color][0], palette[color][1], palette[color][2]);
        if (distance < best_distance) {
            best_distance = distance;
            best = color;
        }
    }
    return best;
}

static UBYTE bmp_error(FILE *file, UBYTE *row, const char *path, const char *message)
{
    Debug("BMP '%s': %s\n", path != NULL ? path : "(null)", message);
    free(row);
    if (file != NULL) {
        fclose(file);
    }
    return 1;
}

UBYTE GUI_ReadBmp_RGB_4Color(const char *path, UWORD x_start, UWORD y_start)
{
    if (path == NULL || Paint.Image == NULL) {
        return bmp_error(NULL, NULL, path, "invalid argument");
    }

    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return bmp_error(NULL, NULL, path, "unable to open file");
    }

    UBYTE file_header[BMP_FILE_HEADER_SIZE];
    UBYTE info_header[BMP_INFO_HEADER_SIZE];
    if (fread(file_header, 1, sizeof(file_header), file) != sizeof(file_header) ||
        fread(info_header, 1, sizeof(info_header), file) != sizeof(info_header)) {
        return bmp_error(file, NULL, path, "truncated header");
    }

    if (read_le16(file_header) != 0x4D42U) {
        return bmp_error(file, NULL, path, "not a BMP file");
    }

    uint32_t data_offset = read_le32(file_header + 10);
    uint32_t info_size = read_le32(info_header);
    int32_t width = (int32_t)read_le32(info_header + 4);
    int32_t signed_height = (int32_t)read_le32(info_header + 8);
    uint16_t planes = read_le16(info_header + 12);
    uint16_t bits_per_pixel = read_le16(info_header + 14);
    uint32_t compression = read_le32(info_header + 16);

    if (info_size < BMP_INFO_HEADER_SIZE || data_offset < BMP_FILE_HEADER_SIZE ||
        info_size > data_offset - BMP_FILE_HEADER_SIZE ||
        width <= 0 || signed_height == 0 || signed_height == INT32_MIN ||
        planes != 1 || bits_per_pixel != 24 || compression != BMP_COMPRESSION_RGB) {
        return bmp_error(file, NULL, path, "requires an uncompressed 24-bit Windows BMP");
    }

    uint32_t height = signed_height < 0 ? (uint32_t)(-signed_height) : (uint32_t)signed_height;
    size_t pixel_bytes = (size_t)(uint32_t)width * 3U;
    if (pixel_bytes / 3U != (uint32_t)width || pixel_bytes > SIZE_MAX - 3U) {
        return bmp_error(file, NULL, path, "image row is too large");
    }
    size_t row_bytes = (pixel_bytes + 3U) & ~(size_t)3U;
    UBYTE *row = malloc(row_bytes);
    if (row == NULL) {
        return bmp_error(file, NULL, path, "out of memory");
    }

    if (fseek(file, (long)data_offset, SEEK_SET) != 0) {
        return bmp_error(file, row, path, "invalid pixel-data offset");
    }

    Debug("BMP: %ld x %lu, 24-bit\n", (long)width, (unsigned long)height);
    for (uint32_t file_y = 0; file_y < height; ++file_y) {
        if (fread(row, 1, row_bytes, file) != row_bytes) {
            return bmp_error(file, row, path, "truncated pixel data");
        }

        uint32_t image_y = signed_height < 0 ? file_y : height - file_y - 1U;
        uint32_t destination_y = (uint32_t)y_start + image_y;
        if (destination_y >= Paint.Height) {
            continue;
        }

        uint32_t visible_width = (uint32_t)width;
        if ((uint32_t)x_start >= Paint.Width) {
            visible_width = 0;
        } else if (visible_width > Paint.Width - (uint32_t)x_start) {
            visible_width = Paint.Width - (uint32_t)x_start;
        }

        for (uint32_t image_x = 0; image_x < visible_width; ++image_x) {
            const UBYTE *pixel = row + image_x * 3U;
            Paint_SetPixel((UWORD)((uint32_t)x_start + image_x),
                           (UWORD)destination_y,
                           nearest_epd_color(pixel[0], pixel[1], pixel[2]));
        }
    }

    free(row);
    fclose(file);
    return 0;
}
