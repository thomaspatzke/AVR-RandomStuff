/*
uart_echo.c: Read data from serial port and echo on \n
 */

#define BUF_SIZE 0xff

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/setbaud.h>
#include <util/delay.h>

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
#define UCSRB_VALUE (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1)
#define UCSRC UCSR1C
#define UCSRC_VALUE (1 << UCSZ11) | (1 << UCSZ10)
#define UBRRH UBRR1H
#define UBRRL UBRR1L
#define UDRE UDRE1
#define UDR UDR1
#define UDRIE UDRIE1
#define INT_USART_RX USART1_RX_vect
#define INT_USART_TXR USART1_UDRE_vect
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
#define UCSRB_VALUE (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0)
#define UCSRC UCSR0C
#define UCSRC_VALUE (1 << UCSZ01) | (1 << UCSZ00)
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UDRE UDRE0
#define UDR UDR0
#define UDRIE UDRIE0
#define INT_USART_RX USART0_RX_vect
#define INT_USART_TXR USART0_UDRE_vect
#else  /* ATmega8 */
#define LEDPORT PORTC
#define LEDPORTN PORTC5
#define LEDPIN PINC
#define LEDPINN PINC5
#define LEDDDR DDRC
#define LEDDDRN  DDC5
#define UCSRA_VALUE (USE_2X << U2X)
#define UCSRB_VALUE (1 << RXEN) | (1 << TXEN) | (1 << RXCIE)
#define UCSRC_VALUE (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0)
#define INT_USART_RX USART_RXC_vect
#define INT_USART_TXR USART_UDRE_vect
#endif

char buf[BUF_SIZE] ;
int r = 0;			/* received pointer */
int s = 0;			/* sent pointer */
int txoff = 0;

void tx_start(void) {
  UCSRB |= (1 << UDRIE);
  LEDPORT = (1 << LEDPORTN);	/* LED on */
}


void tx_stop(void) {
  UCSRB &= ~(1 << UDRIE);
  LEDPORT &= ~(1 << LEDPORTN);	/* LED off */
}


ISR(INT_USART_RX) {
  char c = UDR;
  buf[r++] = c;
  if (r >= BUF_SIZE)
    r = 0;
  if (c == '\n' || c == '\r') {
    buf[r++] = c == '\n' ? '\r' : '\n';
    tx_start();
  }
  if (r >= BUF_SIZE)
    r = 0;
}


ISR(INT_USART_TXR) {
  char c = buf[s++];
  UDR = c;
  if (txoff || c == '\n' || c == '\r') {
    if (txoff) {
      tx_stop();
      txoff = 0;
    } else {
      txoff = 1;
    }
  }
  if (s >= BUF_SIZE)
    r = 0;
}


int main(void) {
  LEDDDR = (1 << LEDDDRN);		/* PIN E6 (Led) = Output */
  LEDPORT &= ~(1 << LEDPORTN);	/* disable LED */
  UCSRA = (USE_2X << U2X);
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRB = UCSRB_VALUE; /* TX+RX+RX Interrupt */
  UCSRC = UCSRC_VALUE; /* 8N1 */
  sei();

  while (1) {
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_mode();
  }
}
