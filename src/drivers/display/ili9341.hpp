#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "ili9341HardwareCommands.hpp"

namespace PicoPixel
{
    namespace Driver
    {
        // TODO: Add (x, y) position
        struct Buffer
        {
            uint16_t Width;
            uint16_t Height;
            uint16_t* Data;
            bool IsInitialized = false;
        };

        struct Ili9341Data
        {
            // Hardware Configuration
            spi_inst_t* SpiPort;        /** Pointer to the SPI instance (spi0 or spi1). */
            int SpiClockFreqency;       /** SPI clock frequency in Hz. */
            uint8_t GpioCS;             /** Chip Select (CS) pin number (active low). */
            uint8_t GpioRESET;          /** Hardware Reset pin number (active low). */
            uint8_t GpioDC;             /** Data/Command (D/C) selection pin number. */
            uint8_t GpioSDI_MOSI;       /** SPI MOSI (Master Out, Slave In) pin number. */
            uint8_t GpioSCK;            /** SPI Serial Clock (SCK) pin number. */
            uint8_t GpioLed;            /** Backlight LED control pin number (PWM capable). */
            uint8_t GpioSDO_MISO;       /** SPI MISO (Master In, Slave Out) pin number. */

            // State Management
            bool IsInitialized = false; /** True if the display has been initialized. */
            bool SpiIs16Bit = false;    /** True if SPI is currently in 16-bit mode, false for 8-bit. */
            bool IsPortrait;            /** True if display is in portrait orientation, false for landscape. */
            uint16_t Width;             /** Current display width in pixels. */
            uint16_t Height;            /** Current display height in pixels. */
            bool IsAsleep = true;       /** True if the display is in sleep mode. */
        };

        void InitializeIli9341(Ili9341Data* display, spi_inst_t* spiPort, int spiClockFreqency, uint8_t gpioCS, uint8_t gpioRESET, uint8_t gpioDC, uint8_t gpioSDI_MOSI, uint8_t gpioSCK, uint8_t gpioLed, uint8_t gpioSDO_MISO, bool portrait);
        void DeinitializeIli9341(Ili9341Data* display);

        void CreateBuffer(Ili9341Data* display, Buffer* buffer);
        void DestroyBuffer(Buffer* buffer);

        void SetOrientation(Ili9341Data* display, bool portrait);

        void SetBrightness(Ili9341Data* display, uint16_t brightness);
        void SetBrightnessPercent(Ili9341Data* display, float percent);

        void Sleep(Ili9341Data* display);
        void Wake(Ili9341Data* display);

        void DrawBuffer(Ili9341Data* display, uint16_t x, uint16_t y, Buffer* buffer);
        void DrawBuffer(Ili9341Data* display, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer);

        void EnsureSPI8Bit(Ili9341Data* display);
        void EnsureSPI16Bit(Ili9341Data* display);

        void SetCS(Ili9341Data* display, int state);
        void SetCommand(Ili9341Data* display, uint8_t command);
        void CommandParameter(Ili9341Data* display, uint8_t data);
        void SetOutWriting(Ili9341Data* display, const int startCol, const int endCol, const int startPage, const int endPage);
        void WriteData8bit(Ili9341Data* display, const uint8_t* buffer, int bytes);
        void WriteData16bit(Ili9341Data* display, const uint16_t* buffer, int count);
    }
}
