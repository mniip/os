AS=as --32
ASFLAGS=
LD=ld
LDFLAGS=-n -m elf_i386
CC=gcc -m32
CFLAGS=-O1 -Wall -Wextra -ffreestanding -nostdlib

OUTPUT=boot.img
ASM_OBJS=start.o
C_OBJS=main.o vga_io.o malloc.o

all: $(OUTPUT)

%.o: %.S
	$(AS) $+ -o $@ $(ASFLAGS)

%.o: %.c
	$(CC) -c $+ -o $@ $(CFLAGS)

%.bin: %.lo
	objcopy -O binary $+ $@

mbr.lo: mbr.o
	$(LD) $+ -o $@ $(LDFLAGS) -Ttext=0x07C00

data.lo: $(ASM_OBJS) $(C_OBJS)
	$(LD) $+ -o $@ $(LDFLAGS) -Ttext=0x07E00

$(OUTPUT): mbr.bin data.bin
	dd if=mbr.bin of=$@ bs=512 count=1
	dd if=data.bin of=$@ bs=512 conv=sync seek=1

clean:
	rm -f $(ASM_OBJS) $(C_OBJS) $(OUTPUT) mbr.o mbr.lo mbr.bin data.lo data.bin
