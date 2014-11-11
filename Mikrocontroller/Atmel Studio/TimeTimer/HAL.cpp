// ToDo: CPU Traps durch eine Art "blue screen" ersetzen, falls #ifndef debug
// ToDo: Mega: Ein UART für Debug, den anderen für BAP, Mini/Uno: Nur BAP
// Pin-Belegungen:
// iom328p / iom2560:
//  - TXD (USART Output Pin) PD1
//  - RXD (USART Input Pin) PD0
// iom2560:
//  - SDA (TWI Serial DAta) PD1
//  - SCL (TWI Serial CLock) PD0
//	

#include <avr/io.h>									// importiert im Wesentlichen iom328p.h (Pro Mini) bzw. iom2560.h (Mega)
#include <avr/interrupt.h>							// ISR(vector, ...), sei, cli, ...
#include <stdint.h>									// importiert im Wesentlichen stdintgcc.h (uint8_t, uint16_t, …)
#include <avr/wdt.h>								// importiert wdt_disable();
#include "HAL.h"

void (*volatile HAL_OnTickSecond)() = 0;
void (*volatile HAL_OnTickMinute)();

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));
void get_mcusr(void) {
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();									// WDT aus, falls die WDTON-Fuse nicht aktiv ist.
}

volatile bool Tick100Flag = false;
volatile uint8_t Tick100Counter = 24;
volatile uint8_t SecondCounter = 50;
volatile uint8_t MinuteCounter = 40;
volatile uint8_t HourCounter = 19;
volatile uint8_t DayCounter = 26;
volatile uint8_t MonthCounter = 1;
volatile uint16_t YearCounter = 2014;
volatile bool FCP_SecondDirty = false;
volatile bool FCP_DateTimeDirty = false;

void HAL_getTime(HAL_Time *CurrentTime) {
	cli();										// Todo: Alles außer Tick100Counter außerhalb der ISR, und dann nicht volatile!
	CurrentTime->Hour   = HourCounter;
	CurrentTime->Minute = MinuteCounter;
	CurrentTime->Second = SecondCounter;
	sei();
}

void HAL_resetTime() {
	cli();										// Todo: Alles außer Tick100Counter außerhalb der ISR, und dann nicht volatile!
	HourCounter = 1;
	MinuteCounter = 20;
	SecondCounter = 50;
	sei();
}

#define TXBuf0Size 5							// Size = _BV(5) = 32 Bytes
volatile uint8_t TXBuffer0[_BV(TXBuf0Size)];
volatile uint8_t TXBufWr = 0;
volatile uint8_t TXBufRd = 0;

void UART0Transmit(uint8_t B) {
	UCSR0B &= ~_BV(UDRIE0);							// ISR(USART0_UDRE_vect) unterdrücken, damit sich der Zustand nicht ändern kann
	if ((TXBufWr == TXBufRd) && (UCSR0A & _BV(UDRE0))) {
			UDR0 = B;								// wenn Sendepuffer leer, dann direkt schreiben, ISR inaktiv
	} else {
		TXBuffer0[TXBufWr++] = B;					// Todo: Mit CPU Trap gegen Überlauf sichern
		TXBufWr &= (_BV(TXBuf0Size) - 1);			// Modulo TXBuf0Size
		UCSR0B |= _BV(UDRIE0);						// ISR(USART0_UDRE_vect) aktivieren, damit gesendet wird.
	}
}

#define RXBuf0Size 5							// Size = _BV(5) = 32 Bytes
volatile uint8_t RXBuffer0[_BV(TXBuf0Size)];
volatile uint8_t RXBufWr = 0;
volatile uint8_t RXBufRd = 0;

uint8_t UART0Receive() {
	if (RXBufRd == RXBufWr) {
		for(;;); //CPU-Trap
		return 0;
	} else {
		uint8_t Returnvalue;
		UCSR0B &= ~_BV(RXCIE0);
		Returnvalue = RXBuffer0[RXBufRd++];
		RXBufRd &= (_BV(RXBuf0Size) - 1);
		UCSR0B |= _BV(RXCIE0);
		return Returnvalue;
	}
}
uint8_t UART0ReceiveCount() {
	uint8_t Returnvalue;
	UCSR0B &= ~_BV(RXCIE0);
	if (RXBufRd <= RXBufWr) {
		Returnvalue = RXBufWr - RXBufRd;
	} else {
		Returnvalue = RXBufWr + _BV(RXBuf0Size) - RXBufRd;
	}
	UCSR0B |= _BV(RXCIE0);
	return Returnvalue;
}

#define UART0ReceiveEmpty (RXBufRd == RXBufWr)

void UART0SendHexNibble(uint8_t B) {
	static uint8_t Character;
	Character = B & 0x0F;
	if (Character > 9) {
		Character += 0x37;
		} else {
		Character += 0x30;
	}
	UART0Transmit(Character);
}
void UART0SendHex4Nibble(uint16_t W) {
    UART0SendHexNibble(W >> 12);
    UART0SendHexNibble(W >> 8);
    UART0SendHexNibble(W >> 4);
    UART0SendHexNibble(W);
}

void UART0SendHex3Nibble(uint16_t W) {
	UART0SendHexNibble(W >> 8);
	UART0SendHexNibble(W >> 4);
	UART0SendHexNibble(W);
}
void UART0SendHexByte(uint8_t B) {
	UART0SendHexNibble(B >> 4);
	UART0SendHexNibble(B);
}
bool UART0ReceiveHexNibble(uint8_t *Nibble) {
	*Nibble = UART0Receive();
	bool NoError = true;
	if (*Nibble > 0x40) {
		*Nibble -= 0x37;
		NoError = ((*Nibble > 9) && (*Nibble < 0x10));
	} else {
		*Nibble -= 0x30;
		NoError = ((*Nibble >= 0) && (*Nibble < 0xA));
	}
	if (!NoError) {
		for(;;);
	}
	return NoError;
}
void UART0Receive(uint8_t *B, uint8_t C) {
	if (C > UART0ReceiveCount()) for(;;); //CPU-Trap
	while (C--) {
		if (!UART0ReceiveHexNibble(B++)) {
			for(;;);
		}
	}
}


int main(void) {
	// Timer 1
	// -------
	// Der 16-bit Timer/Counter1 läuft mit der höchsten Geschwindigkeit und bis zu seinem Wert von 0xFA00 = 64000 (OCR1A).
	// "No prescaling", siehe "Table 15-5 Clock Select Bit Description" (doc8161.pdf)
	// Bei 16MHz läuft der Timer also alle 4ms über, 250 Überläufe ergeben genau eine Sekunde.
	// Siehe Mode 4 in "Waveform Generation Mode Bit Description"
	// OCR1A / ISR(TIMER1_OVF_vect): 4ms-Tick
	// OCR1B / ISR(TIMER1_COMPB_vect): 400µs Takt für TX433
	
	TCCR1B = _BV(WGM12) + _BV(CS10);				// Achtung, Input Capture Noise Canceler und Input Capture Edge Select werden überschrieben
	TCCR1A = 0;										// (OCR1A) Achtung, Compare Output Mode for Channel A und B werden überschrieben
	OCR1A = 64000;
	TIMSK1 |= _BV(OCIE1A);							// Timer 1 Output Compare A Match Interrupt Enable, ISR(TIMER1_COMPA_vect)
	sei();

	// UART

    UBRR0 = 51;										// UASART 19200Bd
	//UBRR0 = 8;										// UASART 115200Bd: U2Xn = 0, UBRR=8, 16 Samples, 3,5% Error (see "Examples of Baud Rate Setting").
	UCSR0A = _BV(UDRE0);							// Flags löschen (evt. gar nicht nötig)
	UCSR0B = _BV(RXEN0) + _BV(TXEN0);
	UCSR0C = _BV(UCSZ01) + _BV(UCSZ00);				// "N, 8, 1"
	UCSR0B |= _BV(RXCIE0);

	App_init();
    while(1) {
		static uint8_t LastMinute = 0;
		static uint8_t LastSecond = 0;
		// Applikations-Aufrufe
		if (Tick100Flag) {
			Tick100Flag = false;
			App_tick_100ms();
		} else if (LastSecond != SecondCounter) {
			LastSecond = SecondCounter;
			if (HAL_OnTickSecond) (*HAL_OnTickSecond)();
		} else if (LastMinute != MinuteCounter) {
			LastMinute = MinuteCounter;
			if (HAL_OnTickMinute) (HAL_OnTickMinute)();
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	volatile static uint8_t TickCounter = 0;					// 250 Ticks pro Sekunde bei 16MHz
	if (!(Tick100Counter--)) {
		Tick100Counter = 24;
		TickCounter += 1;
		// if (Tick100Flag) for(;;); // Todo: wieder aktivieren, oder Zähler statt Flag // trap CPU
		Tick100Flag = true;
	}
	
	if (TickCounter >= 10) {
		TickCounter = 0;							// Todo: Den Rest aus der ISR in die Main-Loop verlagern
		SecondCounter += 1;
		FCP_SecondDirty = true;
		if (SecondCounter >=60) {
			SecondCounter = 0;
			MinuteCounter += 1;
			FCP_DateTimeDirty = true;
			if (MinuteCounter >= 60) {
				MinuteCounter = 0;
				HourCounter += 1;
				if (HourCounter >=24) {
					HourCounter = 0;
					// Todo: Datum hochzählen
				}
			}
		}
	}
}

ISR(USART_RX_vect) {
	RXBuffer0[RXBufWr++] = UDR0;
	RXBufWr &= (_BV(RXBuf0Size) - 1);			// Modulo TXBuf0Size
}
	
ISR(USART_UDRE_vect) {
	UDR0 = TXBuffer0[TXBufRd++];
	TXBufRd &= (_BV(TXBuf0Size) - 1);			// Modulo TXBuf0Size
	if (TXBufWr == TXBufRd) {
		UCSR0B &= ~_BV(UDRIE0);					// ISR(USART0_UDRE_vect) inaktivieren, da Puffer leer
	}
}


// Zur Sicherheit Fallen in allen unbenutzten ISRs
ISR(INT0_vect) {for(;;);} //CPU Trap
ISR(INT1_vect) {for(;;);} //CPU Trap
//ISR(USART_RX_vect) {for(;;);} //CPU Trap
//ISR(USART_UDRE_vect) {for(;;);} //CPU Trap
ISR(USART_TX_vect) {for(;;);} //CPU Trap
ISR(PCINT0_vect) {for(;;);} //CPU Trap
ISR(PCINT1_vect) {for(;;);} //CPU Trap
ISR(PCINT2_vect) {for(;;);} //CPU Trap
ISR(WDT_vect) {for(;;);} //CPU Trap
ISR(TIMER2_COMPA_vect) {for(;;);} //CPU Trap
ISR(TIMER2_COMPB_vect) {for(;;);} //CPU Trap
ISR(TIMER2_OVF_vect) {for(;;);} //CPU Trap
ISR(TIMER1_CAPT_vect) {for(;;);} //CPU Trap
//ISR(TIMER1_COMPA_vect) {for(;;);} //CPU Trap
ISR(TIMER1_COMPB_vect) {for(;;);} //CPU Trap
ISR(TIMER1_OVF_vect) {for(;;);} //CPU Trap
ISR(TIMER0_COMPA_vect) {for(;;);} //CPU Trap
ISR(TIMER0_COMPB_vect) {for(;;);} //CPU Trap
ISR(TIMER0_OVF_vect) {for(;;);} //CPU Trap
ISR(SPI_STC_vect) {for(;;);} //CPU Trap
ISR(ANALOG_COMP_vect) {for(;;);} //CPU Trap
ISR(ADC_vect) {for(;;);} //CPU Trap
ISR(EE_READY_vect) {for(;;);} //CPU Trap
ISR(TWI_vect) {for(;;);} //CPU Trap
ISR(SPM_READY_vect) {for(;;);} //CPU Trap
