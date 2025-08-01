#pragma once

#include "drivers/display/ili9341.hpp"

namespace PicoPixel
{
    // Initializes hardware, and shows a splashscreen.
    bool BootSequence(PicoPixel::Driver::Ili9341Data* ili9341Data);

    // Diagnostics and testing.
    bool RunDiagnostics(PicoPixel::Driver::Ili9341Data* ili9341Data);
}
