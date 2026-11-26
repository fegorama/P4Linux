/*
 * P4Linux - Minimal userspace init
 * Architecture: RISC-V RV32
 * No libc dependencies
 */

.option norelax
.option nopic

.section .text
.global _start
.type _start, @function

_start:

    /* write(STDOUT_FILENO, banner, length) */

    li      a0, 1
    lla     a1, banner
    lla     a2, banner_end
    sub     a2, a2, a1
    li      a7, 64
    ecall

    /* getpid() */

    li      a7, 172
    ecall

    /* Verify PID == 1 */

    li      t0, 1
    bne     a0, t0, failure

    li      a0, 1
    lla     a1, success_msg
    lla     a2, success_end
    sub     a2, a2, a1
    li      a7, 64
    ecall

idle:

    /* sched_yield() */

    li      a7, 124
    ecall

    j       idle

failure:

    li      a0, 1
    lla     a1, failure_msg
    lla     a2, failure_end
    sub     a2, a2, a1
    li      a7, 64
    ecall

    j       idle

.size _start, . - _start

.section .rodata

banner:
    .ascii "\n"
    .ascii "============================\n"
    .ascii " P4Linux v0.2.0-dev\n"
    .ascii "============================\n"
    .ascii "Architecture : RISC-V RV32\n"
    .ascii "Memory model : NOMMU\n"
    .ascii "Userspace    : Native\n"
    .ascii "Runtime      : No libc\n\n"
banner_end:

success_msg:
    .ascii "[INIT] PID 1 confirmed\n"
    .ascii "[INIT] Linux syscalls working\n"
    .ascii "[INIT] SUCCESS\n"
success_end:

failure_msg:
    .ascii "[INIT] Unexpected PID\n"
    .ascii "[INIT] FAILURE\n"
failure_end:

.section .note.GNU-stack,"",@progbits

