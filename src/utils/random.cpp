#include "random.hpp"
#include <cstdlib>
#include "log.hpp"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

namespace PicoPixel
{
    namespace Utils
    {

        // Helper to gather entropy from ADC noise
        static uint16_t GetADCEntropy()
        {
            adc_init();
            adc_select_input(0); // Use ADC0 (GPIO26)
            sleep_us(10); // Give it a moment to sit
            uint16_t noise = 0;
            for (int i = 0; i < 16; i++)
            {
                noise ^= adc_read();
                sleep_us(2);
            }
            return noise;
        }

        void InitRand()
        {
            // Use microsecond timer
            uint32_t seed = time_us_64();

            // Use ADC noise
            seed ^= GetADCEntropy();

            // Use core temperature sensor
            adc_select_input(4); // 4 = internal temperature sensor
            seed ^= adc_read();

            // Use address of a stack variable
            int dummy;
            seed ^= (uintptr_t)&dummy;

            // Use the system clock
            seed ^= (uint32_t)clock_get_hz(clk_sys);

            srand(seed);
            LOG("Seed: %u\n", seed);
            LOG("First 10 random numbers: ");
            for (int i = 0; i < 10; i++)
            {
                printf("%u ", (unsigned)rand());
            }
            printf("\n");
        }

        uint16_t Rand()
        {
            return (uint16_t)rand();
        }

        uint16_t RandRange(uint16_t max)
        {
            return (max == 0) ? 0 : (Rand() % max);
        }

    }
}
