#ifndef LAFVIN_EPD_DEBUG_H
#define LAFVIN_EPD_DEBUG_H

#include <Arduino.h>

#ifndef EPD_USE_DEBUG
#define EPD_USE_DEBUG 1
#endif

#if EPD_USE_DEBUG
#define Debug(message) Serial.print(message)
#else
#define Debug(message) ((void)0)
#endif

#endif
