CFLAGS = -Wall -Os
LDFLAGS = -L /usr/x86_64-pc-linux-gnu/avr/lib
TYPE = atmega16

all: progs

progs: main.hex pwm_test.hex pwm_test2.hex pwm_test3.hex uart.hex chip_retter.hex takt_tester.hex rfm12_transmitter.hex rf01_receive.hex rf02_transmitter.hex sender.hex empfaenger.hex rf12_send.hex rf12_recv.hex rf02.old.hex

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@

%.elf: %.c
	avr-gcc -mmcu=$(TYPE) $(CFLAGS) $(LDFLAGS) -o $@ $<

prog:
	avrdude -p m16 -c stk200 -e -U pwm_test.hex


