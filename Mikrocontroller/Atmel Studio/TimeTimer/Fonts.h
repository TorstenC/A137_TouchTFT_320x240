/*
 * Fonts.h
 *
 * Created: 13.02.2014 23:44:21
 *  Author: Torsten
 */ 


#ifndef FONTS_H_
#define FONTS_H_
#include <stdint.h>


struct LCD_Font {
    bool isMonospaced;
    const uint8_t height;
    const void* const CodePage;   // 0..255
};

enum LCD_DrawOn {
    black,
    white
};

enum LCD_Rotations {
    LCD_book,
    LCD_clockWise,
    LCD_counterClock,
    LCD_upsideDown
};

// TODO: Erweitern auf Farbe und Farbverlauf

// extern void LCD_DrawGlyph(LCD_Glyph* G, uint8_t height, LCD_DrawOn DrawOn);
// extern void LCD_DrawGlyph(LCD_Glyph* G, uint8_t height);
// extern void LCD_Draw#endif Glyph(LCD_Glyph* G, LCD_DrawOn DrawOn);
// extern void LCD_DrawGlyph(LCD_Glyph* G);

//typedef  uintptr_t* LCD_FontRef;

// extern void LCD_DrawGlyph(const LCD_GlyphR2ppB0* G, const LCD_FontRef* F, LCD_DrawOn DrawOn);
// extern void LCD_DrawGlyph(const LCD_GlyphR2ppB0* G, const LCD_FontRef* F);
// extern void LCD_DrawGlyph(const LCD_GlyphR2PpB0* G, LCD_DrawOn DrawOn);
extern void LCD_DrawGlyph(const void* G);
extern void LCD_DrawGlyph(const void* G, LCD_DrawOn DrawOn);
extern void LCD_DrawChar(uint16_t X, uint16_t Y, char C, const LCD_Font* F);
extern void LCD_DrawChar(char C);
extern void LCD_DrawSpacer(uint8_t width);

// extern void LCD_DrawChar(char C, LCD_FontRef F);
// extern void LCD_DrawChar(char C);

#endif /* FONTS_H_ */