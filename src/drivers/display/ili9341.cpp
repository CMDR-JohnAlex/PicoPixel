#include "ili9341.hpp"
#include "hardware/pwm.h"
#include <cstdio>
#include <cstdlib>
#include <array>

namespace PicoPixel
{
    namespace Driver
    {
        void InitializeIli9341(Ili9341Data* display, spi_inst_t* spiPort, int spiClockFreqency, uint8_t gpioCS, uint8_t gpioRESET, uint8_t gpioDC, uint8_t gpioSDI_MOSI, uint8_t gpioSCK, uint8_t gpioLed, uint8_t gpioSDO_MISO, bool portrait)
        {
            if (display->IsInitialized) return;

            display->IsPortrait = portrait;
            display->SpiPort = spiPort;
            display->SpiClockFreqency = spiClockFreqency;
            display->GpioCS = gpioCS;
            display->GpioRESET = gpioRESET;
            display->GpioDC = gpioDC;
            display->GpioSDI_MOSI = gpioSDI_MOSI;
            display->GpioSCK = gpioSCK;
            display->GpioLed = gpioLed;
            display->GpioSDO_MISO = gpioSDO_MISO;
            // Width and Height are set later.

            // Setup GPIO stuffs
            gpio_init(display->GpioLed);
            gpio_set_function(display->GpioLed, GPIO_FUNC_PWM);
            pwm_set_gpio_level(display->GpioLed, 0xFFFF);
            uint sliceNum = pwm_gpio_to_slice_num(display->GpioLed);
            pwm_set_enabled(sliceNum, true);

            spi_init(display->SpiPort, display->SpiClockFreqency);
            const int actualBaudrate = spi_set_baudrate(display->SpiPort, display->SpiClockFreqency);
            printf("[ili9341] Requested: %d Hz, Actual: %d Hz\n", display->SpiClockFreqency, actualBaudrate);

            EnsureSPI8Bit(display);

            gpio_set_function(display->GpioSDO_MISO, GPIO_FUNC_SPI);
            gpio_set_function(display->GpioSCK, GPIO_FUNC_SPI);
            gpio_set_function(display->GpioSDI_MOSI, GPIO_FUNC_SPI);

            gpio_init(display->GpioCS);
            gpio_set_dir(display->GpioCS, GPIO_OUT);
            gpio_put(display->GpioCS, 1);

            gpio_init(display->GpioRESET);
            gpio_set_dir(display->GpioRESET, GPIO_OUT);
            gpio_put(display->GpioRESET, 1);

            gpio_init(display->GpioDC);
            gpio_set_dir(display->GpioDC, GPIO_OUT);
            gpio_put(display->GpioDC, 0);

            // Hardware reset
            sleep_ms(10);
            gpio_put(display->GpioRESET, 0);
            sleep_ms(10);
            gpio_put(display->GpioRESET, 1);

            // Software reset
            SetCommand(display, ILI9341_SWRESET);
            sleep_ms(100); // NOTE: Required to wait at least 5ms before sending new commands, but appears that we need more than 5ms.

            // Gamma correction
            SetCommand(display, ILI9341_GAMMASET);
            CommandParameter(display, 0b00000100); // Gamma curve 4

            // Orientation / ILI9341_MADCTL + Width/Height setting.
            SetOrientation(display, portrait);

            // TODO: Ability to customize pixel format.
            SetCommand(display, ILI9341_PIXFMT);
            CommandParameter(display, 0b01010101); // 16-bit pixel format.

            // TODO: Ability to customize/set fps
            SetCommand(display, ILI9341_FRMCTR1);
            CommandParameter(display, 0b00000000); // Internal oscillator frequency division ratio (0)
            //CommandParameter(display, 0b00011111); // 60 fps / 31 clocks per line
            CommandParameter(display, 0b00011011); // 70 fps / 27 clocks per line (default)
            //CommandParameter(display, 0b00010101); // 90 fps / 21 clocks per line
            //CommandParameter(display, 0b00010000); // 119 fps / 16 clocks per line (broken)

            Wake(display);
            SetBrightnessPercent(display, 100.0f);

            display->IsInitialized = true;
        }

        void DeinitializeIli9341(Ili9341Data* display)
        {
            Sleep(display);
        }

        void CreateBuffer(Ili9341Data* display, Buffer* buffer)
        {
            if (buffer->IsInitialized)
                free(buffer->Data);

            size_t bufferSize = display->Width * display->Height * sizeof(uint16_t);
            uint16_t* newBuffer = (uint16_t*)malloc(bufferSize);
            if (!newBuffer)
            {
                printf("[ili9341] Failed to allocate framebuffer!\n");
            }

            buffer->Width = display->Width;
            buffer->Height = display->Height;
            buffer->Data = newBuffer;
            buffer->IsInitialized = true;
        }

        void DestroyBuffer(Buffer *buffer)
        {
            free(buffer->Data);
            buffer->Width = 0;
            buffer->Height = 0;
            buffer->Data = nullptr;
            buffer->IsInitialized = false;
        }

        void SetOrientation(Ili9341Data* display, bool portrait)
        {
            display->IsPortrait = portrait;
            if (display->IsPortrait)
            {
                display->Width = 240;
                display->Height = 320;
            }
            else
            {
                display->Width = 320;
                display->Height = 240;
            }

            EnsureSPI8Bit(display);
            SetCommand(display, ILI9341_MADCTL);
            if (display->IsPortrait)
            {
                // 0b01001000: MY=0, MX=1, MV=0, BGR=1
                CommandParameter(display, 0b01001000);
            }
            else
            {
                // 0b00101000: MY=0, MX=0, MV=1, BGR=1
                CommandParameter(display, 0b00101000);
            }
        }

        void SetBrightness(Ili9341Data* display, uint16_t brightness)
        {
            // Get the PWM slice for the LED pin
            uint sliceNum = pwm_gpio_to_slice_num(display->GpioLed);

            // Enable PWM if brightness > 0, disable if 0
            if (brightness > 0)
            {
                pwm_set_enabled(sliceNum, true);
                pwm_set_gpio_level(display->GpioLed, brightness);
            }
            else
            {
                // Turn off PWM completely for zero brightness
                pwm_set_gpio_level(display->GpioLed, 0);
                pwm_set_enabled(sliceNum, false);
            }
        }

        void SetBrightnessPercent(Ili9341Data* display, float percent)
        {
            // Clamp percentage to valid range
            if (percent < 0.0f) percent = 0.0f;
            if (percent > 100.0f) percent = 100.0f;

            // Convert percentage to 16-bit PWM value
            uint16_t brightness = (uint16_t)(percent * 655.35f); // 65535 / 100 = 655.35

            SetBrightness(display, brightness);
        }

        void Sleep(Ili9341Data* display)
        {
            if (display->IsAsleep) return;

            EnsureSPI8Bit(display);
            SetCommand(display, ILI9341_DISPOFF); // Turn off display
            sleep_ms(10); // Required delay
            SetCommand(display, ILI9341_SLPIN); // Enter sleep mode
            sleep_ms(10); // Extra delay for sleep to take effect (possibly not needed)

            // Turn off backlight to save power
            pwm_set_gpio_level(display->GpioLed, 0);       // Set LED to 0%
            uint sliceNum = pwm_gpio_to_slice_num(display->GpioLed);
            pwm_set_enabled(sliceNum, false);   // Disable PWM slice
            gpio_set_function(display->GpioLed, GPIO_FUNC_SIO); // Set pin to GPIO
            gpio_set_dir(display->GpioLed, GPIO_OUT);
            gpio_put(display->GpioLed, 0); // Drive pin LOW to ensure backlight is off

            display->IsAsleep = true;
        }

        void Wake(Ili9341Data * display)
        {
            if (!display->IsAsleep) return;

            EnsureSPI8Bit(display);

            // Wake from sleep mode
            SetCommand(display, ILI9341_SLPOUT); // Exit sleep mode
            sleep_ms(120); // NOTE: Datasheet requires 120ms minimum!

            // Turn display back on
            SetCommand(display, ILI9341_DISPON); // Display on
            sleep_ms(10); // Small delay for stability

            // Restore backlight pin to PWM mode and set full brightness
            gpio_set_function(display->GpioLed, GPIO_FUNC_PWM); // Set pin back to PWM
            uint sliceNum = pwm_gpio_to_slice_num(display->GpioLed);
            pwm_set_enabled(sliceNum, true);
            pwm_set_gpio_level(display->GpioLed, 0xFFFF); // Full brightness

            display->IsAsleep = false;
        }

        void DrawBuffer(Ili9341Data *display, uint16_t x, uint16_t y, Buffer* buffer)
        {
            DrawBuffer(display, x, y, buffer->Width, buffer->Height, buffer->Data);
        }

        void DrawBuffer(Ili9341Data* display, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer)
        {
            if (width == 0 || height == 0 || buffer == nullptr) return;

            SetOutWriting(display, x, x + width - 1, y, y + height - 1);
            EnsureSPI16Bit(display);

            SetCS(display, CS_ENABLE);

            spi_write16_blocking(display->SpiPort, buffer, width * height);

            SetCS(display, CS_DISABLE);
        }

        void EnsureSPI8Bit(Ili9341Data* display)
        {
            if (display->SpiIs16Bit)
            {
                spi_set_format(display->SpiPort, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
                display->SpiIs16Bit = false;
            }
        }

        void EnsureSPI16Bit(Ili9341Data* display)
        {
            if (!display->SpiIs16Bit)
            {
                spi_set_format(display->SpiPort, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
                display->SpiIs16Bit = true;
            }
        }

        void SetCS(Ili9341Data* display, int state)
        {
            gpio_put(display->GpioCS, state);
        }

        void SetCommand(Ili9341Data* display, uint8_t command)
        {
            EnsureSPI8Bit(display);
            gpio_put(display->GpioDC, 0);
            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, &command, 1);
            SetCS(display, CS_DISABLE);
            gpio_put(display->GpioDC, 1);
        }

        void CommandParameter(Ili9341Data* display, uint8_t data)
        {
            EnsureSPI8Bit(display);
            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, &data, 1);
            SetCS(display, CS_DISABLE);
        }

        void SetOutWriting(Ili9341Data* display, const int startCol, const int endCol, const int startPage, const int endPage)
        {
            EnsureSPI8Bit(display);

            // Batch command and parameters
            uint8_t caset_data[5] = {
                static_cast<uint8_t>(ILI9341_CASET),
                static_cast<uint8_t>((startCol >> 8) & 0xFF), static_cast<uint8_t>(startCol & 0xFF),
                static_cast<uint8_t>((endCol >> 8) & 0xFF), static_cast<uint8_t>(endCol & 0xFF)
            };

            uint8_t paset_data[5] = {
                static_cast<uint8_t>(ILI9341_PASET),
                static_cast<uint8_t>((startPage >> 8) & 0xFF), static_cast<uint8_t>(startPage & 0xFF),
                static_cast<uint8_t>((endPage >> 8) & 0xFF), static_cast<uint8_t>(endPage & 0xFF)
            };

            // Send CASET
            gpio_put(display->GpioDC, 0);
            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, &caset_data[0], 1);
            SetCS(display, CS_DISABLE);
            gpio_put(display->GpioDC, 1);

            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, &caset_data[1], 4);
            SetCS(display, CS_DISABLE);

            // Send PASET
            gpio_put(display->GpioDC, 0);
            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, &paset_data[0], 1);
            SetCS(display, CS_DISABLE);
            gpio_put(display->GpioDC, 1);

            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, &paset_data[1], 4);
            SetCS(display, CS_DISABLE);

            // Start writing
            gpio_put(display->GpioDC, 0);
            SetCS(display, CS_ENABLE);
            uint8_t ramwr = ILI9341_RAMWR;
            spi_write_blocking(display->SpiPort, &ramwr, 1);
            SetCS(display, CS_DISABLE);
            gpio_put(display->GpioDC, 1);
        }

        void WriteData8bit(Ili9341Data* display, const uint8_t *buffer, int bytes)
        {
            EnsureSPI8Bit(display);
            SetCS(display, CS_ENABLE);
            spi_write_blocking(display->SpiPort, buffer, bytes);
            SetCS(display, CS_DISABLE);
        }

        void WriteData16bit(Ili9341Data* display, const uint16_t *buffer, int count)
        {
            EnsureSPI16Bit(display);
            SetCS(display, CS_ENABLE);
            spi_write16_blocking(display->SpiPort, buffer, count);
            SetCS(display, CS_DISABLE);
        }
    }
}
