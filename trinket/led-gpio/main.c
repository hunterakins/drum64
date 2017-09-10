#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/io.h>


// Hunter's note: stole this code from matsi (https://github.com/geomatsi)
// Link to original https://github.com/geomatsi/avr-tests/tree/master/boards/attiny85-bare-metal/apps/led-gpio


/* timer interrupt: toggle LED on PB1 */

ISR(TIMER0_COMPA_vect)
{
	PORTB ^= (1 << PB1);
}

int main(void)
{

	/* set PB1 as output and turn on LED */

	DDRB = (1 << DDB1);
	PORTB |= (1 << PB1);

	/* configure timer */

	cli();

	// timer0 clock select: F_CLK
	TCCR0B |= (1 << CS02) | (1 << CS00);

	// timer0 CTC mode (Clear Timer on Compare match)
	TCCR0A |= (1 << WGM01);

	// F_CLK/1024 = 976.5625 if F_CLK = 1Mhz => ~1 interrupt per 100ms:
	OCR0A = 500;

	// timer0 compare interrupt
	TIMSK |= (1 << OCIE0A);

	sei();

	/* main loop */

	while(1)
	{
		_delay_ms(1000);
	}
}


