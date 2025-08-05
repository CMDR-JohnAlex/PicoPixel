#include "exampleGame.hpp"
#include "log.hpp"
#include <string>

namespace PicoPixel
{
    namespace Games
    {
        ExampleGame::ExampleGame(PicoPixel::Driver::Buffer* buffer)
            : Game(buffer)
        {
        }

        void ExampleGame::OnInit()
        {
            // Initialization logic here
            LOG("OnInit()\n");
        }

        void ExampleGame::OnShutdown()
        {
            // Cleanup logic here
            LOG("OnShutdown()\n");
        }

        bool ExampleGame::OnUpdate(float dt)
        {
            // Game update logic here
            LOG("OnUpdate(%f)\n", dt);

            sleep_ms(1000);

            return false; // We do not want to quit.
        }

        void ExampleGame::OnRender()
        {
            // Rendering logic here
            LOG("OnRender()\n");

            PicoPixel::Graphics::FillBuffer(Buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));

            uint16_t maxWidth = Buffer->Width;
            uint16_t maxHeight = Buffer->Height;
            uint16_t rectWidth = (rand() % (maxWidth / 2)) + 20;    // 20 to 1/2 of screen width
            uint16_t rectHeight = (rand() % (maxHeight / 2)) + 20;  // 20 to 1/2 of screen height
            uint16_t rectX = rand() % (maxWidth - rectWidth);       // Ensure rect fits horizontally
            uint16_t rectY = rand() % (maxHeight - rectHeight);     // Ensure rect fits vertically

            PicoPixel::Graphics::DrawRectangle(
                Buffer,
                rectX,
                rectY,
                rectWidth,
                rectHeight,
                PicoPixel::Utils::RGBto16bit(rand() % 256, rand() % 256, rand() % 256)
            );
        }

        std::string ExampleGame::GetName()
        {
            return "Example Game";
        }

        std::string ExampleGame::GetDescription()
        {
            return "An example game template.";
        }
    }
}
