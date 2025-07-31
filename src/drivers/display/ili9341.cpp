#include "ili9341.hpp"
#include "hardware/pwm.h"
#include <array>
#include <string>

#include "fonts/RobotoMono-SemiBold.h"

ili9341::ili9341(spi_inst_t *spiPort, int spiClockFreqency, DisplayGPIO displayGPIO, bool portrait)
{
    m_IsPortrait = portrait;
    if (m_IsPortrait)
    {
        m_Width = 240;
        m_Height = 320;
    }
    else
    {
        m_Width = 320;
        m_Height = 240;
    }

    m_SpiPort = spiPort;
    m_SpiClockFreqency = spiClockFreqency;
    m_GpioCS = displayGPIO.CS;
    m_GpioRESET = displayGPIO.RESET;
    m_GpioDC = displayGPIO.DC;
    m_GpioMOSI = displayGPIO.SDI_MOSI;
    m_GpioSCK = displayGPIO.SCK;
    m_Led = displayGPIO.LED;
    m_GpioMISO = displayGPIO.SDO_MISO;

    // Setup GPIO stuffs
    gpio_init(m_Led);
    gpio_set_function(m_Led, GPIO_FUNC_PWM);
    pwm_set_gpio_level(m_Led, 0xFFFF);
    uint sliceNum = pwm_gpio_to_slice_num(m_Led);
    pwm_set_enabled(sliceNum, true);

    spi_init(m_SpiPort, m_SpiClockFreqency);
    const int actualBaudrate = spi_set_baudrate(m_SpiPort, m_SpiClockFreqency);
    printf("Requested: %d Hz, Actual: %d Hz\n", m_SpiClockFreqency, actualBaudrate);

    // Set default to 8-bit for commands
    spi_set_format(m_SpiPort, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(m_GpioMISO, GPIO_FUNC_SPI);
    gpio_set_function(m_GpioSCK, GPIO_FUNC_SPI);
    gpio_set_function(m_GpioMOSI, GPIO_FUNC_SPI);

    gpio_init(m_GpioCS);
    gpio_set_dir(m_GpioCS, GPIO_OUT);
    gpio_put(m_GpioCS, 1);

    gpio_init(m_GpioRESET);
    gpio_set_dir(m_GpioRESET, GPIO_OUT);
    gpio_put(m_GpioRESET, 1);

    gpio_init(m_GpioDC);
    gpio_set_dir(m_GpioDC, GPIO_OUT);
    gpio_put(m_GpioDC, 0);

    // Hardware reset
    sleep_ms(10);
    gpio_put(m_GpioRESET, 0);
    sleep_ms(10);
    gpio_put(m_GpioRESET, 1);

    // Software reset
    SetCommand(ILI9341_SWRESET);
    sleep_ms(100); // NOTE: Required to wait at least 5ms before sending new commands, but appears that we need more than 5ms.

    // Gamma correction
    SetCommand(ILI9341_GAMMASET);
    CommandParameter(0b00000100); // Gamma curve 4

    // // Positive gamma correction
    // {
    //     SetCommand(ILI9341_GMCTRP1);
    //     uint8_t data[15] = { 0b00001111, 0b00110001, 0b00101011, 0b00001100, 0b00001110, 0b00001000, 0b01001110, 0b11110001, 0b00110111, 0b00000111, 0b00010000, 0b00000011, 0b00001110, 0b00001001, 0b00000000 };
    //     WriteData8bit(data, 15);
    // }

    // // Negative gamma correction
    // {
    //     SetCommand(ILI9341_GMCTRN1);
    //     uint8_t data[15]= { 0b00000000, 0b00001110, 0b00010100, 0b00000011, 0b00010001, 0b00000111, 0b00110001, 0b11000001, 0b01001000, 0b00001000, 0b00001111, 0b00001100, 0b00110001, 0b00110110, 0b00001111 };
    //     WriteData8bit(data, 15);
    // }

    SetCommand(ILI9341_MADCTL);
    if (IsPortrait())
    {
        // 0b01001000: MY=0, MX=1, MV=0, BGR=1
        // Portrait mode: Top-left origin, normal row/column order, BGR color order
        CommandParameter(0b01001000);
    }
    else
    {
        // 0b00101000: MY=0, MX=0, MV=1, BGR=1
        // Landscape mode: Row/column exchange, BGR color order
        CommandParameter(0b00101000);
    }

    // TODO: Ability to customize pixel format. 18-bit would be cool...?
    SetCommand(ILI9341_PIXFMT);
    CommandParameter(0b01010101); // 16-bit pixel format.

    // TODO: Ability to customize/set fps
    SetCommand(ILI9341_FRMCTR1);
    CommandParameter(0b00000000); // Internal oscillator frequency division ratio (0)
    CommandParameter(0b00011011); // 70 fps / 27 clocks per line (default)
    //CommandParameter(0b00011111); // 61 fps / 31 clock per line

    Wake();
    SetBrightnessPercent(100.0f);

    // Switch to 16-bit mode for all future operations (rendering)
    spi_set_format(m_SpiPort, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    m_SpiIs16Bit = true;
}

ili9341::~ili9341()
{
    Sleep();
}

// TODO: Move to CMake file as an option.
#define SPEED_OPTIMIZATION
#ifdef SPEED_OPTIMIZATION // (Uses more memory)

// Each lookup table: 256 entries × 2 bytes = 512 bytes
// Total: 3 tables × 512 bytes = 1,536 bytes (1.5 KB)?

// Generate lookup tables at compile time using lambdas
static constexpr auto r5_lookup =
    []()
    {
        std::array<uint16_t, 256> arr{};
        for (int i = 0; i < 256; ++i)
        {
            arr[i] = ((i >> 3) << 11);
        }
        return arr;
    }();

static constexpr auto g6_lookup =
    []()
    {
        std::array<uint16_t, 256> arr{};
        for (int i = 0; i < 256; ++i)
        {
            arr[i] = ((i >> 2) << 5);
        }
        return arr;
    }();

static constexpr auto b5_lookup =
    []()
    {
        std::array<uint16_t, 256> arr{};
        for (int i = 0; i < 256; ++i)
        {
            arr[i] = (i >> 3);
        }
        return arr;
    }();

uint16_t ili9341::RGBto16bit(uint8_t r, uint8_t g, uint8_t b)
{
    // Ultra-fast lookup table conversion! Woo!
    return r5_lookup[r] | g6_lookup[g] | b5_lookup[b];
}
#else
uint16_t ili9341::RGBto16bit(uint8_t r, uint8_t g, uint8_t b)
{
    // Optimized for ARM Cortex-M0+
    // Fast bit manipulation, no division or multiplication needed
    uint16_t r5 = r >> 3;           // 8-bit to 5-bit (faster than division)
    uint16_t g6 = g >> 2;           // 8-bit to 6-bit
    uint16_t b5 = b >> 3;           // 8-bit to 5-bit

    uint16_t res = (r5 << 11) | (g6 << 5) | b5;

    return res;
}
#endif

uint16_t ili9341::RGBAto16bit(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
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

void ili9341::DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    SetOutWriting(x, x, y, y);
    EnsureSPI16Bit();
    SetCS(CS_ENABLE);
    spi_write16_blocking(m_SpiPort, &color, 1);
    SetCS(CS_DISABLE);
}

void ili9341::DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    // Handle special cases first for maximum speed
    if (x0 == x1)
    {
        // Vertical line - use optimized vertical line drawing
        if (y0 > y1) { uint16_t temp = y0; y0 = y1; y1 = temp; }
        SetOutWriting(x0, x0, y0, y1);
        EnsureSPI16Bit();

        int height = y1 - y0 + 1;
        SetCS(CS_ENABLE);
        for (int i = 0; i < height; i++)
        {
            spi_write16_blocking(m_SpiPort, &color, 1);
        }
        SetCS(CS_DISABLE);
        return;
    }

    if (y0 == y1)
    {
        // Horizontal line - use optimized horizontal line drawing
        if (x0 > x1) { uint16_t temp = x0; x0 = x1; x1 = temp; }
        SetOutWriting(x0, x1, y0, y0);
        EnsureSPI16Bit();

        int width = x1 - x0 + 1;
        SetCS(CS_ENABLE);
        for (int i = 0; i < width; i++)
        {
            spi_write16_blocking(m_SpiPort, &color, 1);
        }
        SetCS(CS_DISABLE);
        return;
    }

    // Bresenham's line algorithm for diagonal lines
    int dx = abs((int)x1 - (int)x0);
    int dy = abs((int)y1 - (int)y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int x = x0, y = y0;

    while (true)
    {
        // TODO: More optimized method of rendering, other than the pixel drawing?
        DrawPixel(x, y, color);

        // Check if we've reached the end
        if (x == x1 && y == y1) break;

        // Calculate error and step
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y += sy;
        }
    }
}

void ili9341::DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
    DrawLine(x1, y1, x2, y2, color); // Side 1->2
    DrawLine(x2, y2, x3, y3, color); // Side 2->3
    DrawLine(x3, y3, x1, y1, color); // Side 3->1
}

void ili9341::DrawTriangleFilled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color)
{
    // Sort vertices by Y coordinate for efficient scanline filling
    if (y1 > y2)
    {
        uint16_t tx = x1, ty = y1;
        x1 = x2; y1 = y2;
        x2 = tx; y2 = ty;
    }
    if (y2 > y3)
    {
        uint16_t tx = x2, ty = y2;
        x2 = x3; y2 = y3;
        x3 = tx; y3 = ty;
        // Re-check first pair after swap
        if (y1 > y2)
        {
            tx = x1; ty = y1;
            x1 = x2; y1 = y2;
            x2 = tx; y2 = ty;
        }
    }

    // Handle degenerate case (all points on same horizontal line)
    if (y1 == y3)
    {
        uint16_t minX = (x1 < x2) ? ((x1 < x3) ? x1 : x3) : ((x2 < x3) ? x2 : x3);
        uint16_t maxX = (x1 > x2) ? ((x1 > x3) ? x1 : x3) : ((x2 > x3) ? x2 : x3);
        if (maxX > minX)
        {
            DrawHorizontalLine(minX, y1, maxX - minX + 1, color);
        }
        return;
    }

    // Pre-calculate slopes using fixed-point arithmetic (16.16 format) for speed
    int32_t dx13 = ((int32_t)(x3 - x1) << 16) / (y3 - y1); // Long edge 1->3
    int32_t dx12 = (y2 != y1) ? (((int32_t)(x2 - x1) << 16) / (y2 - y1)) : 0; // Edge 1->2
    int32_t dx23 = (y3 != y2) ? (((int32_t)(x3 - x2) << 16) / (y3 - y2)) : 0; // Edge 2->3

    // Starting positions in fixed-point
    int32_t x_left = (int32_t)x1 << 16;
    int32_t x_right = (int32_t)x1 << 16;

    // Fill upper part (y1 to y2-1) - exclude y2 to avoid double-drawing
    for (int y = y1; y < y2; y++)
    {
        int xl = x_left >> 16;
        int xr = x_right >> 16;

        // Ensure left <= right
        if (xl > xr)
        {
            int temp = xl; xl = xr; xr = temp;
        }

        // Draw horizontal line for this scanline
        if (xr >= xl)
        {
            DrawHorizontalLine(xl, y, xr - xl + 1, color);
        }

        // Step to next scanline
        x_left += dx13;
        x_right += dx12;
    }

    // Fill lower part (y2 to y3) - reset right edge and include y2
    if (y2 <= y3)
    {
        x_right = (int32_t)x2 << 16;

        for (int y = y2; y <= y3; y++)
        {
            int xl = x_left >> 16;
            int xr = x_right >> 16;

            // Ensure left <= right
            if (xl > xr)
            {
                int temp = xl; xl = xr; xr = temp;
            }

            // Draw horizontal line for this scanline
            if (xr >= xl)
            {
                DrawHorizontalLine(xl, y, xr - xl + 1, color);
            }

            // Step to next scanline
            x_left += dx13;
            x_right += dx23;
        }
    }
}

void ili9341::DrawTriangleGradient(uint16_t x1, uint16_t y1, uint16_t color1,
                                   uint16_t x2, uint16_t y2, uint16_t color2,
                                   uint16_t x3, uint16_t y3, uint16_t color3)
{
    // Sort vertices by Y coordinate
    if (y1 > y2)
    {
        uint16_t tx = x1, ty = y1, tc = color1;
        x1 = x2; y1 = y2; color1 = color2;
        x2 = tx; y2 = ty; color2 = tc;
    }
    if (y2 > y3)
    {
        uint16_t tx = x2, ty = y2, tc = color2;
        x2 = x3; y2 = y3; color2 = color3;
        x3 = tx; y3 = ty; color3 = tc;
        if (y1 > y2)
        {
            tx = x1; ty = y1; tc = color1;
            x1 = x2; y1 = y2; color1 = color2;
            x2 = tx; y2 = ty; color2 = tc;
        }
    }

    // Handle degenerate case
    if (y1 == y3)
    {
        uint16_t minX = (x1 < x2) ? ((x1 < x3) ? x1 : x3) : ((x2 < x3) ? x2 : x3);
        uint16_t maxX = (x1 > x2) ? ((x1 > x3) ? x1 : x3) : ((x2 > x3) ? x2 : x3);
        if (maxX > minX)
        {
            // Simple color interpolation for horizontal line
            DrawHorizontalLineGradient(minX, y1, maxX - minX + 1, color1, color3);
        }
        return;
    }

    // Extract RGB components from 16-bit colors (fixed-point for speed)
    int32_t r1 = ((color1 >> 11) & 0x1F) << 16; // Red 5-bit -> 21-bit fixed point
    int32_t g1 = ((color1 >> 5) & 0x3F) << 16;  // Green 6-bit -> 22-bit fixed point
    int32_t b1 = (color1 & 0x1F) << 16;         // Blue 5-bit -> 21-bit fixed point

    int32_t r2 = ((color2 >> 11) & 0x1F) << 16;
    int32_t g2 = ((color2 >> 5) & 0x3F) << 16;
    int32_t b2 = (color2 & 0x1F) << 16;

    int32_t r3 = ((color3 >> 11) & 0x1F) << 16;
    int32_t g3 = ((color3 >> 5) & 0x3F) << 16;
    int32_t b3 = (color3 & 0x1F) << 16;

    // Pre-calculate edge deltas in fixed-point
    int32_t dx13 = ((int32_t)(x3 - x1) << 16) / (y3 - y1);
    int32_t dx12 = (y2 != y1) ? (((int32_t)(x2 - x1) << 16) / (y2 - y1)) : 0;
    int32_t dx23 = (y3 != y2) ? (((int32_t)(x3 - x2) << 16) / (y3 - y2)) : 0;

    // Color deltas for interpolation
    int32_t dr13 = (y3 != y1) ? ((r3 - r1) / (y3 - y1)) : 0;
    int32_t dg13 = (y3 != y1) ? ((g3 - g1) / (y3 - y1)) : 0;
    int32_t db13 = (y3 != y1) ? ((b3 - b1) / (y3 - y1)) : 0;

    int32_t dr12 = (y2 != y1) ? ((r2 - r1) / (y2 - y1)) : 0;
    int32_t dg12 = (y2 != y1) ? ((g2 - g1) / (y2 - y1)) : 0;
    int32_t db12 = (y2 != y1) ? ((b2 - b1) / (y2 - y1)) : 0;

    // Starting positions
    int32_t x_left = (int32_t)x1 << 16;
    int32_t x_right = (int32_t)x1 << 16;
    int32_t r_left = r1, g_left = g1, b_left = b1;
    int32_t r_right = r1, g_right = g1, b_right = b1;

    // Fill upper part (y1 to y2)
    for (int y = y1; y <= y2 && y <= y3; y++)
    {
        int xl = x_left >> 16;
        int xr = x_right >> 16;

        if (xl > xr)
        {
            // Swap positions and colors
            int temp = xl; xl = xr; xr = temp;
            int32_t tr = r_left, tg = g_left, tb = b_left;
            r_left = r_right; g_left = g_right; b_left = b_right;
            r_right = tr; g_right = tg; b_right = tb;
        }

        if (xr > xl)
        {
            uint16_t color_left = ((r_left >> 16) << 11) | ((g_left >> 16) << 5) | (b_left >> 16);
            uint16_t color_right = ((r_right >> 16) << 11) | ((g_right >> 16) << 5) | (b_right >> 16);
            DrawHorizontalLineGradient(xl, y, xr - xl + 1, color_left, color_right);
        }

        // Step to next scanline
        x_left += dx13; x_right += dx12;
        r_left += dr13; g_left += dg13; b_left += db13;
        r_right += dr12; g_right += dg12; b_right += db12;
    }

    // Fill lower part (y2+1 to y3) - reset right edge
    if (y2 < y3)
    {
        x_right = (int32_t)x2 << 16;
        r_right = r2; g_right = g2; b_right = b2;

        // Calculate color deltas for lower part
        int32_t dr23 = (y3 != y2) ? ((r3 - r2) / (y3 - y2)) : 0;
        int32_t dg23 = (y3 != y2) ? ((g3 - g2) / (y3 - y2)) : 0;
        int32_t db23 = (y3 != y2) ? ((b3 - b2) / (y3 - y2)) : 0;

        for (int y = y2 + 1; y <= y3; y++)
        {
            int xl = x_left >> 16;
            int xr = x_right >> 16;

            if (xl > xr)
            {
                // Swap positions and colors
                int temp = xl; xl = xr; xr = temp;
                int32_t tr = r_left, tg = g_left, tb = b_left;
                r_left = r_right; g_left = g_right; b_left = b_right;
                r_right = tr; g_right = tg; b_right = tb;
            }

            if (xr > xl)
            {
                uint16_t color_left = ((r_left >> 16) << 11) | ((g_left >> 16) << 5) | (b_left >> 16);
                uint16_t color_right = ((r_right >> 16) << 11) | ((g_right >> 16) << 5) | (b_right >> 16);
                DrawHorizontalLineGradient(xl, y, xr - xl + 1, color_left, color_right);
            }

            // Step to next scanline
            x_left += dx13; x_right += dx23;
            r_left += dr13; g_left += dg13; b_left += db13;
            r_right += dr23; g_right += dg23; b_right += db23;
        }
    }
}

void ili9341::DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    SetOutWriting(x, x + width - 1, y, y + height - 1);
    EnsureSPI16Bit();

    if (width <= 32)
    {
        // Small rectangles: use stack buffer for speed
        uint16_t sBufLine[32];
        for (int i = 0; i < width; i++)
        {
            sBufLine[i] = color;
        }

        SetCS(CS_ENABLE);
        for (int i = 0; i < height; i++)
        {
            spi_write16_blocking(m_SpiPort, sBufLine, width);
        }
        SetCS(CS_DISABLE);
    }
    else
    {
        // Large rectangles: reuse static buffer
        static uint16_t lineBuffer[64];  // 128 bytes max

        // Fill chunks of the line buffer
        for (int i = 0; i < 64; i++) lineBuffer[i] = color;

        SetCS(CS_ENABLE);
        for (int row = 0; row < height; row++)
        {
            int remaining = width;
            while (remaining > 0)
            {
                int chunk = (remaining > 64) ? 64 : remaining;
                spi_write16_blocking(m_SpiPort, lineBuffer, chunk);
                remaining -= chunk;
            }
        }
        SetCS(CS_DISABLE);
    }
}

void ili9341::DrawText(const char *text, uint16_t x, uint16_t y, uint16_t color, uint16_t backgroundColor)
{
    if (!text) return;

    uint16_t currentX = x;
    uint16_t currentY = y;

    while (*text != '\0')
    {
        char c = *text;

        // Handle special characters
        if (c == '\n')
        {
            currentX = x; // Return to start of line
            currentY += GetFontHeight() + 4; // Move down one line (32px font + 4px spacing)
            text++;
            continue;
        }
        else if (c == '\r')
        {
            currentX = x; // Carriage return
            text++;
            continue;
        }
        // NOTE: We now have a method of drawing unsupported chars in the DrawChar() function.
        // else if (c < 32 || c > 126)
        // {
        //     // Skip unsupported characters
        //     text++;
        //     continue;
        // }

        // Draw the character and advance position
        uint16_t charWidth = DrawChar(c, currentX, currentY, color, backgroundColor);
        currentX += charWidth;

        text++;
    }
}

uint16_t ili9341::DrawChar(char c, uint16_t x, uint16_t y, uint16_t color, uint16_t backgroundColor)
{
    if (c < 32 || c > 126)
    {
        // Draw a replacement character for unsupported chars
        return DrawUnsupportedChar(x, y, color, backgroundColor);
    }

    const font_dsc_t* glyph = &RobotoMono_SemiBold_glyph_dsc[c - ' ' + 1]; // +1 from the start char (' ') because of an odd reserved 0th element.
    const uint8_t* bitmap = &RobotoMono_SemiBold_glyph_bitmap[glyph->bitmap_index];

    int16_t drawX = x + glyph->ofs_x;
    int16_t drawY = y - glyph->box_h - glyph->ofs_y;

    uint16_t charAdvance = glyph->adv_w / 16;

    printf("DrawChar '%c' at baseline (%d,%d), draw pos (%d,%d), box_h=%d, ofs_y=%d\n",
           c, x, y, drawX, drawY, glyph->box_h, glyph->ofs_y);

    int pixelsDrawn = 0;

    for (int row = 0; row < glyph->box_h; row++)
    {
        for (int col = 0; col < glyph->box_w; col++)
        {
            int bitIndex = row * glyph->box_w + col;
            int byteIndex = bitIndex / 8;
            int bitOffset = 7 - (bitIndex % 8);

            int expectedBytes = (glyph->box_w * glyph->box_h + 7) / 8;
            if (byteIndex >= expectedBytes) continue;

            bool pixelSet = (bitmap[byteIndex] >> bitOffset) & 1;

            if (pixelSet)
            {
                int16_t pixelX = drawX + col;
                int16_t pixelY = drawY + row;

                if (pixelX >= 0 && pixelX < GetWidth() &&
                    pixelY >= 0 && pixelY < GetHeight())
                {
                    DrawPixel(pixelX, pixelY, color);
                    pixelsDrawn++;
                }
            }
        }
    }

    printf("Drew %d pixels\n", pixelsDrawn);
    return charAdvance;
}

uint16_t ili9341::DrawUnsupportedChar(uint16_t x, uint16_t y, uint16_t color, uint16_t backgroundColor)
{
    const uint16_t charWidth = 10;
    const uint16_t charHeight = 16;

    // Draw background box
    if (backgroundColor != color)
    {
        DrawRectangle(x, y - charHeight, charWidth, charHeight, backgroundColor);
    }

    // Draw border box
    DrawRectangle(x, y - charHeight, charWidth, 1, color); // Top
    DrawRectangle(x, y - 1, charWidth, 1, color); // Bottom
    DrawRectangle(x, y - charHeight, 1, charHeight, color); // Left
    DrawRectangle(x + charWidth - 1, y - charHeight, 1, charHeight, color); // Right

    // Question mark
    // Top curve
    DrawPixel(x + 3, y - 14, color);
    DrawPixel(x + 4, y - 15, color);
    DrawPixel(x + 5, y - 15, color);
    DrawPixel(x + 6, y - 14, color);

    // Right side
    DrawPixel(x + 7, y - 13, color);
    DrawPixel(x + 7, y - 12, color);

    // Middle curve
    DrawPixel(x + 6, y - 11, color);
    DrawPixel(x + 5, y - 10, color);
    DrawPixel(x + 4, y - 9, color);

    // Stem
    DrawPixel(x + 4, y - 8, color);

    // Dot
    DrawPixel(x + 4, y - 6, color);
    DrawPixel(x + 4, y - 5, color);

    return 14;
}

uint16_t ili9341::GetTextWidth(const char* text)
{
    if (!text) return 0;

    uint16_t totalWidth = 0;

    while (*text != '\0')
    {
        char c = *text;

        if (c == '\n' || c == '\r' || c < 32 || c > 126)
        {
            text++;
            continue;
        }

        const font_dsc_t* glyph = &RobotoMono_SemiBold_glyph_dsc[c - ' ' + 1]; // +1 from the start char (' ') because of an odd reserved 0th element.
        totalWidth += glyph->adv_w / 16;

        text++;
    }

    return totalWidth;
}

void ili9341::DrawRawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer)
{
    if (width == 0 || height == 0 || buffer == nullptr) return;

    SetOutWriting(x, x + width - 1, y, y + height - 1);
    EnsureSPI16Bit();

    SetCS(CS_ENABLE);

    spi_write16_blocking(m_SpiPort, buffer, width * height);

    SetCS(CS_DISABLE);
}

void ili9341::Clear(uint16_t color)
{
    static uint16_t clearLine[320]; // Max width, reused across calls
    static uint16_t lastColor = 0xFFFF; // Track if we need to refill

    if (color != lastColor)
    {
        for (int i = 0; i < GetWidth(); i++)
        {
            clearLine[i] = color;
        }
        lastColor = color;
    }

    SetOutWriting(0, GetWidth() - 1, 0, GetHeight() - 1);
    EnsureSPI16Bit();

    SetCS(CS_ENABLE);
    for (int y = 0; y < GetHeight(); y++)
    {
        spi_write16_blocking(m_SpiPort, clearLine, GetWidth());
    }
    SetCS(CS_DISABLE);
}

void ili9341::ScreenTest()
{
    Clear();
    int width = GetWidth();
    int height = GetHeight();

    // Draw corner rectangles
    DrawRectangle(0, 0, 50, 50, RGBto16bit(0, 0xff, 0));
    DrawRectangle(0, height - 50, 50, 50, RGBto16bit(0, 0, 0xff));
    DrawRectangle(width - 50, 0, 50, 50, RGBto16bit(0xff, 0, 0));
    DrawRectangle(width - 50, height - 50, 50, 50, RGBto16bit(0x84, 0x45, 0x13));
    DrawRectangle(width - 50, height - 50, 25, 25, RGBto16bit(0xff, 0x00, 0xff));

    int bar = (width - 100) / 4;

    for (int y = 0; y < height; y++)
    {
        float p = (float)y / (float)height;
        uint8_t c = p * 255.0f;

        DrawHorizontalLine(50 + bar * 0, y, bar, RGBto16bit(c, 0, 0));
        DrawHorizontalLine(50 + bar * 1, y, bar, RGBto16bit(0, c, 0));
        DrawHorizontalLine(50 + bar * 2, y, bar, RGBto16bit(0, 0, c));

        // HSV calculation
        uint8_t r, g, b;
        float hue = (1.0f - p) * 6.0f;
        int sector = (int)hue;
        float f = hue - sector;
        sector = sector % 6;

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
        DrawHorizontalLine(50 + bar * 3, y, bar, RGBto16bit(r, g, b));
    }
}

void ili9341::PixelTest()
{
    Clear();

    int spacing = 10;
    int xAmount = GetWidth() / spacing;
    int yAmount = GetHeight() / spacing;

    // Draw individual pixels in a pattern
    for (int x = 0; x < xAmount; x++)
    {
        for (int y = 0; y < yAmount; y++)
        {
            uint16_t color;
            if ((x + y) % 3 == 0) color = RGBto16bit(255, 0, 0);      // Red
            else if ((x + y) % 3 == 1) color = RGBto16bit(0, 255, 0); // Green
            else color = RGBto16bit(0, 0, 255);                       // Blue

            //DrawRectangle(x * spacing, y * spacing, 1, 1, color);
            DrawPixel(x * spacing, y * spacing, color);
        }
    }
}

void ili9341::RectangleTest()
{
    Clear();

    uint16_t maxWidth = GetWidth();
    uint16_t maxHeight = GetHeight();
    uint16_t rectWidth = (rand() % (maxWidth / 4)) + 20;    // 20 to 25% of screen width
    uint16_t rectHeight = (rand() % (maxHeight / 4)) + 20;  // 20 to 25% of screen height
    uint16_t rectX = rand() % (maxWidth - rectWidth);       // Ensure rect fits horizontally
    uint16_t rectY = rand() % (maxHeight - rectHeight);     // Ensure rect fits vertically

    DrawRectangle(
        rectX,
        rectY,
        rectWidth,
        rectHeight,
        RGBto16bit(rand() % 256, rand() % 256, rand() % 256)
    );
}

void ili9341::TextTest()
{
    Clear();

    // Text with unsupported char
    std::string testStr = std::string("Hello ") + static_cast<char>(31) + " World!";
    DrawText(testStr.c_str(), 10, GetFontHeight(), RGBto16bit(255, 255, 255), 0x0000);

    // Text with background
    DrawText("Status: OK", 10, GetFontHeight() * 2, RGBto16bit(0, 255, 0), RGBto16bit(0, 0, 0));

    // Multi-line text
    DrawText("Line 1\nLine 2\nLine 3", 10, GetFontHeight() * 3, RGBto16bit(255, 0, 0));

    // Centered text
    const char* text = "Centered";
    uint16_t textWidth = GetTextWidth(text);
    uint16_t centerX = (GetWidth() - textWidth) / 2;
    DrawText(text, centerX, 150, RGBto16bit(255, 255, 0));

    // Right-aligned text
    const char* rightText = "Right Aligned";
    uint16_t rightTextWidth = GetTextWidth(rightText);
    DrawText(rightText, GetWidth() - rightTextWidth - 10, 200, RGBto16bit(255, 0, 255));
}

void ili9341::CharTest()
{
    Clear();

    DrawChar(' ', 10, 50, RGBto16bit(255, 255, 255), 0xFFFF);   // Space
    DrawChar('!', 30, 50, RGBto16bit(255, 255, 255), 0xFFFF);   // Exclamation
    DrawChar('"', 50, 50, RGBto16bit(255, 255, 255), 0xFFFF);   // Quote
    DrawChar('#', 70, 50, RGBto16bit(255, 255, 255), 0xFFFF);   // Hash
    DrawChar('$', 90, 50, RGBto16bit(255, 255, 255), 0xFFFF);   // Dollar

    DrawChar('@', 10, 100, RGBto16bit(255, 0, 0), 0xFFFF);      // @
    DrawChar('A', 50, 100, RGBto16bit(0, 255, 0), 0xFFFF);      // A
    DrawChar('B', 90, 100, RGBto16bit(0, 0, 255), 0xFFFF);      // B
    DrawChar('C', 130, 100, RGBto16bit(255, 255, 0), 0xFFFF);   // C

    const char* text = "Hello ABC gyp";
    uint16_t textWidth = GetTextWidth(text);

    // Draw a reference line to show the baseline
    DrawHorizontalLine(10, 200, textWidth, RGBto16bit(255, 0, 0)); // Red baseline

    // Draw text on the baseline - characters should sit ON the red line
    DrawText(text, 10, 200, RGBto16bit(255, 255, 255));
}

void ili9341::Sleep()
{
    if (m_IsAsleep) return;

    EnsureSPI8Bit();
    SetCommand(ILI9341_DISPOFF);        // Turn off display
    sleep_ms(10);                       // Required delay
    SetCommand(ILI9341_SLPIN);          // Enter sleep mode
    sleep_ms(10);                       // Extra delay for sleep to take effect (possibly not needed)

    // Turn off backlight to save power
    pwm_set_gpio_level(m_Led, 0);       // Set LED to 0%
    uint sliceNum = pwm_gpio_to_slice_num(m_Led);
    pwm_set_enabled(sliceNum, false);   // Disable PWM slice
    gpio_set_function(m_Led, GPIO_FUNC_SIO); // Set pin to GPIO
    gpio_set_dir(m_Led, GPIO_OUT);
    gpio_put(m_Led, 0);                 // Drive pin LOW to ensure backlight is off

    m_IsAsleep = true;
}

void ili9341::Wake()
{
    if (!m_IsAsleep) return;

    EnsureSPI8Bit();

    // Wake from sleep mode
    SetCommand(ILI9341_SLPOUT);         // Exit sleep mode
    sleep_ms(120);                      // NOTE: Datasheet requires 120ms minimum!

    // Turn display back on
    SetCommand(ILI9341_DISPON);         // Display on
    sleep_ms(10);                       // Small delay for stability

    // Restore backlight pin to PWM mode and set full brightness
    gpio_set_function(m_Led, GPIO_FUNC_PWM); // Set pin back to PWM
    uint sliceNum = pwm_gpio_to_slice_num(m_Led);
    pwm_set_enabled(sliceNum, true);
    pwm_set_gpio_level(m_Led, 0xFFFF);  // Full brightness

    m_IsAsleep = false;
}

void ili9341::SetBrightness(uint16_t brightness)
{
    // Get the PWM slice for the LED pin
    uint sliceNum = pwm_gpio_to_slice_num(m_Led);

    // Enable PWM if brightness > 0, disable if 0
    if (brightness > 0)
    {
        pwm_set_enabled(sliceNum, true);
        pwm_set_gpio_level(m_Led, brightness);
    }
    else
    {
        // Turn off PWM completely for zero brightness
        pwm_set_gpio_level(m_Led, 0);
        pwm_set_enabled(sliceNum, false);
    }
}

void ili9341::SetBrightnessPercent(float percent)
{
    // Clamp percentage to valid range
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    // Convert percentage to 16-bit PWM value
    uint16_t brightness = (uint16_t)(percent * 655.35f); // 65535 / 100 = 655.35

    SetBrightness(brightness);
}

inline void ili9341::EnsureSPI8Bit()
{
    if (m_SpiIs16Bit)
    {
        spi_set_format(m_SpiPort, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        m_SpiIs16Bit = false;
    }
}

inline void ili9341::EnsureSPI16Bit()
{
    if (!m_SpiIs16Bit)
    {
        spi_set_format(m_SpiPort, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
        m_SpiIs16Bit = true;
    }
}

inline void ili9341::SetCS(int state)
{
    gpio_put(m_GpioCS, state);
}

void ili9341::SetCommand(uint8_t command)
{
    EnsureSPI8Bit();
    gpio_put(m_GpioDC, 0);
    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, &command, 1);
    SetCS(CS_DISABLE);
    gpio_put(m_GpioDC, 1);
}

void ili9341::CommandParameter(uint8_t data)
{
    EnsureSPI8Bit();
    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, &data, 1);
    SetCS(CS_DISABLE);
}

void ili9341::SetOutWriting(const int startCol, const int endCol, const int startPage, const int endPage)
{
    EnsureSPI8Bit();

    // Batch command and parameters
    uint8_t caset_data[5] = {
        static_cast<uint8_t>(ILI9341_CASET),
        static_cast<uint8_t>((startCol >> 8) & 0xFF), static_cast<uint8_t>(startCol & 0xFF),
        static_cast<uint8_t>((endCol >> 8) & 0xFF), static_cast<uint8_t>(endCol & 0xFF)
    };

    uint8_t paset_data[5] = {
        static_cast<uint8_t>(ILI9341_PASET),
        static_cast<uint8_t>((startPage >> 8) & 0xFF), static_cast<uint8_t>(startPage & 0xFF),
        static_cast<uint8_t>((endPage >> 8) & 0xFF), static_cast<uint8_t>(endPage & 0xFF)
    };

    // Send CASET
    gpio_put(m_GpioDC, 0);
    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, &caset_data[0], 1);
    SetCS(CS_DISABLE);
    gpio_put(m_GpioDC, 1);

    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, &caset_data[1], 4);
    SetCS(CS_DISABLE);

    // Send PASET
    gpio_put(m_GpioDC, 0);
    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, &paset_data[0], 1);
    SetCS(CS_DISABLE);
    gpio_put(m_GpioDC, 1);

    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, &paset_data[1], 4);
    SetCS(CS_DISABLE);

    // Start writing
    gpio_put(m_GpioDC, 0);
    SetCS(CS_ENABLE);
    uint8_t ramwr = ILI9341_RAMWR;
    spi_write_blocking(m_SpiPort, &ramwr, 1);
    SetCS(CS_DISABLE);
    gpio_put(m_GpioDC, 1);
}

void ili9341::WriteData8bit(const uint8_t* buffer, int bytes)
{
    EnsureSPI8Bit();
    SetCS(CS_ENABLE);
    spi_write_blocking(m_SpiPort, buffer, bytes);
    SetCS(CS_DISABLE);
}

void ili9341::WriteData16bit(const uint16_t* buffer, int count)
{
    EnsureSPI16Bit();
    SetCS(CS_ENABLE);
    spi_write16_blocking(m_SpiPort, buffer, count);
    SetCS(CS_DISABLE);
}

void ili9341::DrawHorizontalLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color)
{
    if (width == 0) return;

    SetOutWriting(x, x + width - 1, y, y);
    EnsureSPI16Bit();

    SetCS(CS_ENABLE);
    for (int i = 0; i < width; i++)
    {
        spi_write16_blocking(m_SpiPort, &color, 1);
    }
    SetCS(CS_DISABLE);
}

void ili9341::DrawHorizontalLineGradient(uint16_t x, uint16_t y, uint16_t width, uint16_t color1, uint16_t color2)
{
    if (width <= 1)
    {
        if (width == 1) DrawPixel(x, y, color1);
        return;
    }

    // Extract RGB components
    int32_t r1 = (color1 >> 11) & 0x1F;
    int32_t g1 = (color1 >> 5) & 0x3F;
    int32_t b1 = color1 & 0x1F;

    int32_t r2 = (color2 >> 11) & 0x1F;
    int32_t g2 = (color2 >> 5) & 0x3F;
    int32_t b2 = color2 & 0x1F;

    // Calculate deltas
    int32_t dr = ((r2 - r1) << 16) / (width - 1);
    int32_t dg = ((g2 - g1) << 16) / (width - 1);
    int32_t db = ((b2 - b1) << 16) / (width - 1);

    SetOutWriting(x, x + width - 1, y, y);
    EnsureSPI16Bit();

    SetCS(CS_ENABLE);
    int32_t r = r1 << 16, g = g1 << 16, b = b1 << 16;
    for (int i = 0; i < width; i++)
    {
        uint16_t pixel = ((r >> 16) << 11) | ((g >> 16) << 5) | (b >> 16);
        spi_write16_blocking(m_SpiPort, &pixel, 1);
        r += dr; g += dg; b += db;
    }
    SetCS(CS_DISABLE);
}
