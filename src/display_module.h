#pragma once

#include <Arduino.h>
#include "SSD1306Wire.h"
#include "config.h"
#include "scales.h"
#include "encoder_module.h"

void initDisplay();
void startDisplayTask();
