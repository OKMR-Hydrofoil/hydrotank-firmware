#include <Arduino.h>
#include "encoder_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

PongGame pong;

QueueHandle_t encoderQueue = NULL;
ESP32Encoder encoder;

void encoderTask(void *pvParameters) {
    pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SWITCH_PIN, INPUT_PULLUP);
    encoder.attachHalfQuad(ENCODER_PIN_A, ENCODER_PIN_B);
    encoder.setCount(0);

    int64_t lastPosition = 0;
    bool lastButtonState = HIGH;
    uint32_t buttonPressedTime = 0;

    // Secret code state
    enum class SecretState { WAIT_42, WAIT_MINUS_42 };
    SecretState secretState = SecretState::WAIT_42;

    for (;;) {
        // 1. Handle Rotation
        int64_t currentPosition = encoder.getCount();
        if (currentPosition != lastPosition) {
            lastPosition = currentPosition;
            int reportPos = (int)currentPosition;
            xQueueOverwrite(encoderQueue, &reportPos);
        }

        // 2. Handle Button and Secret Sequence
        bool currentButtonState = digitalRead(ENCODER_BUTTON_PIN);
        
        if (currentButtonState == LOW && lastButtonState == HIGH) {
            // Button just pressed
            buttonPressedTime = millis();
            
            if (!pong.active) {
                if (secretState == SecretState::WAIT_42 && lastPosition == 42) {
                    secretState = SecretState::WAIT_MINUS_42;
                    Serial.println("Secret Part 1/2!");
                } else if (secretState == SecretState::WAIT_MINUS_42 && lastPosition == -42) {
                    pong.active = true;
                    pong.reset();
                    secretState = SecretState::WAIT_42;
                    Serial.println("Pong Activated!");
                } else {
                    secretState = SecretState::WAIT_42; // Reset on wrong entry
                }
            }
        } else if (currentButtonState == LOW && (millis() - buttonPressedTime > PONG_TRIGGER_HOLD_MS)) {
            // Exit Pong on long press
            if (pong.active) {
                pong.active = false;
                secretState = SecretState::WAIT_42;
                while (digitalRead(ENCODER_BUTTON_PIN) == LOW) vTaskDelay(pdMS_TO_TICKS(10));
            }
        }

        lastButtonState = currentButtonState;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void initEncoder() {
    encoderQueue = xQueueCreate(ENCODER_QUEUE_LEN, sizeof(int));
}

void startEncoderTask() {
    xTaskCreatePinnedToCore(encoderTask, "EncoderTask", STACK_SIZE_ENCODER, NULL, PRIORITY_ENCODER, NULL, CORE_ID_COMM);
}
