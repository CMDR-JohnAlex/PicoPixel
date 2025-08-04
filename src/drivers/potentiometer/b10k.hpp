#pragma once

#include <cstdint>

namespace PicoPixel
{
    namespace B10kDriver
    {
        struct B10kData
        {
            // Hardware Configuration
            uint8_t gpio;				/** Potentiometer pin number. */
			uint8_t adc;				/** ADC identifier of the potentiometer. Ensure the potentiometer is connected to one of 3 ADC pins. */
        };

        void InitializeB10k(B10kData* potentiometer, uint8_t gpio, uint8_t adc);
        void DeinitializeB10k(B10kData* potentiometer);

		int ReadB10k(B10kData* potentiometer);
    }
}
