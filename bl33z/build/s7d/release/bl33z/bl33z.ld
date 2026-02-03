OUTPUT_FORMAT("elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(bl33z_entrypoint)
MEMORY {
    RAM (rwx): ORIGIN = 0x00400000, LENGTH = 0x00004000
}
MEMORY {
    STACK (rwx): ORIGIN = 0x00600000, LENGTH = 0x00010000
}
SECTIONS
{
    . = 0x00400000;
    . = ALIGN(8);
    ro . : {
    __RO_START__ = .;
        *bl33z_entrypoint.o(.text*)
        *(.text*)
        *(.rodata*)
        *(.vectors)
        __RO_END_UNALIGNED__ = .;
        . = NEXT(8);
        __RO_END__ = .;
    } >RAM
    .data . : {
        __DATA_START__ = .;
        *(.data*)
        __DATA_END__ = .;
    } >RAM
    MARK_ADDR = .;
    stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >STACK
    . = MARK_ADDR;
    .bss : ALIGN(8) {
        __BSS_START__ = .;
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)
        __BSS_END__ = .;
    } >RAM
    __BL33Z_END__ = .;
    __BSS_SIZE__ = SIZEOF(.bss);
    ASSERT(. <= (0x00400000 + 0x00004000), "BL33Z image has exceeded its limit.")
}
