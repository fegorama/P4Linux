volatile unsigned int result;

void _start(void)
{
    result = 0x50344C58;

    while (1) {
        __asm__ volatile ("nop");
    }
}
