#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>

#define STATE ((PINB >> PB4) & 1)
#define NOP()

volatile uint8_t tick;
volatile uint8_t tock;
volatile uint8_t flag;
volatile uint32_t controller;
volatile char nop;
unsigned char data;
uint8_t i;
uint8_t last;


void timer0_init() {
	// timer/counter control register A, used to set CTC (see pg. 79)
	TCCR0A |= (1 << WGM01);
	// set prescale 
	TCCR0B |= (1 << CS01);	
	// initialize timer to 0
	TCNT0 = 0;
	// output compare register
	//  1 us per tick
	OCR0A = 8;	
	// enable compare interrupt:
	TIMSK |= (1 << OCIE0A);
	// initialize tick
	tick = 0;
	// enable global interrupts
	sei();

}	

void timer1_init() {
	// enable CTC interrups		
	TCCR1 |= (1 << CTC1);
	//set prescale: CK / 8 should be 1 megaHz
	TCCR1 |= (1 << CS12);
	// in us
	OCR1A = 128; 
	// enable intterups
	TIMSK |= (1 << OCIE1A);
}

void pwm_init() {
	// enable compare output mode	
	GTCCR |= ((1 << COM1B0)|(1 << COM1B1));
	// had to enable this due to bug 27.2.3 article 4 (pg. 213)
	TCCR1 |= ((1 << COM1A0) | (1 << COM1A1));
	// enable pulse width modulator
	GTCCR |= (1 << PWM1B);
	// set prescale
	TCCR1 |= (1 << CS10);
	TCNT1 = 0;
	// enable super speed clock (64MHz)
	PLLCSR = (1 << PCKE);
	OCR1B = 64;   // duty cycle
	OCR1C = 255; // period of waveform
}	

void pwm_disable() {
	// enable compare output mode	
	GTCCR &= ~((1 << COM1B0)|(1 << COM1B1));
	// enable pulse width modulator
	GTCCR &= ~(1 << PWM1B);
}

void pwm_enable() {
	// enable compare output mode	
	GTCCR |= ((1 << COM1B0)|(1 << COM1B1));
	// enable pulse width modulator
	GTCCR |= (1 << PWM1B);
}
	
	

void pin_interrupt_init() {
	// enable interrupt on PC4	
	PCMSK |= (1 << PCINT4);
	// rising edge interrupt
	MCUCR |= (1 | ISC01) | (1 | ISC00);
}

void pin_interrupt_disable() {
	PCMSK &= ~(1 << PCINT4);
}


void spi_init() {
	// enable data output
	USICR |= (1 << USIWM0);	
}

void spi_transfer(unsigned char data) {
	USIDR = data;
	while (!(USIDR & (1 << USISIF)));
		
}


// pin interrupt
ISR(INT0_vect) {
	i += 1;
	controller |= (flag & 1) << i;
}

// when timer 1 goes off, 128us have passed since polling
ISR(TIMER1_COMPA_vect) {
	// let's relax for 32 seconds
	OCR0A = 32;
	// move into next phase of timer0 interrups
	tick = 2;
	// disable pin interrupts...
	pin_interrupt_disable();
}
	
ISR(TIMER0_COMPA_vect) {
	// time to start reading
	if (tick == 0) {
		// free up timer 1
		pwm_disable();
		OCR1A = 128;
		// set interrupt to halt these short interrupts
		timer1_init();
		tick = 1;
		OCR0A = 4;
	}
	else if (tick == 1) {
		// time to start reading
		OCR0A = 4;
		pin_interrupt_init();
	}
	// short poll times
	else if (tick == 1) {
		flag ^= 1;
	}
	//time to poll 	
	else if (tick == 2) {
		// set 25% duty cycle 
		OCR1B = 191;
		// switch on PWM
		pwm_enable();
		// interrupt in 32 us (adjust depending on other instructions)
		OCR0A = 28;
		tick = 2;
	}
	else {
		// send the one signal
		// interrupt again in 8 us
		// change the duty cycle to 75%
		tick = 0;
		OCR1B = 64;
		OCR0A = 10;
		pin_interrupt_init();	
	}	
}

int main(void) {
	timer0_init();
	pwm_init();
	spi_init();
	// set PB4 to output and PB1 (MISO) 
	DDRB = (1 << DDB4) | (1 << DDB1) | (1 << DDB3);
	// set PB4 high;
	while(1) {
		;
	}
}


