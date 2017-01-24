AS=as --32
ASFLAGS=
LD=ld
LDFLAGS=-n -m elf_i386
CC=gcc -m32
CFLAGS=-O0 -Wall -Wextra -ffreestanding -nostdlib -mno-sse -Ilua-5.3.3/src/ -Ilibc/ -I.

OUTPUT=boot.img
LINK=data.T
ASM_OBJS=start.o
C_OBJS=main.o vga_io.o malloc.o sprintf.o keyboard.o bios.o disk.o debug.o handler.o string.o stubs.o libc/setjmp.o libc/stdio.o lua-5.3.3/src/liblua.a

all: $(OUTPUT)

%.o: %.S
	$(AS) $+ -o $@ $(ASFLAGS)

%.o: %.c
	$(CC) -c $+ -o $@ $(CFLAGS)

%.bin: %.lo
	objcopy -O binary $+ $@

symtab.o: $(ASM_OBJS) $(C_OBJS) $(LINK)
	$(LD) -T $(LINK) $(ASM_OBJS) $(C_OBJS) -o tmp.o $(LDFLAGS)
	readelf tmp.o -x .symtab | tail +3 | sed 's/0x\(\S*\)/\1:/g' | xxd -r > symtab.bin
	readelf tmp.o -x .strtab | tail +3 | sed 's/0x\(\S*\)/\1:/g' | xxd -r > strtab.bin
	$(AS) -c -o $@ /dev/null
	objcopy --add-section=.psymtab=symtab.bin --set-section-flags=.psymtab=alloc --add-section=.pstrtab=strtab.bin --set-section-flags=.pstrtab=alloc $@
	rm -f tmp.o symtab.bin strtab.bin

mbr.lo: mbr.o
	$(LD) $+ -o $@ $(LDFLAGS) -Ttext=0x07C00

data.lo: $(ASM_OBJS) $(C_OBJS) symtab.o $(LINK)
	$(LD) -T $(LINK) $(ASM_OBJS) $(C_OBJS) symtab.o -o $@ $(LDFLAGS)

$(OUTPUT): mbr.bin data.bin
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=mbr.bin of=$@ bs=512 conv=notrunc count=1
	dd if=data.bin of=$@ bs=512 conv=sync,notrunc seek=1

clean:
	rm -f $(ASM_OBJS) $(C_OBJS) $(OUTPUT) mbr.o mbr.lo mbr.bin data.lo data.bin
