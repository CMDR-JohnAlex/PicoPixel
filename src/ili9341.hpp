/**
 * @file ili9341.hpp
 * @brief ILI9341 TFT LCD Display Driver for Raspberry Pi Pico
 */

#pragma once

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include "ili9341HardwareCommands.hpp"

/**
 * @brief ILI9341 TFT LCD Display Driver Class
 *
 * This class provides a driver interface for the ILI9341 240x320 pixel TFT LCD display.
 *
 * Features:
 * - Optimized SPI communication with 8/16-bit mode switching
 * - Basic drawing primitives (pixels, lines, rectangles, triangles)
 * - Gradient rendering support (for certain drawing functions)
 * - Text rendering with bitmap fonts (RobotoMono SemiBold 32px)
 * - Memory-optimized operations for embedded systems
 * - Portrait and landscape orientation support
 *
 * @note This driver is optimized for the Raspberry Pi Pico W and uses Pico SDK functions
 *
 * @todo Use DMA to speed up the driver
 * @note Lots of testing required...
 * @todo SD card control stuffs
 * @todo Multi-font support
 * @todo Font size support
 * @todo Touchscreen support
 */
class ili9341
{
public:
    /**
     * @brief Construct a new ili9341 display driver
     *
     * Initializes the ILI9341 display with the specified SPI configuration and GPIO pins.
     * Performs hardware reset, gamma correction setup, and display initialization.
     *
     * @param spiPort Pointer to SPI instance (spi0 or spi1)
     * @param spiClockFreqency SPI clock frequency in Hz (recommended: 62.5MHz. The maximum.)
     * @param gpioMISO Master Input Slave Output pin number
     * @param gpioCS Chip Select pin number (active low)
     * @param gpioSCK Serial Clock pin number
     * @param gpioMOSI Master Output Slave Input pin number (data line)
     * @param gpioRESET Hardware reset pin number (active low)
     * @param gpioDC Data/Command selection pin (0=command, 1=data)
     * @param led Backlight LED control pin (PWM capable)
     * @param portrait Display orientation (true=240x320, false=320x240)
     *
     * @warning Ensure all GPIO pins are available and not used by other peripherals
     * @note The constructor will configure all pins and initialize the display immediately
     */
    ili9341(spi_inst_t *spiPort, int spiClockFreqency, uint8_t gpioCS, uint8_t gpioRESET, uint8_t gpioDC, uint8_t gpioMOSI, uint8_t gpioSCK, uint8_t led, uint8_t gpioMISO, bool portrait = true);

    /**
     * @brief Destroy the ili9341 object
     *
     * Performs cleanup operations.
     */
    virtual ~ili9341();

    /**
     * @brief Check if display is in portrait orientation
     * @return true if display is in portrait mode (240x320), false if landscape (320x240)
     */
    inline bool IsPortrait() const { return m_IsPortrait; }

    /**
     * @brief Get current display width in pixels
     * @return Display width (240 in portrait, 320 in landscape)
     */
    inline uint16_t GetWidth() const { return m_Width; }

    /**
     * @brief Get current display height in pixels
     * @return Display height (320 in portrait, 240 in landscape)
     */
    inline uint16_t GetHeight() const { return m_Height; }

    /**
     * @brief Check if display is currently in sleep mode
     *
     * Returns the current sleep state of the display. When asleep, the display
     * is turned off and backlight is disabled to save power.
     *
     * @return true if display is in sleep mode, false if awake and operational
     *
     * @note Use Sleep() and Wake() functions to control sleep state
     */
    inline uint16_t IsAsleep() const { return m_IsAsleep; }

     /**
     * @brief Convert 8-bit RGB values to 16-bit color format
     *
     * Converts 8-bit RGB components to ILI9341's 16-bit RGB565 format.
     * Uses optimized bit shifting for fast conversion on ARM Cortex-M0+,
     * or a lookup table of pre-computed values.
     *
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @return 16-bit color value in RGB565 format (RRRRRGGGGGGBBBBB)
     *
     * @note Green gets 6 bits while R and B get 5 bits each
     */
    uint16_t RGBto16bit(uint8_t r, uint8_t g, uint8_t b);

     /**
     * @brief Convert 8-bit RGBA values to 16-bit color with alpha blending
     *
     * Performs alpha blending and converts to 16-bit RGB565 format.
     *
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @param a Alpha component (0=transparent, 255=opaque)
     * @return 16-bit color value after alpha blending
     */
    uint16_t RGBAto16bit(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

     /**
     * @brief Draw a single pixel at specified coordinates
     *
     * Sets a single pixel to the specified color.
     *
     * @param x X coordinate (0 to width-1)
     * @param y Y coordinate (0 to height-1)
     * @param color 16-bit color value (RGB565 format)
     *
     * @note Use optimized functions for better performance
     */
    void DrawPixel(uint16_t x, uint16_t y, uint16_t color);

    /**
     * @brief Draw a line between two points
     *
    * Uses Bresenham's line algorithm for diagonal lines and optimized methods for
    * horizontal/vertical lines.
    *
     * @param x0 X coordinate of start point
     * @param y0 Y coordinate of start point
     * @param x1 X coordinate of end point
     * @param y1 Y coordinate of end point
     * @param color 16-bit color value
     */
    void DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

    /**
     * @brief Draw a triangle outline using three vertices
     *
     * Draws the outline of a triangle by connecting three points with lines.
     *
     * @param x1 X coordinate of first vertex
     * @param y1 Y coordinate of first vertex
     * @param x2 X coordinate of second vertex
     * @param y2 Y coordinate of second vertex
     * @param x3 X coordinate of third vertex
     * @param y3 Y coordinate of third vertex
     * @param color 16-bit color value for the outline
     */
    void DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

    /**
     * @brief Draw a filled triangle using three vertices
     *
     * Fills a triangle using an optimized scanline algorithm. Vertices are automatically
     * sorted by Y-coordinate for efficient rasterization.
     *
     * @param x1 X coordinate of first vertex
     * @param y1 Y coordinate of first vertex
     * @param x2 X coordinate of second vertex
     * @param y2 Y coordinate of second vertex
     * @param x3 X coordinate of third vertex
     * @param y3 Y coordinate of third vertex
     * @param color 16-bit color value for the fill
     */
    void DrawTriangleFilled(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color);

    /**
     * @brief Draw a triangle with gradient colors at each vertex
     *
     * Creates a smooth color gradient across the triangle surface by interpolating
     * colors between the three vertices. Uses fixed-point arithmetic for performance.
     *
     * @param x1 X coordinate of first vertex
     * @param y1 Y coordinate of first vertex
     * @param color1 Color at first vertex
     * @param x2 X coordinate of second vertex
     * @param y2 Y coordinate of second vertex
     * @param color2 Color at second vertex
     * @param x3 X coordinate of third vertex
     * @param y3 Y coordinate of third vertex
     * @param color3 Color at third vertex
     *
     * @note This is computationally intensive and may be slow for large triangles
     */
    void DrawTriangleGradient(uint16_t x1, uint16_t y1, uint16_t color1,
                              uint16_t x2, uint16_t y2, uint16_t color2,
                              uint16_t x3, uint16_t y3, uint16_t color3);

    /**
     * @brief Draw a filled rectangle
     *
     * Draws a solid rectangle. Automatically chooses between stack buffers
     * (small rectangles) and static buffers (large rectangles) for optimal
     * memory usage and performance.
     *
     * @param x Top-left X coordinate
     * @param y Top-left Y coordinate
     * @param width Rectangle width in pixels
     * @param height Rectangle height in pixels
     * @param color 16-bit fill color
     *
     * @note Use this over drawing two triangles!
     */
    void DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

    /**
     * @brief Draw text using bitmap font
     *
     * Renders text at specified position using the built-in RobotoMono SemiBold font.
     * Text is rendered left-to-right with automatic character spacing and baseline alignment.
     * Supports multi-line text with '\n' characters and automatic line spacing.
     *
     * @param x Starting X coordinate (left edge)
     * @param y Starting Y coordinate (baseline)
     * @param color Text color (RGB565 format)
     * @param backgroundColor Background color (RGB565 format, 0xFFFF = transparent)
     *
     * @note Font: RobotoMono SemiBold, 32px height, monospace
     * @note Character set: ASCII 32-126 (space to tilde)
     * @note Unsupported characters display as question mark in box
     * @note Line spacing: Font height + 4 pixels (36px total)
     * @note Transparent background when backgroundColor = 0xFFFF
     *
     * @warning Ensure Y coordinate allows space for character height above baseline
     */
    void DrawText(const char* text, uint16_t x, uint16_t y, uint16_t color, uint16_t backgroundColor = 0xFFFF);

    /**
     * @brief Draw replacement character for unsupported characters
     *
     * Draws a visual placeholder (question mark in bordered box) for characters
     * outside the supported ASCII range.
     *
     * @param x X coordinate (left edge)
     * @param y Y coordinate (baseline)
     * @param color Text/border color (RGB565 format)
     * @param backgroundColor Background color (RGB565 format)
     * @return Width of the replacement character
     *
     * @note Replacement character: Question mark in 10x16 pixel bordered box
     * @note Called automatically by DrawChar() for invalid characters
     */
    uint16_t DrawUnsupportedChar(uint16_t x, uint16_t y, uint16_t color, uint16_t backgroundColor);

    /**
     * @brief Calculate text width in pixels
     *
     * Calculates the total width needed to render the given text string.
     * Useful for centering text, right-alignment, or checking if text fits.
     *
     * @param text Text string to measure (null-terminated)
     * @return Total width in pixels (sum of all character advance widths)
     *
     * @note Ignores newline characters ('\n', '\r') in width calculation
     * @note Skips unsupported characters (< 32 or > 126)
     * @warning For multi-line text, measure each line separately
     *
     * @section width_examples Width Calculation Examples
     * @code{.cpp}
     * // Center text horizontally
     * const char* text = "Hello World";
     * uint16_t textWidth = display.GetTextWidth(text);
     * uint16_t centerX = (display.GetWidth() - textWidth) / 2;
     * display.DrawText(text, centerX, 100, display.RGBto16bit(255, 255, 255));
     *
     * // Right-align text
     * const char* rightText = "Right Aligned";
     * uint16_t rightWidth = display.GetTextWidth(rightText);
     * uint16_t rightX = display.GetWidth() - rightWidth - 10;
     * display.DrawText(rightText, rightX, 150, display.RGBto16bit(255, 255, 255));
     *
     * // Check if text fits
     * if (display.GetTextWidth("Long text string") <= display.GetWidth())
     * {
     *     // Text fits, safe to draw
     * }
     * @endcode
     */
    uint16_t GetTextWidth(const char* text);

    /**
     * @brief Get font height in pixels
     * @return Font height in pixels (32 pixels for RobotoMono)
     */
    inline uint16_t GetFontHeight() const { return 32; }

    /**
     * @brief Draw raw pixel buffer to display
     *
     * Draws a rectangular region from a raw 16-bit color buffer. Buffer must contain
     *  at most width * height pixels in row-major order (left-to-right, top-to-bottom).
     *
     * @param x Top-left X coordinate
     * @param y Top-left Y coordinate
     * @param width Buffer width in pixels
     * @param height Buffer height in pixels
     * @param buffer Pointer to 16-bit color buffer
     *
     * @warning Buffer must contain at most width * height elements
     */
    void DrawRawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t* buffer);

    /**
     * @brief Clear the entire screen to a solid color
     *
     * Efficiently fills the entire display with a single color using optimized
     * line buffer operations. Uses static buffer caching to avoid repeated memory fills.
     *
     * @param color Fill color (default: black 0x0000)
     *
     * @note Fast due to static buffer reuse
     */
    void Clear(uint16_t color = 0x000000);

    /***
     * @brief Display a test pattern
     *
     * Shows corner rectangles in different colors and gradient bars across the screen.
     * Useful for testing display functionality, color accuracy, and orientation.
     *
     * @note For development and testing purposes only
     */
    void ScreenTest();

    /**
     * @brief Display a pixel grid test pattern
     *
     * Creates a grid of colored pixels in a repeating RGB pattern.
     *
     * @note For development and testing purposes only
     */
    void PixelTest();

    /**
     * @brief Display a text rendering test pattern
     *
     * Demonstrates text rendering capabilities including:
     * - Basic text rendering
     * - Unsupported character handling
     * - Text with background colors
     * - Multi-line text with newlines
     * - Centered text alignment
     * - Right-aligned text
     *
     * @note For development and testing purposes only
     * @see CharTest() for character testing
     */
    void TextTest();

    /**
     * @brief Display a character test pattern
     *
     * Renders individual characters to test font rendering, spacing,
     * and baseline alignment.
     *
     * @note For development and testing purposes only
     * @note Draws red reference line to show baseline alignment
     * @see TextTest() for full text rendering tests
     */
    void CharTest();

    /**
     * @brief Put display into sleep mode
     */
    void Sleep();

    /**
     * @brief Wake display from sleep mode
     */
    void Wake();

    /**
     * @brief Set display backlight brightness
     *
     * Controls the PWM duty cycle of the backlight LED to adjust brightness.
     * Brightness is set using 16-bit PWM resolution for smooth dimming.
     *
     * @param brightness Brightness level (0-65535)
     *                   - 0 = completely off (backlight disabled)
     *                   - 32767 = 50% brightness
     *                   - 65535 = maximum brightness
     *
     * @note Does not affect display sleep state - only backlight intensity
     * @note If display is asleep, this will enable PWM but display remains off
     *
     * @warning Very low brightness levels may make display difficult to see.
     */
    void SetBrightness(uint16_t brightness);

    /**
     * @brief Set display backlight brightness as percentage
     *
     * Convenience function to set brightness using percentage values.
     * Internally converts percentage to 16-bit PWM value.
     *
     * @param percent Brightness percentage (0.0 to 100.0)
     */
    void SetBrightnessPercent(float percent);

protected:
    /**
     * @brief Ensure SPI is in 8-bit mode for commands
     *
     * Switches SPI interface to 8-bit mode if currently in 16-bit mode.
     * Required for sending commands and single-byte parameters to display.
     */
    void EnsureSPI8Bit();

    /**
     * @brief Ensure SPI is in 16-bit mode for pixel data
     *
     * Switches SPI interface to 16-bit mode if currently in 8-bit mode.
     * Optimizes pixel data transfer by sending 16-bit color values directly.
     */
    void EnsureSPI16Bit();

    /**
     * @brief Control chip select (CS) pin state
     * @param state CS_ENABLE (0) to select chip, CS_DISABLE (1) to deselect
     */
    void SetCS(int state);

    /**
     * @brief Send command to display controller
     *
     * Sends an 8-bit command to the ILI9341 controller. Automatically handles
     * DC pin switching and SPI mode selection.
     *
     * @param command 8-bit command code (see ili9341HardwareCommands.hpp)
     */
    void SetCommand(uint8_t command);

    /**
     * @brief Send parameter byte following a command
     *
     * Sends a single parameter byte for the previously issued command.
     *
     * @param data 8-bit parameter value
     */
    void CommandParameter(uint8_t data);

    /**
     * @brief Set drawing window and prepare for pixel data
     *
     * Configures the display controller's column and page address ranges,
     * then issues RAMWR command to begin pixel data transfer.
     *
     * @param startCol Starting column (X coordinate)
     * @param endCol Ending column (inclusive)
     * @param startPage Starting page (Y coordinate)
     * @param endPage Ending page (inclusive)
     */
    void SetOutWriting(const int startCol, const int endCol, const int startPage, const int endPage);

    /**
     * @brief Write raw 8-bit data to display
     * @param buffer Pointer to data buffer
     * @param bytes Number of bytes to write
     */
    void WriteData8bit(const uint8_t* buffer, int bytes);

    /**
     * @brief Write raw 16-bit data to display
     * @param buffer Pointer to 16-bit data buffer
     * @param count Number of 16-bit words to write
     */
    void WriteData16bit(const uint16_t* buffer, int count);

    /**
     * @brief Draw horizontal line
     *
     * Optimized horizontal line drawing.
     *
     * @param x Starting X coordinate
     * @param y Y coordinate
     * @param width Line width in pixels
     * @param color 16-bit color value
     */
    void DrawHorizontalLine(uint16_t x, uint16_t y, uint16_t width, uint16_t color);

    /**
     * @brief Draw horizontal line with color gradient
     *
     * Draws a horizontal line using linear interpolation for the color gradient.
     *
     * @param x Starting X coordinate
     * @param y Y coordinate
     * @param width Line width in pixels
     * @param color1 Starting color (left)
     * @param color2 Ending color (right)
     */
    void DrawHorizontalLineGradient(uint16_t x, uint16_t y, uint16_t width, uint16_t color1, uint16_t color2);

    /**
     * @brief Draw a single character from bitmap font
     *
     * Renders one character from the RobotoMono font.
     * Handles bitmap reading, baseline positioning, and pixel rendering.
     * Automatically calls DrawUnsupportedChar() for invalid characters.
     *
     * @param c Character to draw
     * @param x X coordinate (left edge of character)
     * @param y Y coordinate (text baseline)
     * @param color Text color (RGB565 format)
     * @param backgroundColor Background color (RGB565 format, 0xFFFF = transparent)
     * @return Width of the drawn character in pixels (advance width for next character)
     *
     * @note Called internally by DrawText() for each character
     * @note Uses MSB-first bit reading for LVGL font format
     * @note Applies font glyph offsets for proper baseline alignment
     * @note Returns consistent advance width for character spacing
     *
     * @warning Only supports ASCII characters 32-126
     * @see DrawUnsupportedChar() for handling invalid characters
     * @see DrawText() for high-level text rendering
     */
    uint16_t DrawChar(char c, uint16_t x, uint16_t y, uint16_t color, uint16_t backgroundColor);

private:
    // Hardware Configuration
    spi_inst_t* m_SpiPort;          ///< SPI peripheral instance (spi0 or spi1)
    int m_SpiClockFreqency;         ///< SPI clock frequency in Hz
    uint8_t m_GpioMISO;             ///< Master Input Slave Output pin
    uint8_t m_GpioCS;               ///< Chip Select pin (active low)
    uint8_t m_GpioSCK;              ///< Serial Clock pin
    uint8_t m_GpioMOSI;             ///< Master Output Slave Input pin
    uint8_t m_GpioRESET;            ///< Hardware reset pin (active low)
    uint8_t m_GpioDC;               ///< Data/Command selection pin
    uint8_t m_Led;                  ///< Backlight LED control pin

    // State Management
    bool m_SpiIs16Bit = false;      ///< Current SPI mode (true=16-bit, false=8-bit)
    bool m_IsPortrait;              ///< Display orientation flag
    uint16_t m_Width;               ///< Current display width in pixels
    uint16_t m_Height;              ///< Current display height in pixels
    bool m_IsAsleep = true;         ///< Display is sleep mode flag
};
