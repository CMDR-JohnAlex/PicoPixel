#pragma once

#include "games/game.hpp"
#include "games/gameRegistry.hpp"
#include <string>

namespace PicoPixel
{
    namespace Games
    {
        class ExampleGame : public Game
        {
        public:
            ExampleGame(PicoPixel::Driver::Buffer* buffer);
            virtual ~ExampleGame() = default;

            void OnInit() override;
            void OnShutdown() override;
            void OnUpdate(float dt) override;
            void OnRender() override;

            std::string GetName() override;
            std::string GetDescription() override;
        };
    }
}
