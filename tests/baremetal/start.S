.section .text.init
.global _start

_start:
    /* Initialize global pointer */
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    /* Initialize stack pointer */
    la sp, _stack_top

    /* Clear BSS section */
    la t0, _bss_start
    la t1, _bss_end

clear_bss:
    bgeu t0, t1, boot_main
    sw zero, 0(t0)
    addi t0, t0, 4
    j clear_bss

boot_main:
    call main

halt:
    wfi
    j halt
