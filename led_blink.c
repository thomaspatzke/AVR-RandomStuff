/*
led_blink.c: blink LED
 */

#include <avr/io.h>
#include <util/delay.h>

#if defined (__AVR_ATmega32U4__)
#define LEDPORT PORTE
#define LEDPORTN PORTE6
#define LEDPIN PINE
#define LEDPINN PINE6
#define LEDDDR DDRE
#define LEDDDRN  DDE6
#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#define LEDPORT PORTC
#define LEDPORTN PORTC5
#define LEDPIN PINC
#define LEDPINN PINC5
#define LEDDDR DDRC
#define LEDDDRN  DDC5
#else
#define LEDPORT PORTC
#define LEDPORTN PORTC5
#define LEDPIN PINC
#define LEDPINN PINC5
#define LEDDDR DDRC
#define LEDDDRN  DDC5
#endif

int main(void) {
  LEDDDR = (1 << LEDDDRN);	/* PIN C5 (Led) = Output */
  LEDPORT = (1 << LEDPORTN);	/* turn LED on */

  while (1) {
    _delay_ms(500);
    if (LEDPIN & (1 << LEDPINN)) {
      LEDPORT &= ~(1 << LEDPORTN); /* LED off */
    } else {
      LEDPORT = (1 << LEDPORTN); /* LED on */
    }
  }
}
