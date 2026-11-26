OUTPUT_ARCH(riscv)
ENTRY(_start)

MEMORY
{
    KERNEL (rwx) : ORIGIN = 0x80000000, LENGTH = 2M
    USER   (rwx) : ORIGIN = 0x80200000, LENGTH = 2M
}

SECTIONS
{
    .text : {
        KEEP(*(.text.init))
        *(.text*)
    } > KERNEL

    .rodata : {
        *(.rodata*)
        *(.srodata*)
    } > KERNEL

    .data : {
        . = ALIGN(4);
        *(.data*)
        PROVIDE(__global_pointer$ = . + 0x800);
        *(.sdata*)
    } > KERNEL

    .bss (NOLOAD) : {
        . = ALIGN(4);
        _bss_start = .;
        *(.sbss*)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > KERNEL

    .user : {
        . = ALIGN(4);
        KEEP(*(.user.text))
        KEEP(*(.user.rodata))
        KEEP(*(.user.data))
    } > USER

    _stack_top = ORIGIN(KERNEL) + LENGTH(KERNEL);
    _user_stack_top = ORIGIN(USER) + LENGTH(USER);

    ASSERT(_bss_end <= _stack_top - 4096,
           "Kernel stack overlap")

    ASSERT(ADDR(.user) + SIZEOF(.user)
           <= _user_stack_top - 4096,
           "User stack overlap")
}
