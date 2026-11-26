#define UART_BASE 0x10000000UL

#define UART_THR 0
#define UART_LSR 5
#define UART_LSR_THRE 0x20

static volatile unsigned char *const uart =
    (volatile unsigned char *)UART_BASE;

static void uart_putc(char c)
{
    while (!(uart[UART_LSR] & UART_LSR_THRE)) {
        __asm__ volatile ("nop");
    }

    uart[UART_THR] = (unsigned char)c;
}

static void uart_puts(const char *str)
{
    while (*str) {
        if (*str == '\n')
            uart_putc('\r');

        uart_putc(*str++);
    }
}

int main(void)
{
    uart_puts("\n");
    uart_puts("============================\n");
    uart_puts(" P4Linux v0.1.0-dev\n");
    uart_puts("============================\n");
    uart_puts("Architecture : RISC-V RV32\n");
    uart_puts("Platform     : QEMU virt\n");
    uart_puts("Mode         : Bare Metal\n");
    uart_puts("UART         : OK\n");
    uart_puts("Boot         : SUCCESS\n");

    return 0;
}
