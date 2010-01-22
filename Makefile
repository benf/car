all: sender.hex empfaenger.hex

sender.elf:     sender.o rfxx.o usart.o
empfaenger.elf: empfaenger.o rfxx.o sensor.o


TYPE = atmega16

CC = avr-gcc

LDFLAGS = -L /usr/x86_64-pc-linux-gnu/avr/lib
CFLAGS = -mmcu=$(TYPE) -Wall -Os

SOURCES := $(wildcard *.c)

.PHONY: all

%.o: 
	$(CC) -mmcu=$(TYPE) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

%.elf:
	$(CC) -mmcu=$(TYPE) $(CFLAGS) $(LDFLAGS) -o $@ $+

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

clean:
	rm -f *.o *.elf
distclean: clean
	rm -f *.d *.hex

ifneq ($(MAKECMDGOALS),clean)
include $(SOURCES:.c=.d)
endif


%.d: %.c
	$(CC) -M $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@;

