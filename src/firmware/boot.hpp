#pragma once

#include "drivers/display/ili9341.hpp"

namespace PicoPixel
{
    // Initializes hardware, display, and shows a splashscreen.
    bool BootSequence(const DisplayGPIO& displayGPIO);

    // Diagnostics and testing.
    bool RunDiagnostics(ili9341* display);
}