#pragma once

#include "drivers/display/ili9341.hpp"
#include "game.hpp"
#include <vector>
#include <functional>

namespace PicoPixel
{
    namespace Games
    {
        using GameFactory = std::function<Game*(PicoPixel::Driver::Buffer*)>;
        class GameRegistry
        {
        public:
            static std::vector<GameFactory>& GetFactories();
            static void Register(const GameFactory& factory);

        private:
            static std::vector<GameFactory> Factories;
        };
    }
}



// NOTE: A static self-registration macro did not work. The compiler/linker must have optimized away the static variable, despite being marked as volatile.
// All I know is that the lambda didn't get called, and so the game wasn't registered
#if 0
// TODO: Perhaps turn this into a struct and include game info (name, description) here?
#define REGISTER_GAME(GameClass) \
    static const bool Is##GameClass##Registered = \
        []() \
        { \
            PicoPixel::Games::GameRegistry::Register( \
                [](PicoPixel::Driver::Buffer* buffer) \
                { \
                    return new PicoPixel::Games::GameClass(buffer); \
                } \
            ); \
            return true; \
        };
#endif

// Instead, this macro will assist in creating the code to register a game. Be sure to call it during the initialization sequence of PicoPixel.
#define REGISTER_GAME(GameClass) \
    PicoPixel::Games::GameRegistry::Register( \
        [](PicoPixel::Driver::Buffer* buffer) \
        { \
            return new PicoPixel::Games::GameClass(buffer); \
        } \
    );
