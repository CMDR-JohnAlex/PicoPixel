#include "exampleGame.hpp"
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
            printf("ExampleGame::OnInit()\n");
        }

        void ExampleGame::OnShutdown()
        {
            // Cleanup logic here
            printf("ExampleGame::OnShutdown()\n");
        }

        void ExampleGame::OnUpdate(float dt)
        {
            // Game update logic here
            printf("ExampleGame::OnUpdate(dt)\n");

            sleep_ms(1000);
        }

        void ExampleGame::OnRender()
        {
            // Rendering logic here
            printf("ExampleGame::OnRender()\n");

            PicoPixel::Graphics::FillBuffer(Buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));

            uint16_t maxWidth = Buffer->Width;
            uint16_t maxHeight = Buffer->Height;
            uint16_t rectWidth = (rand() % (maxWidth / 4)) + 20;    // 20 to 25% of screen width
            uint16_t rectHeight = (rand() % (maxHeight / 4)) + 20;  // 20 to 25% of screen height
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
