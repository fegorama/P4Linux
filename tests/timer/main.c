#include <stdint.h>

#define UART_BASE       0x10000000UL
#define UART_THR        0
#define UART_LSR        5
#define UART_LSR_THRE   0x20

#define MTIME_BASE      0x0200BFF8UL
#define MTIMECMP_BASE   0x02004000UL

#define TIMER_FREQ      10000000UL
#define TICK_HZ         10UL
#define TICK_INTERVAL   (TIMER_FREQ / TICK_HZ)

#define MSTATUS_MIE     (1UL << 3)
#define MIE_MTIE        (1UL << 7)

#define MCAUSE_TIMER    0x80000007UL

static volatile uint8_t *const uart =
    (volatile uint8_t *)UART_BASE;

static volatile uint32_t *const mtime =
    (volatile uint32_t *)MTIME_BASE;

static volatile uint32_t *const mtimecmp =
    (volatile uint32_t *)MTIMECMP_BASE;

static volatile uint32_t ticks;
static uint64_t next_deadline;

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

/* Safe 64-bit timer read on RV32 */
static uint64_t timer_read(void)
{
    uint32_t high1;
    uint32_t low;
    uint32_t high2;

    do {
        high1 = mtime[1];
        low   = mtime[0];
        high2 = mtime[1];
    } while (high1 != high2);

    return ((uint64_t)high1 << 32) | low;
}

/* Safe 64-bit comparator write on RV32 */
static void timer_set(uint64_t value)
{
    mtimecmp[0] = 0xFFFFFFFFUL;
    mtimecmp[1] = (uint32_t)(value >> 32);
    mtimecmp[0] = (uint32_t)value;
}

static void timer_init(void)
{
    next_deadline = timer_read() + TICK_INTERVAL;

    timer_set(next_deadline);

    __asm__ volatile (
        "csrs mie, %0"
        :
        : "r"(MIE_MTIE)
        : "memory"
    );

    __asm__ volatile (
        "csrs mstatus, %0"
        :
        : "r"(MSTATUS_MIE)
        : "memory"
    );
}

uint32_t trap_handler(uint32_t cause,
                      uint32_t epc,
                      uint32_t tval)
{
    if (cause == MCAUSE_TIMER) {

        /* Ignore a possible spurious timer interrupt */
        if (timer_read() < next_deadline)
            return epc;

        ticks++;

        next_deadline += TICK_INTERVAL;
        timer_set(next_deadline);

        if (ticks >= 10) {
            __asm__ volatile (
                "csrc mie, %0"
                :
                : "r"(MIE_MTIE)
                : "memory"
            );
        }

        return epc;
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
    uint32_t reported = 0;

    uart_puts("\n");
    uart_puts("============================\n");
    uart_puts(" P4Linux v0.1.0-dev\n");
    uart_puts("============================\n");
    uart_puts("Platform     : QEMU virt\n");
    uart_puts("Architecture : RISC-V RV32\n");
    uart_puts("Test         : Timer IRQ\n");
    uart_puts("Frequency    : 10 Hz\n\n");

    timer_init();

    while (reported < 10) {

        while (reported < ticks) {
            reported++;

            uart_puts("[TIMER] Tick ");
            uart_hex(reported);
            uart_puts("\n");
        }
    }

    uart_puts("\n[TEST] SUCCESS\n");
    uart_puts("[SYSTEM] Timer interrupts validated\n");

    return 0;
}
