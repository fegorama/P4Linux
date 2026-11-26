/*
 * P4Linux - Minimal NOMMU PIE linker script
 */

OUTPUT_ARCH(riscv)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FILEHDR PHDRS FLAGS(5);
}

SECTIONS
{
    . = SIZEOF_HEADERS;

    .text : ALIGN(4)
    {
        *(.text)
        *(.text.*)
    } :text

    .rodata : ALIGN(4)
    {
        *(.rodata)
        *(.rodata.*)
    } :text

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
        *(.eh_frame*)
    }
}

