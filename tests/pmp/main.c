#include <stdint.h>

#define UART_BASE       0x10000000UL
#define USER_BASE       0x80200000UL
#define USER_END        0x80400000UL
#define USER_SIZE       0x00200000UL

#define SYS_WRITE       1U
#define SYS_EXIT        2U

#define REG_A0          8U
#define REG_A7          15U

#define MCAUSE_ECALL_U  8U
#define MCAUSE_LOAD_ACCESS_FAULT 5U

#define USER_TEXT \
    __attribute__((section(".user.text"), noinline))

#define USER_RODATA \
    __attribute__((section(".user.rodata"), used))

static volatile uint8_t *const uart =
    (volatile uint8_t *)UART_BASE;

static uint32_t denied_accesses;

extern void enter_user_mode(uint32_t entry)
    __attribute__((noreturn));

static const char user_message[] USER_RODATA =
    "[USER] Running in isolated memory\n";

static const char attack_message[] USER_RODATA =
    "[USER] Attempting kernel memory read\n";

static void uart_putc(char c)
{
    while (!(uart[5] & 0x20))
        __asm__ volatile ("nop");

    uart[0] = (uint8_t)c;
}

static void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n')
            uart_putc('\r');

        uart_putc(*s++);
    }
}

static void uart_hex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";

    uart_puts("0x");

    for (int i = 28; i >= 0; i -= 4)
        uart_putc(digits[(value >> i) & 0xF]);
}

static void pmp_init(void)
{
    uint32_t address =
        (USER_BASE >> 2) |
        ((USER_SIZE >> 3) - 1);

    /* NAPOT, read/write/execute */
    uint32_t config = 0x1F;

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

    uart_puts("[PMP] pmpaddr0 = ");
    uart_hex(address);
    uart_puts("\n");
}

static USER_TEXT uint32_t syscall(
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

static USER_TEXT void user_main(void)
{
    syscall(SYS_WRITE,
        (uint32_t)(uintptr_t)user_message);

    syscall(SYS_WRITE,
        (uint32_t)(uintptr_t)attack_message);

    /*
     * Force a 32-bit load instruction so that
     * the handler can safely advance mepc by 4.
     */
    __asm__ volatile (
        ".option push\n"
        ".option norvc\n"
        "lw t0, 0(%0)\n"
        ".option pop\n"
        :
        : "r"(0x80000000UL)
        : "t0", "memory"
    );

    syscall(SYS_EXIT, 0);

    while (1)
        __asm__ volatile ("nop");
}

static int user_string_valid(uint32_t address)
{
    if (address < USER_BASE || address >= USER_END)
        return 0;

    while (address < USER_END) {
        if (*(const char *)(uintptr_t)address == '\0')
            return 1;

        address++;
    }

    return 0;
}

uint32_t trap_handler(
    uint32_t cause,
    uint32_t epc,
    uint32_t tval,
    uint32_t *frame)
{
    if (cause == MCAUSE_LOAD_ACCESS_FAULT &&
        tval == 0x80000000UL &&
        epc >= USER_BASE && epc < USER_END) {

        denied_accesses++;

        uart_puts("[PMP] Unauthorized access blocked\n");

        uart_puts("[PMP] mcause = ");
        uart_hex(cause);
        uart_puts("\n");

        uart_puts("[PMP] mtval  = ");
        uart_hex(tval);
        uart_puts("\n");

        return epc + 4;
    }

    if (cause == MCAUSE_ECALL_U) {

        uint32_t number = frame[REG_A7];
        uint32_t argument = frame[REG_A0];

        switch (number) {

        case SYS_WRITE:
            if (user_string_valid(argument)) {
                uart_puts(
                    (const char *)(uintptr_t)argument
                );
                frame[REG_A0] = 0;
            } else {
                frame[REG_A0] = (uint32_t)-1;
            }

            return epc + 4;

        case SYS_EXIT:
            uart_puts("[KERNEL] Denied accesses: ");
            uart_hex(denied_accesses);
            uart_puts("\n");

            if (argument == 0 && denied_accesses == 1)
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
    uart_puts("Test         : PMP isolation\n\n");

    pmp_init();

    uart_puts("[KERNEL] Entering User Mode\n");

    enter_user_mode(
        (uint32_t)(uintptr_t)user_main
    );
}
