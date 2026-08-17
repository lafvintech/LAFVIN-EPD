#!/usr/bin/env python3
"""Prepare a PNG/JPG image as a BMP for a supported four-color EPD.

The Raspberry Pi demos read an uncompressed 24-bit BMP. Merely saving a
full-resolution PNG as BMP leaves resizing and color reduction to the simple
runtime decoder. This tool does the important work beforehand:

1. resize to the selected panel's pixel grid;
2. sharpen after downscaling;
3. quantize to black, white, yellow, and red;
4. save an uncompressed 24-bit BMP with dithering disabled.

The preprocessing matches tools/img2epd.py up to the point where that tool
rotates and packs pixels into a C array.

Usage:
  python png2bmp.py img1.png --panel 2.9 -o img1.bmp
  python png2bmp.py logo.png --panel 1.54 -o logo.bmp

Requires Pillow:
  python -m pip install Pillow
"""

import argparse
import struct
import sys
import tempfile
from pathlib import Path

try:
    from PIL import Image, ImageFilter
except ImportError:
    sys.exit("Pillow is required. Install it with: python -m pip install Pillow")


PANELS = {
    # The 2.9-inch Raspberry Pi GUI works on its 296x128 landscape canvas.
    "2.9": {
        "width": 296,
        "height": 128,
        "description": "2.9-inch 296x128",
    },
    "1.54": {
        "width": 200,
        "height": 200,
        "description": "1.54-inch 200x200",
    },
}

PALETTE = [
    (0, 0, 0),        # black
    (255, 255, 255),  # white
    (255, 255, 0),    # yellow
    (255, 0, 0),      # red
]


def build_quant_image():
    """Return a Pillow palette image containing the four panel colors."""
    palette_image = Image.new("P", (1, 1))
    entries = []
    for rgb in PALETTE:
        entries.extend(rgb)
    entries.extend((0, 0, 0) * (256 - len(PALETTE)))
    palette_image.putpalette(entries)
    return palette_image


def flatten_to_white(image):
    """Convert transparency to white instead of Pillow's default black."""
    if image.mode in ("RGBA", "LA") or "transparency" in image.info:
        rgba = image.convert("RGBA")
        flattened = Image.new("RGB", rgba.size, (255, 255, 255))
        flattened.paste(rgba, mask=rgba.getchannel("A"))
        return flattened
    return image.convert("RGB")


def prepare_image(image, width, height, rotate, sharpen):
    """Resize, sharpen, and quantize with the same policy as img2epd.py."""
    image = flatten_to_white(image)
    if rotate:
        image = image.rotate(-rotate, expand=True)

    try:
        resample = Image.Resampling.LANCZOS
        dither_none = Image.Dither.NONE
    except AttributeError:  # Pillow < 9.1
        resample = Image.LANCZOS
        dither_none = Image.NONE

    image = image.resize((width, height), resample)
    if sharpen > 0:
        image = image.filter(
            ImageFilter.UnsharpMask(
                radius=1.0,
                percent=int(sharpen * 100),
                threshold=2,
            )
        )

    quantized = image.quantize(
        palette=build_quant_image(), dither=dither_none)
    counts = [0, 0, 0, 0]
    for count, palette_index in quantized.getcolors(
            maxcolors=width * height):
        if palette_index < len(counts):
            counts[palette_index] = count
    return quantized.convert("RGB"), counts


def validate_bmp(path, expected_width, expected_height):
    """Verify the exact BMP properties required by the Raspberry Pi loader."""
    with path.open("rb") as bmp_file:
        header = bmp_file.read(54)

    if len(header) != 54 or header[:2] != b"BM":
        raise ValueError("output is not a Windows BMP")

    width = struct.unpack_from("<i", header, 18)[0]
    height = struct.unpack_from("<i", header, 22)[0]
    planes = struct.unpack_from("<H", header, 26)[0]
    bits_per_pixel = struct.unpack_from("<H", header, 28)[0]
    compression = struct.unpack_from("<I", header, 30)[0]
    if (width != expected_width or abs(height) != expected_height or
            planes != 1 or
            bits_per_pixel != 24 or compression != 0):
        raise ValueError(
            f"expected {expected_width}x{expected_height}, 24-bit, "
            "uncompressed BMP; got "
            f"{width}x{height}, {bits_per_pixel}-bit, compression={compression}"
        )


def output_path_for(input_path, requested_output):
    if requested_output:
        output_path = Path(requested_output)
        if output_path.suffix.lower() != ".bmp":
            raise ValueError("output filename must end with .bmp")
        return output_path
    return input_path.with_suffix(".bmp")


def main():
    parser = argparse.ArgumentParser(
        description=(
            "Convert an image to a four-color, 24-bit BMP for a supported "
            "LAFVIN Raspberry Pi e-paper demo."
        )
    )
    parser.add_argument("image", help="source PNG/JPG/BMP image")
    parser.add_argument(
        "--panel",
        choices=PANELS,
        default="2.9",
        help="target panel: 2.9=296x128 or 1.54=200x200 (default: 2.9)",
    )
    parser.add_argument("-o", "--output", help="output .bmp path")
    parser.add_argument(
        "--rotate",
        type=int,
        default=0,
        choices=(0, 90, 180, 270),
        help="rotate the source clockwise before resizing (default: 0)",
    )
    parser.add_argument(
        "--sharpen",
        type=float,
        default=1.5,
        help="unsharp-mask strength; 0 disables sharpening (default: 1.5)",
    )
    args = parser.parse_args()

    panel = PANELS[args.panel]
    width = panel["width"]
    height = panel["height"]

    input_path = Path(args.image)
    if not input_path.is_file():
        parser.error(f"input image does not exist: {input_path}")
    if args.sharpen < 0:
        parser.error("--sharpen must be zero or greater")

    try:
        output_path = output_path_for(input_path, args.output)
    except ValueError as error:
        parser.error(str(error))
    if input_path.resolve() == output_path.resolve():
        parser.error("input and output paths must be different")

    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = None
    try:
        with Image.open(input_path) as source:
            bmp_image, counts = prepare_image(
                source, width, height, args.rotate, args.sharpen)
        with tempfile.NamedTemporaryFile(
                prefix=f".{output_path.stem}-",
                suffix=".bmp",
                dir=output_path.parent,
                delete=False) as temporary_file:
            temporary_path = Path(temporary_file.name)
        bmp_image.save(temporary_path, format="BMP")
        validate_bmp(temporary_path, width, height)
        temporary_path.replace(output_path)
        temporary_path = None
    except (OSError, ValueError) as error:
        if temporary_path is not None and temporary_path.exists():
            temporary_path.unlink()
        parser.error(str(error))

    print(f"wrote {output_path}  ({panel['description']}, 24-bit BMP)")
    print(
        "palette pixels: "
        f"black={counts[0]} white={counts[1]} "
        f"yellow={counts[2]} red={counts[3]}"
    )
    print("Raspberry Pi: GUI_ReadBmp_RGB_4Color(\"<file>.bmp\", 0, 0)")


if __name__ == "__main__":
    main()
