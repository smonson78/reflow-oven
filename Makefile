MCPU=atmega328p
PROGTYPE=stk500v2
PROGPORT=/dev/ttyACM0
LFUSE=0x62
HFUSE=0xDB

AVRDUDE=avrdude -p $(MCPU) -c $(PROGTYPE) -P $(PROGPORT) -B 20 -i 20

CC=avr-gcc
CFLAGS=-g -Wall -Os -mmcu=$(MCPU) -DF_CPU=1000000
#LDLIBS=-lgcc

TARGET=lawgiver

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

$(TARGET).eeprom: $(TARGET).elf
	avr-objcopy -j .eeprom -O ihex $^ $@

$(TARGET).elf: main.o clock.o buttons.o logo.o font8.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.tif
	convert -rotate 90 $< gray:$<.raw
	avr-objcopy -I binary -O elf32-avr --rename-section .data=.text $<.raw $@

flash: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$^:i

eeprom: $(TARGET).eeprom
	$(AVRDUDE) -U eeprom:w:$^:i

fuses:
	$(AVRDUDE) -U lfuse:w:$(LFUSE):m
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m

clean:
	$(RM) *.o *.elf *.hex
