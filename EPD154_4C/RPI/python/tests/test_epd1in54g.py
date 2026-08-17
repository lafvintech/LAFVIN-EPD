import sys
import unittest
from pathlib import Path

from PIL import Image

PROJECT_DIR = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_DIR / "lib"))

from lafvin_epd.epd1in54g import EPD, FRAME_BYTES


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

    def command_data_pairs(self):
        pairs = []
        for index, event in enumerate(self.events):
            if event[0] == "command":
                data = None
                if index + 2 < len(self.events):
                    # send_data changes DC before the following SPI event.
                    following = self.events[index + 2]
                    if following[0] == "data":
                        data = following[1]
                pairs.append((event[1][0], data))
        return pairs


class EPD154Tests(unittest.TestCase):
    def test_geometry_and_native_frame_size(self):
        epd = EPD(FakeHardware())
        self.assertEqual((epd.width, epd.height), (200, 200))
        self.assertEqual(FRAME_BYTES, 10000)

    def test_four_native_colors_pack_msb_first(self):
        epd = EPD(FakeHardware())
        image = Image.new("RGB", (200, 200), epd.WHITE)
        image.putpixel((0, 0), epd.BLACK)
        image.putpixel((1, 0), epd.WHITE)
        image.putpixel((2, 0), epd.YELLOW)
        image.putpixel((3, 0), epd.RED)
        frame = epd.getbuffer(image)
        self.assertEqual(len(frame), 10000)
        self.assertEqual(frame[0], 0x1B)  # 00 01 10 11

    def test_rotation_90_is_clockwise(self):
        epd = EPD(FakeHardware())
        image = Image.new("RGB", (200, 200), epd.WHITE)
        image.putpixel((0, 0), epd.RED)
        frame = epd.getbuffer(image, rotation=90)
        # Clockwise rotation maps source (0, 0) to destination (199, 0).
        self.assertEqual(frame[49], 0x57)
        self.assertEqual(frame[0], 0x55)

    def test_full_init_matches_verified_sequence(self):
        hardware = FakeHardware()
        EPD(hardware).init()
        self.assertEqual(hardware.commands(), [0xE9, 0x04])

    def test_fast_init_matches_verified_sequence(self):
        hardware = FakeHardware()
        EPD(hardware).init_fast()
        self.assertEqual(
            hardware.commands(),
            [0xE9, 0xEF, 0xF6, 0xEF, 0xE0, 0xE6, 0xA5, 0x04],
        )
        data_payloads = [event[1] for event in hardware.events if event[0] == "data"]
        self.assertEqual(
            data_payloads,
            [b"\x01", b"\x01", b"\x24", b"\x00", b"\x02", b"\x5c"],
        )

    def test_clear_sends_one_complete_white_frame(self):
        hardware = FakeHardware()
        epd = EPD(hardware)
        epd.clear(epd.WHITE)
        data_payloads = [event[1] for event in hardware.events if event[0] == "data"]
        self.assertEqual(len(data_payloads[0]), 10000)
        self.assertEqual(set(data_payloads[0]), {0x55})
        self.assertEqual(hardware.commands(), [0x10, 0x12])


if __name__ == "__main__":
    unittest.main()
