#pragma once

#include "drivers/display/ili9341.hpp"

namespace PicoPixel
{
    namespace Menu
    {
        void LaunchMenu(PicoPixel::Driver::Ili9341Data* ili9341Data, PicoPixel::Driver::Buffer* buffer);
    }
}
