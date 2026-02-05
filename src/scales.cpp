#include <Arduino.h>
#include "scales.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

HX711 scales[4];
QueueHandle_t weightQueue = NULL;

void weightTask(void *pvParameters) {
    ScaleData currentData;
    for (;;) {
        for (int i = 0; i < 4; i++) {
            if (scales[i].is_ready()) {
                currentData.weights[i] = scales[i].get_units(1);
            } else {
                currentData.weights[i] = -1.0;
            }
        }
        if (weightQueue != NULL) {
            xQueueSend(weightQueue, &currentData, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(25));
    }
}

void initScales() {
    for (int i = 0; i < 4; i++) {
        scales[i].begin(DOUT_PINS[i], SCK_PINS[i]);
        scales[i].set_scale(SCALE_FACTOR);
        scales[i].tare();
    }
    weightQueue = xQueueCreate(WEIGHT_QUEUE_LEN, sizeof(ScaleData));
}

void startScalesTask() {
    xTaskCreatePinnedToCore(weightTask, "WeightTask", STACK_SIZE_SCALES, NULL, PRIORITY_SCALES, NULL, CORE_ID_COMM);
}
