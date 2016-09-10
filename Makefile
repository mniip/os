AS=as --32
ASFLAGS=
LD=ld
LDFLAGS=-n -m elf_i386
CC=gcc -m32
CFLAGS=-O0 -Wall -Wextra -ffreestanding -nostdlib -mno-sse -Ilua-5.3.3/src/

OUTPUT=boot.img
LINK=data.T
ASM_OBJS=start.o
C_OBJS=main.o vga_io.o malloc.o sprintf.o keyboard.o debug.o string.o stubs.o libc/setjmp.o lua-5.3.3/src/liblua.a

all: $(OUTPUT)

%.o: %.S
	$(AS) $+ -o $@ $(ASFLAGS)

%.o: %.c
	$(CC) -c $+ -o $@ $(CFLAGS)

%.bin: %.lo
	objcopy -O binary $+ $@

mbr.lo: mbr.o
	$(LD) $+ -o $@ $(LDFLAGS) -Ttext=0x07C00

data.lo: $(ASM_OBJS) $(C_OBJS) $(LINK)
	$(LD) -T $(LINK) $+ -o $@ $(LDFLAGS)

$(OUTPUT): mbr.bin data.bin
	dd if=mbr.bin of=$@ bs=512 conv=notrunc count=1
	dd if=data.bin of=$@ bs=512 conv=sync,notrunc seek=1

clean:
	rm -f $(ASM_OBJS) $(C_OBJS) $(OUTPUT) mbr.o mbr.lo mbr.bin data.lo data.bin
