"""Raspberry Pi hardware interface implemented with Python lgpio."""

from __future__ import annotations

import logging
import os
import time
from pathlib import Path

logger = logging.getLogger(__name__.split(".")[-1])

# BCM GPIO numbering used by the Raspberry Pi header.
RST_PIN = 17
DC_PIN = 25
BUSY_PIN = 24
CS_PIN = 8       # CE0 is controlled by the hardware SPI driver.
MOSI_PIN = 10
SCLK_PIN = 11

SPI_BUS = 0
SPI_CHANNEL = 0
SPI_SPEED_HZ = 4_000_000
SPI_WRITE_CHUNK = 4096


class HardwareError(RuntimeError):
    """Raised when GPIO or SPI cannot be initialized or accessed."""


class RaspberryPi:
    """Owns the lgpio GPIO and SPI handles used by the display."""

    RST_PIN = RST_PIN
    DC_PIN = DC_PIN
    BUSY_PIN = BUSY_PIN

    def __init__(self, lgpio_module=None):
        self._lgpio = lgpio_module
        self._gpio_handle = -1
        self._spi_handle = -1

    def _api(self):
        if self._lgpio is None:
            try:
                import lgpio
            except ImportError as exc:
                raise HardwareError(
                    "Cannot import lgpio. On Raspberry Pi OS install python3-lgpio."
                ) from exc
            self._lgpio = lgpio
        return self._lgpio

    @staticmethod
    def _check(result, operation):
        if result < 0:
            raise HardwareError(f"{operation} failed with lgpio error {result}")
        return result

    @staticmethod
    def _is_raspberry_pi_5():
        try:
            model = Path("/proc/device-tree/model").read_bytes()
        except OSError:
            return False
        return b"Raspberry Pi 5" in model

    def _chip_candidates(self):
        # Raspberry Pi 5 exposes its GPIO header on gpiochip4 instead of the
        # gpiochip0 used by earlier models; try the likely chip first and
        # fall back to the other one rather than hard-coding just one.
        requested = os.getenv("EPD_GPIO_CHIP")
        if requested is not None and requested.strip():
            try:
                chip = int(requested, 10)
            except ValueError as exc:
                raise HardwareError(
                    f"EPD_GPIO_CHIP must be a non-negative integer, got {requested!r}"
                ) from exc
            if chip < 0:
                raise HardwareError("EPD_GPIO_CHIP must be non-negative")
            return (chip,)
        return (4, 0) if self._is_raspberry_pi_5() else (0, 4)

    def _open_and_claim_gpio(self):
        api = self._api()
        errors = []

        for chip in self._chip_candidates():
            handle = -1
            try:
                handle = self._check(
                    api.gpiochip_open(chip), f"opening /dev/gpiochip{chip}"
                )
                self._check(
                    api.gpio_claim_input(handle, BUSY_PIN),
                    f"claiming BUSY GPIO{BUSY_PIN}",
                )
                self._check(
                    api.gpio_claim_output(handle, RST_PIN, 1),
                    f"claiming RST GPIO{RST_PIN}",
                )
                self._check(
                    api.gpio_claim_output(handle, DC_PIN, 0),
                    f"claiming DC GPIO{DC_PIN}",
                )
                logger.debug("Using /dev/gpiochip%d", chip)
                return handle
            except Exception as exc:
                errors.append(f"gpiochip{chip}: {exc}")
                if handle >= 0:
                    try:
                        api.gpiochip_close(handle)
                    except Exception:
                        pass

        detail = "; ".join(errors)
        raise HardwareError(f"Unable to initialize Raspberry Pi GPIO ({detail})")

    def module_init(self):
        """Open GPIO and SPI. Calling this repeatedly is safe."""
        if self._gpio_handle >= 0 and self._spi_handle >= 0:
            return 0

        api = self._api()
        self._gpio_handle = self._open_and_claim_gpio()
        try:
            self._spi_handle = self._check(
                api.spi_open(SPI_BUS, SPI_CHANNEL, SPI_SPEED_HZ, 0),
                f"opening SPI{SPI_BUS}.{SPI_CHANNEL}",
            )
        except Exception:
            self.module_exit()
            raise

        logger.debug("Raspberry Pi lgpio initialized at %d Hz", SPI_SPEED_HZ)
        return 0

    def digital_write(self, pin, value):
        if self._gpio_handle < 0:
            raise HardwareError("GPIO is not initialized")
        result = self._api().gpio_write(self._gpio_handle, pin, 1 if value else 0)
        self._check(result, f"writing GPIO{pin}")

    def digital_read(self, pin):
        if self._gpio_handle < 0:
            raise HardwareError("GPIO is not initialized")
        result = self._api().gpio_read(self._gpio_handle, pin)
        return self._check(result, f"reading GPIO{pin}")

    @staticmethod
    def delay_ms(milliseconds):
        time.sleep(milliseconds / 1000.0)

    def spi_write(self, data):
        """Write bytes in kernel SPI-safe chunks and verify every transfer."""
        if self._spi_handle < 0:
            raise HardwareError("SPI is not initialized")

        payload = bytes(data)
        offset = 0
        while offset < len(payload):
            # Some Raspberry Pi OS lgpio builds accept bytes/bytearray but not
            # memoryview, so pass a bytes slice to spi_write().
            chunk = payload[offset : offset + SPI_WRITE_CHUNK]
            result = self._api().spi_write(self._spi_handle, chunk)
            self._check(result, "writing SPI data")
            if result == 0 or result > len(chunk):
                raise HardwareError(
                    f"SPI returned an invalid transfer length: {result}"
                )
            offset += result

    def module_exit(self):
        """Release SPI and GPIO resources. Calling this repeatedly is safe."""
        api = self._lgpio
        if api is None:
            return

        if self._gpio_handle >= 0:
            # Best-effort: drop DC/RST low before closing so the panel isn't
            # left mid-command if the process exits unexpectedly.
            for pin in (DC_PIN, RST_PIN):
                try:
                    api.gpio_write(self._gpio_handle, pin, 0)
                except Exception:
                    pass

        if self._spi_handle >= 0:
            try:
                api.spi_close(self._spi_handle)
            except Exception as exc:
                logger.warning("Unable to close SPI cleanly: %s", exc)
            finally:
                self._spi_handle = -1

        if self._gpio_handle >= 0:
            try:
                api.gpiochip_close(self._gpio_handle)
            except Exception as exc:
                logger.warning("Unable to close GPIO cleanly: %s", exc)
            finally:
                self._gpio_handle = -1

        logger.debug("SPI and GPIO resources released")


_hardware = RaspberryPi()


def module_init():
    return _hardware.module_init()


def module_exit():
    _hardware.module_exit()


def digital_write(pin, value):
    _hardware.digital_write(pin, value)


def digital_read(pin):
    return _hardware.digital_read(pin)


def delay_ms(milliseconds):
    _hardware.delay_ms(milliseconds)


def spi_write(data):
    _hardware.spi_write(data)
