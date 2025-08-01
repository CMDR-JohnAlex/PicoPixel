#pragma once

#include <cstdint>

namespace PicoPixel
{
    namespace Utils
    {
        uint16_t RGBto16bit(uint8_t r, uint8_t g, uint8_t b);
        uint16_t RGBAto16bit(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    }
}
