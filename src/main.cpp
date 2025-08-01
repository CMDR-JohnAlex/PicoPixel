#include <stdio.h>
#include <iostream>
#include <pico/stdlib.h> // Required
#include <pico/cyw43_arch.h> // Onboard LED
#include <filesystem/vfs.h> // Filesystem
#include <hardware/gpio.h>
// #include <hardware/watchdog.h>
#include <hardware/clocks.h>

#include <errno.h>
#include <string.h>

#include "drivers/display/ili9341.hpp"
#include "firmware/boot.hpp"

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    // Put your timeout handler code in here

    printf("Alarm callback\n");

    return 0;
}

void FlashGPIOLEDPin(bool longFlash)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(longFlash ? 750 : 250);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(500);
}

void ToggleGPIOLEDPin(bool toggle)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, (int)toggle);
}

int main()
{
    // Filesystem tests
    // {
    //     fs_init();

    //     FILE *fp = fopen("/HELLO.TXT", "w");
    //     if (fp == NULL)
    //         printf("fopen error: %s\n", strerror(errno));
    //     fprintf(fp, "Hello World! This is a message stored in a file.\n");
    //     int err = fclose(fp);
    //     if (err == -1)
    //         printf("close error: %s\n", strerror(errno));

    //     fp = fopen("/HELLO.TXT", "r");
    //     if (fp == NULL)
    //         printf("fopen error: %s\n", strerror(errno));
    //     char buffer[512] = {0};
    //     fgets(buffer, sizeof(buffer), fp);
    //     fclose(fp);

    //     printf("HELLO.TXT: %s", buffer);
    // }


    // Timer example code - This example fires off the callback after 5000ms
    add_alarm_in_ms(5000, alarm_callback, NULL, false);
    // For more examples of timer use see https://github.com/raspberrypi/pico-examples/tree/master/timer

    // // Watchdog example code
    // if (watchdog_caused_reboot()) {
    //     printf("Rebooted by Watchdog!\n");
    //     // Whatever action you may take if a watchdog caused a reboot
    //     printf("Insert BSOD info.\n");
    // }

    // // Enable the watchdog, requiring the watchdog to be updated every 5000ms or the chip will reboot
    // // second arg is pause on debug which means the watchdog will pause when stepping through code
    // watchdog_enable(5000, 0);

    // // You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
    // watchdog_update();

    printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
    printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));
    // For more examples of clocks use see https://github.com/raspberrypi/pico-examples/tree/master/clocks

    // while (true) {
    //     printf("Watchdog updating\n");
    //     watchdog_update();
    //     printf("Watchdog updated\n");
    //     FlashGPIOLEDPin(true);
    //     FlashGPIOLEDPin(false);
    //     sleep_ms(500);
    // }





    Ili9341PinConfig ili9341PinConfig = {
        spi1,
        static_cast<int>(62.5 * MHz),
        18, // CS
        17, // RESET
        16, // DC
        11, // SDI_MOSI
        10, // SCK
        19, // LED
        12  // SDO_MISO
    };

    struct TouchGPIO
    {
        int T_CLK = 9;
        int T_CS = 8;
        int T_DIN = 7;
        int T_DO = 6;
        int T_IRQ = 5;
    } touch_gpio;

    // FIXME: Any initialization "printf"s will not be heard...
    ili9341* display = new ili9341(
        ili9341PinConfig,
        true
    );

    PicoPixel::BootSequence(display);
    PicoPixel::RunDiagnostics(display);

    display->SetOrientation(false);
    PicoPixel::RunDiagnostics(display);

    //PicoPixel::Menu();


    //while (true)
    {
        display->Clear();

        display->DrawPixel(10, 10, display->RGBto16bit(255, 0, 0));

        display->DrawTriangle(50, 50, 100, 100, 50, 100, display->RGBto16bit(0, 0, 255));

        display->DrawTriangleFilled(100, 50, 150, 100, 100, 100, display->RGBto16bit(10, 10, 255));

        display->DrawTriangle(50, 150, 100, 150, 50, 200, display->RGBto16bit(0, 0, 255));
        display->DrawTriangle(100, 200, 100, 150, 50, 200, display->RGBto16bit(0, 0, 255));

        display->DrawPixel(50, 150, display->RGBto16bit(255, 0, 0));
        display->DrawPixel(100, 150, display->RGBto16bit(255, 0, 0));
        display->DrawPixel(50, 200, display->RGBto16bit(255, 0, 0));
        display->DrawPixel(100, 200, display->RGBto16bit(255, 0, 0));

        display->DrawTriangleFilled(100, 150, 150, 150, 100, 200, display->RGBto16bit(0, 0, 255));
        display->DrawTriangleFilled(150, 200, 150, 150, 100, 200, display->RGBto16bit(0, 0, 255));

        display->DrawPixel(100, 150, display->RGBto16bit(255, 0, 0));
        display->DrawPixel(150, 150, display->RGBto16bit(255, 0, 0));
        display->DrawPixel(100, 200, display->RGBto16bit(255, 0, 0));
        display->DrawPixel(150, 200, display->RGBto16bit(255, 0, 0));

        display->DrawTriangleGradient(200, 150, display->RGBto16bit(0, 255, 0),
                                  250, 150, display->RGBto16bit(0, 0, 255),
                                  200, 200, display->RGBto16bit(255, 0, 0));
        display->DrawTriangleGradient(250, 200, display->RGBto16bit(255, 255, 255),
                                  250, 150, display->RGBto16bit(0, 0, 255),
                                  200, 200, display->RGBto16bit(255, 0, 0));

        sleep_ms(5000);
    }

    display->Sleep();
    sleep_ms(5000);
    display->Wake();
    sleep_ms(5000);
    display->ScreenTest();
    sleep_ms(5000);

    delete(display);
}
