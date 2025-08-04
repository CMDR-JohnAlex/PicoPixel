#include "menu.hpp"
#include "games/gameRegistry.hpp"
#include "games/game.hpp"

namespace PicoPixel
{
    namespace Menu
    {
        /*
        TODO:
          - Clean this code file up
          - Visible menu with navigate-able list of games
          - Input for menu (either temp input over serial connection messages, or physical GPIO keys)
          - Either a dedicated button for menu to stop game, or dedicated quit button for game to implement a close dialogue..?
          - Power on/off menu button that would turn off the display and wait to wake up
        */

        void LaunchMenu(PicoPixel::Driver::Ili9341Data* ili9341Data, PicoPixel::Driver::Buffer* buffer)
        {
            auto& factories = PicoPixel::Games::GameRegistry::GetFactories();
            printf("[Menu] Registered games: %zu\n", factories.size());
            int selected = 0;
            bool exitMenu = false;
            bool exitGame = false;
            enum class MenuState { Menu, Game };
            MenuState state = MenuState::Menu;
            PicoPixel::Games::Game* currentGame = nullptr;

            while (!exitMenu)
            {
                switch (state)
                {
                case MenuState::Menu:
                    for (size_t i = 0; i < factories.size(); i++)
                    {
                        PicoPixel::Games::Game* temp = factories[i](buffer);
                        printf("[Menu] %s - %s\n", temp->GetName().c_str(), temp->GetDescription().c_str());
                        delete temp;
                    }
                    // FIXME: TEMP! Need to be able to select options.
                    {
                        sleep_ms(5000);
                        printf("[Menu] Auto-selecting option 2\n");
                        currentGame = factories[1](buffer);
                        state = MenuState::Game;
                    }
                    break;

                case MenuState::Game:
                    exitGame = false;
                    currentGame->OnInit();
                    uint64_t lastTime = time_us_64();
                    while (!exitGame)
                    {
                        uint64_t now = time_us_64();
                        float dt = (now - lastTime) / 1e6f;
                        lastTime = now;
                        exitGame = currentGame->OnUpdate(dt);
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
