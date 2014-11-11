/*
 * UASG01.h
 *
 * Created: 19.01.2014 23:14:31
 *  Author: Torsten
 */ 

//#include <stdint.h>
//#include <string.h>

#define _getlen(a) (sizeof(a) / sizeof(a[0]))

// For XML Comments see:
// - http://msdn.microsoft.com/de-de/library/ms177227.aspx and
// - http://www.atmel.no/webdoc/visualassist/visualassist._hover.html

#ifndef UASG01_H_
#define UASG01_H_

// Wird vom HAL alle 100ms aufgerufen. Achtung: Nur für kurze Routinen
void App_tick_100ms();

// Event, einmal pro Sekunde. Achtung: Nur für kurze Routinen
extern void (*volatile HAL_OnTickSecond)();

// Event, einmal pro Minute. Achtung: Nur für kurze Routinen
extern void (*volatile HAL_OnTickMinute)();

// Wird vom HAL einmal beim RESET aufgerufen
void App_init();


extern void UART0SendHex4Nibble(uint16_t W);

extern void UART0Transmit(uint8_t B);

struct HAL_Time {
	uint8_t	Hour;
	uint8_t	Minute;
	uint8_t	Second;
};

void HAL_getTime(HAL_Time *CurrentTime);
void HAL_resetTime();

#endif /* UASG01_H_ */