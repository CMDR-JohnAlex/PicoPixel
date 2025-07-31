#include "boot.hpp"
#include "utils/random.hpp"
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h> // Onboard LED

namespace PicoPixel
{
    bool BootSequence(const DisplayGPIO& displayGPIO)
    {
        stdio_init_all();

//#ifdef STARTUP_DELAY_MS
//        sleep_ms(STARTUP_DELAY_MS);
//#endif

        ili9341* display = new ili9341(
            spi1, 62.5 * MHz,
            displayGPIO,
            true
        );

        // TODO: Proper splashscreen/logo
        display->Clear(display->RGBto16bit(0, 0, 0));
        const char* text = "PicoPixel";
        uint16_t textWidth = display->GetTextWidth(text);
        uint16_t centerX = (display->GetWidth() - textWidth) / 2;
        display->DrawText(text, centerX, display->GetHeight() / 2, display->RGBto16bit(255, 0, 0));

        const char* text2 = "Loading...";
        textWidth = display->GetTextWidth(text2);
        centerX = (display->GetWidth() - textWidth) / 2;
        display->DrawText(text2, centerX, display->GetHeight() / 2 + display->GetFontHeight() + 4, display->RGBto16bit(255, 255, 255));

// TODO: Should this be here, or before the display?
#ifdef STARTUP_DELAY_MS
        sleep_ms(STARTUP_DELAY_MS);
#endif

        // TODO: Watchdog recovery stuff here
        // TODO: Silly progress bar or idle animation for loading

        if (cyw43_arch_init())
        {
            printf("cyw43_arch_init failed! (Required to access the onboard LED)\n");
            return false;
        }

        Utils::InitRand();

        //sleep_ms(1500);
        sleep_ms(3000);
        return true;
    }

    bool RunDiagnostics(ili9341* display)
    {
        // TODO: Do whatever diagnostic checks we can. Until then, display tests for now!

        display->ScreenTest();
        sleep_ms(3000);
        display->PixelTest();
        sleep_ms(1500);
        display->CharTest();
        sleep_ms(3000);
        display->TextTest();
        sleep_ms(3000);

        for (int i = 0; i < 10; i++)
        {
            display->RectangleTest();
            sleep_ms(1000);
        }

        return true; // TODO: Return false if any test fails
    }
}