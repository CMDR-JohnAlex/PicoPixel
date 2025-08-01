#include "color.hpp"
#include <array>

namespace PicoPixel
{
    namespace Utils
    {
// TODO: Move to CMake file as an option.
#define SPEED_OPTIMIZATION
#ifdef SPEED_OPTIMIZATION // (Uses more memory)

        // Each lookup table: 256 entries × 2 bytes = 512 bytes
        // Total: 3 tables × 512 bytes = 1,536 bytes (1.5 KB)?

        // Generate lookup tables at compile time using constexpr functions
        constexpr std::array<uint16_t, 256> make_r5_lookup()
        {
            std::array<uint16_t, 256> arr{};
            for (int i = 0; i < 256; i++)
            {
                arr[i] = ((i >> 3) << 11);
            }
            return arr;
        }

        constexpr std::array<uint16_t, 256> make_g6_lookup()
        {
            std::array<uint16_t, 256> arr{};
            for (int i = 0; i < 256; i++)
            {
                arr[i] = ((i >> 2) << 5);
            }
            return arr;
        }

        constexpr std::array<uint16_t, 256> make_b5_lookup()
        {
            std::array<uint16_t, 256> arr{};
            for (int i = 0; i < 256; i++)
            {
                arr[i] = (i >> 3);
            }
            return arr;
        }

        static const auto r5_lookup = make_r5_lookup();
        static const auto g6_lookup = make_g6_lookup();
        static const auto b5_lookup = make_b5_lookup();

        uint16_t RGBto16bit(uint8_t r, uint8_t g, uint8_t b)
        {
            // Ultra-fast lookup table conversion! Woo!
            return r5_lookup[r] | g6_lookup[g] | b5_lookup[b];
        }
#else
        uint16_t RGBto16bit(uint8_t r, uint8_t g, uint8_t b)
        {
            // Fast bit manipulation
            uint16_t r5 = r >> 3;   // 8-bit to 5-bit
            uint16_t g6 = g >> 2;   // 8-bit to 6-bit
            uint16_t b5 = b >> 3;   // 8-bit to 5-bit

            uint16_t res = (r5 << 11) | (g6 << 5) | b5;

            return res;
        }
#endif

        uint16_t RGBAto16bit(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            if (a == 0xFF)
            {
                // Fully opaque - use fast path
                return RGBto16bit(r, g, b);
            }
            else if (a == 0x00)
            {
                // Fully transparent - return black
                return 0x0000;
            }
            else
            {
                // Alpha blend using fast integer math (avoid float operations)
                uint16_t invAlpha = 255 - a;
                r = (r * a) >> 8;  // Fast division by 256 (close to 255)
                g = (g * a) >> 8;
                b = (b * a) >> 8;
                return RGBto16bit(r, g, b);
            }
        }
    }
}
