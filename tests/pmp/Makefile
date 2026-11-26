CROSS_COMPILE ?= riscv64-unknown-elf-

CC      = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE    = $(CROSS_COMPILE)size

BUILD = ../../build/pmp
TARGET = $(BUILD)/p4linux-pmp

ARCH = -march=rv32imac_zicsr -mabi=ilp32

CFLAGS = $(ARCH) -O2 -Wall -Wextra \
         -ffreestanding -fno-builtin \
         -fno-stack-protector -fno-pic

LDFLAGS = $(ARCH) -nostdlib -nostartfiles \
          -static -T linker.ld \
          -Wl,--build-id=none \
          -Wl,-Map=$(TARGET).map

.RECIPEPREFIX := >

.PHONY: all run clean

all:
>mkdir -p $(BUILD)
>$(CC) $(CFLAGS) -c start.S -o $(BUILD)/start.o
>$(CC) $(CFLAGS) -c main.c -o $(BUILD)/main.o
>$(CC) $(LDFLAGS) $(BUILD)/start.o $(BUILD)/main.o -o $(TARGET).elf
>$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin
>$(OBJDUMP) -d $(TARGET).elf > $(TARGET).asm
>$(SIZE) $(TARGET).elf

run: all
>qemu-system-riscv32 -machine virt -cpu rv32 \
>    -m 128M -smp 1 -bios none -nographic \
>    -kernel $(TARGET).elf

clean:
>rm -rf $(BUILD)
