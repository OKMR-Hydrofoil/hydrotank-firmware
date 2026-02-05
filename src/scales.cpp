#include <Arduino.h>
#include "scales.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <ArduinoJson.h>

HX711 scales[4];
QueueHandle_t weightQueue = NULL;

void weightTask(void *pvParameters)
{
    ScaleData currentData;
    for (;;)
    {
        for (int i = 0; i < 4; i++)
        {
            if (scales[i].is_ready())
            {
                currentData.weights[i] = scales[i].get_units(1);
            }
            else
            {
                currentData.weights[i] = -1.0;
            }
        }
        if (weightQueue != NULL)
        {
            xQueueSend(weightQueue, &currentData, 0);
        }
        // Serialize and send via Serial using MessagePack (framed)
        StaticJsonDocument<256> doc;
        doc["t"] = (uint32_t)millis();
        JsonArray arr = doc.createNestedArray("w");
        for (int i = 0; i < 4; i++)
            arr.add(currentData.weights[i]);

        uint8_t buf[512];
        size_t len = serializeMsgPack(doc, buf, sizeof(buf));
        if (len > 0)
        {
            // Frame: 1 byte 'M' (msgpack), 4-byte little-endian length, then payload
            uint8_t hdr = 'M';
            Serial.write(&hdr, 1);
            uint32_t l = (uint32_t)len;
            Serial.write((const uint8_t *)&l, sizeof(l));
            Serial.write(buf, len);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void initScales()
{
    for (int i = 0; i < 4; i++)
    {
        scales[i].begin(DOUT_PINS[i], SCK_PINS[i]);
        // Ensure channel A (A+ / A-) with gain 128 for typical 1kg load cells
        scales[i].set_gain(128);
        scales[i].set_scale(SCALE_FACTOR);
        scales[i].tare();
    }
    weightQueue = xQueueCreate(WEIGHT_QUEUE_LEN, sizeof(ScaleData));
}

void startScalesTask()
{
    xTaskCreatePinnedToCore(weightTask, "WeightTask", STACK_SIZE_SCALES, NULL, PRIORITY_SCALES, NULL, CORE_ID_COMM);
}
