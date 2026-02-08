#include <scales.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <processed_data.h>

void processingTask(void *pvParameters)
{
    ScaleData data;

    for (;;)
    {
        if (xQueueReceive(weightQueue, &data, portMAX_DELAY))
        {
            size_t size = sizeof(data.weights) / sizeof(data.weights[0]);
            long double sum = 0;
            size_t count = 0;

            for (size_t i = 0; i < size; i++)
            {
                sum += data.weights[i];
                count++;
            }

            if (count == 0) continue;
            
            ProcessedData out;
            out.timestamp = millis();
            out.avg_force = sum / count;

            xQueueSend(processedQueue, &out, 0);
        }
    }
}
