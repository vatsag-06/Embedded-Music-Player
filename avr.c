/**
 * avr.c
 * Copyright (C) 2001-2020, Tony Givargis
 */

#include "avr.h"



void avr_wait_us(unsigned short usec)
{
	TCCR0 = (1 << CS00); // No prescaler, 1 tick = 0.125 µs
	while (usec--) {
		TCNT0 = 256 - 8; // 8 ticks = 1 µs at 8 MHz
		SET_BIT(TIFR, TOV0); // Clear overflow flag
		while (!GET_BIT(TIFR, TOV0)); // Wait for overflow
	}
	TCCR0 = 0; // Stop timer
}

void avr_wait(unsigned short usec) {
	TCCR0 = 2;  // Prescaler = 8 (1 µs per tick)
	while (usec--) {
		TCNT0 = (unsigned char) (256 - (XTAL_FRQ/8) * 0.000005);  // 256 - 125 = 131
		SET_BIT(TIFR, TOV0);  // Clear overflow flag
		WDR();
		while (!GET_BIT(TIFR, TOV0));  // Wait 1 ms
	}
	TCCR0 = 0;
}