#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include <Wire.h>
#include "SSD1306Wire.h"

#include <ESP32Encoder.h>
#include "HX711.h"

#define LED_PIN 2 // ESP32 DevKitV1 builtin LED

#define OLED_SDA_PIN 32
#define OLED_SCL_PIN 33

#define ENCODER_PIN_A 25
#define ENCODER_PIN_B 26

// Initialize OLED display (I2C address, SDA, SCL)
SSD1306Wire display(0x3c, OLED_SDA_PIN, OLED_SCL_PIN);

static QueueHandle_t sensorQueue = NULL;
static SemaphoreHandle_t serialMutex = NULL;

struct ScaleData
{
    float weights[4];
};

HX711 scales[4];
static QueueHandle_t weightQueue = NULL;
const int doutPins[4] = {21, 23, 4, 18};
const int sckPins[4] = {19, 22, 15, 5};

void weightTask(void *pvParameters)
{
    // 1. Initialize the scale
    for (int i = 0; i < 4; i++)
    {
        scales[i].begin(doutPins[i], sckPins[i]);
        scales[i].set_scale(420.0); // Calibration factor
        scales[i].tare();           // Reset to 0
    }

    ScaleData currentData;

    for (;;)
    {
        for (int i = 0; i < 4; i++)
        {
            if (scales[i].is_ready())
            {
                currentData.weights[i] = scales[i].get_units(1); // 1 sample for speed
            }
            else
            {
                currentData.weights[i] = -1.0; // Indicate error
            }
        }

        // Send the entire struct to the queue
        if (weightQueue != NULL)
        {
            xQueueSend(weightQueue, &currentData, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(25));
    }
}

// Create a queue to hold encoder samples
static QueueHandle_t encoderQueue = NULL;

ESP32Encoder encoder;

void encoderTask(void *pvParameters)
{
    // Initialize the encoder pins
    // Standard ESP32Encoder setup uses internal pullups by default
    encoder.attachHalfQuad(ENCODER_PIN_A, ENCODER_PIN_B);

    // Optional: set starting position
    encoder.setCount(0);

    int64_t lastPosition = 0;

    for (;;)
    {
        int64_t currentPosition = encoder.getCount();

        if (currentPosition != lastPosition)
        {
            lastPosition = currentPosition;

            // Send updated position to queue
            // Note: Casting to int if your queue expects 32-bit integers
            int reportPos = (int)currentPosition;
            xQueueSend(encoderQueue, &reportPos, portMAX_DELAY);
        }

        // We can delay longer now because the hardware handles the counting
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

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

void printTask(void *pvParameters)
{
    for (;;)
    {
        Serial.printf("Free Heap: %u bytes\n", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void displayTask(void *pvParameters)
{
    display.init();
    display.flipScreenVertically();

    int lastPosition = 0;
    ScaleData lastWeights = {{0, 0, 0, 0}};

    for (;;)
    {
        // Get Encoder (Optional: use 0 timeout if you don't want to wait)
        xQueueReceive(encoderQueue, &lastPosition, 0);

        // ACTUALLY RECEIVE the weight data (this clears the queue slot)
        // We use a 0 timeout to keep the display snappy
        if (xQueueReceive(weightQueue, &lastWeights, 0) == pdTRUE)
        {
            // Optional: Serial.println("New weights received by display");
        }

        display.clear();
        // ... (rest of your drawing code) ...
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 2, "ENC:");
        display.setFont(ArialMT_Plain_16);
        display.drawString(30, 0, String(lastPosition));
        display.drawHorizontalLine(0, 18, 128);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 22, "S0: " + String(lastWeights.weights[0], 1));
        display.drawString(0, 34, "S1: " + String(lastWeights.weights[1], 1));
        display.drawString(64, 22, "S2: " + String(lastWeights.weights[2], 1));
        display.drawString(64, 34, "S3: " + String(lastWeights.weights[3], 1));
        display.drawProgressBar(0, 52, 120, 8, abs(lastPosition) % 101);
        display.display();

        vTaskDelay(pdMS_TO_TICKS(30)); // ~33 FPS is plenty
    }
}

void setup()
{
    Serial.begin(115200);

    // Create FreeRTOS tasks (pin to cores optionally)
    xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(printTask, "PrintTask", 4096, NULL, 1, NULL, 1);
    encoderQueue = xQueueCreate(10, sizeof(int));
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 4096, NULL, 1, NULL, 1);
    weightQueue = xQueueCreate(10, sizeof(ScaleData));
    xTaskCreatePinnedToCore(weightTask, "WeightTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", 8192, NULL, 1, NULL, 0);
}

void loop()
{
    // Idle loop to keep Arduino framework happy
    vTaskDelay(pdMS_TO_TICKS(1000));
}