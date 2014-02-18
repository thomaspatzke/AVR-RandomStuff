/*
  cmd.c: Command line interpreter.
 */

#define INBUF_SIZE 128
#define OUTBUF_SIZE 256

#define PROMPT "> "

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/setbaud.h>
#include <util/atomic.h>


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


char inbuf[INBUF_SIZE];		/* input buffer - filled until return is pressed. */
char outbuf[OUTBUF_SIZE];	/* output ring buffer */
uint16_t inpos = 0;		/* current input buffer position */
uint16_t outpos = 0;		/* current output buffer end */
uint16_t outcur = 0; /* character currently printed while output phase */
uint16_t output = 0; /* output in progress */
uint8_t cmd = 0;     /* 1 = command ready for parsing (on return) */


void led_init(void) {
  LEDDDR = (1 << LEDDDRN);		/* PIN E6 (Led) = Output */
  LEDPORT &= ~(1 << LEDPORTN);	/* disable LED */
}


void led_on(void) {
  LEDPORT = (1 << LEDPORTN);	/* LED on */
}


void led_off(void) {
  LEDPORT &= ~(1 << LEDPORTN);	/* LED off */
}


void sleep(void) {
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_mode();
}


/* Enables USART Data Register Empty Interrupt */
void start_output(void) {
  UCSRB |= (1 << UDRIE);
}


/* Disables USART Data Register Empty Interrupt */
void stop_output(void) {
  UCSRB &= ~(1 << UDRIE);
}


/* return if output is in progress */
uint8_t output_in_progress(void) {
  return UCSRB & (1 << UDRIE);
}


/* put character in buffer without starting output */
void print_char_buf(char c) {
  cli();
  while (output_in_progress() && outcur == outpos) { /* wait until space gets available if output buffer is full */
    sei();
    sleep();
    cli();
  }
  sei();
  outbuf[outpos] = c;
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    outpos++;
    if (outpos >= OUTBUF_SIZE)
      outpos = 0;
  }
}


/* put character in buffer and start output */
void print_char(char c) {
  print_char_buf(c);
  start_output();
}


/* put a whole null terminated string in the output buffer and start output */
void print_str(char *str) {
  char *c = str;
  while (*c != 0) {
    print_char_buf(*c);
    c++;
  }
  start_output();
}


/* Put recieved byte in input buffer */
ISR(INT_USART_RX) {
  char c = UDR;

  if (inpos < INBUF_SIZE) {	/* Ignore char if buffer full (TODO: implement warning) */
    inbuf[inpos++] = c;
    print_char(c);
  } else {
    led_on();
  }
  if (c == '\n' || c == '\r') {
    cmd = 1;
    print_str("\r\n");
  }
}


/* Send next byte from output buffer to USART */
ISR(INT_USART_TXR) {
  char c = outbuf[outcur++];
  UDR = c;
  if (outcur >= OUTBUF_SIZE)
    outcur = 0;
  if (outcur == outpos)		/* output finished - no interrupts needed */
    stop_output();
}


void init_uart(void) {
  UCSRA = UCSRA_VALUE;
  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
  UCSRC = UCSRC_VALUE; /* 8N1 */
  UCSRB = UCSRB_VALUE; /* TX+RX+RX Interrupt */
}


/* Parse and execute command. Characters received while this are ignored. */
void execute_command(void) {
  uint16_t endpos;
  uint16_t i = 1;
  char conv[10];

  ATOMIC_BLOCK(ATOMIC_FORCEON)
    endpos = inpos;

  switch (inbuf[0]) {
  case '\n':				/* no command - exit silently */
  case '\r':
    break;
  case 'L':
    led_on();
    break;
  case 'l':
    led_off();
    break;
  case 'e':				/* echo input */
    while (i < endpos && i < INBUF_SIZE && inbuf[i] == ' ') /* ignore spaces after command */
      i++;
    while (i < endpos && i < INBUF_SIZE) /* print input characters */
      print_char(inbuf[i++]);
    print_str("\r\n");
    break;
  case 's':
    print_str("outpos=");
    utoa(outpos, &conv, 10);
    print_str(conv);
    print_str("\r\n");
    break;
  case 'h':
    print_str("Tiny command shell for AVRs. Commands:\r\nL: led on\r\nl: led off\r\ne <msg>: echo msg\r\ns: state\r\n");
  default:			/* unknown command - print error */
    print_str("Unknown command! h helps.\r\n");
    break;
  }

  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    inpos = 0;
    cmd = 0;
  }
  print_str(PROMPT);
}


int main(void) {
  led_init();
  init_uart();
  sei();

  print_str(PROMPT);
  while (1) {
    sleep();
    if (cmd) {			/* command available? */
      execute_command();
    }
  }
}
