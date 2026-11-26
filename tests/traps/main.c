#include <stdint.h>

#define UART_BASE 0x10000000UL
#define UART_THR  0
#define UART_LSR  5
#define UART_LSR_THRE 0x20

#define MCAUSE_ECALL_MMODE 11U

static volatile uint8_t *const uart =
    (volatile uint8_t *)UART_BASE;

static volatile uint32_t trap_count;

static void uart_putc(char c)
{
    while (!(uart[UART_LSR] & UART_LSR_THRE)) {
        __asm__ volatile ("nop");
    }

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

uint32_t trap_handler(uint32_t cause,
                      uint32_t epc,
                      uint32_t tval)
{
    trap_count++;

    uart_puts("\n[TRAP] Exception received\n");

    uart_puts("[TRAP] mcause = ");
    uart_hex(cause);
    uart_puts("\n");

    uart_puts("[TRAP] mepc   = ");
    uart_hex(epc);
    uart_puts("\n");

    uart_puts("[TRAP] mtval  = ");
    uart_hex(tval);
    uart_puts("\n");

    if (cause == MCAUSE_ECALL_MMODE) {
        uart_puts("[TRAP] Machine-mode ECALL\n");
        uart_puts("[TRAP] Returning to caller\n");

        return epc + 4;
    }

    uart_puts("[PANIC] Unhandled trap\n");

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
    uart_puts("Test         : Trap handling\n");

    uart_puts("\n[MAIN] Executing ECALL\n");

    __asm__ volatile ("ecall" ::: "memory");

    uart_puts("[MAIN] Execution resumed\n");

    uart_puts("[MAIN] Trap count = ");
    uart_hex(trap_count);
    uart_puts("\n");

    if (trap_count == 1)
        uart_puts("[TEST] SUCCESS\n");
    else
        uart_puts("[TEST] FAILED\n");

    return 0;
}
