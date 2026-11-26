# P4Linux

Native Linux port for the Espressif ESP32-P4 SoC.

## Project goals

- Native RISC-V RV32 execution.
- Linux kernel without MMU.
- No CPU emulation.
- No underlying RTOS during kernel execution.
- BusyBox-based userspace.
- SD card storage and Ethernet networking.
- Reproducible build system.

## Target hardware

- ESP32-P4
- 32 MB PSRAM
- 16 MB Flash
- Waveshare ESP32-P4-Module-DEV-KIT

## Development environment

- Debian 13 (Trixie)
- GCC RISC-V cross-compilation toolchain
- GNU Make
- GDB
- QEMU
- ESP-IDF
- Buildroot

## Project status

Version: 0.1.0-dev

Phase 0: Development environment and architecture validation.
# P4Linux
