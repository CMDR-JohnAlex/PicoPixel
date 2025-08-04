#pragma once

#include "drivers/potentiometer/b10k.hpp"
#include "games/game.hpp"
#include <string>

namespace PicoPixel
{
    namespace Games
    {
        class PongGame : public Game
        {
        public:
            PongGame(PicoPixel::Driver::Buffer* buffer);
            virtual ~PongGame() = default;

            void OnInit() override;
            void OnShutdown() override;
            bool OnUpdate(float dt) override;
            void OnRender() override;

            std::string GetName() override;
            std::string GetDescription() override;

        private:
            // Paddle and ball state
            float paddle1Y, paddle2Y;
            float paddleHeight;
            float paddleWidth;
            float paddleSpeed;
            float ballX, ballY;
            float ballVelocityX, ballVelocityY;
            float ballSize;
            int score1, score2;
            float fieldWidth, fieldHeight;
            float paddleAISplitRatio;
            void ResetBall();
            // Center line dash config
            uint16_t centerLineDashSpacing;
            uint16_t centerLineDashWidth;
            uint16_t centerLineDashHeight;
            // Input
            B10kDriver::B10kData* paddle1Potentiometer;
        };
    }
}
