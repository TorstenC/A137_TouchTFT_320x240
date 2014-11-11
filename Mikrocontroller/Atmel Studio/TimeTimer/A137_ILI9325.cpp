/*
 * UNO_TFT.cpp
 *
 * Created: 05.02.2014 19:58:29
 *  Author: Torsten
 */ 

// ToDo: umstellen auf Embedded-C (AVR Toolchain 3.4.2-1573, avr-gcc 4.7.2,)

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "A137_ILI9325.h"
#define TXEN_is_set                 // undefine, if PORTD0 or PORTD1 are used as GPIO
#define trap_CPU for (;;) __asm("nop")

//#pragma region Port-Pins
    // LCD_CS = PORT C3
    #define LCD_CS_1 PORTC |= _BV(3)
    #define LCD_CS_0 PORTC &= ~_BV(3)
    // LCD_RS = PORT C2 ("CD" = Command/Data)
    #define LCD_CD_1 PORTC |= _BV(2)
    #define LCD_CD_0 PORTC &= ~_BV(2)
    // LCD_WR = PORT C1
    #define LCD_WR_1 PORTC |= _BV(1)
    #define LCD_WR_0 PORTC &= ~_BV(1)
    // LCD_RD = PORT C0
    #define LCD_RD_1 PORTC |= _BV(0)
    #define LCD_RD_0 PORTC &= ~_BV(0)
    // LCD_RST = PORT C4
    #define LCD_RST_1 PORTC |= _BV(4)
    #define LCD_RST_0 PORTC &= ~_BV(4)
    // Shortcuts
    #define LCD_CS_0_Data PORTC |= (_BV(2) + _BV(1) + _BV(0)); PORTC &= ~_BV(3)
    #define LCD_CS_0_Command PORTC &= ~(_BV(3) + _BV(2))

    // LCD_Data = PB0, PB1, PD2..PD7
    #define LCD_DataMask 0b00000011  // Port B - Maske
    //     ~LCD_DataMask             // Port D - Maske
    
    inline void LCD_setWriteDir(void) {
        DDRB |= LCD_DataMask;
        DDRD |= ~LCD_DataMask;
    }
    inline void LCD_setReadDir(void) {
        DDRB &= ~LCD_DataMask;
        #ifdef TXEN_is_set
            DDRD = LCD_DataMask;        // is it faster then "&="?
        #else
            DDRD &= LCD_DataMask;
        #endif
    }
    inline void LCD_write8(uint8_t d) {
        PORTB = (PORTB & ~LCD_DataMask) | (d & LCD_DataMask);
        #ifdef TXEN_is_set
            PORTD = d;
        #else 
            PORTD = (PORTD & LCD_DataMask) | (d & ~LCD_DataMask);
        #endif
        //TODO: Move "LCD_WR_0; LCD_WR_1" to here;
    }
    inline uint8_t LCD_read8(void) {
        uint8_t d;
        d = PINB & LCD_DataMask;
        d |= PIND & ~LCD_DataMask;
        //TODO: return (PINB & LCD_DataMask) | (PIND & ~LCD_DataMask): faster?
        return d;
    }
//#pragma endregion Port-Pins
//#pragma region Data
    static const uint16_t _regValues[] PROGMEM = {
        0x0000, 0x0001,
        0x0001, 0x0100,
        0x0002, 0x0300,
        0x0003, 0xD030,
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
        0x0012, 0x0018,
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
        0x0095, 0x0100,
        0x0097, 0x0000,
        0x0007, 0x0173,
        0x00FF, 500
    };


    // register names
    #define LCD_R_START_OSC			    0x00
    #define LCD_R_DRIVERCODE            0x00
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
//#pragma endregion Data
// -LCD_-functions-----------------------------------------------------------------------------
//#pragma region LCD_-functions
    //uint16_t LCD_cursor_x;
    //uint16_t LCD_cursor_y;
    //uint8_t  LCD_textsize;
    //uint16_t LCD_textcolor;
    //uint8_t  LCD_rotation;              // deprecated, use LCD_SelectModeOfEntry & LCD_EntryMode_

    struct LCD_ViewPortT {
        LCD_Orientations EntryMode;
        LCD_UpdateDirections UpdDir;
        uint16_t X;
        uint16_t Y;
        uint16_t W;
        uint16_t H;
    };
    
    struct LCD_ViewPortT LCD_ViewPort_;

    enum LCD_RGBWriteModes {
        RGB_65536  = 0x0000,
        RGB_262144 = 0xC000,
        BGR_65536  = 0x1000,
        BGR_262144 = 0xD000             // default for fast writing with „Arduino Uno R3” + „A137”
    } LCD_RGBWriteMode = BGR_262144;


    /********************************
    * Touch
    ********************************/



    // XM = PC2 / ADC 2
    #define XM_LOW   PORTC &= ~_BV(2); DDRC  |= _BV(2)
    #define XM_HIZ   PORTC &= ~_BV(2); DDRC  &= ~_BV(2)
    #define XM_Input XM_HIZ; ADMUX = _BV(REFS0) + 0x02

    // YP = PC1 / ADC 1
    #define YP_HIGH  PORTC |= _BV(1);  DDRC  |=  _BV(1)
    #define YP_HIZ   PORTC &= ~_BV(1); DDRC  &= ~_BV(1)
    #define YP_Input YP_HIZ; ADMUX = _BV(REFS0) + 0x01

    // XP = PD6 (digital)
    #define XP_HIZ   PORTD &= ~_BV(6); DDRD  &= ~_BV(6)
    #define XP_HIGH  PORTD |= _BV(6);  DDRD  |=  _BV(6)

    // YM = PD7 (digital)
    #define YM_HIZ   PORTD &= ~_BV(7); DDRD  &= ~_BV(7)
    #define YM_LOW   PORTD &= ~_BV(7); DDRD  |=  _BV(7)

    LCD_TouchData LCD_getTouchData() {
        LCD_TouchData result;
        // TODO: ADC Noise Canceler (ADC Noise Reduction mode or Idle mode)
        // SETUP:
        // By default, the successive approximation circuitry requires an input clock frequency
        // between 50 kHz and 200 kHz to get maximum resolution.

        // Voltage Reference Selection (ADMUX)
        // REFS1 = 0; REFS0 = 1: AVCC with external capacitor at AREF pin

        // ADCSRA:ADPS2…0: ADC Prescaler Select Bits:
        // 6 = 64 = 250KHz (/13 = 19KHz / 52µs) (x 8 = 0,42ms)
        // 7 = 128 = 125KHz (/ 13 = 9,6KHz / 104µs) (x 8 = 0,83ms)
        // The prescaling is set by the ADPS bits in ADCSRA. The prescaler starts counting from the moment
        // the ADC is switched on by setting the ADEN bit in ADCSRA.
        
        // ADCSRA = … + _BV(ADEN);

        // A normal conversion takes 13 ADC clock cycles. The first conversion after the ADC is switched
        // on (ADEN in ADCSRA is set) takes 25 ADC clock cycles in order to initialize the analog circuitry.
        // The actual sample-and-hold takes place 1.5 ADC clock cycles after the start of a normal conver-
        // sion and 13.5 ADC clock cycles after the start of an first conversion.
        // In Single Conversion mode, always select the channel before starting the conversion.
        // The simplest method is to wait for the conversion to complete before changing the channel selection.

        ADCSRA = 0x7 + _BV(ADEN); // 128 = 125KHz

        // Select Input:
        // ADMUX:MUX3..0: 0001=ADC1; 0010 = ADC2
        
        // ...

        // START CONVERSION
        // A single conversion is started by disabling the Power Reduction ADC bit, PRADC, by writing a
        // logical zero to it and writing a logical one to the ADC Start Conversion bit ADSC.

        PRR &= ~_BV(PRADC);

        // ...
        
        // CONVERTED
        // ADSC stays high as long as the conversion is in progress and will be cleared by hardware when the
        // conversion is completed.
        // After the conversion is complete (ADIF is high), the conversion result can be found in the ADC
        // Result Registers (ADCL, ADCH). ADCL must be read first, then ADCH to ensure that the content of the Data
        // Registers belongs to the same conversion.





        // 4..8 Messungen, dann Offset und Steigung => 2 x X, 3 x Y, 3 x Z
        // Z, Y, X, Z, (Y, X, Z, Y)
        // Gewichtung 1: X = X1 << 1 ; Y = Y1 << 1; Z = Z1 + Z2
        // Gewichtung 2: X = X1 + X2; Y = (Y1 + Y2 + X3 + X3) >> 1; Z = (Z1 + Z2 + Z3 + Z3) >> 1;

        uint16_t X1;
        //uint16_t X2;
        uint16_t Y1;
        //uint16_t Y2;
        //uint16_t Y3;
        uint16_t Zx1;
        //uint16_t Zx2;
        //uint16_t Zx3;
        uint16_t Zy1;
        //uint16_t Zy2;
        //uint16_t Zy3;


        // bestimme Y1
        XP_HIZ;     // X+ (PD6) - HIZ (low, ggf. wg. PullUp)
        XM_Input;   // X- (PC2) - Input (low, ggf. wg. PullUp, ADC-MUX setzen)
        YP_HIGH;    // Y+ (PC1) - Out HIGH
        YM_LOW;     // Y- (PD7) - Out LOW
        // ADU von "X-" (PC2) -> Y-Pos
        ADCSRA |= _BV(ADSC); // start Conversion
        while (ADCSRA & _BV(ADSC)); // conversion is in progress
        Y1 = ADC;
        
        // bestimme X1
        YM_HIZ;     // Y- (PD7) - HIZ (low, ggf. wg. PullUp)
        YP_Input;   // Y+ (PC1) - Input (low, ggf. wg. PullUp)
        XP_HIGH;    // X+ (PD6) - Out HIGH
        XM_LOW;     // X- (PC2) - Out LOW
        
        // ADU von "Y+" (PC1) -> X-POS
        ADCSRA |= _BV(ADSC); // start Conversion
        while (ADCSRA & _BV(ADSC)); // conversion is in progress
        X1 = ADC;
        
        // bestimme Druck ZX1
        YP_HIZ;     // Y+ (PC1) - Input (low, ggf. wg. PullUp)
        XM_Input;   // X- (PC2) - Input (low, ggf. wg. PullUp)
        XP_HIGH;    // X+ (PD6) - Out HIGH
        YM_LOW;     // Y- (PD7) - Out LOW

        // ADU von "X-" (PC2) -> ZX
        ADCSRA |= _BV(ADSC); // start Conversion
        while (ADCSRA & _BV(ADSC)); // conversion is in progress
        Zx1 = ADC;

        // bestimme Druck ZY1
        XM_HIZ;     // X- (PC2) - Input (low, ggf. wg. PullUp)
        YP_Input;   // Y+ (PC1) - Input (low, ggf. wg. PullUp)
        // ADU von "Y+" (PC1) -> Za
        ADCSRA |= _BV(ADSC); // start Conversion
        while (ADCSRA & _BV(ADSC)); // conversion is in progress
        Zy1 = ADC;
        
        // Gewichtung 1: X = X1 << 1 ; Y = Y1 << 1; Z = Z1 + Z2


        double X_ = X1;
        double Y_ = Y1;

        X_ *= -0.318328774;
        X_ += 276.9241482;
        Y_ *= -0.380937264;
        Y_ += 361.177731;

        result.X = X_;
        result.Y = Y_;
        result.Zx = Zx1; 
        result.Zy = Zy1;
        if ((Zy1 > 18) && (Zx1 < 992) && (X1 > 127)) {
            result.Z = 200;
        } else {
            result.Z = 0;
        }
        DDRC |= _BV(2);
        LCD_WR_1;
        DDRC |= _BV(1);
        return result;
    }




/************************************************************************
 * internal 
 * ¯¯¯¯¯¯¯¯
 ************************************************************************/

void LCD_Delay(uint16_t t) {
    while (t--) {
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
        __asm("nop");
    }
}

void LCD_StartDrawUnsave() {
    LCD_CS_0_Data;  // LCD_CS_0; LCD_CD_1;
    LCD_setWriteDir();
}

void LCD_EndDrawUnsave() {
    LCD_CS_1;
}

//TODO: Inline

void LCD_writeRGBt_unsafe(const LCD_RGBColor *color) {
    PORTD = (PORTD & LCD_DataMask) | ((*color)[0] & ~LCD_DataMask); // (PORTD & LCD_DataMask) | not necessary for TXEN 
    //color += 1;
    LCD_WR_0;
    LCD_WR_1;
    PORTD = (PORTD & LCD_DataMask) | ((*color)[1] & ~LCD_DataMask); // (PORTD & LCD_DataMask) | not necessary for TXEN 
    //color += 1;
    LCD_WR_0;
    LCD_WR_1;
    PORTD = (PORTD & LCD_DataMask) | ((*color)[2] & ~LCD_DataMask); // (PORTD & LCD_DataMask) | not necessary for TXEN 
    LCD_WR_0;
    LCD_WR_1;
}
void LCD_DrawPixel(uint8_t Red, uint8_t Green, uint8_t Blue) {
    LCD_RGBColor C = {Red, Green, Blue};
    LCD_DrawPixel (&C);
}

void LCD_DrawPixel(const LCD_RGBColor *color) {
    LCD_CS_0_Data;  // LCD_CS_0; LCD_CD_1;
    LCD_setWriteDir();
    LCD_writeRGBt_unsafe(color);
    LCD_CS_1;
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
    LCD_CS_0_Data;  // LCD_CS_0;
                    // LCD_CD_1;
    LCD_setWriteDir();
    LCD_writeData_unsafe(data);
    LCD_CS_1;
}

// If a mismatch arises due to noise and so on, LCD_resync
// ensures to restart with a transfer of upper byte.
void LCD_resync() {
    LCD_CS_0_Command;
    LCD_setWriteDir();
    LCD_write8(0);
    LCD_WR_0; LCD_WR_1;
    LCD_write8(0);
    LCD_WR_0; LCD_WR_1;
    LCD_write8(0); LCD_WR_0;
    LCD_WR_1; LCD_write8(0);
    LCD_WR_0; LCD_WR_1;
    LCD_CS_1;
}

void LCD_writeCommand(uint16_t cmd) {
    LCD_CS_0_Command;   // LCD_CS_0;
                        // LCD_CD_0;
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
volatile uint16_t LCD_DelayRD0 = 1; // 10?
uint16_t LCD_readData() {
    uint16_t d = 0;
    LCD_CS_0;
    LCD_CD_1;
    // LCD_RD is 1;
    // LCD_WR is 1;
    LCD_setReadDir();
    LCD_RD_0;
    LCD_RD_0;
    LCD_RD_0;
    // LCD_Delay(LCD_DelayRD0);
    d = LCD_read8();
    LCD_RD_1;
    d <<= 8;
    LCD_RD_0;
    LCD_RD_0;
    LCD_RD_0;
    // LCD_Delay(LCD_DelayRD0);
    d |= LCD_read8();
    LCD_RD_1;
    LCD_CS_1;
    return d;
}

// addr = Register-Adresse im Grafiktreiber
uint16_t LCD_readRegister(uint16_t addr) {
    static uint16_t test;
    LCD_writeCommand(addr);
    test = LCD_readData();
    return test;
}
void LCD_writeRegister(uint8_t addr, uint16_t data) {
    LCD_writeCommand(addr);
    LCD_writeData(data);
    static uint16_t test;
    if (addr) {
        test = LCD_readRegister(addr);
        if (test != data) {
            trap_CPU;
        } else {
            __asm("nop");   // for breakpoint
        }
    }
}

//TODO: Eleganter umsetzen und die restlichen EntryModes umsetzen
#define swap_uint16_t(a, b) { uint16_t t = a; a = b; b = t; }


// posx / posy = 0..319 bzw. 0..239
void LCD_calcGRAMPosition(uint16_t *posx, uint16_t *posy) {
    uint16_t x = *posx;
    uint16_t y = *posy;
    switch(LCD_ViewPort_.EntryMode)  {
        case LCD_Left_N:  // 90
            swap_uint16_t(x, y);
            x = LCD_width - x - 1;
        break;
        case LCD_Top_N:  // 180
            y = LCD_height - y - 1;
            x = LCD_width - x - 1;
        break;
        case LCD_Right_N: // 270
            swap_uint16_t(x, y);
            y = LCD_height - y - 1;
        break;
        case LCD_Bottom_N:
            // nothing 2 do
        break;
        default:
            trap_CPU;
        break;
    }
    *posx = x;
    *posy = y;
}

/************************************************************************
 * Public (A137_ILI9325.h) 
 * ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
 ************************************************************************/

void LCD_goTo(uint16_t x, uint16_t y) {
    LCD_resync();
    LCD_calcGRAMPosition(&x, &y);
    LCD_writeRegister(LCD_R_GRAM_HOR_AD, x);    // GRAM Address Set (Horizontal Address) (R20h)
    LCD_writeRegister(LCD_R_GRAM_VER_AD, y);    // GRAM Address Set (Vertical Address) (R21h)
    LCD_writeCommand(LCD_R_RW_GRAM);            // Write Data to GRAM (R22h)
}

// inline void LCD_goTo() {
//     LCD_goTo(0,0);    
// }
// 
// inline void LCD_goHome(void) {                  // Deprecated, use LCD_goTo();
//     LCD_goTo(0,0);
// }

// void LCD_SelectModeOfEntry(LCD_ModesOfEntry MOE) {
//     LCD_EntryMode_ = MOE;
//     LCD_goHome();
// }

void LCD_fillScreen(uint16_t color) {
    LCD_setViewport();
    uint32_t i;
    i = LCD_width;
    i *= LCD_height;
    LCD_CS_0_Data; // LCD_RD has to be 1, LCD_WR has to be 1;
    LCD_setWriteDir();
    while (i--) {
    //TODO: use inline LCD_writeData_unsafe(color);
    #ifdef TXEN_is_set
        PORTD = color >> 8;
        LCD_WR_0; LCD_WR_1;
        PORTD = color >> 3;
        LCD_WR_0; LCD_WR_1;
        PORTD =  color << 3;
        LCD_WR_0; LCD_WR_1;
    #else
        #error "TXEN_is_set id undefined"
        //TODO: Variante mit:
        // (PORTD & LCD_DataMask) | ((color >> 8) & ~LCD_DataMask)
        // (PORTD & LCD_DataMask) | ((color >> 3) & ~LCD_DataMask)
        // (PORTD & LCD_DataMask) | ((color << 3) & ~LCD_DataMask)
    #endif
    }
    LCD_CS_1;
}

void LCD_fillScreen(LCD_RGBColor* color) {
    //TODO: Kann ein const hier erlaubt werden, um auf Progmem zu zeigen?
    uint8_t red, green, blue;
    red   = (*(color))[0];
    green = (*(color))[1];
    blue  = (*(color))[2];
    LCD_setViewport();
    LCD_CS_0_Data;
    LCD_setWriteDir();

    #ifndef TXEN_is_set
        #error "TXEN_is_set id undefined"
    #endif
    #if 0xFFFF < LCD_width * LCD_height / 2
        #error "Display too large"
    #endif

    if ((red == green) && (green == blue)) {
        PORTD = red;
        for (uint16_t i = (uint32_t)LCD_width * LCD_height / 2; i; i--) {
            LCD_WR_0; LCD_WR_1;     // "LCD_height / 2" and "uint16_t i"
            LCD_WR_0; LCD_WR_1;     //       is faster than "uint32_t i"
            LCD_WR_0; LCD_WR_1;
            LCD_WR_0; LCD_WR_1;
            LCD_WR_0; LCD_WR_1;
            LCD_WR_0; LCD_WR_1;
        }
    } else {
        //TODO: faster for (red == green) || (green == blue) || (red == blue)
        for (uint16_t i = (uint32_t)LCD_width * LCD_height / 2; i; i--) {
            PORTD = red; 
            LCD_WR_0; LCD_WR_1;
            PORTD = green;
            LCD_WR_0; LCD_WR_1;
            PORTD = blue;
            LCD_WR_0; LCD_WR_1;
            PORTD = red;
            LCD_WR_0; LCD_WR_1;
            PORTD = green;
            LCD_WR_0; LCD_WR_1;
            PORTD = blue;
            LCD_WR_0; LCD_WR_1;
        }
    }
    LCD_CS_1;
    LCD_resync();
}


// void LCD_drawFastLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color, uint8_t rotflag) {
//     uint16_t newentrymod;
//     uint16_t prevEntryMod = LCD_readRegister(LCD_R_ENTRY_MOD);
//     switch (LCD_rotation) {
//         case 0:
//             if (rotflag) 
//                 newentrymod = 0x1028;               // we want a 'vertical line'
//             else
//                 newentrymod = 0x1030;               // we want a 'horizontal line'
//         break;
//         case 1:
//             swap_uint16_t(x, y);
//             x = LCD_width - x - 1;                   // first up fix the X
//             if (rotflag)
//                 newentrymod = 0x1000;               // we want a 'vertical line'
//             else
//                 newentrymod = 0x1028;               // we want a 'horizontal line'
//         break;
//         case 2:
//             x = LCD_width - x - 1;
//             y = LCD_height - y - 1;
//             if (rotflag)
//                 newentrymod = 0x1008;               // we want a 'vertical line'
//             else
//                 newentrymod = 0x1020;               // we want a 'horizontal line'
//         break;
//         case 3:
//             swap_uint16_t(x,y);
//             y = LCD_height - y - 1;
//             if (rotflag)
//                 newentrymod = 0x1030;               // we want a 'vertical line'
//             else
//                 newentrymod = 0x1008;               // we want a 'horizontal line'
//         break;
//         default:
//             newentrymod = 0x1000;
//             trap_CPU;
//         break;
//     }
//     newentrymod |= 0xC000;
//     LCD_writeRegister(LCD_R_ENTRY_MOD, newentrymod);
//     LCD_writeRegister(LCD_R_GRAM_HOR_AD, x);       // GRAM Address Set (Horizontal Address) (R20h)
//     LCD_writeRegister(LCD_R_GRAM_VER_AD, y);       // GRAM Address Set (Vertical Address) (R21h)
//     LCD_writeCommand(LCD_R_RW_GRAM);               // Write Data to GRAM (R22h)
//     LCD_CS_0_Data;
//     LCD_setWriteDir();
//     while (length--) {
// //         LCD_writeData_unsafe(color);
// //---------------------------- (DFM=1)
//         PORTD =  color >> 8; // ((color >> 8) & ~LCD_DataMask);
//         LCD_WR_0; LCD_WR_1;
//         PORTD = color >> 3; // ((color >> 3) & ~LCD_DataMask);
//         LCD_WR_0; LCD_WR_1;
//         PORTD = color << 3; // ((color << 3) & ~LCD_DataMask);
//         LCD_WR_0; LCD_WR_1;
// //---------------------------- (DFM=0)
// //         LCD_write8(color >> 14);
// //         LCD_WR_0;
// //         LCD_WR_1;
// //         LCD_write8(color >> 7);
// //         LCD_WR_0;
// //         LCD_WR_1;
// //         LCD_write8(color << 1);
// //         LCD_WR_0;
// //         LCD_WR_1;
//     }
//     LCD_CS_1;
//     LCD_writeRegister(LCD_R_ENTRY_MOD, prevEntryMod); // set back to default
// }

// void LCD_drawHorizontalLine(uint16_t x, uint16_t y, uint16_t length, uint16_t color) {
//     if (y >= LCD_height) return;
//     LCD_drawFastLine(x, y, length, color, 0);
// }
// void LCD_fillRect_old(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor) {
//     while (h--) {
//         LCD_drawHorizontalLine(x, y++, w, fillcolor);
//     }
// }

//LCD_fillRect(54, 1, 2, 318, LCD_ORANGE);
void LCD_fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t fillcolor) {
    // Todo: benutze SetViewport
    LCD_writeRegister(LCD_R_ENTRY_MOD, LCD_Left_N | BGR_262144);
    LCD_writeRegister(LCD_R_HOR_START_AD, x);
    LCD_writeRegister(LCD_R_HOR_END_AD, x + w -1);
    LCD_writeRegister(LCD_R_VER_START_AD, y);
    LCD_writeRegister(LCD_R_VER_END_AD, y + h - 1);
    LCD_writeRegister(LCD_R_GRAM_HOR_AD, x);    // GRAM Address Set (Horizontal Address) (R20h)
    LCD_writeRegister(LCD_R_GRAM_VER_AD, y);    // GRAM Address Set (Vertical Address) (R21h)
    LCD_writeCommand(LCD_R_RW_GRAM);            // Write Data to GRAM (R22h)
    uint8_t red, green, blue;
    red   = (fillcolor >> 8) & 0xFC;
    green = (fillcolor >> 3) & 0xFC;
    blue  = fillcolor << 3;
    LCD_CS_0_Data;
    LCD_setWriteDir();
    if ((red == green) && (green == blue)) {
        #ifdef TXEN_is_set
            PORTD = red;
            for (uint32_t i = w * h; i; i--) {
                LCD_WR_0; LCD_WR_1;
                LCD_WR_0; LCD_WR_1;
                LCD_WR_0; LCD_WR_1;
            }
        #else
            #error "TXEN_is_set id undefined"
        #endif
    } else {
        for (uint32_t i = w * h; i; i--) {
            #ifdef TXEN_is_set
                PORTD = red;
                LCD_WR_0; LCD_WR_1;
                PORTD = green;
                LCD_WR_0; LCD_WR_1;
                PORTD = blue;
                LCD_WR_0; LCD_WR_1;
            #else
                #error "TXEN_is_set id undefined"
            #endif
        }
    }
}

void LCD_setViewport(LCD_UpdateDirections UD, uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height) {
    LCD_ViewPort_.UpdDir = UD;
    LCD_setViewport(Left, Top, Width, Height);
}
void LCD_setViewport(uint16_t Left, uint16_t Top, uint16_t Width, uint16_t Height){
    LCD_ViewPort_.H = Height;
    LCD_ViewPort_.W = Width;
    LCD_ViewPort_.X = Left;
    LCD_ViewPort_.Y = Top;
    uint16_t y0;
    uint16_t x0;
    uint16_t y1;
    uint16_t x1;
    uint16_t y2;
    uint16_t x2;

    switch(LCD_ViewPort_.EntryMode)  {
        case LCD_Bottom_N:
            // horizontal increment
            x1 = Left;
            x2 = Left + Width - 1;
            x0 = Left;

            // vertical decrement
            y1 = LCD_height - Top - Height;
            y2 = LCD_height - Top - 1;
            y0 = LCD_height - Top - 1;
        break;
        default:
            trap_CPU;
        break;
    }


#ifdef DEBUG
    // From datasheet:
    // In setting, be sure  “00”h ≤ HSA[7:0] < HEA[7:0] ≤ “EF”h and “04”h ≦ HEA - HAS.
    // In setting, be sure “000”h ≤ VSA[8:0] < VEA[8:0] ≤ “13F”h.
    // The RAM address set register must be an address within the window address area.
    // HSA[7:0] ≤ AD[7:0] ≤ HEA[7:0] and VSA[8:0] ≤ AD[15:8] ≤ VEA[8:0]

    if ((x0 < x1) || (x0 > x2) || (x2 > 239) || (x2 < (4 + x1)))
        trap_CPU;
    if ((y0 < y1) || (y0 > y2) || (y2 > 319) ||(y2 <= y1))
        trap_CPU;
#endif // DEBUG
    
    //TODO: in einem Schritt
    uint16_t EntryMode;
    EntryMode = LCD_ViewPort_.EntryMode;
    EntryMode ^= LCD_ViewPort_.UpdDir;
    EntryMode |= LCD_RGBWriteMode;


    LCD_resync();
    LCD_writeRegister(LCD_R_ENTRY_MOD, EntryMode);
    LCD_writeRegister(LCD_R_HOR_START_AD, x1);
    LCD_writeRegister(LCD_R_HOR_END_AD, x2);
    LCD_writeRegister(LCD_R_VER_START_AD, y1);
    LCD_writeRegister(LCD_R_VER_END_AD,y2);

// Vertical decrement: y2
// Horizontal decrement: y2

    LCD_writeRegister(LCD_R_GRAM_HOR_AD, x0);    // GRAM Address Set (Horizontal Address) (R20h)
    LCD_writeRegister(LCD_R_GRAM_VER_AD, y0);    // GRAM Address Set (Vertical Address) (R21h)

    static uint16_t test;
    test = LCD_readRegister(LCD_R_ENTRY_MOD);
    if (test != EntryMode) {
        trap_CPU;
    }

    LCD_writeCommand(LCD_R_RW_GRAM);            // Write Data to GRAM (R22h)
}

void LCD_setViewport() {
    switch (LCD_ViewPort_.EntryMode) {
        case LCD_Bottom_N:
            LCD_setViewport(0, 0, LCD_width, LCD_height);
   	    break;
        case LCD_Left_N:
            LCD_setViewport(0, 0, LCD_height, LCD_width);
        break;
        default:
            trap_CPU;
        break;
    }
}
//#pragma endregion LCD_-functions
// --------------------------------------------------------------------------------------------
void LCD_init(LCD_Orientations MOE) {
    volatile uint16_t LCD_DelayReset0 = 1; // 100?
    volatile uint16_t LCD_DelayReset1 = 1; // 1000?
// -Port- und Signal-Initialisierung-----------------------------------------------------------
//#pragma region Initialisierung
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
        //LCD_cursor_x = 0;
        //LCD_cursor_y = 0;
        //LCD_textsize = 1;
        //LCD_textcolor = 0xFFFF;
        LCD_ViewPort_.EntryMode = MOE;
        LCD_ViewPort_.X = 0;
        LCD_ViewPort_.Y = 0; 
        LCD_ViewPort_.UpdDir = LCD_X_Y;
        switch (MOE) {
            case LCD_Bottom_N:
                LCD_ViewPort_.W = LCD_width;
                LCD_ViewPort_.H = LCD_height;
            break;
            case LCD_Left_N:
            break;
            default:
                trap_CPU;
            break;
        }

//#pragma endregion Initialisierung
// -Reset--------------------------------------------------------------------------------------
//#pragma region Reset
        LCD_RST_0;
        LCD_Delay(LCD_DelayReset0);
        LCD_RST_1;
        LCD_Delay(LCD_DelayReset1);
        // ToDo: is that wrong?
//         LCD_writeData(0);   // resync
//         LCD_writeData(0);
//         LCD_writeData(0);
//         LCD_writeData(0);
        LCD_resync();
//#pragma endregion Reset
// -initDisplay-------------------------------------------------------------------------------
//#pragma region initDisplay
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
//#pragma endregion initDisplay
// -CheckDisplay-------------------------------------------------------------------------------
//#pragma region CheckDisplay
        static uint16_t identifier;
        identifier = LCD_readRegister(LCD_R_DRIVERCODE);
        if (identifier != 0x9325) {
            trap_CPU;
        }
//#pragma endregion CheckDisplay
}