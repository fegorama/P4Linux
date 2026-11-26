#include <stdint.h>

#define UART_BASE       0x10000000UL
#define UART_THR        0
#define UART_LSR        5
#define UART_LSR_THRE   0x20

#define MCAUSE_ECALL_U  8U

#define SYS_WRITE       1U
#define SYS_EXIT        2U

#define REG_A0          8U
#define REG_A7          15U

static volatile uint8_t *const uart =
    (volatile uint8_t *)UART_BASE;

static volatile uint32_t syscall_count;

extern void enter_user_mode(uint32_t entry)
    __attribute__((noreturn));

static void uart_putc(char c)
{
    while (!(uart[UART_LSR] & UART_LSR_THRE))
        __asm__ volatile ("nop");

    uart[UART_THR] = (uint8_t)c;
}

static void uart_puts(const char *str)
{
    while (*str) {
        if (*str == '\n')
            uart_putc('\r');

        uart_putc(*str++);
    }
}

static void uart_hex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";

    uart_puts("0x");

    for (int i = 28; i >= 0; i -= 4)
        uart_putc(digits[(value >> i) & 0xF]);
}

/*
 * Configure PMP for the initial privilege test.
 * All physical memory is accessible from U-mode.
 */
static void pmp_init(void)
{
    uint32_t address = 0xFFFFFFFFU;
    uint32_t config  = 0x1FU;

    __asm__ volatile (
        "csrw pmpaddr0, %0"
        :
        : "r"(address)
        : "memory"
    );

    __asm__ volatile (
        "csrw pmpcfg0, %0"
        :
        : "r"(config)
        : "memory"
    );
}

static inline uint32_t syscall(
    uint32_t number,
    uint32_t argument)
{
    register uint32_t a0 __asm__("a0") = argument;
    register uint32_t a7 __asm__("a7") = number;

    __asm__ volatile (
        "ecall"
        : "+r"(a0)
        : "r"(a7)
        : "memory"
    );

    return a0;
}

static void user_main(void)
{
    syscall(
        SYS_WRITE,
        (uint32_t)(uintptr_t)
        "[USER] Hello from User Mode\n"
    );

    syscall(
        SYS_WRITE,
        (uint32_t)(uintptr_t)
        "[USER] System call interface working\n"
    );

    syscall(SYS_EXIT, 0);

    while (1)
        __asm__ volatile ("nop");
}

uint32_t trap_handler(
    uint32_t cause,
    uint32_t epc,
    uint32_t tval,
    uint32_t *frame)
{
    if (cause == MCAUSE_ECALL_U) {

        uint32_t number = frame[REG_A7];
        uint32_t argument = frame[REG_A0];

        syscall_count++;

        uart_puts("[KERNEL] U-mode ECALL: ");
        uart_hex(number);
        uart_puts("\n");

        switch (number) {

        case SYS_WRITE:
            uart_puts(
                (const char *)(uintptr_t)argument
            );

            frame[REG_A0] = 0;
            return epc + 4;

        case SYS_EXIT:
            uart_puts("[KERNEL] User exit code: ");
            uart_hex(argument);
            uart_puts("\n");

            uart_puts("[KERNEL] Syscall count: ");
            uart_hex(syscall_count);
            uart_puts("\n");

            if (argument == 0 && syscall_count == 3)
                uart_puts("[TEST] SUCCESS\n");
            else
                uart_puts("[TEST] FAILED\n");

            while (1)
                __asm__ volatile ("wfi");

        default:
            frame[REG_A0] = (uint32_t)-1;
            return epc + 4;
        }
    }

    uart_puts("[PANIC] Unexpected trap\n");

    uart_puts("mcause = ");
    uart_hex(cause);
    uart_puts("\n");

    uart_puts("mepc   = ");
    uart_hex(epc);
    uart_puts("\n");

    uart_puts("mtval  = ");
    uart_hex(tval);
    uart_puts("\n");

    while (1)
        __asm__ volatile ("wfi");
}

int main(void)
{
    uart_puts("\n");
    uart_puts("============================\n");
    uart_puts(" P4Linux v0.1.0-dev\n");
    uart_puts("============================\n");
    uart_puts("Platform     : QEMU virt\n");
    uart_puts("Architecture : RISC-V RV32\n");
    uart_puts("Test         : Privilege modes\n\n");

    pmp_init();

    uart_puts("[KERNEL] PMP configured\n");
    uart_puts("[KERNEL] Entering User Mode\n");

    enter_user_mode(
        (uint32_t)(uintptr_t)user_main
    );
}
