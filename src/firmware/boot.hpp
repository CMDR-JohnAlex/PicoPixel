#pragma once

#include "drivers/display/ili9341.hpp"

namespace PicoPixel
{
    // Initializes hardware, and shows a splashscreen.
    bool BootSequence(ili9341* display);

    // Diagnostics and testing.
    bool RunDiagnostics(ili9341* display);
}