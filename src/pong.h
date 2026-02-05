#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "SSD1306Wire.h"

class PongGame
{
private:
    float ballX = 64.0, ballY = 32.0;
    float ballDX = 1.5, ballDY = 1.0;
    int paddleY = 20;
    int score = 0;
    int highScore = 0;
    const int paddleH = 16;
    const int paddleW = 4;
    Preferences prefs;

public:
    bool active = false;

    PongGame() {
        prefs.begin("pong", false);
        highScore = prefs.getInt("highScore", 0);
        prefs.end();
    }

    void updateAndDraw(SSD1306Wire &display, int encoderPos)
    {
        // Smooth paddle movement
        int targetPaddleY = map(abs(encoderPos) % 40, 0, 40, 0, 64 - paddleH);
        paddleY = targetPaddleY; // Simple for now, could add easing

        // Move Ball
        ballX += ballDX;
        ballY += ballDY;

        // Wall Collisions (Top/Bottom)
        if (ballY <= 0 || ballY >= 61) {
            ballDY *= -1;
            ballY = (ballY <= 0) ? 0 : 61;
        }

        // Right Wall Collision
        if (ballX >= 125) {
            ballDX *= -1;
            ballX = 125;
        }

        // Paddle Collision (Left side)
        if (ballX <= paddleW)
        {
            if (ballY >= paddleY && ballY <= paddleY + paddleH)
            {
                ballDX = abs(ballDX) + 0.1; // Speed up slightly
                ballX = paddleW;
                score++;
                if (score > highScore) {
                    highScore = score;
                }
            }
            else
            {
                // Game Over - Save High Score and Reset
                if (highScore > 0) {
                    prefs.begin("pong", false);
                    prefs.putInt("highScore", highScore);
                    prefs.end();
                }
                reset();
            }
        }

        // Draw Game
        display.setFont(ArialMT_Plain_10);
        display.drawString(40, 0, "S: " + String(score) + " H: " + String(highScore));
        
        // Draw dashed middle line
        for(int i=0; i<64; i+=4) {
            display.drawVerticalLine(64, i, 2);
        }

        display.fillRect(0, paddleY, paddleW, paddleH); // Paddle
        display.fillCircle((int)ballX, (int)ballY, 2);  // Round Ball
    }

    void reset()
    {
        ballX = 64.0;
        ballY = 32.0;
        ballDX = 1.5;
        ballDY = (random(0, 2) == 0 ? 1.0 : -1.0);
        score = 0;
    }
};
