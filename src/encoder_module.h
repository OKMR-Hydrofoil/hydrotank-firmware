#pragma once

#include <Arduino.h>
#include <ESP32Encoder.h>
#include "config.h"
#include "pong.h"

extern QueueHandle_t encoderQueue;
extern ESP32Encoder encoder;
extern PongGame pong;

void initEncoder();
void startEncoderTask();
