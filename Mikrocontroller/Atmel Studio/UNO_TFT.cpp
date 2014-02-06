/*
 * UNO_TFT.cpp
 *
 * Created: 05.02.2014 19:58:29
 *  Author: Torsten
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>

#pragma region Port-Pins
    // LCD_CS = PORT C3
    #define LCD_CS_1 PORTD |= _BV(3)
    #define LCD_CS_0 PORTD &= ~_BV(3)
    // LCD_RS = PORT C2 ("CD" = Command/Data)
    #define LCD_CD_1 PORTD |= _BV(2)
    #define LCD_CD_0 PORTD &= ~_BV(2)
    // LCD_WR = PORT C1
    #define LCD_WR_1 PORTD |= _BV(1)
    #define LCD_WR_0 PORTD &= ~_BV(1)
    // LCD_RD = PORT C0
    #define LCD_RD_1 PORTD |= _BV(0)
    #define LCD_RD_0 PORTD &= ~_BV(0)
    // LCD_RST = PORT C4
    #define LCD_RST_1 PORTD |= _BV(4)
    #define LCD_RST_0 PORTD &= ~_BV(4)
    // LCD_Data = PB0, PB1, PD2..PD7
    #define LCD_DataMask 0b00000011  // Port B - Maske
    //     ~LCD_DataMask             // Port D - Maske
    inline void LCD_setWriteDir(void) {
        DDRB |= LCD_DataMask;
        DDRD |= ~LCD_DataMask;
    }
    inline void LCD_setReadDir(void) {
        DDRB &= ~LCD_DataMask;
        DDRD &= LCD_DataMask;
    }
    inline void LCD_write8(uint8_t d) {
        PORTB = (PORTB & ~LCD_DataMask) | (d & LCD_DataMask);
        PORTD = (PORTD & LCD_DataMask) | (d & ~LCD_DataMask);
    }
    inline uint8_t LCD_read8(void) {
        uint8_t d;
        d = PORTB & LCD_DataMask;
        d |= PORTD & ~LCD_DataMask;
        return d;
    }
#pragma endregion Port-Pins
#pragma region Data
    static const uint16_t _regValues[] PROGMEM = {
        0x0000, 0x0001,
        0x0001, 0x0100,
        0x0002, 0x0700,
        0x0003, 0x1030,
        0x0004, 0x0000,
        0x0008, 0x0202,
        0x0009, 0x0000,
        0x000a, 0x0000,
        0x000c, 0x0000,
        0x000d, 0x0000,
        0x000f, 0x0000,
        0x0010, 0x0000,
        0x0011, 0x0000,
        0x0012, 0x0000,
        0x0013, 0x0000,
        0x00FF, 1000,
        0x0010, 0x17b0,
        0x00FF, 500,
        0x0011, 0x0037,
        0x00FF, 500,
        0x0012, 0x0138,
        0x0013, 0x1700,
        0x0029, 0x000d,
        0x00FF, 500,
        0x0020, 0x0000,
        0x0021, 0x0000,
        0x0030, 0x0001,
        0x0031, 0x0606,
        0x0032, 0x0304,
        0x0035, 0x0103,
        0x0036, 0x011d,
        0x0037, 0x0404,
        0x0038, 0x0404,
        0x0039, 0x0404,
        0x003c, 0x0700,
        0x003d, 0x0a1f,
        0x0050, 0x0000,
        0x0051, 0x00ef,
        0x0052, 0x0000,
        0x0053, 0x013f,
        0x0060, 0x2700,
        0x0061, 0x0001,
        0x006a, 0x0000,
        0x0090, 0x0010,
        0x0092, 0x0000,
        0x0093, 0x0003,
        0x0095, 0x0101,
        0x0097, 0x0000,
        0x0098, 0x0000,
        0x0007, 0x0173,
        0x00FF, 500
    };
    // Color definitions
    #define	LCD_BLACK                   0x0000
    #define	LCD_BLUE                    0x001F
    #define	LCD_RED                     0xF800
    #define	LCD_GREEN                   0x07E0
    #define LCD_CYAN                    0x07FF
    #define LCD_MAGENTA                 0xF81F
    #define LCD_YELLOW                  0xFFE0
    #define LCD_WHITE                   0xFFFF

    // Display Size
    #define LCD_width                   240
    #define LCD_height                  320

    // register names
    #define LCD_R_START_OSC			    0x00
    #define LCD_R_DRIV_OUT_CTRL		    0x01
    #define LCD_R_DRIV_WAV_CTRL		    0x02
    #define LCD_R_ENTRY_MOD			    0x03
    #define LCD_R_RESIZE_CTRL			0x04
    #define LCD_R_DISP_CTRL1			0x07
    #define LCD_R_DISP_CTRL2			0x08
    #define LCD_R_DISP_CTRL3			0x09
    #define LCD_R_DISP_CTRL4			0x0A
    #define LCD_R_RGB_DISP_IF_CTRL1	    0x0C
    #define LCD_R_FRM_MARKER_POS		0x0D
    #define LCD_R_RGB_DISP_IF_CTRL2	    0x0F
    #define LCD_R_POW_CTRL1			    0x10
    #define LCD_R_POW_CTRL2			    0x11
    #define LCD_R_POW_CTRL3			    0x12
    #define LCD_R_POW_CTRL4			    0x13
    #define LCD_R_GRAM_HOR_AD			0x20
    #define LCD_R_GRAM_VER_AD			0x21
    #define LCD_R_RW_GRAM				0x22
    #define LCD_R_POW_CTRL7			    0x29
    #define LCD_R_FRM_RATE_COL_CTRL	    0x2B
    #define LCD_R_GAMMA_CTRL1			0x30
    #define LCD_R_GAMMA_CTRL2			0x31
    #define LCD_R_GAMMA_CTRL3			0x32
    #define LCD_R_GAMMA_CTRL4			0x35
    #define LCD_R_GAMMA_CTRL5			0x36
    #define LCD_R_GAMMA_CTRL6			0x37
    #define LCD_R_GAMMA_CTRL7			0x38
    #define LCD_R_GAMMA_CTRL8			0x39
    #define LCD_R_GAMMA_CTRL9			0x3C
    #define LCD_R_GAMMA_CTRL10			0x3D
    #define LCD_R_HOR_START_AD			0x50
    #define LCD_R_HOR_END_AD			0x51
    #define LCD_R_VER_START_AD			0x52
    #define LCD_R_VER_END_AD			0x53
    #define LCD_R_GATE_SCAN_CTRL1		0x60
    #define LCD_R_GATE_SCAN_CTRL2		0x61
    #define LCD_R_GATE_SCAN_CTRL3		0x6A
    #define LCD_R_PART_IMG1_DISP_POS	0x80
    #define LCD_R_PART_IMG1_START_AD	0x81
    #define LCD_R_PART_IMG1_END_AD		0x82
    #define LCD_R_PART_IMG2_DISP_POS	0x83
    #define LCD_R_PART_IMG2_START_AD	0x84
    #define LCD_R_PART_IMG2_END_AD		0x85
    #define LCD_R_PANEL_IF_CTRL1		0x90
    #define LCD_R_PANEL_IF_CTRL2		0x92
    #define LCD_R_PANEL_IF_CTRL3		0x93
    #define LCD_R_PANEL_IF_CTRL4		0x95
    #define LCD_R_PANEL_IF_CTRL5		0x97
    #define LCD_R_PANEL_IF_CTRL6		0x98
#pragma endregion Data
// -LCD_-functions-----------------------------------------------------------------------------
#pragma region LCD_-functions
    uint16_t LCD_cursor_x;
    uint16_t LCD_cursor_y;
    uint8_t LCD_textsize;
    uint16_t LCD_textcolor;
    uint8_t LCD_rotation;

void LCD_Delay(uint16_t t) {
    static uint8_t t1;
    while (t--) {
        for (t1 = 100; t1 > 0; t1-- ) {
            __asm("nop");
        }
    }
}

inline void LCD_writeData_unsafe(uint16_t data) {
    LCD_write8(data >> 8);
    LCD_WR_0;
    LCD_WR_1;
    LCD_write8(data);
    LCD_WR_0;
    LCD_WR_1;
}
void LCD_writeData(uint16_t data) {
    LCD_CS_0;
    LCD_CD_1;
    // LCD_RD is 1;
    // LCD_WR is 1;
    LCD_setWriteDir();
    LCD_writeData_unsafe(data);
    LCD_CS_1;
}
void LCD_writeCommand(uint16_t cmd) {
    LCD_CS_0;
    LCD_CD_0;
    // LCD_RD is 1;
    // LCD_WR is 1;
    LCD_setWriteDir();
    LCD_write8(cmd >> 8);
    LCD_WR_0;
    LCD_WR_1;
    LCD_write8(cmd);
    LCD_WR_0;
    LCD_WR_1;
    LCD_CS_1;
}
uint16_t LCD_readData() {
    uint16_t d = 0;
    LCD_CS_0;
    LCD_CD_1;
    // LCD_RD is 1;
    // LCD_WR is 1;
    LCD_setReadDir();
    LCD_RD_0;
    LCD_Delay(100);
    d = LCD_read8();
    d <<= 8;
    LCD_RD_1;
    LCD_Delay(100);
    d |= LCD_read8();
    LCD_RD_1;
    LCD_CS_1;
    return d;
}
uint16_t LCD_readRegister(uint16_t addr) {
    LCD_writeCommand(addr);
    return LCD_readData();
}
void LCD_writeRegister(uint16_t addr, uint16_t data) {
    LCD_writeCommand(addr);
    LCD_writeData(data);
}
#define swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }
void LCD_calcGRAMPosition(uint16_t *posx, uint16_t *posy) {
    uint16_t x = *posx;
    uint16_t y = *posy;
    switch(LCD_rotation)  {
        case 1:  // 90
            swap_uint16_t(x, y);
            x = LCD_width - x - 1;
        break;
        case 2:  // 180
            y = LCD_height - y - 1;
            x = LCD_width - x - 1;
        break;
        case 3: // 270
            swap_uint16_t(x, y);
            y = LCD_height - y - 1;
        break;
    }
    *posx = x;
    *posy = y;
}
void LCD_goTo(uint16_t x, uint16_t y) {
    LCD_calcGRAMPosition(&x, &y);
    LCD_writeRegister(LCD_R_GRAM_HOR_AD, x);    // GRAM Address Set (Horizontal Address) (R20h)
    LCD_writeRegister(LCD_R_GRAM_VER_AD, y);    // GRAM Address Set (Vertical Address) (R21h)
    LCD_writeCommand(LCD_R_RW_GRAM);            // Write Data to GRAM (R22h)
}
inline void LCD_goHome(void) {
    LCD_goTo(0,0);
}
void LCD_fillScreen(uint16_t color) {
    LCD_goHome();
    static uint32_t i;
    i = LCD_width;
    i *= LCD_height;
    LCD_CS_0;
    LCD_CD_1;
    // LCD_RD is 1;
    // LCD_WR is 1;
    LCD_setWriteDir();
    while (i--) {
        LCD_writeData_unsafe(color);
    }
    LCD_CS_1;
}

void LCD_drawFastLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color, uint8_t rotflag) {
    uint16_t newentrymod;
    uint16_t prevEntryMod = LCD_readRegister(LCD_R_ENTRY_MOD);
    switch (LCD_rotation) {
        case 0:
            if (rotflag)
                newentrymod = 0x1028;               // we want a 'vertical line'
            else
                newentrymod = 0x1030;               // we want a 'horizontal line'
        break;
        case 1:
            swap_uint16_t(x, y);
            x = LCD_width - x - 1;                   // first up fix the X
            if (rotflag)
                newentrymod = 0x1000;               // we want a 'vertical line'
            else
                newentrymod = 0x1028;               // we want a 'horizontal line'
        break;
        case 2:
            x = LCD_width - x - 1;
            y = LCD_height - y - 1;
            if (rotflag)
                newentrymod = 0x1008;               // we want a 'vertical line'
            else
                newentrymod = 0x1020;               // we want a 'horizontal line'
        break;
        case 3:
            swap_uint16_t(x,y);
            y = LCD_height - y - 1;
            if (rotflag)
                newentrymod = 0x1030;               // we want a 'vertical line'
            else
                newentrymod = 0x1008;               // we want a 'horizontal line'
        break;
        default:
            newentrymod = 0x1000;
            for(;;); // trap cpu
        break;
    }
    LCD_writeRegister(LCD_R_ENTRY_MOD, newentrymod);
    LCD_writeRegister(LCD_R_GRAM_HOR_AD, x);       // GRAM Address Set (Horizontal Address) (R20h)
    LCD_writeRegister(LCD_R_GRAM_VER_AD, y);       // GRAM Address Set (Vertical Address) (R21h)
    LCD_writeCommand(LCD_R_RW_GRAM);               // Write Data to GRAM (R22h)
    LCD_CS_0;
    LCD_CD_1;
    // LCD_RD is 1;
    // LCD_WR is 1;
    LCD_setWriteDir();
    while (length--) {
        LCD_writeData_unsafe(color);
    }
    LCD_CS_1;
    LCD_writeRegister(LCD_R_ENTRY_MOD, prevEntryMod); // set back to default
}
void LCD_drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
    if (y >= LCD_height) return;
    LCD_drawFastLine(x,y,length,color,0);
}
void LCD_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor) {
    while (h--) {
        LCD_drawHorizontalLine(x, y++, w, fillcolor);
    }
}
#pragma endregion LCD_-functions
// --------------------------------------------------------------------------------------------
int main(void)
{
// -Port- und Signal-Initialisierung-----------------------------------------------------------
    #pragma region Initialisierung
        // LCD_CS = PORT C3, High, output
        LCD_CS_1;
        DDRC |= _BV(3); 
        // LCD_RS = PORT C2, High, output ("CD" = Command/Data)
        LCD_CD_0;
        DDRC |= _BV(2);
        // LCD_WR = PORT C1, High, output
        LCD_WR_1;
        DDRC |= _BV(1); 
        // LCD_RD = PORT C0, High, output
        LCD_RD_1;
        DDRC |= _BV(0);
        // LCD_RST = PORT C4, High, output
        LCD_RST_1;
        DDRC |= _BV(4); 
        LCD_cursor_x = 0;
        LCD_cursor_y = 0;
        LCD_textsize = 1;
        LCD_textcolor = 0xFFFF;
        LCD_rotation = 0;
    #pragma endregion Initialisierung
// -Reset--------------------------------------------------------------------------------------
    #pragma region Reset
        LCD_RST_0;
        LCD_Delay(100);
        LCD_RST_1;
        LCD_Delay(1000);
        LCD_writeData(0);   // resync
        LCD_writeData(0);
        LCD_writeData(0);
        LCD_writeData(0);
    #pragma endregion Reset
// -initDisplay-------------------------------------------------------------------------------
    #pragma region initDisplay
        static uint16_t a, d;
        for (uint8_t i = 0; i < sizeof(_regValues) / 4; i++) {
            a = pgm_read_word_near(_regValues + i*2);
            d = pgm_read_word_near(_regValues + i*2 + 1);
            if (a == 0xFF) {
                LCD_Delay(d);
            } else {
                LCD_writeRegister(a, d);
            }
        }
    #pragma endregion initDisplay
// -CheckDisplay-------------------------------------------------------------------------------
    #pragma region CheckDisplay
        static uint16_t identifier;
        identifier = LCD_readRegister(0x0);
//         if (identifier != 0x9325) {
//             for(;;); // trap CPU
//         }
//         if (identifier != 0x5408) {
//             for(;;); // trap CPU
//         }
        if (identifier != 0x0404) { // SPFD5408A
            for(;;); // trap CPU
        }
    #pragma endregion CheckDisplay
// -draw something-----------------------------------------------------------------------------
    #define BOXSIZE 40
    LCD_fillScreen(LCD_BLUE);
    LCD_fillRect(0, 0, BOXSIZE, BOXSIZE, LCD_RED);
    while(1)
    {
        //TODO:: Please write your application code 
    }
}