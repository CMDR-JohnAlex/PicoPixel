#pragma once

#include <cstdint>

namespace PicoPixel
{
    namespace Utils
    {
        // Initialize the random number generator
        void InitRand();

        // Get a random 16-bit value
        uint16_t Rand();

        // Get a random value in [0, max)
        uint16_t RandRange(uint16_t max);
    }
}