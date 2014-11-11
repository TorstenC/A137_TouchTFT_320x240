/*
 * A137_ILI9325.h
 *
 * Created: 08.02.2014 14:12:08
 *  Author: Torsten
 */ 

#ifndef A137_ILI9325_H_
#define A137_ILI9325_H_
#include <avr/pgmspace.h>
#include <stdint.h>


/* ┌─┐┏┓┍┑┎┒╓┐╒═╕→●
 * │  │┃┃││┃┃║││  │↓
 * ├─┤┣┫┝┥┠┨╙┘└─┘←
 * ┢━┪┞┦┟┧┡┩┌╖┌─┐↑
 * ┕━┙└┘┗┛└┘└╜╘═╛
 *  For cut&paste only ;-)
 */
// ┌──────┐ ╒══════╕ ┌──────┐ ╒══════╕
// │● → X │ │● → X │ │ X ← ●│ │● → X │
// │↓ ╒¹  │ │↓ ╒¹  │ │  ╕² ↓│ │↓ ╕²  │
// │Y     │ │Y     │ │     Y│ │Y     │
// ╘══════╛ └──────┘ ╘══════╛ └──────┘
// Bottom_N  Top_N   Bottom_M   Top_M
//
// ╓────────┐ ┌────────╖ ╓────────┐ ┌────────╖
// ║ ● → X  │ │ ● → X  ║ ║  X ← ● │ │  X ← ● ║
// ║ ↓  ╒¹  │ │ ↓  ╒¹  ║ ║   ╕² ↓ │ │   ╕² ↓ ║
// ╙────────┘ └────────╜ ╙────────┘ └────────╜
//  Left_N     Right_N     Left_M     Right_M
//
// ¹) ╒ = normal   letter F
// ²) ╕ = mirrored letter F
// //TODO:
// I/D[1:0] = 00, AM = 0
// I/D[1:0] = 01, AM = 0
// I/D[1:0] = 10, AM = 0
// I/D[1:0] = 00, AM = 1
// I/D[1:0] = 01, AM = 1
// I/D[1:0] = 11, AM = 1
// I/D[1:0] = 01, AM = 0
// I/D[1:0] = 10, AM = 0
// I/D[1:0] = 11, AM = 0

enum LCD_Orientations {      // 8 Möglichkeiten der Ausrichtung:
    LCD_Bottom_N = 0x10,    // portrait (, A137-USB top)
    LCD_Top_N = 0x20,       // portrait (, A137-USB bottom)
    LCD_Bottom_M,           // mirrored portrait (A137-USB top)
    LCD_Top_M,              // mirrored portrait (A137-USB bottom)
    LCD_Left_N,    // ?? landscape (, A137-USB right)
    LCD_Right_N,            // landscape (A137-USB left)
    LCD_Left_M,             // mirrored landscape (A137-USB right)
    LCD_Right_M             // mirrored landscape (A137-USB left)
};

enum LCD_UpdateDirections {
    LCD_X_Y = 0x00,                // normal GRAM update direction
    LCD_Y_X = 0x08                 // inverted GRAM update direction
};

// Display Size
#define LCD_width                   240
#define LCD_height                  320


// 16 bit Color definitions (64535 colors)
#define	LCD_BLACK         (uint16_t)0x0000
#define	LCD_RED           (uint16_t)0xF800
#define LCD_ORANGE        (uint16_t)0xFBE0
#define LCD_YELLOW        (uint16_t)0xFFE0
#define	LCD_GREEN         (uint16_t)0x07E0
#define LCD_CYAN          (uint16_t)0x07FF
#define	LCD_BLUE          (uint16_t)0x001F
#define LCD_MAGENTA       (uint16_t)0xF81F
#define LCD_GREY          (uint16_t)0x7BEF
#define LCD_WHITE         (uint16_t)0xFFFF
// 0x7800 03E0 0F
typedef uint8_t LCD_RGBColor[3];

// Color definitions (few data in RAM, no PROGMEM)
//TODO: Better use #define?
const LCD_RGBColor LCD_black    = {  0,   0,   0};
const LCD_RGBColor LCD_red      = {255,   0,   0};
const LCD_RGBColor LCD_orange   = {255, 127,   0};
const LCD_RGBColor LCD_yellow   = {255, 255,   0};
const LCD_RGBColor LCD_green    = {  0, 255,   0};
const LCD_RGBColor LCD_cyan     = {  0, 255, 255};
const LCD_RGBColor LCD_blue     = {  0,   0, 255};
const LCD_RGBColor LCD_magenta  = {255,   0, 255};
const LCD_RGBColor LCD_grey     = {127, 127, 127};
const LCD_RGBColor LCD_white    = {255, 255, 255};

// init ports, reset display and turn on
extern void LCD_init(LCD_Orientations MOE);
// clears screen with color, e.g.
// "LCD_fillScreen((LCD_RGBColor*)&LCD_yellow"); or
// "LCD_fillScreen(LCD_BLUE);"

// ┌─────────────────
// │ LCD_fillViewPort
// └─────────────────

// LCD_fillViewPort füllt den gesamten Viewport mit einer Farbe
// ● color: 16Bit-Farbwert (565-RGB)
extern void LCD_fillScreen(uint16_t color);

// LCD_fillViewPort füllt den gesamten Viewport mit einer Farbe
// ● color: Zeiger auf 18Bit-Farbwert (LCD_RGBColor)
extern void LCD_fillScreen(LCD_RGBColor* color);

// LCD_fillViewPort füllt den gesamten Viewport mit einer Farbe
// ● Red, Green, Blue: 3 x 8Bit-Farbwert (0..255)
extern void LCD_fillScreen(uint8_t Red, uint8_t Green, uint8_t Blue);

// ┌─────────────
// │ LCD_fillRect
// └─────────────

// LCD_fillRect füllt ein angegebenes Rechteck mit einer Farbe
// ● color: 16Bit-Farbwert (565-RGB)
extern void LCD_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor);

// LCD_fillRect füllt ein angegebenes Rechteck mit einer Farbe
// ● color: Zeiger auf 18Bit-Farbwert (LCD_RGBColor)
extern void LCD_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, LCD_RGBColor* color);

// LCD_fillRect füllt ein angegebenes Rechteck mit einer Farbe
// ● Red, Green, Blue: 3 x 8Bit-Farbwert (0..255)
extern void LCD_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t Red, uint8_t Green, uint8_t Blue);

// extern void LCD_drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color);

// ┌─────────
// │ LCD_goTo
// └─────────

// With parameter: place cursor onto position x and y
// with respect to Viewport
extern void LCD_goTo(uint16_t x, uint16_t y);

// Without parameter: place cursor on x = 0 and y = 0
// in current Viewport
extern void LCD_goTo();

//extern void LCD_writeRegister(uint16_t addr, uint16_t data);

// ┌──────────────
// │ LCD_DrawPixel
// └──────────────
extern void LCD_DrawPixel(uint16_t color);
extern void LCD_DrawPixel(const LCD_RGBColor *color);
extern void LCD_DrawPixel(uint8_t Red, uint8_t Green, uint8_t Blue);


void LCD_StartDrawUnsave();
void LCD_EndDrawUnsave();
void LCD_writeRGBt_unsafe(const LCD_RGBColor *color);


// Writes Data
// Don´t forget to select Graphic RAM with LCD_goTo()
//extern void LCD_writeData(uint16_t data);

// LCD_SetViewport ohne Parameter hebt alle Beschränkungen auf.
extern void LCD_setViewport();

// LCD_SetViewport schränkt den Schreibzugriff auf eine rechteckige Fläche ein.
extern void LCD_setViewport(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

// LCD_SetViewport schränkt den Schreibzugriff auf eine rechteckige Fläche ein.
extern void LCD_setViewport(LCD_UpdateDirections UD, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

// LCD_SetViewport ohne Parameter setzt den Viewport auf die Gesamte Fläche zurück.
extern void LCD_setViewport(LCD_UpdateDirections UD);

struct LCD_TouchData {
    int16_t X;
    int16_t Y;
    int16_t Zx;
    int16_t Zy;
    uint8_t Z;
}; 

extern LCD_TouchData LCD_getTouchData();


/* ┌─┐┏┓┍┑┎┒╓┐╒═╕→
 * │  │┃┃││┃┃║││  │↓
 * ├─┤┣┫┝┥┠┨╙┘└─┘←
 * ┢━┪┞┦┟┧┡┩┌╖┌─┐↑
 * ┕━┙└┘┗┛└┘└╜╘═╛
 *  For cut&paste only ;-)
 */

#endif /* A137_ILI9325_H_ */