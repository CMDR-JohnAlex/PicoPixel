#include "pong.hpp"
#include "graphics/graphics.hpp"
#include "utils/random.hpp"
#include <hardware/gpio.h>
#include <hardware/adc.h>
#include <cstdio>
#include <algorithm>
#include <cmath>

namespace PicoPixel
{
    namespace Games
    {
        PongGame::PongGame(PicoPixel::Driver::Buffer* buffer)
            : Game(buffer)
        {
            fieldWidth = buffer->Width;
            fieldHeight = buffer->Height;
            paddleHeight = fieldHeight / 5.0f;
            paddleWidth = 6;
            paddleSpeed = 130.0f;       // Paddle speed (pixels/sec)
            paddleAISplitRatio = 1.5f;  // AI region split
            ballSize = 6;
            score1 = 0;
            score2 = 0;

            centerLineDashSpacing = 8;
            centerLineDashWidth = 2;
            centerLineDashHeight = 4;

            OnInit();
        }

        void PongGame::OnInit()
        {
            // Reset paddles and ball to center
            paddle1Y = fieldHeight / 2.0f - paddleHeight / 2.0f;
            paddle2Y = fieldHeight / 2.0f - paddleHeight / 2.0f;
            ResetBall();

            adc_init();
            adc_gpio_init(28);
        }

        void PongGame::OnShutdown()
        {
        }

        void PongGame::ResetBall()
        {
            // Reset ball to center with random angle and speed
            ballX = fieldWidth / 2.0f - ballSize / 2.0f;
            ballY = fieldHeight / 2.0f - ballSize / 2.0f;
            float angle = (PicoPixel::Utils::RandRange(2) ? 1 : -1) * (3.14159f / 4.0f + (PicoPixel::Utils::Rand() % 100) / 400.0f);
            float speed = 90.0f + (PicoPixel::Utils::Rand() % 40); // 90-130 px/sec
            ballVelocityX = speed * std::cos(angle) * (PicoPixel::Utils::RandRange(2) ? 1 : -1);
            ballVelocityY = speed * std::sin(angle);
        }

        bool PongGame::OnUpdate(float dt)
        {
            // Calculate where paddles should move to intercept ball
            // TODO: Could make this smarter. Intercept where it *will* be.
            float target1 = ballY + ballSize / 2.0f - paddleHeight / 2.0f;
            float target2 = ballY + ballSize / 2.0f - paddleHeight / 2.0f;

            // Left paddle AI: only move if ball is on left region
            // if (ballX + ballSize / 2.0f < fieldWidth / paddleAISplitRatio)
            // {
            //     if (paddle1Y < target1)
            //         paddle1Y = std::min(paddle1Y + paddleSpeed * dt, target1);
            //     else if (paddle1Y > target1)
            //         paddle1Y = std::max(paddle1Y - paddleSpeed * dt, target1);
            // }

            adc_select_input(2);
            paddle1Y = adc_read() / 15 + (paddleHeight / 2) - 30;

            // Right paddle AI: only move if ball is on right region
            if (ballX + ballSize / 2.0f >= fieldWidth - (fieldWidth / paddleAISplitRatio))
            {
                if (paddle2Y < target2)
                    paddle2Y = std::min(paddle2Y + paddleSpeed * dt, target2);
                else if (paddle2Y > target2)
                    paddle2Y = std::max(paddle2Y - paddleSpeed * dt, target2);
            }

            // Clamp paddles to field
            paddle1Y = std::max(0.0f, std::min(paddle1Y, fieldHeight - paddleHeight));
            paddle2Y = std::max(0.0f, std::min(paddle2Y, fieldHeight - paddleHeight));

            // Move ball
            ballX += ballVelocityX * dt;
            ballY += ballVelocityY * dt;

            // Bounce off top/bottom
            if (ballY < 0)
            {
                ballY = 0;
                ballVelocityY = -ballVelocityY;
            }
            else if (ballY + ballSize > fieldHeight)
            {
                ballY = fieldHeight - ballSize;
                ballVelocityY = -ballVelocityY;
            }

            // Paddle collision (left)
            if (ballX < paddleWidth && ballY + ballSize > paddle1Y && ballY < paddle1Y + paddleHeight)
            {
                ballX = paddleWidth;
                ballVelocityX = -ballVelocityX;
                // Randomize angle somewhat
                ballVelocityY += (PicoPixel::Utils::Rand() % 40 - 20);
                // Random multiplier between 1.05 and 1.18 (biased to increase)
                float speedMult = 1.05f + (PicoPixel::Utils::Rand() % 14) * 0.01f; // 1.05 to 1.18
                if (PicoPixel::Utils::RandRange(5) == 0) // 1 in 5 chance to slow a bit
                    speedMult = 0.95f + (PicoPixel::Utils::Rand() % 4) * 0.01f; // 0.95 to 1.00
                ballVelocityX *= speedMult;
                ballVelocityY *= speedMult;
            }
            // Paddle collision (right)
            if (ballX + ballSize > fieldWidth - paddleWidth && ballY + ballSize > paddle2Y && ballY < paddle2Y + paddleHeight)
            {
                ballX = fieldWidth - paddleWidth - ballSize;
                ballVelocityX = -ballVelocityX;
                // Randomize angle somewhat
                ballVelocityY += (PicoPixel::Utils::Rand() % 40 - 20);
                // Speed up: random multiplier between 1.05 and 1.18 (biased to increase)
                float speedMult = 1.05f + (PicoPixel::Utils::Rand() % 14) * 0.01f; // 1.05 to 1.18
                if (PicoPixel::Utils::RandRange(5) == 0) // 1 in 5 chance to slow a bit
                    speedMult = 0.95f + (PicoPixel::Utils::Rand() % 4) * 0.01f; // 0.95 to 1.00
                ballVelocityX *= speedMult;
                ballVelocityY *= speedMult;
            }

            // Score and reset if ball goes out
            if (ballX < 0)
            {
                score2++;
                ResetBall();
            }
            else if (ballX + ballSize > fieldWidth)
            {
                score1++;
                ResetBall();
            }

            // We don't wanna quit the game.
            // But maybe quit once the score reaches the max?
            // Could implement a win/lose game screen before quitting.
            return false;
        }

        void PongGame::OnRender()
        {
            // Clear screen
            PicoPixel::Graphics::FillBuffer(Buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));

            // Draw left and right paddles
            PicoPixel::Graphics::DrawRectangle(Buffer, 0, (uint16_t)paddle1Y, (uint16_t)paddleWidth, (uint16_t)paddleHeight, PicoPixel::Utils::RGBto16bit(255, 255, 255), true);
            PicoPixel::Graphics::DrawRectangle(Buffer, (uint16_t)(fieldWidth - paddleWidth), (uint16_t)paddle2Y, (uint16_t)paddleWidth, (uint16_t)paddleHeight, PicoPixel::Utils::RGBto16bit(255, 255, 255), true);

            // Draw ball
            PicoPixel::Graphics::DrawRectangle(Buffer, (uint16_t)ballX, (uint16_t)ballY, (uint16_t)ballSize, (uint16_t)ballSize, PicoPixel::Utils::RGBto16bit(255, 255, 255), true);

            // Draw center line
            for (uint16_t y = 0; y < fieldHeight; y += centerLineDashSpacing)
            {
                PicoPixel::Graphics::DrawRectangle(
                    Buffer,
                    (uint16_t)(fieldWidth / 2.0f - centerLineDashWidth / 2.0f),
                    y,
                    centerLineDashWidth,
                    centerLineDashHeight,
                    PicoPixel::Utils::RGBto16bit(128, 128, 128),
                    true);
            }

            // Draw scores as green (first 10) or red (overflow)
            for (int i = 0; i < score1 && i <= 10; i++)
            {
                if (i < 10)
                    PicoPixel::Graphics::DrawRectangle(Buffer, 10 + i * 8, 10, 6, 6, PicoPixel::Utils::RGBto16bit(0, 255, 0), true);
                else
                    PicoPixel::Graphics::DrawRectangle(Buffer, 10 + i * 8, 10, 6, 6, PicoPixel::Utils::RGBto16bit(255, 0, 0), true);
            }
            for (int i = 0; i < score2 && i <= 10; i++)
            {
                if (i < 10)
                    PicoPixel::Graphics::DrawRectangle(Buffer, fieldWidth - 10 - (i + 1) * 8, 10, 6, 6, PicoPixel::Utils::RGBto16bit(0, 255, 0), true);
                else
                    PicoPixel::Graphics::DrawRectangle(Buffer, fieldWidth - 10 - (i + 1) * 8, 10, 6, 6, PicoPixel::Utils::RGBto16bit(255, 0, 0), true);
            }
        }

        std::string PongGame::GetName()
        {
            return "Pong";
        }

        std::string PongGame::GetDescription()
        {
            return "Classic Pong: AI vs AI";
        }
    }
}
