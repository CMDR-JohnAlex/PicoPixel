#pragma once

// ili9341 hardware commands
// https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf
// https://www.displayfuture.com/Display/datasheet/controller/ILI9341.pdf

#define ILI9341_NOP         0x00 ///< No-op register
#define ILI9341_SWRESET     0x01 ///< Software reset register
#define ILI9341_RDDID       0x04 ///< Read display identification information
#define ILI9341_RDDST       0x09 ///< Read Display Status

#define ILI9341_SLPIN       0x10 ///< Enter Sleep Mode
#define ILI9341_SLPOUT      0x11 ///< Sleep Out
#define ILI9341_PTLON       0x12 ///< Partial Mode ON
#define ILI9341_NORON       0x13 ///< Normal Display Mode ON

#define ILI9341_RDMODE      0x0A ///< Read Display Power Mode
#define ILI9341_RDMADCTL    0x0B ///< Read Display MADCTL
#define ILI9341_RDPIXFMT    0x0C ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT    0x0D ///< Read Display Image Format
#define ILI9341_RDSELFDIAG  0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF      0x20 ///< Display Inversion OFF
#define ILI9341_INVON       0x21 ///< Display Inversion ON
#define ILI9341_GAMMASET    0x26 ///< Gamma Set
#define ILI9341_DISPOFF     0x28 ///< Display OFF
#define ILI9341_DISPON      0x29 ///< Display ON

#define ILI9341_CASET       0x2A ///< Column Address Set
#define ILI9341_PASET       0x2B ///< Page Address Set
#define ILI9341_RAMWR       0x2C ///< Memory Write
#define ILI9341_RAMRD       0x2E ///< Memory Read

#define ILI9341_PTLAR       0x30 ///< Partial Area
#define ILI9341_VSCRDEF     0x33 ///< Vertical Scrolling Definition
#define ILI9341_MADCTL      0x36 ///< Memory Access Control
#define ILI9341_VSCRSADD    0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT      0x3A ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1     0xB1 ///< Frame Rate Control (In Norm Mode/Full Colors)
#define ILI9341_FRMCTR2     0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3     0xB3 ///< Frame Rate control (In Part Mode/Full Colors)
#define ILI9341_INVCTR      0xB4 ///< Display Inversion Control
#define ILI9341_DFUNCTR     0xB6 ///< Display Function Control

#define ILI9341_PWCTR1      0xC0 ///< Power Control 1
#define ILI9341_PWCTR2      0xC1 ///< Power Control 2
#define ILI9341_PWCTR3      0xC2 ///< Power Control 3
#define ILI9341_PWCTR4      0xC3 ///< Power Control 4
#define ILI9341_PWCTR5      0xC4 ///< Power Control 5
#define ILI9341_VMCTR1      0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2      0xC7 ///< VCOM Control 2

#define ILI9341_RDID1       0xDA ///< Read ID 1
#define ILI9341_RDID2       0xDB ///< Read ID 2
#define ILI9341_RDID3       0xDC ///< Read ID 3
#define ILI9341_RDID4       0xDD ///< Read ID 4

#define ILI9341_GMCTRP1     0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1     0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC

#define Hz                  1L          ///< Hertz unit multiplier
#define kHz                 1000L       ///< Kilohertz unit multiplier
#define MHz                 1000000L    ///< Megahertz unit multiplier
#define PIX_WIDTH           240
#define PIX_HEIGHT          320
#define PIX_BITCOUNT        (PIX_WIDTH * PIX_HEIGHT)
#define PIX_BYTECOUNT       (PIX_BITCOUNT * 2)
#define PIX_W32COUNT        (PIX_BITCOUNT / 16)

#define CS_ENABLE           0
#define CS_DISABLE          1
