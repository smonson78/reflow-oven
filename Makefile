MCPU=atmega328p
PROGTYPE=stk500v2
PROGPORT=/dev/ttyACM0
LFUSE=0x62
HFUSE=0xDB

AVRDUDE=avrdude -p $(MCPU) -c $(PROGTYPE) -P $(PROGPORT) -B 20 -i 20

CC=avr-gcc
CFLAGS=-g -Wall -Os -mmcu=$(MCPU) -DF_CPU=16000000
#LDLIBS=-lgcc
HOSTCC=gcc

TARGET=toaster

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

$(TARGET).eeprom: $(TARGET).elf
	avr-objcopy -j .eeprom -O ihex $^ $@

$(TARGET).elf: main.o ssd1306.o spi.o font.o font8.o clock.o max6675.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.tif binalign
	convert -rotate 90 $< gray:$<.bin
	./binalign $<.bin $<.raw
	$(RM) $<.bin
	avr-objcopy -I binary -O elf32-avr --rename-section .data=.text $<.raw $@

# Util for padding binary objects
binalign: binalign.c
	$(HOSTCC) -o $@ $^

flash: $(TARGET).hex
	$(AVRDUDE) -U flash:w:$^:i

eeprom: $(TARGET).eeprom
	$(AVRDUDE) -U eeprom:w:$^:i

fuses:
	$(AVRDUDE) -U lfuse:w:$(LFUSE):m
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m

clean:
	$(RM) *.o *.elf *.hex
