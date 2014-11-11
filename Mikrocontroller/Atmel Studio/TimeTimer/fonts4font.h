/*
 * fonts4font.h
 *
 * Created: 14.02.2014 23:42:25
 *  Author: Torsten
 */ 

#ifndef FONTS4FONT_H_
#define FONTS4FONT_H_
#include <stdint.h>
#include <avr/pgmspace.h>

//#include <avr/pgmspace.h>
template <int N> struct LCD_Glyph {
    uint8_t Width;
    uint16_t UnitsCount; // BytesCount = 0 heißt RLEdata[1] mit RLEdata[0] = Höhe (Leerzeichen)
    uint8_t RLEdata[N];
};



#endif /* FONTS4FONT_H_ */