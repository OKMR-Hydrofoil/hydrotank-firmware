#include <Arduino.h>
#include "config.h"
#include "scales.h"
#include "encoder_module.h"
#include "display_module.h"

void blinkTask(void *pvParameters)
{
    pinMode(LED_PIN, OUTPUT);
    bool state = false;
    for (;;)
    {
        state = !state;
        digitalWrite(LED_PIN, state ? HIGH : LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// void printTask(void *pvParameters) {
//     for (;;) {
//         Serial.printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

void setup()
{
    Serial.begin(115200);

    initScales();
    initEncoder();
    initDisplay();

    xTaskCreatePinnedToCore(blinkTask, "BlinkTask", STACK_SIZE_BLINK, NULL, PRIORITY_BLINK, NULL, CORE_ID_COMM);
    // xTaskCreatePinnedToCore(printTask, "PrintTask", STACK_SIZE_PRINT, NULL, PRIORITY_PRINT, NULL, CORE_ID_COMM);

    startScalesTask();
    startEncoderTask();
    startDisplayTask();
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}