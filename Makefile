.PHONY: all

C_SOURCES = main.c
ASM_SOURCES = crt0.s65 zeropage.s65

all: fram.bin

imgs.c: *.png
	go run images_c.go $^ > $@

main.o: main.c imgs.c
	cc65 --cpu 6502 -O -t none -o $(@:.o=.s) $<
	ca65 --cpu 6502 -o $@ -l $(@:.o=.lst) $(<:.c=.s)

%.o: %.s65
	ca65 --cpu 6502 -o $@ -l $(@:.o=.lst) $<

fram.bin: $(ASM_SOURCES:.s65=.o) $(C_SOURCES:.c=.o)
	cl65 -C fram.cfg -m fram.map -o $@ $^ dodo.lib

clean:
	rm -f fram.bin *.s *.o *.lst *.map imgs.[ch]
