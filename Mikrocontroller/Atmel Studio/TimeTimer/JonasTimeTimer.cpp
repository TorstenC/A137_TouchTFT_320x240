/*
 * JonasTimeTimer.cpp
 *
 * Created: 01.03.2014 13:10:28
 *  Author: Torsten
 */ 


#include "A137_ILI9325.h"
#include <avr/interrupt.h>							// ISR(vector, ...), sei, cli, ...
#include "LCD_SansSerif24a.h"
#include "Fonts.h"
#include "HAL.h"
#include <math.h> 
#define PI 3.14159265

// struct TouchData Result;
// bool countDown = true;
// bool gitter = false;
bool countDown = false;
bool gitter = true;

double Faktor;
uint32_t ZehntelSekunden;
int16_t Sekunden_Disp_Zehntel;
char Minuten_1_Disp;
char Minuten_10_Disp;
int8_t LetzteSekunde = 99;

HAL_Time CurrentTime;


enum Quadranten {
    ObenLinks,
    UntenLinks,
    UntenRechts,
    ObenRechts
};


struct Auswahl {
    double Faktor;
    uint32_t ZehntelSekunden;
    char Minuten_10_Disp;
    char Minuten_1_Disp;
};

#define Kaest(a,b) {720.0 / 600.0 / a##b, (uint32_t)600 * a##b, #a[0], #b[0]}

#define MaxKaestchen 18
Auswahl Kaestchen[MaxKaestchen] = { \
    {720.0 / 600.0 / 05, 600 * 05, '0', '5'}, \
    {720.0 / 600.0 / 10, 600 * 10, '1', '0'}, \
    {720.0 / 600.0 / 15, 600 * 15, '1', '5'}, \
    Kaest(2,0),
    Kaest(2,5),
    Kaest(3,0),
    Kaest(3,5),
    Kaest(4,0),
    Kaest(4,5),
    Kaest(5,0),
    Kaest(5,5),
    Kaest(6,0),
    Kaest(6,5),
    Kaest(7,0),
    Kaest(7,5),
    Kaest(8,0),
    Kaest(8,5),
    Kaest(9,0)};

void EinmalProSekunde();

void App_init() {
    LCD_init(LCD_Bottom_N);
    EinmalProSekunde(); // hinterher gitter = false
//     UART0Transmit('\r');
//     UART0Transmit('\n');
    
}

void GesamterKreis(bool rot) {
    LCD_setViewport(0, 79, 239, 239);
    for (int8_t y = -119; y <= 119; y++) {
        // Zähle mit X von -119 bis 119 in 1-er Schritten
        for (int8_t x = -119; x <= 119; x = x + 1) {
            // berechne die Quadratzahlen für den Pythagoras
            uint16_t x_Quadrat = (uint16_t)x * x;
            uint16_t y_Quadrat = (uint16_t)y * y;
            // wenn X² + Y² < 119² dann …
            if ((x_Quadrat + y_Quadrat) < (119 * 119)) {
                // setze den nächsten Pixel auf GRÜN
                if (rot) 
                    LCD_DrawPixel(255, 0, 0);
                else
                    LCD_DrawPixel(0, 255, 0);
                } else { // … ansonsten nicht im Kreis, dann …
                // setze den nächsten Pixel auf SCHWARZ
                LCD_DrawPixel(0, 0, 0);
            }
        }
    }
}

void App_tick_100ms() {
#if false // TiuchScreenKalib
    LCD_TouchData T;
    T = LCD_getTouchData();
    UART0SendHex4Nibble(T.X);
    UART0Transmit(',');
    UART0SendHex4Nibble(T.Y);
    UART0Transmit(',');
    UART0SendHex4Nibble(T.Zx);
    UART0Transmit(',');
    UART0SendHex4Nibble(T.Zy);
    UART0Transmit('\r');
    UART0Transmit('\n');
#else
    if (!countDown) {
        LCD_TouchData T;
        T = LCD_getTouchData();
        if (T.Z > 127) {
            uint8_t X;
            uint8_t Y;
            uint8_t Index;
            X = T.X / 40;
            Y = T.Y / 40;
            Index = X + 6 * Y;
            if (Index < MaxKaestchen) {
                Auswahl A = Kaestchen[Index];
                Faktor = A.Faktor;
                ZehntelSekunden = A.ZehntelSekunden;
                Sekunden_Disp_Zehntel = 200 + 10 * 0;
                Minuten_10_Disp = A.Minuten_10_Disp;
                Minuten_1_Disp = A.Minuten_1_Disp;
                countDown = true;
            }
            if (countDown) {
                // Lösche den Bildschirm (0 = schwarz)
                LCD_fillScreen(LCD_BLACK);
                GesamterKreis(false);
                LCD_DrawChar(100, 20, 'o', &LCD_SansSerif24a);
                LCD_DrawSpacer(1);
                LCD_DrawChar('u');
                LCD_DrawSpacer(1);
                LCD_DrawChar('t');
                LCD_DrawSpacer(12);
                LCD_DrawChar(Minuten_10_Disp);
                LCD_DrawSpacer(1);
                LCD_DrawChar(Minuten_1_Disp);
                LCD_DrawChar(20, 20, Minuten_10_Disp, &LCD_SansSerif24a);
                LCD_DrawChar(Minuten_1_Disp);
            }

        }
    } else {
        //         HAL_getTime(&CurrentTime);
        //         if (CurrentTime.Second != LetzteSekunde) {
        //             LetzteSekunde = CurrentTime.Second;
        //             EinmalProSekunde();
        //          }
        EinmalProSekunde();
    }
#endif
}

int16_t LetzterWinkel = 999;

void EinmalProSekunde()
{
    if (gitter)
    {
//         Faktor = 720.0 / 60.0 / 9.0;
//         Sekundenn = 9 * 60;
        //LCD_ClearScreenGS(0);
        LCD_fillScreen(LCD_BLACK);

        // senkrechte Linien
        for (uint8_t X = 40; X < 201; X = X + 40)
        {
            LCD_setViewport(LCD_Y_X, X, 0, 5, 320);
            // Zeichne 960 Pixel, also 3 Spalten, nicht alle 5!
            for (uint16_t jonas = 0; jonas < 960; jonas = jonas + 1)
            {
                LCD_DrawPixel(0, 0, 255);
            };
        }

        // waagerechte Linien
        for (uint16_t Y = 40; Y < 281; Y = Y + 40)
        {
            LCD_setViewport(LCD_X_Y, 0, Y, 240, 5);
            // Zeichne 720 Pixel, also 3 Zeilen, nicht alle 5!
            for (uint16_t jonas = 0; jonas < 720; jonas = jonas + 1)
            {
                LCD_DrawPixel(0, 0,255);
            };
        }

//         LCD_DrawChar(13, 11, '4', &LCD_SansSerif24a);
//         LCD_DrawChar('5');
//         LCD_DrawChar(53, 11, '9', &LCD_SansSerif24a);
//         LCD_DrawChar('0');
        uint8_t X;
        uint8_t Y;
        for (X = 0; X < 6; X++) {
            for (Y = 0; Y < 8; Y++) {
                uint8_t Index;
                Index = X + 6 * Y;
                if (Index < MaxKaestchen) {
                    Auswahl A = Kaestchen[Index];
                    LCD_DrawChar(13 + (40 * X), 11 + (40 * Y), A.Minuten_10_Disp, &LCD_SansSerif24a);
                    LCD_DrawChar(A.Minuten_1_Disp);
                }
            }
        }

        gitter = false;
    }
    if (countDown)
    {
        if (Sekunden_Disp_Zehntel < 0) {
            Sekunden_Disp_Zehntel = 590;
            Minuten_1_Disp = Minuten_1_Disp - 1;
            if (Minuten_1_Disp < '0') {
                Minuten_1_Disp = '9';
                Minuten_10_Disp = Minuten_10_Disp - 1;
            }
            LCD_DrawChar(20, 20, Minuten_10_Disp, &LCD_SansSerif24a);
            LCD_DrawSpacer(1);
            LCD_DrawChar(Minuten_1_Disp);
            LCD_DrawSpacer(4);
        }
        Sekunden_Disp_Zehntel = Sekunden_Disp_Zehntel - 1;


        Quadranten Quadrant;

        int16_t Winkel = ZehntelSekunden * Faktor; // 0 .. 720, also halbe-Grad-Schritte
        if (Winkel != LetzterWinkel) {
            LetzterWinkel = Winkel;
            int8_t X_min;
            int8_t X_max;
            int8_t Y_min;
            int8_t Y_max;
            uint16_t Tan_mal_512 = 0;

            // Trigonometrie-Start (von Papa)
            double Dbl_Tangens = 512 * tan(((ZehntelSekunden * Faktor) - 180) / 360 * PI);
            if (Dbl_Tangens < 0.0) {
                Tan_mal_512  = (Dbl_Tangens < -65530.0) ? 65530 : (uint16_t)(-Dbl_Tangens);
            } else { // Dbl_Tangens >= 0.0
                Tan_mal_512  = (Dbl_Tangens > 65530.0) ? 65530 : (uint16_t)Dbl_Tangens;
            }
            // Trigonometrie-Ende

            if (Winkel >= 540) {
                // Aktueller Quadrant: Oben Links;
                Quadrant = ObenLinks;
                LCD_setViewport(LCD_X_Y, 0, 79, 120, 120);
                X_min = -119; X_max = 0; Y_min = 0; Y_max = 119;
            } else if (Winkel >= 360) {
                // Aktueller Quadrant: Unten Links;
                Quadrant = UntenLinks;
                LCD_setViewport(LCD_X_Y, 0, 198, 120, 120);
                X_min = -119; X_max = 0; Y_min = -119; Y_max = 0;
            } else if (Winkel >= 180) {
                // Aktueller Quadrant: UntenRechts;
                Quadrant = UntenRechts;
                LCD_setViewport(LCD_X_Y, 119, 198, 120, 120);
                X_min = 0; X_max = 119; Y_min = -119; Y_max = 0;
            } else {
                // Aktueller Quadrant: ObenRechts;
                LCD_setViewport(LCD_X_Y, 119, 79, 120, 120);
                X_min = 0; X_max = 119; Y_min = 0; Y_max = 119;
                Quadrant =  ObenRechts;
            }

//             UART0Transmit('\n');
//             UART0SendHex4Nibble(Winkel);
//             UART0Transmit(',');
//             UART0SendHex4Nibble(Tangens);

            // Hinweis: Bei jedem "DrawPixel" geht der Pixel automatisch
            // um eine Position weiter. Der EntryMode "LCD_X_Y" heißt
            // dabei: Erst geht die Position in der obersten Zeile von links
            // bis ganz nach rechts (X-Richtung), und am Ende der Zeile geht
            // es am Anfang derZeile darunter (Y-Richtung) weiter.


            LCD_StartDrawUnsave();

            // Insgesamt 120 x 120 = 14400 Pixel om Quadranten
            // Zähle mit Y von Y_max bis Y_min in 1-er Schritten (rückwärts)
            for (int8_t y = Y_max; y >= Y_min; y = y - 1)
            {
                uint16_t y_mal_512;
                switch (Quadrant) {
                    case ObenLinks: //y > 0, x < 0
                    case ObenRechts: // x > 0, y > 0
                        y_mal_512 = (uint32_t)y << 9;
                    break;
                    case UntenRechts: // y < 0, x > 0
                    case UntenLinks: // x < 0, y < 0
                        y_mal_512 = (uint32_t)(-y) << 9;
                    break;
                }


                // Zähle mit X von X_min bis X_max in 1-er Schritten
                for (int8_t x = X_min; x <= X_max; x = x + 1)
                {
                    // wenn X² + Y² < 119² dann …
                    uint16_t x_Quadrat = (uint16_t)x * x;
                    uint16_t y_Quadrat = (uint16_t)y * y;
                    if ((x_Quadrat + y_Quadrat) < (119 * 119))
                    {

                        bool imblauen;
//                         uint16_t y_durch_x;
//                         if (y > 0) {
//                             y_durch_x = y << 9;
//                         } else {
//                             y_durch_x = (-y) << 9;
//                         }
//                         if (x > 0) {
//                             y_durch_x /= x;
//                             } else if (x < 0) {
//                             y_durch_x /= (-x);
//                             } else { // x == 0
//                             if (y == 0) {
//                                 y_durch_x = 0;
//                                 } else {
//                                 y_durch_x = 65535;
//                             }
//                         }
//                         switch (Quadrant) {
//                             case ObenLinks:
//                             case UntenRechts:
//                                 imblauen = !(y_durch_x <= Tangens);
//                             break;
//                             case ObenRechts:
//                             case UntenLinks:
//                                 imblauen = (y_durch_x <= Tangens);
//                             break;
//                         }


                        uint32_t x_mal_Tangens;
                        switch (Quadrant) {
                            case ObenLinks: //y > 0, x < 0
                                x_mal_Tangens = (uint32_t)(-x) * Tan_mal_512;
                                imblauen = !(y_mal_512 <= x_mal_Tangens);
                            break;
                            case UntenRechts: // y < 0, x > 0
                                x_mal_Tangens = (uint32_t)x * Tan_mal_512;
                                imblauen = !(y_mal_512 <= x_mal_Tangens);
                            break;
                            case ObenRechts: // x > 0, y > 0
                                x_mal_Tangens = (uint32_t)x * Tan_mal_512;
                                imblauen = (y_mal_512 < x_mal_Tangens);
                            break;
                            case UntenLinks: // x < 0, y < 0
                                x_mal_Tangens = (uint32_t)(-x) * Tan_mal_512;
                                imblauen = (y_mal_512 < x_mal_Tangens);
                            break;
                        }

                        if (imblauen) {
                            // setze den nächsten Pixel auf BLAU
                            // LCD_DrawPixel(0, 0, 255);
                            LCD_writeRGBt_unsafe(&LCD_blue);
                        } else {
                            // setze den nächsten Pixel auf GRÜN
                            // LCD_DrawPixel(0, 252, 0);
                            LCD_writeRGBt_unsafe(&LCD_green);
                        }

                    } else { // … ansonsten nicht im Kreis, dann …
                    
                        // setze den nächsten Pixel auf SCHWARZ
                        // LCD_DrawPixel(0, 0, 0);
                        LCD_writeRGBt_unsafe(&LCD_black);
                    }
                }
            };
        } else { // Winkel wie vorher
            // UART0Transmit('=');
        }

        LCD_EndDrawUnsave();

        if (ZehntelSekunden <= 0) {
            countDown = false;
            GesamterKreis(true);
        } else {
            ZehntelSekunden = ZehntelSekunden - 1;
        }
    }
}
