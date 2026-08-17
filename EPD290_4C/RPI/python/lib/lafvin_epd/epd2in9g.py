"""Driver for the LAFVIN 2.9-inch, 128 x 296, four-color e-paper."""

from __future__ import annotations

import logging
import time

from PIL import Image

from . import epdconfig

logger = logging.getLogger(__name__.split(".")[-1])

EPD_WIDTH = 128
EPD_HEIGHT = 296
FRAME_BYTES = EPD_WIDTH * EPD_HEIGHT // 4

# Pillow uses RGB tuples. The controller later converts these colors to 2-bit codes.
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
YELLOW = (255, 255, 0)
RED = (255, 0, 0)

CMD_PANEL_SETTING = 0x00
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
    """High-level display driver used by the teaching example."""

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
        """Generate the high-low-high reset pulse required by the controller."""
        self._hardware.digital_write(self.reset_pin, 1)
        self._hardware.delay_ms(200)
        self._hardware.digital_write(self.reset_pin, 0)
        self._hardware.delay_ms(2)
        self._hardware.digital_write(self.reset_pin, 1)
        self._hardware.delay_ms(200)

    def send_command(self, command):
        """Send one controller command (DC = 0)."""
        self._hardware.digital_write(self.dc_pin, 0)
        self._hardware.spi_write(bytes((command & 0xFF,)))

    def send_data(self, data):
        """Send one byte or a byte sequence as controller data (DC = 1)."""
        payload = bytes((data & 0xFF,)) if isinstance(data, int) else bytes(data)
        self._hardware.digital_write(self.dc_pin, 1)
        self._hardware.spi_write(payload)

    def wait_until_ready(self, timeout=BUSY_TIMEOUT_SECONDS):
        """Wait until BUSY becomes high, or raise TimeoutError."""
        logger.debug("e-Paper busy H")
        self._hardware.delay_ms(100)
        deadline = time.monotonic() + timeout

        while self._hardware.digital_read(self.busy_pin) == 0:
            if time.monotonic() >= deadline:
                raise TimeoutError(f"e-Paper BUSY timed out after {timeout:g} seconds")
            self._hardware.delay_ms(BUSY_POLL_MS)

        logger.debug("e-Paper busy H release")

    def _configure_panel(self):
        """Send the common initialization commands in datasheet order."""
        # Send each command before its associated data. Use the initialization
        # values specified for the exact panel model.
        self.send_command(0x4D)
        self.send_data(0x78)

        self.send_command(CMD_PANEL_SETTING)  # Panel setting
        self.send_data((0x0F, 0x29))

        self.send_command(0x01)  # Power setting
        self.send_data((0x07, 0x00))

        self.send_command(0x03)  # Power-off sequence
        self.send_data((0x10, 0x54, 0x44))

        self.send_command(0x06)  # Booster soft start
        self.send_data((0x0F, 0x0A, 0x2F, 0x25, 0x22, 0x2E, 0x21))

        self.send_command(0x41)  # Temperature sensor setting
        self.send_data(0x00)

        self.send_command(0x50)  # VCOM and data interval
        self.send_data(0x37)

        self.send_command(0x60)  # TCON setting
        self.send_data((0x02, 0x02))

        self.send_command(0x61)  # Resolution: 128 x 296
        self.send_data(
            (
                self.width >> 8,
                self.width & 0xFF,
                self.height >> 8,
                self.height & 0xFF,
            )
        )

        self.send_command(0x65)  # Gate/source start position
        self.send_data((0x00, 0x00, 0x00, 0x00))

        # Controller internal drive parameters; users normally should not change these.
        self.send_command(0xE7)
        self.send_data(0x1C)
        self.send_command(0xE3)
        self.send_data(0x22)
        self.send_command(0xB4)
        self.send_data(0xD0)
        self.send_command(0xB5)
        self.send_data(0x03)
        self.send_command(0xE9)
        self.send_data(0x01)
        self.send_command(0x30)
        self.send_data(0x08)

    def init(self):
        """Initialize the panel for a normal full refresh."""
        self._hardware.module_init()
        self.reset()
        self.wait_until_ready()
        self._configure_panel()

        self.send_command(CMD_POWER_ON)
        self._hardware.delay_ms(500)
        self.wait_until_ready()
        return 0

    def init_fast(self):
        """Initialize the panel for the controller's fast refresh mode."""
        self._hardware.module_init()
        self.reset()
        self.wait_until_ready()
        self._configure_panel()

        # Fast-refresh-specific settings.
        self.send_command(0xE0)
        self.send_data(0x02)
        self.send_command(0xE6)
        self.send_data(90)
        self.send_command(0xA5)
        self.wait_until_ready()

        self.send_command(CMD_POWER_ON)
        self.wait_until_ready()
        return 0

    def _turn_on_display(self):
        """Apply the image already stored in controller RAM to the panel."""
        self.send_command(CMD_DISPLAY_REFRESH)
        self.send_data(0x00)
        self.wait_until_ready()

    @staticmethod
    def _palette_image():
        palette = Image.new("P", (1, 1))
        colors = [
            0, 0, 0,        # 0: black
            255, 255, 255,  # 1: white
            255, 255, 0,    # 2: yellow
            255, 0, 0,      # 3: red
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
        """Convert a Pillow image into a native 9,472-byte frame."""
        rotations = {0: None, 90: ROTATE_90, 180: ROTATE_180, 270: ROTATE_270}
        if rotation not in rotations:
            raise ValueError("rotation must be 0, 90, 180 or 270 degrees")

        expected_size = (
            (self.width, self.height)
            if rotation in (0, 180)
            else (self.height, self.width)
        )
        if image.size != expected_size:
            raise ValueError(
                f"invalid image size {image.width}x{image.height}; expected "
                f"{expected_size[0]}x{expected_size[1]} for rotation {rotation}"
            )

        image_temp = self._flatten_to_white(image)
        if rotations[rotation] is not None:
            image_temp = image_temp.transpose(rotations[rotation])

        dither_mode = DITHER_FLOYDSTEINBERG if dither else DITHER_NONE
        image_4color = image_temp.quantize(
            palette=self._palette_image(), dither=dither_mode
        )
        color_codes = bytearray(image_4color.tobytes())

        if len(color_codes) != self.width * self.height:
            raise ValueError("Pillow returned an unexpected pixel count")

        frame = bytearray(FRAME_BYTES)
        for source in range(0, len(color_codes), 4):
            # Palette entries 4..255 are duplicate black entries.
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
                raise ValueError("clear color must be BLACK, WHITE, YELLOW or RED") from exc
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
        """Fill the entire panel with one of the four supported colors."""
        packed = self._packed_clear_color(color)
        self.send_command(CMD_DATA_START)
        self.send_data(bytes((packed,)) * FRAME_BYTES)
        self._turn_on_display()

    def Clear(self, color=1):
        """Compatibility alias for scripts written against the legacy Clear() spelling."""
        self.clear(color)

    def sleep(self):
        """Power off the controller, enter deep sleep, and release GPIO/SPI."""
        try:
            self.send_command(CMD_POWER_OFF)
            self.send_data(0x00)
            self.wait_until_ready()

            self.send_command(CMD_DEEP_SLEEP)
            self.send_data(0xA5)
            self._hardware.delay_ms(2000)
        finally:
            self.close()

    def close(self):
        """Release hardware resources without sending panel sleep commands."""
        self._hardware.module_exit()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()
