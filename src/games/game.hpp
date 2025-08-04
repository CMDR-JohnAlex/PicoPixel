#pragma once

#include "drivers/display/ili9341.hpp"
#include "graphics/graphics.hpp"
#include <cstdint>
#include <string>

namespace PicoPixel
{
    namespace Games
    {
        // Base class for all games. All games should inherit from this.
        class Game
        {
        public:
            Game(PicoPixel::Driver::Buffer* buffer);
            virtual ~Game() = default;

            virtual void OnInit() = 0;
            virtual void OnShutdown() = 0;
            virtual bool OnUpdate(float dt) = 0;
            virtual void OnRender() = 0;

            // For menu
            virtual std::string GetName() = 0;
            virtual std::string GetDescription() = 0;

        protected:
            PicoPixel::Driver::Buffer* Buffer;
        };
    }
}
