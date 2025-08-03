#include "boot.hpp"
#include "graphics/graphics.hpp"
#include "utils/color.hpp"
#include "utils/random.hpp"
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h> // Onboard LED
#include <hardware/clocks.h>
#include <cstdlib>
#include <cmath>

namespace PicoPixel
{
    bool BootSequence(PicoPixel::Driver::Ili9341Data* ili9341Data)
    {
        stdio_init_all();
        printf("Hello, World!");

        PicoPixel::Driver::InitializeIli9341(ili9341Data,
            spi1,
            static_cast<int>(62.5 * MHz),
            18, // CS
            17, // RESET
            16, // DC
            11, // SDI_MOSI
            10, // SCK
            19, // LED
            12, // SDO_MISO
            true
        );

        struct TouchGPIO
        {
            int T_CLK = 9;
            int T_CS = 8;
            int T_DIN = 7;
            int T_DO = 6;
            int T_IRQ = 5;
        } touch_gpio;

        // TODO: Proper splashscreen/logo
        // display->Clear(display->RGBto16bit(0, 0, 0));
        // const char* text = "PicoPixel";
        // uint16_t textWidth = display->GetTextWidth(text);
        // uint16_t centerX = (display->GetWidth() - textWidth) / 2;
        // display->DrawText(text, centerX, display->GetHeight() / 2, display->RGBto16bit(255, 0, 0));

        // const char* text2 = "Loading...";
        // textWidth = display->GetTextWidth(text2);
        // centerX = (display->GetWidth() - textWidth) / 2;
        // display->DrawText(text2, centerX, display->GetHeight() / 2 + display->GetFontHeight() + 4, display->RGBto16bit(255, 255, 255));

        // FIXME: We probably should have some global framebuffer... we shouldn't be managing one here
        PicoPixel::Driver::Buffer buffer;
        PicoPixel::Driver::CreateBuffer(ili9341Data, &buffer);

        for (int i = 0; i < buffer.Width * buffer.Height; i++)
            buffer.Data[i] = PicoPixel::Utils::RGBto16bit(255, 0, 140);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);

#ifdef STARTUP_DELAY_MS
        sleep_ms(STARTUP_DELAY_MS);
#endif

        printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
        printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
        // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

        // TODO: Watchdog recovery stuff here
        // TODO: Silly progress bar or idle animation for loading

        if (cyw43_arch_init())
        {
            printf("cyw43_arch_init failed! (Required to access the onboard LED)\n");
            return false;
        }

        Utils::InitRand();

        sleep_ms(1500);
        PicoPixel::Driver::DestroyBuffer(&buffer);
        return true;
    }

    bool RunDiagnostics(PicoPixel::Driver::Ili9341Data* ili9341Data)
    {
        // TODO: Do whatever diagnostic checks we can. Until then, display tests for now!

        // FIXME: We probably should have some global framebuffer... we shouldn't be managing one here
        PicoPixel::Driver::Buffer buffer;
        PicoPixel::Driver::CreateBuffer(ili9341Data, &buffer);

        // Red
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(255, 0, 0));
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(1500);

        // Green
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 255, 0));
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(1500);

        // Blue
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 255));
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(1500);

        // White
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(255, 255, 255));
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(1500);

        // Black
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(1500);

        // Test DrawLine
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));
        PicoPixel::Graphics::DrawLine(&buffer, 0, 0, buffer.Width - 1, buffer.Height - 1, PicoPixel::Utils::RGBto16bit(255, 0, 0));
        PicoPixel::Graphics::DrawLine(&buffer, 0, buffer.Height - 1, buffer.Width - 1, 0, PicoPixel::Utils::RGBto16bit(0, 255, 0));
        PicoPixel::Graphics::DrawLine(&buffer, buffer.Width / 2, 0, buffer.Width / 2, buffer.Height - 1, PicoPixel::Utils::RGBto16bit(0, 0, 255));
        PicoPixel::Graphics::DrawLine(&buffer, 0, buffer.Height / 2, buffer.Width - 1, buffer.Height / 2, PicoPixel::Utils::RGBto16bit(255, 255, 0));
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(2000);

        // Test DrawRectangle
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));
        PicoPixel::Graphics::DrawRectangle(&buffer, 10, 10, buffer.Width / 2, buffer.Height / 2, PicoPixel::Utils::RGBto16bit(255, 0, 0), false);
        PicoPixel::Graphics::DrawRectangle(&buffer, buffer.Width / 2, buffer.Height / 2, buffer.Width / 2 - 10, buffer.Height / 2 - 10, PicoPixel::Utils::RGBto16bit(0, 255, 0), true);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(2000);

        // Test DrawCircle
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));
        PicoPixel::Graphics::DrawCircle(&buffer, buffer.Width / 2, buffer.Height / 2, buffer.Height / 3, PicoPixel::Utils::RGBto16bit(0, 0, 255), false);
        PicoPixel::Graphics::DrawCircle(&buffer, buffer.Width / 2, buffer.Height / 2, buffer.Height / 4, PicoPixel::Utils::RGBto16bit(255, 0, 255), true);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(2000);

        // Test DrawTriangle
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));
        PicoPixel::Graphics::DrawTriangle(&buffer, 20, buffer.Height - 20, buffer.Width / 2, 20, buffer.Width - 20, buffer.Height - 20, PicoPixel::Utils::RGBto16bit(255, 255, 0), false);
        PicoPixel::Graphics::DrawTriangle(&buffer, 40, buffer.Height - 40, buffer.Width / 2, 40, buffer.Width - 40, buffer.Height - 40, PicoPixel::Utils::RGBto16bit(0, 255, 255), true);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(2000);

        // Test DrawPolygon
        PicoPixel::Graphics::FillBuffer(&buffer, PicoPixel::Utils::RGBto16bit(0, 0, 0));
        // Hexagon outline
        uint16_t hexX[6], hexY[6];
        for (int i = 0; i < 6; i++)
        {
            float angle = 2.0f * 3.1415926f * i / 6.0f;
            hexX[i] = (uint16_t)(buffer.Width / 2.0f + (buffer.Height / 3.0f) * cosf(angle));
            hexY[i] = (uint16_t)(buffer.Height / 2.0f + (buffer.Height / 3.0f) * sinf(angle));
        }
        PicoPixel::Graphics::DrawPolygon(&buffer, hexX, hexY, 6, PicoPixel::Utils::RGBto16bit(255, 128, 0), false);
        // Pentagon filled
        uint16_t pentX[5], pentY[5];
        for (int i = 0; i < 5; i++)
        {
            float angle = 2.0f * 3.1415926f * i / 5.0f - 3.1415926f/2.0f;
            pentX[i] = (uint16_t)(buffer.Width / 2.0f + (buffer.Height / 4.0f) * cosf(angle));
            pentY[i] = (uint16_t)(buffer.Height / 2.0f + (buffer.Height / 4.0f) * sinf(angle));
        }
        PicoPixel::Graphics::DrawPolygon(&buffer, pentX, pentY, 5, PicoPixel::Utils::RGBto16bit(0, 255, 128), true);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(2000);

        // DisplayTest pattern
        PicoPixel::Graphics::DisplayTest(&buffer);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(3000);
        // PicoPixel::Driver::PixelTest(ili9341Data);
        // sleep_ms(1500);
        // PicoPixel::Driver::CharTest(ili9341Data);
        // sleep_ms(3000);
        // PicoPixel::Driver::TextTest(ili9341Data);
        // sleep_ms(3000);

        // for (int i = 0; i < 10; i++)
        // {
        //     PicoPixel::Driver::RectangleTest(ili9341Data);
        //     sleep_ms(1000);
        // }

        PicoPixel::Driver::DestroyBuffer(&buffer);

        return true; // TODO: Return false if any test fails
    }
}
