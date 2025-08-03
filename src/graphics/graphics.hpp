#pragma once

#include "drivers/display/ili9341.hpp"
#include <cstdint>

namespace PicoPixel
{
    namespace Graphics
    {
        // TODO: Implement a math library. Think of GLM. Vec2, Vec3, Mat4, etc.

        void DrawPixel(PicoPixel::Driver::Buffer* buffer, uint16_t x, uint16_t y, uint16_t color);
        void DrawLine(PicoPixel::Driver::Buffer* buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
        void DrawTriangle(PicoPixel::Driver::Buffer* buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color, bool filled = true);
        void DrawRectangle(PicoPixel::Driver::Buffer* buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, bool filled = true);
        void DrawCircle(PicoPixel::Driver::Buffer* buffer, uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color, bool filled = true);
        void DrawPolygon(PicoPixel::Driver::Buffer* buffer, const uint16_t* xPoints, const uint16_t* yPoints, uint16_t numPoints, uint16_t color, bool filled = true);
        void DrawBitmap(PicoPixel::Driver::Buffer* buffer, uint16_t x, uint16_t y, const uint16_t* bitmap, uint16_t width, uint16_t height);
        void FillBuffer(PicoPixel::Driver::Buffer* buffer, uint16_t color);

        void DisplayTest(PicoPixel::Driver::Buffer* buffer);
    }
}
