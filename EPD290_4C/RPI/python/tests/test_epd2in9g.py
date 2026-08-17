import sys
import unittest
from pathlib import Path

from PIL import Image

PROJECT_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_DIR / "lib"))

from lafvin_epd.epd2in9g import EPD, FRAME_BYTES


class FakeHardware:
    RST_PIN = 17
    DC_PIN = 25
    BUSY_PIN = 24

    def __init__(self):
        self.dc = 0
        self.events = []

    def module_init(self):
        self.events.append(("module_init",))
        return 0

    def module_exit(self):
        self.events.append(("module_exit",))

    def digital_write(self, pin, value):
        if pin == self.DC_PIN:
            self.dc = value
        self.events.append(("gpio", pin, value))

    def digital_read(self, pin):
        return 1

    def delay_ms(self, milliseconds):
        self.events.append(("delay", milliseconds))

    def spi_write(self, data):
        kind = "data" if self.dc else "command"
        self.events.append((kind, bytes(data)))

    def commands(self):
        return [event[1][0] for event in self.events if event[0] == "command"]


COMMON_INIT_COMMANDS = [
    0x4D,
    0x00,
    0x01,
    0x03,
    0x06,
    0x41,
    0x50,
    0x60,
    0x61,
    0x65,
    0xE7,
    0xE3,
    0xB4,
    0xB5,
    0xE9,
    0x30,
]


class EPD290Tests(unittest.TestCase):
    def test_geometry_and_native_frame_size(self):
        epd = EPD(FakeHardware())
        self.assertEqual((epd.width, epd.height), (128, 296))
        self.assertEqual(FRAME_BYTES, 9472)

    def test_four_native_colors_pack_msb_first(self):
        epd = EPD(FakeHardware())
        image = Image.new("RGB", (128, 296), epd.WHITE)
        image.putpixel((0, 0), epd.BLACK)
        image.putpixel((1, 0), epd.WHITE)
        image.putpixel((2, 0), epd.YELLOW)
        image.putpixel((3, 0), epd.RED)
        frame = epd.getbuffer(image)
        self.assertEqual(len(frame), 9472)
        self.assertEqual(frame[0], 0x1B)  # 00 01 10 11

    def test_rotation_90_is_clockwise(self):
        epd = EPD(FakeHardware())
        image = Image.new("RGB", (296, 128), epd.WHITE)
        image.putpixel((0, 0), epd.RED)
        frame = epd.getbuffer(image, rotation=90)
        # Clockwise rotation maps source (0, 0) to panel-memory (127, 0).
        self.assertEqual(frame[31], 0x57)
        self.assertEqual(frame[0], 0x55)

    def test_full_init_matches_verified_sequence(self):
        hardware = FakeHardware()
        EPD(hardware).init()
        self.assertEqual(hardware.commands(), COMMON_INIT_COMMANDS + [0x04])

    def test_fast_init_matches_verified_sequence(self):
        hardware = FakeHardware()
        EPD(hardware).init_fast()
        self.assertEqual(
            hardware.commands(),
            COMMON_INIT_COMMANDS + [0xE0, 0xE6, 0xA5, 0x04],
        )

    def test_clear_sends_one_complete_white_frame(self):
        hardware = FakeHardware()
        epd = EPD(hardware)
        epd.clear(epd.WHITE)
        data_payloads = [event[1] for event in hardware.events if event[0] == "data"]
        self.assertEqual(len(data_payloads[0]), 9472)
        self.assertEqual(set(data_payloads[0]), {0x55})
        self.assertEqual(hardware.commands(), [0x10, 0x12])


if __name__ == "__main__":
    unittest.main()
