#include <Arduino.h>
#include "display_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

SSD1306Wire display(0x3c, OLED_SDA_PIN, OLED_SCL_PIN);

void displayTask(void *pvParameters) {
    display.init();
    display.flipScreenVertically();

    int lastPosition = 0;
    ScaleData lastWeights = {{0, 0, 0, 0}};

    for (;;) {
        xQueueReceive(encoderQueue, &lastPosition, 0);
        xQueueReceive(weightQueue, &lastWeights, 0);

        display.clear();

        if (pong.active) {
            pong.updateAndDraw(display, lastPosition);
            display.setFont(ArialMT_Plain_10);
            display.drawString(80, 54, "Hold to Exit");
        } else {
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 2, "ENC:");
            display.setFont(ArialMT_Plain_16);
            display.drawString(30, 0, String(lastPosition));

            if (digitalRead(ENCODER_BUTTON_PIN) == LOW)
                display.drawString(85, 2, "BTN");
            if (digitalRead(SWITCH_PIN) == LOW)
                display.drawString(110, 2, "SW");

            display.drawHorizontalLine(0, 18, 128);

            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 22, "S0: " + String(lastWeights.weights[0], 1));
            display.drawString(0, 34, "S1: " + String(lastWeights.weights[1], 1));
            display.drawString(64, 22, "S2: " + String(lastWeights.weights[2], 1));
            display.drawString(64, 34, "S3: " + String(lastWeights.weights[3], 1));

            display.drawProgressBar(0, 52, 120, 8, abs(lastPosition) % 101);
        }

        display.display();
        vTaskDelay(pdMS_TO_TICKS(1000 / DISPLAY_FPS));
    }
}

void initDisplay() {
    // Initialized in task
}

void startDisplayTask() {
    xTaskCreatePinnedToCore(displayTask, "DisplayTask", STACK_SIZE_DISPLAY, NULL, PRIORITY_DISPLAY, NULL, CORE_ID_GFX);
}
