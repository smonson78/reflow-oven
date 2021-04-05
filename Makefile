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

$(TARGET).elf: main.o ssd1306.o spi.o font.o font8.o clock.o max6675.o adc.o buttons.o
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

info: $(TARGET).elf
	@m68k-elf-objdump -h toaster.elf -j .text -j .data -j .bss | \
		sed -e "s/^... \.text\s*0*\([^ ]*\).*/Code size: 0x\1 of 0x8012/ p; \
		s/^... \.data\s*0*\([^ ]*\).*/Data size: 0x\1 of 0x8012/ p; \
		s/^... \.bss\s*0*\([^ ]*\).*/RAM use: 0x\1 of 0x800/ p; \
		d"

predict_test: predict_test.c predict.c predict.h
	gcc -o predict_test predict_test.c predict.c
