MEMORY {

    // Reserve 0x8000 bytes for use by target-resident code or debug monitor
    dram_memory  : ORIGIN = 0x00000000,	LENGTH = (SRAM_TOTAL_SIZE - ARM_STACK_SIZE - DMA_NC_SIZE - CAN_NC_SIZE - LOADCODE_SIZE) /* total(0x40000) - ARM_STACK_SIZE(0x680) - NC_SRAM(0x2000 or...config) - CAN Message RAM(0x8000) */
    // for preload code on ram
    SRAM_CODELOAD : ORIGIN = 0x00000000 + (SRAM_TOTAL_SIZE - ARM_STACK_SIZE - DMA_NC_SIZE - CAN_NC_SIZE - LOADCODE_SIZE), LENGTH = LOADCODE_SIZE
    // Reserve 0x680 bytes for arm stack
    stack_memory  : ORIGIN = 0x00000000 + (SRAM_TOTAL_SIZE - ARM_STACK_SIZE - DMA_NC_SIZE - CAN_NC_SIZE), LENGTH = ARM_STACK_SIZE
    // Reserve CAN_NC_SIZE bytes for non-cacheable area
    can_memory  : ORIGIN = 0x00000000 + (SRAM_TOTAL_SIZE - DMA_NC_SIZE - CAN_NC_SIZE), LENGTH = CAN_NC_SIZE
    // Reserve DMA_NC_SIZE bytes for non-cacheable area, The dma address is valid only physical address(0xC0000000), it can not use mirrored mem address(0x06000000)
    dma_memory  : ORIGIN = 0x00000000 + (SRAM_TOTAL_SIZE - DMA_NC_SIZE), LENGTH = DMA_NC_SIZE
    // S-NOR code area XIP
    flash_memory : ORIGIN = 0x01043000, LENGTH = 0x00200000 //2MByte
}

//
// Program layout for running out of RAM.
//
//

SECTIONS
{

//
// The text segment
//

    .picbase                                            : > flash_memory
    .text                                               :{ "startup.o(.text)" } > .
    .syscall                                            : > .
    .intercall                                          : > .
    .interfunc                                          : > .
    .fixaddr                                            : > .
    .fixtype                                            : > .
    .rodata                                             : > .
    .secinfo                                            : > .
    .preload_code     ROM(.PRELOADCodeSection) ALIGN(4) : > .
    .ROM.data 	                     ALIGN(4)ROM(.data) : > .
    .ROM.bss 	                        ALIGN(4) NOLOAD : > .

//
// The data segment
//

    .pidbase                                  ALIGN(16) : > dram_memory
    .data                                       ALIGN(4):{vector.o(.text) *(.data)}> .
    .bss                                        ALIGN(0x20): > .
    .ghcovfz					  CLEAR : > .
    .ghcovcz					  CLEAR : > .
    .ghcovdz					  CLEAR : > .

//
// The preload code segment
//
    .PRELOADCodeSection                          ALIGN(4) : { *(.codeonsram) } > SRAM_CODELOAD
//
// These special symbols mark the bounds of RAM and ROM memory.
//
    __ghs_romstart  = MEMADDR(flash_memory);
    __ghs_romend    = ADDR(.ROM.data);
    __ghs_ramstart  = ADDR(.data);
    __ghs_ramend    = ENDADDR(.bss);
    __ghs_bssstart = ADDR(.bss);
    __arm_stack = MEMADDR(stack_memory);
    __nc_canstart = MEMADDR(can_memory);
    __nc_dmastart = MEMADDR(dma_memory);
    _end_of_nc_can = __nc_canstart + CAN_NC_SIZE;
    _end_of_nc_dma = __nc_dmastart + DMA_NC_SIZE;
    __PRECODE_START_LOAD       = ADDR(.preload_code);
    __PRECODE_RAM_START__      = MEMADDR(SRAM_CODELOAD);
    __PRECODE_SIZE__           = __PRECODE_RAM_START__+ SIZEOF(.preload_code);
}
