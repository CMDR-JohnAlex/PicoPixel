#include <stdio.h>
#include <iostream>
#include <pico/stdlib.h> // Required
#include <pico/cyw43_arch.h> // Onboard LED
#include <filesystem/vfs.h> // Filesystem
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <hardware/watchdog.h>
#include <hardware/clocks.h>

#include <errno.h>
#include <string.h>

#include "ili9341.hpp"

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
    stdio_init_all();

    // Sleep for 3 seconds, so serial connections can be made.
    sleep_ms(3000);

    if (cyw43_arch_init())
    {
        std::cout << "Wi-Fi init failed! (Required to access the onboard LED)\n";
        return -1;
    }

    printf("Hello, world!\n");

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
    //     // printf("Watchdog updating\n");
    //     // watchdog_update();
    //     // printf("Watchdog updated\n");
    //     FlashGPIOLEDPin(true);
    //     FlashGPIOLEDPin(false);
    //     sleep_ms(500);
    // }



    struct DisplayGPIO
    {
        int CS = 18;
        int RESET = 17;
        int DC = 16;
        int SDI_MOSI = 11;
        int SCK = 10;
        int LED = 19;
        int SDO_MISO = 12;
    } display_gpio;

    struct TouchGPIO
    {
        int T_CLK = 9;
        int T_CS = 8;
        int T_DIN = 7;
        int T_DO = 6;
        int T_IRQ = 5;
    } touch_gpio;

    ili9341 *tft = new ili9341(
        spi1, 62.5 * MHz,
        display_gpio.CS,
        display_gpio.RESET,
        display_gpio.DC,
        display_gpio.SDI_MOSI,
        display_gpio.SCK,
        display_gpio.LED,
        display_gpio.SDO_MISO,
        true
    );

    tft->ScreenTest();
    sleep_ms(3000);
    tft->PixelTest();
    sleep_ms(1000);
    delete(tft);

    tft = new ili9341(
        spi1, 62.5 * MHz,
        display_gpio.CS,
        display_gpio.RESET,
        display_gpio.DC,
        display_gpio.SDI_MOSI,
        display_gpio.SCK,
        display_gpio.LED,
        display_gpio.SDO_MISO,
        false
    );

    tft->ScreenTest();
    sleep_ms(3000);
    tft->PixelTest();
    sleep_ms(1000);
    tft->CharTest();
    sleep_ms(10000);
    tft->TextTest();
    sleep_ms(10000);

    //while (true)
    {
        tft->Clear();

        tft->DrawPixel(10, 10, tft->RGBto16bit(255, 0, 0));

        tft->DrawTriangle(50, 50, 100, 100, 50, 100, tft->RGBto16bit(0, 0, 255));

        tft->DrawTriangleFilled(100, 50, 150, 100, 100, 100, tft->RGBto16bit(10, 10, 255));

        tft->DrawTriangle(50, 150, 100, 150, 50, 200, tft->RGBto16bit(0, 0, 255));
        tft->DrawTriangle(100, 200, 100, 150, 50, 200, tft->RGBto16bit(0, 0, 255));

        tft->DrawPixel(50, 150, tft->RGBto16bit(255, 0, 0));
        tft->DrawPixel(100, 150, tft->RGBto16bit(255, 0, 0));
        tft->DrawPixel(50, 200, tft->RGBto16bit(255, 0, 0));
        tft->DrawPixel(100, 200, tft->RGBto16bit(255, 0, 0));

        tft->DrawTriangleFilled(100, 150, 150, 150, 100, 200, tft->RGBto16bit(0, 0, 255));
        tft->DrawTriangleFilled(150, 200, 150, 150, 100, 200, tft->RGBto16bit(0, 0, 255));

        tft->DrawPixel(100, 150, tft->RGBto16bit(255, 0, 0));
        tft->DrawPixel(150, 150, tft->RGBto16bit(255, 0, 0));
        tft->DrawPixel(100, 200, tft->RGBto16bit(255, 0, 0));
        tft->DrawPixel(150, 200, tft->RGBto16bit(255, 0, 0));

        tft->DrawTriangleGradient(200, 150, tft->RGBto16bit(0, 255, 0),
                                  250, 150, tft->RGBto16bit(0, 0, 255),
                                  200, 200, tft->RGBto16bit(255, 0, 0));
        tft->DrawTriangleGradient(250, 200, tft->RGBto16bit(255, 255, 255),
                                  250, 150, tft->RGBto16bit(0, 0, 255),
                                  200, 200, tft->RGBto16bit(255, 0, 0));

        sleep_ms(5000);
    }

    while (true)
    {
        tft->Clear();
        uint16_t maxWidth = tft->GetWidth();
        uint16_t maxHeight = tft->GetHeight();
        uint16_t rectWidth = (rand() % (maxWidth / 4)) + 20;     // 20 to 25% of screen width
        uint16_t rectHeight = (rand() % (maxHeight / 4)) + 20;   // 20 to 25% of screen height
        uint16_t rectX = rand() % (maxWidth - rectWidth);        // Ensure rect fits horizontally
        uint16_t rectY = rand() % (maxHeight - rectHeight);      // Ensure rect fits vertically

        tft->DrawRectangle(
            rectX,
            rectY,
            rectWidth,
            rectHeight,
            tft->RGBto16bit(rand() % 256, rand() % 256, rand() % 256)
        );

        sleep_ms(1000);
    }
}
