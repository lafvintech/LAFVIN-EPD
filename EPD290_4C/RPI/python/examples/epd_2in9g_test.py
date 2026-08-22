#!/usr/bin/env python3
"""Raspberry Pi demo for the LAFVIN 2.9-inch four-color panel."""

import argparse
import logging
import sys
import time
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

PROJECT_DIR = Path(__file__).resolve().parent.parent
PIC_DIR = PROJECT_DIR / "pic"
LIB_DIR = PROJECT_DIR / "lib"
sys.path.insert(0, str(LIB_DIR))

from lafvin_epd import EPD

logger = logging.getLogger(__name__)


def canvas_size(epd, rotation):
    if rotation in (0, 180):
        return epd.width, epd.height
    return epd.height, epd.width


def draw_text(draw, position, text, font, foreground, background):
    """Draw opaque text so glyphs remain visible over existing content."""
    bounds = draw.textbbox(position, text, font=font)
    draw.rectangle(bounds, fill=background)
    draw.text(position, text, font=font, fill=foreground)


def show_image(epd, path, rotation):
    logger.info("Fast display: %s", path.name)
    expected_size = canvas_size(epd, rotation)
    with Image.open(path) as source:
        if source.size != expected_size:
            raise ValueError(
                f"{path.name} is {source.width}x{source.height}; expected "
                f"{expected_size[0]}x{expected_size[1]} for rotation {rotation}"
            )
        image = source.convert("RGB")

    epd.init_fast()
    epd.display(image, rotation=rotation)
    epd.sleep()
    time.sleep(3)


def draw_demo(epd, rotation):
    image = Image.new("RGB", canvas_size(epd, rotation), epd.WHITE)
    draw = ImageDraw.Draw(image)
    font12 = ImageFont.truetype(str(PIC_DIR / "Font.ttc"), 12)
    font16 = ImageFont.truetype(str(PIC_DIR / "Font.ttc"), 16)
    font20 = ImageFont.truetype(str(PIC_DIR / "Font.ttc"), 20)

    # 1. Draw four color blocks and labels.
    draw.rectangle((10, 30, 50, 70), fill=epd.RED)
    draw_text(draw, (25, 45), "R", font16, epd.YELLOW, epd.RED)
    draw.rectangle((50, 30, 90, 70), fill=epd.YELLOW)
    draw_text(draw, (65, 45), "Y", font16, epd.RED, epd.YELLOW)
    draw.rectangle((10, 70, 50, 110), fill=epd.BLACK)
    draw_text(draw, (25, 85), "B", font16, epd.WHITE, epd.BLACK)
    draw.rectangle((50, 70, 90, 110), outline=epd.BLACK)
    draw_text(draw, (65, 85), "W", font16, epd.BLACK, epd.WHITE)

    # 2. Draw points, a circle, and a triangle.
    draw.point((15, 120), fill=epd.BLACK)
    draw.rectangle((44, 119, 45, 120), fill=epd.BLACK)
    draw.rectangle((73, 118, 75, 120), fill=epd.BLACK)
    draw.ellipse((100, 80, 140, 120), outline=epd.BLACK)
    draw.polygon(((120, 84), (106, 108), (134, 108)), outline=epd.BLACK)

    # 3. Draw text and numbers.
    draw_text(draw, (160, 2), "Four color e-Paper", font16, epd.YELLOW, epd.RED)
    draw_text(draw, (205, 25), "2.9 Inch e-Paper", font12, epd.BLACK, epd.YELLOW)
    draw_text(draw, (215, 60), "你好世界", font20, epd.RED, epd.WHITE)
    draw_text(draw, (230, 45), "Hello World!", font12, epd.BLACK, epd.YELLOW)
    draw_text(draw, (250, 85), "123456", font12, epd.WHITE, epd.RED)
    return image


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--rotation",
        type=int,
        choices=(0, 90, 180, 270),
        default=90,
        help="clockwise rotation for image and drawing pages (default: 90)",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    epd = EPD()
    try:
        logger.info("LAFVIN 2.9-inch Python demo")

        logger.info("Full init and clear")
        epd.init()
        epd.clear(epd.WHITE)
        epd.sleep()
        time.sleep(3)

        for filename in ("img1.bmp", "img2.bmp"):
            show_image(epd, PIC_DIR / filename, args.rotation)

        logger.info("Fast init and draw demo")
        epd.init_fast()
        epd.display(draw_demo(epd, args.rotation), rotation=args.rotation)
        epd.sleep()
        time.sleep(10)

        logger.info("Final full init and clear")
        epd.init()
        epd.clear(epd.WHITE)
        epd.shutdown()
        logger.info("Demo complete")
        return 0
    except KeyboardInterrupt:
        logger.info("Interrupted by user")
        return 130
    except Exception:
        logger.exception("Demo failed")
        return 1
    finally:
        epd.close()


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    raise SystemExit(main())
