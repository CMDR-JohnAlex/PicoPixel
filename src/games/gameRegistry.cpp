#include "gameRegistry.hpp"

namespace PicoPixel
{
    namespace Games
    {
        std::vector<GameFactory> GameRegistry::Factories;

        std::vector<GameFactory>& GameRegistry::GetFactories()
        {
            return Factories;
        }

        void GameRegistry::Register(const GameFactory& factory)
        {
            Factories.push_back(factory);
        }
    }
}
