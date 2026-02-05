#pragma once

#include <Arduino.h>
#include "HX711.h"
#include "config.h"

struct ScaleData {
    float weights[4];
};

extern QueueHandle_t weightQueue;

void initScales();
void startScalesTask();
