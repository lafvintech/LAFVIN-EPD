CC ?= gcc
TARGET := epd
BUILD_DIR := build

DEBUG ?= 1
CPPFLAGS += -DDEBUG=$(DEBUG)
CPPFLAGS += -Ilib/Config -Ilib/e-Paper -Ilib/GUI -Ilib/Fonts -Iexamples
CFLAGS ?= -O2 -g
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -ffunction-sections -fdata-sections
LDFLAGS += -Wl,--gc-sections
LDLIBS += -llgpio -lm

SOURCES := \
	lib/Config/DEV_Config.c \
	lib/e-Paper/EPD_2in9g.c \
	lib/GUI/GUI_Paint.c \
	lib/GUI/GUI_BMPfile.c \
	$(wildcard examples/*.c) \
	$(wildcard lib/Fonts/*.c)

OBJECTS := $(SOURCES:%.c=$(BUILD_DIR)/%.o)
DEPENDS := $(OBJECTS:.o=.d)

.PHONY: all RPI clean

all: $(TARGET)
RPI: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPENDS)
