#!/usr/bin/env python3
"""Raspberry Pi demo for the GDEM0154F61H 1.54-inch four-color panel."""

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
    draw.rectangle((0, 120, 40, 160), fill=epd.RED)
    draw_text(draw, (15, 135), "R", font16, epd.YELLOW, epd.RED)
    draw.rectangle((40, 120, 80, 160), fill=epd.YELLOW)
    draw_text(draw, (55, 135), "Y", font16, epd.RED, epd.YELLOW)
    draw.rectangle((0, 160, 40, 199), fill=epd.BLACK)
    draw_text(draw, (15, 175), "B", font16, epd.WHITE, epd.BLACK)
    draw.rectangle((40, 160, 80, 199), outline=epd.BLACK)
    draw_text(draw, (55, 175), "W", font16, epd.BLACK, epd.WHITE)

    # 2. Draw points, a circle, and a triangle.
    draw.point((15, 110), fill=epd.RED)
    draw.rectangle((43, 108, 47, 112), fill=epd.YELLOW)
    draw.rectangle((72, 107, 78, 113), fill=epd.BLACK)
    draw.ellipse((20, 60, 60, 100), outline=epd.BLACK)
    draw.polygon(((40, 60), (23, 90), (57, 90)), outline=epd.BLACK)

    # 3. Draw text and numbers.
    draw_text(draw, (60, 0), "Four color e-Paper", font16, epd.RED, epd.YELLOW)
    draw_text(draw, (70, 20), "1.54 Inch e-Paper", font16, epd.YELLOW, epd.BLACK)
    draw_text(draw, (120, 40), "你好世界", font20, epd.RED, epd.WHITE)
    draw_text(draw, (110, 60), "Hello World!", font16, epd.YELLOW, epd.BLACK)
    draw_text(draw, (160, 75), "123456", font12, epd.RED, epd.WHITE)
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
        logger.info("GDEM0154F61H Python demo")

        logger.info("Full init and clear")
        epd.init()
        epd.clear(epd.WHITE)
        epd.sleep()
        time.sleep(3)

        for filename in ("img1.bmp",):
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
