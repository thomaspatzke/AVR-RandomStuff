#F_CPU=1000000
F_CPU=16000000
#F_CPU=20000000
#BAUD=9600
BAUD=115200
MCU=atmega8
#MCU=atmega328p
#MCU=atmega32u4
PMCU=m8
#PMCU=m328p
#PMCU=m32u4
PROGTYPE=buspirate
#PROGTYPE=stk500v2
#PROGTYPE=avr109
CFLAGS=-Os -Wall -fpack-struct -fshort-enums -std=c99
PROGDEV=/dev/ttyUSB0
#PROGDEV=/dev/ttyACM3

%: %.hex

%.hex: %.c
	avr-gcc -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) $(CFLAGS) -o $*.elf $<
	avr-objcopy -O ihex -R .eeprom $*.elf $@

.PHONY: clean load_* m8_intosc m8_extosc
clean:
	rm -rf *.elf *.hex

load-%: %.hex
	avrdude -p $(PMCU) -P $(PROGDEV) -c $(PROGTYPE) -U flash:w:$<

m8_intosc:
	echo "write lfuse 0 0xe1" | avrdude -p $(PMCU) -P $(PROGDEV) -c $(PROGTYPE) -t

m8_extosc:
	echo "write lfuse 0 0xff" | avrdude -p $(PMCU) -P $(PROGDEV) -c $(PROGTYPE) -t
