# Makefile for Genesis OS v0.0.1

# Tools
CC = gcc
LD = ld
NASM = nasm

# Flags
CFLAGS = -ffreestanding -m32 -g
LDFLAGS = -m i386pe -Ttext 0x1000 --oformat binary

# Sources
C_SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJ = $(C_SOURCES:.c=.o)

# Default target
all: os-image.bin

# Run in QEMU
run: all
	qemu-system-i386 -fda os-image.bin

# Build OS image
os-image.bin: boot.bin kernel.bin
	cat boot.bin kernel.bin > os-image.bin

# Build kernel binary
kernel.bin: kernel_entry.o kernel.o
	$(LD) -m i386pe -o kernel.pe -Ttext 0x1000 kernel_entry.o kernel.o
	objcopy -O binary kernel.pe kernel.bin

# Compile C kernel
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

# Assemble kernel entry
kernel_entry.o: kernel_entry.asm
	$(NASM) kernel_entry.asm -f win32 -o kernel_entry.o

# Assemble bootloader
boot.bin: boot.asm
	$(NASM) boot.asm -f bin -o boot.bin

# Clean
clean:
	rm *.bin *.o *.pe
