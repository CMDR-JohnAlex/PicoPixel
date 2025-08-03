#include "graphics.hpp"
#include "utils/color.hpp"
#include <cstdlib>
#include <algorithm>

namespace PicoPixel
{
    namespace Graphics
    {
        void DrawPixel(PicoPixel::Driver::Buffer* buffer, uint16_t x, uint16_t y, uint16_t color)
        {
            if (!buffer || !buffer->Data) return;
            if (x >= buffer->Width || y >= buffer->Height) return;
            uint32_t idx = y * buffer->Width + x;
            buffer->Data[idx] = color;
        }

        void DrawLine(PicoPixel::Driver::Buffer* buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
        {
            if (!buffer || !buffer->Data) return;
            if (x1 >= buffer->Width || y1 >= buffer->Height) return;
            if (x2 >= buffer->Width || y2 >= buffer->Height) return;

            // Bresenham's line algorithm
            // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
            int dx = abs((int)x2 - (int)x1);
            int sx = x1 < x2 ? 1 : -1;
            int dy = -abs((int)y2 - (int)y1);
            int sy = y1 < y2 ? 1 : -1;
            int err = dx + dy;
            int x = x1;
            int y = y1;
            while (true)
            {
                DrawPixel(buffer, x, y, color);
                if (x == (int)x2 && y == (int)y2) break;
                int e2 = 2 * err;
                if (e2 >= dy) { err += dy; x += sx; }
                if (e2 <= dx) { err += dx; y += sy; }
            }
        }

        void DrawTriangle(PicoPixel::Driver::Buffer* buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color, bool filled)
        {
            if (!buffer || !buffer->Data) return;
            if (x1 >= buffer->Width || y1 >= buffer->Height) return;
            if (x2 >= buffer->Width || y2 >= buffer->Height) return;
            if (x3 >= buffer->Width || y3 >= buffer->Height) return;

            if (!filled)
            {
                DrawLine(buffer, x1, y1, x2, y2, color);
                DrawLine(buffer, x2, y2, x3, y3, color);
                DrawLine(buffer, x3, y3, x1, y1, color);
            }
            else
            {
                // Sort vertices by y (y1 <= y2 <= y3)
                if (y2 < y1) { std::swap(x1, x2); std::swap(y1, y2); }
                if (y3 < y1) { std::swap(x1, x3); std::swap(y1, y3); }
                if (y3 < y2) { std::swap(x2, x3); std::swap(y2, y3); }

                auto edgeIntercept = [](int x0, int y0, int x1, int y1, int y) -> int {
                    if (y1 == y0) return x0;
                    return x0 + (x1 - x0) * (y - y0) / (y1 - y0);
                };

                for (int y = y1; y <= y3; y++)
                {
                    if (y < 0 || y >= (int)buffer->Height) continue;
                    int xa, xb;
                    if (y < y2)
                    {
                        xa = edgeIntercept(x1, y1, x2, y2, y);
                        xb = edgeIntercept(x1, y1, x3, y3, y);
                    }
                    else
                    {
                        xa = edgeIntercept(x2, y2, x3, y3, y);
                        xb = edgeIntercept(x1, y1, x3, y3, y);
                    }
                    if (xa > xb) std::swap(xa, xb);
                    if (xb < 0 || xa >= (int)buffer->Width) continue;
                    if (xa < 0) xa = 0;
                    if (xb >= (int)buffer->Width) xb = buffer->Width - 1;
                    DrawLine(buffer, xa, y, xb, y, color);
                }
            }
        }

        void DrawRectangle(PicoPixel::Driver::Buffer* buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, bool filled)
        {
            if (!buffer || !buffer->Data) return;
            if (x >= buffer->Width || y >= buffer->Height) return;
            if (width == 0 || height == 0) return;
            if (x + width > buffer->Width || y + height > buffer->Height) return;

            if (!filled)
            {
                DrawLine(buffer, x, y, x + width - 1, y, color);
                DrawLine(buffer, x, y, x, y + height - 1, color);
                DrawLine(buffer, x + width - 1, y, x + width - 1, y + height - 1, color);
                DrawLine(buffer, x, y + height - 1, x + width - 1, y + height - 1, color);
            }
            else
            {
                for (uint16_t yy = y; yy < y + height; yy++)
                    DrawLine(buffer, x, yy, x + width - 1, yy, color);
            }
        }

        void DrawCircle(PicoPixel::Driver::Buffer* buffer, uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color, bool filled)
        {
            if (!buffer || !buffer->Data) return;
            if (centerX >= buffer->Width || centerY >= buffer->Height) return;
            if (radius == 0) return;

            // Midpoint circle algorithm
            // https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
            int x = 0;
            int y = radius;
            int d = 1 - radius;

            auto drawCirclePoints = [&](int cx, int cy, int x, int y)
            {
                if (!filled)
                {
                    DrawPixel(buffer, cx + x, cy + y, color);
                    DrawPixel(buffer, cx - x, cy + y, color);
                    DrawPixel(buffer, cx + x, cy - y, color);
                    DrawPixel(buffer, cx - x, cy - y, color);
                    DrawPixel(buffer, cx + y, cy + x, color);
                    DrawPixel(buffer, cx - y, cy + x, color);
                    DrawPixel(buffer, cx + y, cy - x, color);
                    DrawPixel(buffer, cx - y, cy - x, color);
                }
                else
                {
                    // Draw horizontal lines between symmetric points for filled circle
                    DrawLine(buffer, cx - x, cy + y, cx + x, cy + y, color);
                    DrawLine(buffer, cx - x, cy - y, cx + x, cy - y, color);
                    DrawLine(buffer, cx - y, cy + x, cx + y, cy + x, color);
                    DrawLine(buffer, cx - y, cy - x, cx + y, cy - x, color);
                }
            };

            while (x <= y)
            {
                drawCirclePoints(centerX, centerY, x, y);
                if (d < 0)
                {
                    d += 2 * x + 3;
                }
                else
                {
                    d += 2 * (x - y) + 5;
                    y--;
                }
                x++;
            }
        }

        void DrawPolygon(PicoPixel::Driver::Buffer* buffer, const uint16_t* xPoints, const uint16_t* yPoints, uint16_t numPoints, uint16_t color, bool filled)
        {
            if (!buffer || !buffer->Data) return;
            if (!xPoints || !yPoints) return;
            if (numPoints < 3) return;
            for (uint16_t i = 0; i < numPoints; i++)
            {
                if (xPoints[i] >= buffer->Width || yPoints[i] >= buffer->Height) return;
            }

            if (!filled)
            {
                // Draw outline by connecting each point to the next, and last to first
                for (uint16_t i = 0; i < numPoints; i++)
                {
                    uint16_t x0 = xPoints[i];
                    uint16_t y0 = yPoints[i];
                    uint16_t x1 = xPoints[(i + 1) % numPoints];
                    uint16_t y1 = yPoints[(i + 1) % numPoints];
                    DrawLine(buffer, x0, y0, x1, y1, color);
                }
            }
            else
            {
                // TODO: Filled polygons.
            }
        }

        void DrawBitmap(PicoPixel::Driver::Buffer* buffer, uint16_t x, uint16_t y, const uint16_t* bitmap, uint16_t width, uint16_t height)
        {
            if (!buffer || !buffer->Data || !bitmap) return;
            if (x >= buffer->Width || y >= buffer->Height) return;
            if (width == 0 || height == 0) return;
            if (x + width > buffer->Width || y + height > buffer->Height) return;

            // Copy bitmap data into buffer at (x, y)
            for (uint16_t row = 0; row < height; row++)
            {
                if (y + row >= buffer->Height) break;
                for (uint16_t col = 0; col < width; col++)
                {
                    if (x + col >= buffer->Width) break;
                    buffer->Data[(y + row) * buffer->Width + (x + col)] = bitmap[row * width + col];
                }
            }
        }

        void FillBuffer(PicoPixel::Driver::Buffer* buffer, uint16_t color)
        {
            if (!buffer || !buffer->Data) return;
            for (uint32_t i = 0; i < buffer->Width * buffer->Height; i++)
            {
                buffer->Data[i] = color;
            }
        }

        void DisplayTest(PicoPixel::Driver::Buffer* buffer)
        {
            if (!buffer || !buffer->Data) return;
            uint16_t width = buffer->Width;
            uint16_t height = buffer->Height;

            // Clear the buffer to black
            FillBuffer(buffer, 0x0000);

            // Draw corner rectangles
            DrawRectangle(buffer, 0, 0, 50, 50, PicoPixel::Utils::RGBto16bit(0, 255, 0), true); // Green
            DrawRectangle(buffer, 0, height - 50, 50, 50, PicoPixel::Utils::RGBto16bit(0, 0, 255), true); // Blue
            DrawRectangle(buffer, width - 50, 0, 50, 50, PicoPixel::Utils::RGBto16bit(255, 0, 0), true); // Red
            DrawRectangle(buffer, width - 50, height - 50, 50, 50, PicoPixel::Utils::RGBto16bit(150, 75, 0), true); // Brown
            DrawRectangle(buffer, width - 50, height - 50, 25, 25, PicoPixel::Utils::RGBto16bit(255, 0, 255), true); // Magenta

            // Draw color bars
            int bar = (width - 100) / 4;
            for (int y = 0; y < height; y++)
            {
                float p = (float)y / (float)height;
                uint8_t c = p * 255.0f;
                // Red bar
                DrawLine(buffer, 50, y, 50 + bar - 1, y, PicoPixel::Utils::RGBto16bit(c, 0, 0));
                // Green bar
                DrawLine(buffer, 50 + bar, y, 50 + 2 * bar - 1, y, PicoPixel::Utils::RGBto16bit(0, c, 0));
                // Blue bar
                DrawLine(buffer, 50 + 2 * bar, y, 50 + 3 * bar - 1, y, PicoPixel::Utils::RGBto16bit(0, 0, c));

                // HSV rainbow bar
                float hue = (1.0f - p) * 6.0f;
                int sector = (int)hue;
                float f = hue - sector;
                sector = sector % 6;
                uint8_t r, g, b;
                switch(sector)
                {
                    case 0: r = 255; g = f * 255; b = 0; break;
                    case 1: r = (1-f) * 255; g = 255; b = 0; break;
                    case 2: r = 0; g = 255; b = f * 255; break;
                    case 3: r = 0; g = (1-f) * 255; b = 255; break;
                    case 4: r = f * 255; g = 0; b = 255; break;
                    case 5: r = 255; g = 0; b = (1-f) * 255; break;
                    default: r = 255; g = 0; b = 0; break;
                }
                uint16_t rgb565 = PicoPixel::Utils::RGBto16bit(r, g, b);
                DrawLine(buffer, 50 + 3 * bar, y, 50 + 4 * bar - 1, y, rgb565);
            }
        }
    }
}
