#include "menu.hpp"
#include "games/gameRegistry.hpp"
#include "games/game.hpp"

namespace PicoPixel
{
    namespace Menu
    {
        void LaunchMenu(PicoPixel::Driver::Ili9341Data* ili9341Data, PicoPixel::Driver::Buffer* buffer)
        {
            auto& factories = PicoPixel::Games::GameRegistry::GetFactories();
            printf("Registered games: %zu\n", factories.size());
            int selected = 0;
            bool shouldExit = false;
            bool gameRunning = false;
            enum class MenuState { Menu, Game };
            MenuState state = MenuState::Menu;
            PicoPixel::Games::Game* currentGame = nullptr;

            while (!shouldExit)
            {
                switch (state)
                {
                case MenuState::Menu:
                    for (size_t i = 0; i < factories.size(); i++)
                    {
                        PicoPixel::Games::Game* temp = factories[i](buffer);
                        printf("%s - %s\n", temp->GetName().c_str(), temp->GetDescription().c_str());
                        delete temp;
                    }
                    // FIXME: TEMP! Need to be able to select options.
                    {
                        sleep_ms(15000);
                        printf("Auto-selecting first option");
                        currentGame = factories[0](buffer);
                        state = MenuState::Game;
                    }
                    break;

                case MenuState::Game:
                    gameRunning = true; // TODO: Game.OnUpdate() should return a bool to change this.
                    currentGame->OnInit();
                    while (gameRunning)
                    {
                        currentGame->OnUpdate(0.0f); // Proper deltatime calculation.
                        currentGame->OnRender();
                        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, buffer);
                    }
                    currentGame->OnShutdown();
                    delete currentGame;
                    currentGame = nullptr;
                    state = MenuState::Menu;
                    break;
                }
            }
        }
    }
}
