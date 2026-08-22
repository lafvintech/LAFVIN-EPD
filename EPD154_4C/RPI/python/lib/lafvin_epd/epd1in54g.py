"""GDEM0154F61H 1.54-inch, 200 x 200, four-color e-paper driver."""

from __future__ import annotations

import logging
import time

from PIL import Image

from . import epdconfig

logger = logging.getLogger(__name__.split(".")[-1])

EPD_WIDTH = 200
EPD_HEIGHT = 200
FRAME_BYTES = EPD_WIDTH * EPD_HEIGHT // 4

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
YELLOW = (255, 255, 0)
RED = (255, 0, 0)

CMD_POWER_ON = 0x04
CMD_POWER_OFF = 0x02
CMD_DATA_START = 0x10
CMD_DISPLAY_REFRESH = 0x12
CMD_DEEP_SLEEP = 0x07

BUSY_TIMEOUT_SECONDS = 120.0
BUSY_POLL_MS = 5

try:
    DITHER_FLOYDSTEINBERG = Image.Dither.FLOYDSTEINBERG
    DITHER_NONE = Image.Dither.NONE
    ROTATE_90 = Image.Transpose.ROTATE_270
    ROTATE_180 = Image.Transpose.ROTATE_180
    ROTATE_270 = Image.Transpose.ROTATE_90
except AttributeError:  # Pillow < 9.1
    DITHER_FLOYDSTEINBERG = Image.FLOYDSTEINBERG
    DITHER_NONE = Image.NONE
    ROTATE_90 = Image.ROTATE_270
    ROTATE_180 = Image.ROTATE_180
    ROTATE_270 = Image.ROTATE_90


class EPD:
    """High-level driver for the Good Display GDEM0154F61H panel."""

    width = EPD_WIDTH
    height = EPD_HEIGHT
    BLACK = BLACK
    WHITE = WHITE
    YELLOW = YELLOW
    RED = RED

    def __init__(self, hardware=epdconfig):
        self._hardware = hardware
        self.reset_pin = hardware.RST_PIN
        self.dc_pin = hardware.DC_PIN
        self.busy_pin = hardware.BUSY_PIN

    def reset(self):
        """Use the 2 ms reset pulse verified with the LAFVIN adapter board."""
        self._hardware.digital_write(self.reset_pin, 1)
        self._hardware.delay_ms(200)
        self._hardware.digital_write(self.reset_pin, 0)
        self._hardware.delay_ms(2)
        self._hardware.digital_write(self.reset_pin, 1)
        self._hardware.delay_ms(200)

    def send_command(self, command):
        self._hardware.digital_write(self.dc_pin, 0)
        self._hardware.spi_write(bytes((command & 0xFF,)))

    def send_data(self, data):
        payload = bytes((data & 0xFF,)) if isinstance(data, int) else bytes(data)
        self._hardware.digital_write(self.dc_pin, 1)
        self._hardware.spi_write(payload)

    def wait_until_ready(self, timeout=BUSY_TIMEOUT_SECONDS):
        """Wait for BUSY high (low means busy on this panel)."""
        logger.debug("e-Paper busy")
        self._hardware.delay_ms(100)
        deadline = time.monotonic() + timeout
        while self._hardware.digital_read(self.busy_pin) == 0:
            if time.monotonic() >= deadline:
                raise TimeoutError(
                    f"e-Paper BUSY timed out after {timeout:g} seconds"
                )
            self._hardware.delay_ms(BUSY_POLL_MS)
        logger.debug("e-Paper busy release")

    def init(self):
        """Initialize using the verified GDEM0154F61H full-refresh sequence."""
        self._hardware.module_init()
        self.reset()
        self.send_command(0xE9)
        self.send_data(0x01)
        self.send_command(CMD_POWER_ON)
        self.wait_until_ready()
        return 0

    def init_fast(self):
        """Initialize using the verified GDEM0154F61H 12-second sequence."""
        self._hardware.module_init()
        self.reset()

        self.send_command(0xE9)
        self.send_data(0x01)
        self.send_command(0xEF)
        self.send_data(0x01)
        self.send_command(0xF6)
        self.send_data(0x24)
        self.send_command(0xEF)
        self.send_data(0x00)
        self.send_command(0xE0)
        self.send_data(0x02)
        self.send_command(0xE6)
        self.send_data(92)
        self.send_command(0xA5)
        self.wait_until_ready()

        self.send_command(CMD_POWER_ON)
        self.wait_until_ready()
        return 0

    def _turn_on_display(self):
        self.send_command(CMD_DISPLAY_REFRESH)
        self.send_data(0x00)
        self.wait_until_ready()

    @staticmethod
    def _palette_image():
        palette = Image.new("P", (1, 1))
        colors = [
            0, 0, 0,
            255, 255, 255,
            255, 255, 0,
            255, 0, 0,
        ]
        colors.extend([0, 0, 0] * 252)
        palette.putpalette(colors)
        return palette

    @staticmethod
    def _flatten_to_white(image):
        if image.mode in ("RGBA", "LA") or "transparency" in image.info:
            rgba = image.convert("RGBA")
            flattened = Image.new("RGB", rgba.size, WHITE)
            flattened.paste(rgba, mask=rgba.getchannel("A"))
            return flattened
        return image.convert("RGB")

    def getbuffer(self, image, rotation=0, dither=False):
        """Convert a 200x200 Pillow image into a native 10,000-byte frame."""
        if image.size != (self.width, self.height):
            raise ValueError(
                f"invalid image size {image.width}x{image.height}; expected "
                f"{self.width}x{self.height}"
            )
        rotations = {0: None, 90: ROTATE_90, 180: ROTATE_180, 270: ROTATE_270}
        if rotation not in rotations:
            raise ValueError("rotation must be 0, 90, 180 or 270 degrees")

        image_temp = self._flatten_to_white(image)
        if rotations[rotation] is not None:
            image_temp = image_temp.transpose(rotations[rotation])

        dither_mode = DITHER_FLOYDSTEINBERG if dither else DITHER_NONE
        image_4color = image_temp.quantize(
            palette=self._palette_image(), dither=dither_mode
        )
        color_codes = bytearray(image_4color.tobytes())
        frame = bytearray(FRAME_BYTES)
        for source in range(0, len(color_codes), 4):
            pixels = [
                value if value < 4 else 0
                for value in color_codes[source : source + 4]
            ]
            frame[source // 4] = (
                (pixels[0] << 6)
                | (pixels[1] << 4)
                | (pixels[2] << 2)
                | pixels[3]
            )
        return frame

    def display(self, image, rotation=0, dither=False):
        """Display a Pillow image or an already-packed native frame."""
        if isinstance(image, Image.Image):
            frame = self.getbuffer(image, rotation=rotation, dither=dither)
        else:
            frame = bytes(image)
            if rotation:
                raise ValueError("rotation is only supported for Pillow images")
        if len(frame) != FRAME_BYTES:
            raise ValueError(
                f"display buffer must contain exactly {FRAME_BYTES} bytes"
            )

        self.send_command(CMD_DATA_START)
        self.send_data(frame)
        self._turn_on_display()

    @staticmethod
    def _packed_clear_color(color):
        rgb_to_code = {BLACK: 0, WHITE: 1, YELLOW: 2, RED: 3}
        if isinstance(color, (tuple, list)):
            try:
                code = rgb_to_code[tuple(color)]
            except KeyError as exc:
                raise ValueError(
                    "clear color must be BLACK, WHITE, YELLOW or RED"
                ) from exc
            return code * 0x55
        if isinstance(color, int):
            if 0 <= color <= 3:
                return color * 0x55
            if color in (0x55, 0xAA, 0xFF):
                return color
        raise ValueError(
            "clear color must be a four-color RGB value, code 0..3, "
            "or packed byte 0x55/0xAA/0xFF"
        )

    def clear(self, color=WHITE):
        packed = self._packed_clear_color(color)
        self.send_command(CMD_DATA_START)
        self.send_data(bytes((packed,)) * FRAME_BYTES)
        self._turn_on_display()

    def Clear(self, color=1):
        self.clear(color)

    def _enter_deep_sleep(self):
        self.send_command(CMD_POWER_OFF)
        self.send_data(0x00)
        self.wait_until_ready()
        self.send_command(CMD_DEEP_SLEEP)
        self.send_data(0xA5)

    def sleep(self):
        """Power off, enter deep sleep, then release GPIO/SPI resources."""
        try:
            self._enter_deep_sleep()
            self._hardware.delay_ms(2000)
        finally:
            self.close()

    def shutdown(self):
        """Enter deep sleep, wait for settling, and assert reset low."""
        try:
            self._enter_deep_sleep()
            self._hardware.delay_ms(2000)
            self._hardware.digital_write(self.reset_pin, 0)
        finally:
            self.close()

    def close(self):
        self._hardware.module_exit()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
