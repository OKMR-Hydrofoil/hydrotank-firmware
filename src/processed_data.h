#pragma once

#include <cstdint>
#include <freertos/queue.h>

typedef struct {
    uint32_t timestamp;
    long double avg_force;
} ProcessedData;

extern QueueHandle_t processedQueue;