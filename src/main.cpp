#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include <Wire.h>
#include "SSD1306Wire.h"

#define LED_PIN 2 // ESP32 DevKitV1 builtin LED

#define OLED_SDA_PIN 21
#define OLED_SCL_PIN 22

// Initialize OLED display (I2C address, SDA, SCL)
// SSD1306Wire display(0x3c, OLED_SDA_PIN, OLED_SCL_PIN);

static QueueHandle_t sensorQueue = NULL;
static SemaphoreHandle_t serialMutex = NULL;

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
        Serial.println("Printing from printTask...");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void setup()
{
    Serial.begin(115200);
    // Create queue for sensor samples (10 items)
    sensorQueue = xQueueCreate(10, sizeof(uint16_t));
    // Create mutex for Serial
    serialMutex = xSemaphoreCreateMutex();

    // Initialize the OLED display (ThingPulse SSD1306 library)
    // display.init();
    // display.flipScreenVertically();
    // display.setFont(ArialMT_Plain_10);
    // display.clear();
    // display.drawString(0, 0, "Hello World");
    // display.display();

    // Create FreeRTOS tasks (pin to cores optionally)
    xTaskCreatePinnedToCore(blinkTask, "BlinkTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(printTask, "PrintTask", 4096, NULL, 1, NULL, 0);

    // Optional: let main Arduino loop do nothing (tasks run independently)
}

void loop()
{
    // Idle loop to keep Arduino framework happy
    vTaskDelay(pdMS_TO_TICKS(1000));
}