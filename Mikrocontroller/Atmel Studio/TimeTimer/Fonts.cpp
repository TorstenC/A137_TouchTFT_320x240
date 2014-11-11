/*
 * Fonts.cpp
 *
 * Created: 13.02.2014 23:44:43
 *  Author: Torsten
 */ 
#include "Fonts.h"
#include "A137_ILI9325.h"
#include <avr/pgmspace.h>
#include "fonts4font.h"
#include "LCD_gamma.h"

// inline uint8_t NextNibble(uint8_t* data, bool* upper) {
// }


inline void DrawEqualPixels(uint8_t PxCount, const LCD_Gamma_14Steps* Gamma_Steps, uint8_t Index) {
    for (uint8_t i = PxCount; i; i--) {
        LCD_DrawPixel(&((*Gamma_Steps)[Index]));
    }
}

const LCD_Gamma_14Steps* Gamma_Steps = &LCD_Gamma_14StepsWoB;

// void LCD_DrawGlyph(const LCD_Glyph* G, const LCD_FontRef* F = 0, LCD_DrawOn DrawOn = black) {
void LCD_DrawGlyph(const void* G, LCD_DrawOn DrawOn) {
    switch (DrawOn) {
        case black:
            Gamma_Steps = &LCD_Gamma_14StepsWoB;
        break;
        case white:
            Gamma_Steps = &LCD_Gamma_14StepsBoW;
        break;
    }

    uintptr_t G_Ptr;                                // das finde ich saublöd,
    G_Ptr = (uintptr_t)G;                           // wie geht das besser?

//     uint8_t Width;
//     Width = pgm_read_byte(G_Ptr + 0);

    uint16_t BytesCount;
    BytesCount = pgm_read_word(G_Ptr + 1);

//     if (Width != 17) for (;;) __asm("nop"); // trap CPU
//     if (BytesCount != 110) for (;;) __asm("nop"); // trap CPU

    bool upper;
    upper = true;
    if (BytesCount) { 
        uint8_t* data;                  
        data = (uint8_t*)G_Ptr + 3;
        for (uint16_t i = BytesCount; i ; i--) {
            uint8_t Nibble;
            uint8_t CurrentByte;
            if (upper) CurrentByte = pgm_read_byte(data++);
            Nibble = upper ? CurrentByte >> 4 : CurrentByte & 0x0F;
            upper = !upper;
            if (Nibble) {
                if (Nibble == 0x0F) {                   // Nibble = F, also Vordergrund, anzahl im nächsten Nibble
                    if (upper) CurrentByte = pgm_read_byte(data++);
                    Nibble = upper ? CurrentByte >> 4 : CurrentByte & 0x0F;
                    upper = !upper;
                    DrawEqualPixels(Nibble + 3, Gamma_Steps, 12);
                } else {                                // Nibble = Transparanz für ein Pixel
                                                        // (0x1 = Hintrergrund .. 0xE = Vordergrund)
                    LCD_DrawPixel(&((*Gamma_Steps)[Nibble]));
                                                        //TODO: Gamma in PROGMEM schreiben?
                }
            } else {                    // Nibble = 0, also Hintergrund, anzahl im nächsten Nibble
                    if (upper) CurrentByte = pgm_read_byte(data++);
                    Nibble = upper ? CurrentByte >> 4 : CurrentByte & 0x0F;
                    upper = !upper;
                    DrawEqualPixels(Nibble + 3, Gamma_Steps, 0);
            }
        }
//     } else if (F) {
//         uint8_t PxCount;
//         const Font* F_Ptr;              // TODO: Kann man die Variable F_Ptr umgehen?
//         F_Ptr = (Font*)F;
//         PxCount = pgm_read_word(G_Ptr->width) * pgm_read_word(F_Ptr->height);
//         //TODO: Abfangen, was bei mehr als 16 x 16 Pixeln passiert
//         // siehe http://www.mikrocontroller.net/topic/166621
//         DrawEqualPixels(&PxCount, Gamma_Steps, 0);
    } else for(;;);                     // trap CPU
}

void LCD_DrawGlyph(const void* G) {
    LCD_DrawGlyph(G, black);
}


const LCD_Font* CurrentFont;

void LCD_DrawSpacer(uint8_t width) {
    // TODO: LCD_writeRGBt_unsafe nutzen!
    uint8_t height = pgm_read_byte((uintptr_t)CurrentFont + 1);
    for (uint8_t i = width; i; i--) {
        DrawEqualPixels(height, Gamma_Steps, 0);
    }
}

void LCD_DrawChar(char C) {
    // Adresse für Index C aus Codepage holen
    void* GlyphRef = (void*)(pgm_read_word((uintptr_t)CurrentFont + 2) + (uint8_t)C + (uint8_t)C);
    // Glyph-Adresse aus der Codepage auslesen
    void* Glyph = (void*)pgm_read_word(GlyphRef);
    LCD_DrawGlyph(Glyph, black);
}
// struct LCD_Font {
//     bool isMonospaced;
//     const uint8_t height;
//     const void* const CodePage;   // 0..255
// };
void LCD_DrawChar(uint16_t X, uint16_t Y, char C, const LCD_Font* F) {
    CurrentFont = F;
    uint8_t height = pgm_read_byte((uintptr_t)F + 1);
    LCD_setViewport(LCD_Y_X, X, Y, (uint16_t)LCD_width - X - 1, height); // TODO: LCD_Rotations berücksichtigen
    LCD_DrawChar(C);
}

