/*
 * main.c
 *
 * Created: 5/1/2025 5:41:27 PM
 *  Author: Vatsa Gupta
 */ 


#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>     // for dtostrf
#include "avr.h"
#include "lcd.h"

#define STATS_BTN_PIN  PB3
#define STATS_BTN_PRESSED() (!(PINB & (1<<STATS_BTN_PIN)))

// Perform one 10-bit ADC conversion on channel 0 (PA0/ADC0) using AVCC as reference


// main.c
// Simple 16×2 Volt-Meter on ATmega32 (no avr.h dependency)

#define F_CPU 8000000UL  // CPU frequency

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h> 
#include "lcd.h"







#define MAX_TOKENS  16
static int dec = 0;

int is_pressed(int r, int c) {
	DDRC = 0;
	PORTC = 0;

	DDRC |= (1 << r);
	PORTC &= ~(1 << r);

	DDRC &= ~(1 << (c + 4));
	PORTC |= (1 << (c + 4));
	_delay_us(5);  // settle time (important)

	if ((PINC & (1 << (c + 4))) == 0) {
		return 1;
	}

	return 0;
}

int get_key() {
	int i, j;
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			if (is_pressed(i, j)) {
				return i * 4 + j + 1;
			}
		}
	}
	return 0;
}

int decode_key(int key) {
	// A static flag to remember how many times (even/odd) key 16 was pressed.
	
	int dec = 2;
	switch (key) {
		case 1:   return '1';
		case 2:   return '2';
		case 3:   return '3';
		case 4:   return '+';  // Key “A” ? plus

		case 5:   return '4';
		case 6:   return '5';
		case 7:   return '6';
		case 8:   return '-';  // Key “B” ? minus

		case 9:   return '7';
		case 10:  return '8';
		case 11:  return '9';
		case 12:  return '*';  // Key “C” ? multiply

		case 13:  return '.';  // Key “*” ? Clear
		case 14:  return '0';
		case 15:  return 'X';  // Key “#” ? Equals

		case 16:
		return '/';
		default:
		return -1;  // no valid key pressed
	}
}
double evaluate(const char *expr) {
	double numbers[MAX_TOKENS];
	char   ops[MAX_TOKENS];
	int    num_count = 0;
	int    op_count  = 0;
	int    i = 0;
	while (expr[i] != '\0') {
		double sign = 1.0;
		if (expr[i] == '-') {
			if (i == 0 || strchr("+-*/", expr[i - 1]) != NULL) {
			sign = -1.0;
			i++;
			}
			}

			double value = 0.0;
			while (expr[i] >= '0' && expr[i] <= '9') {
			value = (value * 10.0) + (expr[i] - '0');
			i++;
			}
			if (expr[i] == '.') {
			i++;
			double place = 0.1;
			while (expr[i] >= '0' && expr[i] <= '9') {
			value += (expr[i] - '0') * place;
			place *= 0.1;
			i++;
			}
			}

			value *= sign;
			numbers[num_count++] = value;

			if (expr[i] == '\0') {
			break;
			}

			ops[op_count++] = expr[i++];
			}

			for (i = 0; i < op_count; ) {
			if (ops[i] == '*' || ops[i] == '/') {
			double lhs = numbers[i];
			double rhs = numbers[i + 1];
			double res;

			if (ops[i] == '*') {
			res = lhs * rhs;
			} else {
			res = lhs / rhs;
			}
			numbers[i] = res;
			
			for (int k = i + 1; k < num_count - 1; k++) {
			numbers[k] = numbers[k + 1];
			}
			num_count--;


			for (int k = i; k < op_count - 1; k++) {
			ops[k] = ops[k + 1];
			}
			op_count--;

			} else {
			i++;
			}
			}

			double result = numbers[0];
			for (i = 0; i < op_count; i++) {
			if (ops[i] == '+') {
			result += numbers[i + 1];
			} else {  
			result -= numbers[i + 1];
			}
			}

			return result;
			}

void trim_trailing_zeros(char *s) {
	char *dot = strchr(s, '.');
	if (!dot) return; 

	char *end = s + strlen(s) - 1;
	while (end > dot && *end == '0') {
		*end-- = '\0';
	}
	if (end == dot) {
		*end = '\0';
	}
}

int main(void) {
	char expr[MAX_TOKENS + 1];    
	int pos = 0;                  
	double pre;
	double result;
	int keynum, ch;
	char result_str[12];          
	lcd_init();
	lcd_clr();
	lcd_pos(0, 0);
	while (1) {
		keynum = get_key();
		ch = decode_key(keynum);
		if (ch == 'X'){
			lcd_clr();
			expr[0] = '\0';
			pos = 0;
		}
		if ((ch >= '0' && ch <= '9') ||
		ch == '+' || ch == '-' ||
		ch == '*' || ch == '/' || ch == '.') {			// If there's room, append and echo
			if (pos < MAX_TOKENS) {
				expr[pos++] = (char)ch;
				expr[pos] = '\0';
				lcd_pos(0, pos - 1);
				lcd_put((char)ch);
			}
			avr_wait(1000000000);
			if (ch >= '0' && ch <= '9'){
				result = evaluate(expr);
				dtostrf(result, 8, 3, result_str);
				lcd_pos(1,0);
				trim_trailing_zeros(result_str);
				lcd_puts2(result_str);
			}
			else{
				pre = result;
				dtostrf(pre, 8, 3, result_str);
				lcd_pos(1,0);
				trim_trailing_zeros(result_str);
				lcd_puts2(result_str);
			}
		}
	}

	return 0;
}			
				
/*void wait_us(float usec) {
	while (usec > 1.0) {
		_delay_us(1);
		usec -= 1.0;
	}
}

typedef enum {
	A, As, B, C, Cs, D, Ds, Ee, F, Fs, G,
	Gs
} Note;

typedef enum {
	W, H, Q, Ei
} Duration;

typedef struct {
	Note note;
	Duration duration;
} PlayingNote;

PlayingNote jingleBells[] = {
	{Ee, Q}, {Ee, Q}, {Ee, H},
	{Ee, Q}, {Ee, Q}, {Ee, H},
	{Ee, Q}, {G, Q}, {C, Q}, {D, Q}, {Ee, H},
	{F, Q}, {F, Q}, {F, Q}, {F, Q}, {F, Q}, {Ee, Q}, {Ee, Q}, {Ee, Q}, {Ee, Q},
	{Ee, Q}, {D, Q}, {D, Q}, {Ee, Q}, {D, H}, {G, H}
};

float get_frequency(Note note) {
	// A4 = 440 Hz, and in your enum A = 0
	// So semitone offset = (note - A) + 12 × (octave - 4)
	return 220.0 * pow(2.0, note / 12.0);
}

float get_duration(Duration d){
	switch(d){
		case(W): return 2;
		case(H): return  1;
		case(Q): return 0.5;
		case(Ei): return 0.25;
		default: return 0.5;
	}
}

void play_note(const PlayingNote* note) {
	float freq = get_frequency(note->note);
	float duration_ms = get_duration(note->duration) * 100000.0;
	float period_ms = 100000.0 / freq;
	int delay_ms = (int)(period_ms / 2.0);
	int cycles = (int)(duration_ms / period_ms);

	for (int i = 0; i < cycles; i++) {
		SET_BIT(PORTB, 3);
		avr_wait(delay_ms);
		CLR_BIT(PORTB, 3);
		avr_wait(delay_ms);
	}
}

void play_song(const PlayingNote song[], int length) {
	int i;
	for (i = 0; i < length; i++) {
		if (get_key() == 4) {
			break;  // Stop immediately on key 4
		}
		play_note(&song[i]);
	}
}

int main() {
	DDRB |= (1 << PB3);  // Speaker pin as output (PB3)
	lcd_init();
	int playing = 0;     // 0 = stopped, 1 = playing

	while (1) {
		if (playing){
			play_song(jingleBells, sizeof(jingleBells)/ sizeof(jingleBells[0]));
		}
		char buf[20];

		int k = get_key();
		if (k == 3) {  
			sprintf(buf, "Jingle Bells");         // Toggle play/stop on key 13
			lcd_puts2(buf);
			play_song(jingleBells, sizeof(jingleBells)/ sizeof(jingleBells[0]));
			playing = 1;
		}
		else if (k == 4){
			playing = 0;
			CLR_BIT(PORTB, 3);
		}


	}
}



*/

















































/*
//==================== STRUCT ====================
typedef struct {
	int year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
} DateTime;

//==================== INIT ====================
void init_dt(DateTime *dt) {
	dt->year = 2022;
	dt->month = 1;
	dt->day = 1;
	dt->hour = 0;
	dt->minute = 0;
	dt->second = 0;
}

int is_leap_year(int year) {
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int is_valid_datetime(const DateTime *dt) {
	const unsigned char days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if (dt->month < 1 || dt->month > 12) return 0;
	if (dt->hour > 23 || dt->minute > 59 || dt->second > 59) return 0;

	unsigned char max_day = days_in_month[dt->month - 1];
	if (dt->month == 2 && is_leap_year(dt->year)) max_day = 29;

	if (dt->day < 1 || dt->day > max_day) return 0;
	return 1;
}

void advance_dt(DateTime *dt) {
	++dt->second;
	if (dt->second == 60) {
		dt->second = 0;
		++dt->minute;
		if (dt->minute == 60) {
			dt->minute = 0;
			++dt->hour;
			if (dt->hour == 24) {
				dt->hour = 0;
				++dt->day;

				const unsigned char days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
				unsigned char max_day = days_in_month[dt->month - 1];
				if (dt->month == 2 && is_leap_year(dt->year)) max_day = 29;

				if (dt->day > max_day) {
					dt->day = 1;
					++dt->month;
					if (dt->month > 12) {
						dt->month = 1;
						++dt->year;
					}
				}
			}
		}
	}
}

void print_dt(const DateTime *dt) {
	char buf[17];
	lcd_pos(0, 0);
	sprintf(buf, "%02d-%02d-%04d", dt->month, dt->day, dt->year);
	lcd_puts2(buf);
	lcd_pos(1, 0);
	sprintf(buf, "%02d:%02d:%02d", dt->hour, dt->minute, dt->second);
	lcd_puts2(buf);
}

int decode_key(int key) {
	switch (key) {
		case 1: return 1;
		case 2: return 2;
		case 3: return 3;
		case 5: return 4;
		case 6: return 5;
		case 7: return 6;
		case 9: return 7;
		case 10: return 8;
		case 11: return 9;
		case 14: return 0;
		default: return -1;
	}
}

void get_digits_from_keypad(char *buf, int digits, const char *label) {
	int count = 0;
	lcd_clr();
	lcd_pos(0, 0);
	lcd_puts2(label);

	while (count < digits) {
		int k = get_key();
		int digit = decode_key(k);
		if (digit != -1) {
			buf[count++] = '0' + digit;
			lcd_pos(1, 0);
			for (int i = 0; i < count; ++i)
			lcd_put(buf[i]);
		}
		_delay_ms(250);
	}
	buf[digits] = '\0';
}



				void set_full_datetime(DateTime *dt) {
	char buf[5];

	get_digits_from_keypad(buf, 2, "Set Month:");
	dt->month = atoi(buf);

	get_digits_from_keypad(buf, 2, "Set Date:");
	dt->day = atoi(buf);

	get_digits_from_keypad(buf, 4, "Set Year:");
	dt->year = atoi(buf);

	get_digits_from_keypad(buf, 2, "Set Hour:");
	dt->hour = atoi(buf);

	get_digits_from_keypad(buf, 2, "Set Minute:");
	dt->minute = atoi(buf);

	get_digits_from_keypad(buf, 2, "Set Second:");
	dt->second = atoi(buf);
}

int main() {
	DateTime dt;
	lcd_init();
	init_dt(&dt);
	print_dt(&dt);

	while (1) {
		int k = get_key();

		if (k == 13) {  
			set_full_datetime(&dt);
			if (!is_valid_datetime(&dt)) {
				lcd_clr();
				lcd_puts2("Invalid!");
				_delay_ms(2000);
				init_dt(&dt);
				continue;
			}
			print_dt(&dt);
		}

		avr_wait(1000);
		advance_dt(&dt);
		print_dt(&dt);
	}
	return 0;
}*/




// Perform one 10-bit ADC conversion on channel 0 (PA0/ADC0) using AVCC as reference


/*int get_sample() {
	
	
	ADMUX  = (1 << REFS0);
	// Enable ADC, start conversion, prescaler = 128
	ADCSRA = (1 << ADEN) | (1 << ADSC)
	| (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	// Wait for conversion to complete
	while (!(ADCSRA & (1 << ADIF)));
	// Clear conversion complete flag
	ADCSRA |= (1 << ADIF);
	// Read result (must read ADCL first)
	uint16_t result = ADCL;
	result |= (ADCH << 8);
	return result;
}

int main(void) {
	char s_inst[6], s_max[6], s_min[6], s_avg[6];
	DDRB  &= ~(1 << STATS_BTN_PIN);
	PORTB |=  (1 << STATS_BTN_PIN);


	// Initialize LCD (4-bit mode)
	lcd_init();

	// Initialize ADC: nothing else needed, configured in get_sample

	// Statistics variables
	int max_val = 0;
	int min_val = 1023;
	unsigned long long sum = 0;
	unsigned int count = 0;
	bool sampling = true;

	while (1) {
		if (STATS_BTN_PRESSED()){
			sampling = !sampling;
			max_val = 0;
			min_val = 1023;
			sum = 0;
			count = 0;
		}
		// Read ADC sample
		int sample = get_sample();
		// Update statistics
		if (sample > max_val) max_val = sample;
		if (sample < min_val) min_val = sample;
		sum += sample;
		count++;

		// Convert raw ADC counts to voltages
		float inst_v = sample * 5.0f / 1023.0f;
		float max_v  = max_val * 5.0f / 1023.0f;
		float min_v  = min_val * 5.0f / 1023.0f;
		float avg_v  = (sum * 5.0f) / (1023.0f * count);
		dtostrf(inst_v,  0, 2, s_inst);   // buf == "123.457"
		// Display on 16x2 LCD
		lcd_clr();
		lcd_pos(0, 0);
		lcd_puts2("I:");
		if (!sampling){
			sprintf(s_max,  "--");
			sprintf(s_min, "--");
			sprintf(s_avg , "--");
		}
		else{
			dtostrf(avg_v,  0, 2, s_avg);   // buf == "123.457"
			dtostrf(min_v,  0, 2, s_min);   // buf == "123.457"
			dtostrf(max_v,  0, 2, s_max);   // buf == "123.457"
		}
		lcd_puts2(s_inst);
		lcd_puts2("V");
		lcd_puts2(" Mn:");
		lcd_puts2(s_min);
		lcd_puts2("V");
		lcd_pos(1, 0);
		lcd_puts2("A:");
		lcd_puts2(s_avg);
		lcd_puts2("V");
		lcd_puts2(" Mx:");
		lcd_puts2(s_max);
		lcd_puts2("V");
		// Delay 500 ms between samples
		_delay_ms(500);
	}
	return 0;
}
/*

*/

