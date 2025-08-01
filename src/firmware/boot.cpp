#include "boot.hpp"
#include "utils/color.hpp"
#include "utils/random.hpp"
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h> // Onboard LED
#include <hardware/clocks.h>
#include <cstdlib>

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
        for (int i = 0; i < buffer.Width * buffer.Height; i++)
            buffer.Data[i] = PicoPixel::Utils::RGBto16bit(255, 0, 0);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(3000);

        // Green
        for (int i = 0; i < buffer.Width * buffer.Height; i++)
            buffer.Data[i] = PicoPixel::Utils::RGBto16bit(0, 255, 0);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(3000);

        // Blue
        for (int i = 0; i < buffer.Width * buffer.Height; i++)
            buffer.Data[i] = PicoPixel::Utils::RGBto16bit(0, 0, 255);
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(3000);

        // White
        for (int i = 0; i < buffer.Width * buffer.Height; i++)
            buffer.Data[i] = 0xFFFF;
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(3000);

        // Black
        for (int i = 0; i < buffer.Width * buffer.Height; i++)
            buffer.Data[i] = 0x0000;
        PicoPixel::Driver::DrawBuffer(ili9341Data, 0, 0, &buffer);
        sleep_ms(3000);

        PicoPixel::Driver::DisplayTest(ili9341Data, &buffer);
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
