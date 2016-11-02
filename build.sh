#!/bin/sh

echo "=== C FILES ===" && \
m68k-none-elf-gcc -O3 -funroll-loops -m68000 -msoft-float -fomit-frame-pointer -c -o obj/main.o src/main.c -nostdlib && \
echo "=== ASM FILES ===" && \
m68k-none-elf-as -m68000 -o obj/afmt.o src/afmt.S && \
echo "=== LINK ===" && \
m68k-none-elf-ld -O3 --emit-relocs -T ST.ld -o output.linux-elf obj/main.o obj/afmt.o -lgcc -lc && \
echo "=== CONVERT ===" && \
python ../stelf/stelf.py output.linux-elf cheqular.prg && \
true

#m68k-none-elf-ld -O3 --emit-relocs -T ST.ld -o output.linux-elf obj/main.o obj/afmt.o -lgcc -lc && \

#m68k-none-elf-ld -flto -O3 --emit-relocs -T ST.ld -o output.linux-elf afmt.o main.o -lgcc -lc && \

#m68k-none-elf-ld -O3 --emit-relocs -T ST.ld -o output.linux-elf afmt.o main.o -lgcc -lc && \

#m68k-none-elf-ld -T ST.ld --oformat binary -o derp.prg afmt.o main.o && \

#m68k-none-elf-ld -T ST.ld -o output.linux-elf afmt.o main.o -L/usr/local/m68k-linux-elf/lib/gcc/m68k-linux-elf/3.4.6/msoft-float -lgcc && \

#echo "=== CONVERT ===" && \
#m68k-none-elf-objcopy -O binary output.linux-elf derp.prg && \
