/*
uart_send_hello.c: Hello, World!
 */

#define BUF_SIZE 0xff

#include <avr/io.h>
#include <util/setbaud.h>
#include <util/delay.h>
#include <string.h>

#if defined (__AVR_ATmega32U4__)
#define LEDPORT PORTE
#define LEDPORTN PORTE6
#define LEDPIN PINE
#define LEDPINN PINE6
#define LEDDDR DDRE
#define LEDDDRN  DDE6
#define UCSRA UCSR1A
#define UCSRA_VALUE (USE_2X << U2X1)
#define UCSRB UCSR1B
#define UCSRB_VALUE (1 << TXEN1)
#define UCSRC UCSR1C
#define UCSRC_VALUE (1 << UCSZ11) | (1 << UCSZ10)
#define UBRRH UBRR1H
#define UBRRL UBRR1L
#define UDRE UDRE1
#define UDR UDR1
#elif defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__)
#define LEDPORT PORTC
#define LEDPORTN PORTC5
#define LEDPIN PINC
#define LEDPINN PINC5
#define LEDDDR DDRC
#define LEDDDRN  DDC5
#define UCSRA UCSR0A
#define UCSRA_VALUE (USE_2X << U2X0)
#define UCSRB UCSR0B
#define UCSRB_VALUE (1 << TXEN0)
#define UCSRC UCSR0C
#define UCSRC_VALUE (1 << UCSZ01) | (1 << UCSZ00)
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UDRE UDRE0
#define UDR UDR0
#else  /* ATmega8 */
#define LEDPORT PORTC
#define LEDPORTN PORTC5
#define LEDPIN PINC
#define LEDPINN PINC5
#define LEDDDR DDRC
#define LEDDDRN  DDC5
#define UCSRA_VALUE (USE_2X << U2X)
#define UCSRB_VALUE (1 << TXEN)
#define UCSRC_VALUE (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0)
#endif

char msg[] = "Hello, World!\r\n";
int i = 0;
int l;


void led_on(void) {
  LEDPORT = (1 << LEDPORTN);	/* LED on */
}


void led_off(void) {
  LEDPORT &= ~(1 << LEDPORTN);	/* LED off */
}


void toggle_led(void) {
  if (LEDPIN & (1 << LEDPINN)) {
    led_off();
  } else {
    led_on();
  }
}


int main(void) {
  l = strlen(msg);
  LEDDDR = (1 << LEDDDRN);		/* LED Output */
  LEDPORT = (1 << LEDPORTN);		/* LED off */
  UCSRA = UCSRA_VALUE;
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRB = UCSRB_VALUE;
  UCSRC = UCSRC_VALUE;

  while (1) {
    loop_until_bit_is_set(UCSRA, UDRE);
    toggle_led();
    //_delay_ms(100);
    UDR = msg[i++];
    if (i >= l)
      i = 0;
  }
}
