# P4Linux Native Application SDK
# Target: RISC-V RV32IMAC / NOMMU
# Runtime: uClibc-ng
# Executable format: BFLT v4

P4LINUX_ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

TOOLCHAIN := $(P4LINUX_ROOT)/build/buildroot-qemu/host/bin

CROSS_COMPILE ?= $(TOOLCHAIN)/riscv32-buildroot-linux-uclibc-

CC := $(CROSS_COMPILE)gcc
AR := $(CROSS_COMPILE)ar
LD := $(CROSS_COMPILE)ld
OBJDUMP := $(CROSS_COMPILE)objdump
READELF := $(CROSS_COMPILE)readelf

ARCH_FLAGS := -march=rv32imac -mabi=ilp32

CFLAGS := -std=c11 -O2
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += $(ARCH_FLAGS) -fPIC

LDFLAGS := -static -Wl,-elf2flt=-r

BUILD_DIR := $(P4LINUX_ROOT)/build/apps
INSTALL_DIR := $(P4LINUX_ROOT)/rootfs/overlay/usr/bin

