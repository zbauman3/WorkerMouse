INPUT_FILE=main
DEVICE=attiny85
# Set to usbtiny ISP. Set this to whatever programmer you're using.
AVRDUDE = avrdude -c usbtiny -p $(DEVICE)
COMPILE = avr-gcc -Wall -Os -Iusbdrv -I. -mmcu=$(DEVICE) -DF_CPU=16000000 -DDEBUG_LEVEL=0
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o $(INPUT_FILE).o

all:	$(INPUT_FILE).hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:$(INPUT_FILE).hex

fuse:
	$(AVRDUDE) -U hfuse:w:0xdd:m -U lfuse:w:0b11111111:m

clean:
	rm -f $(INPUT_FILE).hex $(INPUT_FILE).lst $(INPUT_FILE).obj $(INPUT_FILE).cof $(INPUT_FILE).list $(INPUT_FILE).map $(INPUT_FILE).eep.hex $(INPUT_FILE).bin *.o usbdrv/*.o $(INPUT_FILE).s usbdrv/oddebug.s usbdrv/usbdrv.s

$(INPUT_FILE).bin:	$(OBJECTS)
	$(COMPILE) -o $(INPUT_FILE).bin $(OBJECTS)

$(INPUT_FILE).hex:	$(INPUT_FILE).bin
	rm -f $(INPUT_FILE).hex $(INPUT_FILE).eep.hex
	avr-objcopy -j .text -j .data -O ihex $(INPUT_FILE).bin $(INPUT_FILE).hex
	./checksize $(INPUT_FILE).bin 8192 512
