
MEMORY {

    // Reserve 0x8000 bytes for use by target-resident code or debug monitor
    dram_memory  : ORIGIN = 0xF1018000,	LENGTH = 0x8000

    flash_memory : ORIGIN = 0xF1000000, LENGTH = 0x18000
    //flash_memory : ORIGIN = 0xC0020000, LENGTH = 0x18000

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
    .vector 			:{ "startup.o(.text)" } > .
    .text                                               : > .
    .rodata                                             : > .
    .secinfo                                            : > .
    .ROM.data 	                            ROM(.data) : > .
    .ROM.bss 	                            NOLOAD : > .

//
// The data segment
//

    .pidbase                                  ALIGN(16) : > dram_memory
    .data                                               : > .
    .bss                                                : > .
    .ghcovfz					  CLEAR : > .
    .ghcovcz					  CLEAR : > .
    .ghcovdz					  CLEAR : > .
//
// These special symbols mark the bounds of RAM and ROM memory.
// They are used by the MULTI debugger.
//
    __ghs_romstart  = MEMADDR(flash_memory);
    __ghs_romend    = ENDADDR(.secinfo);
    __ghs_ramstart  = ADDR(.pidbase);
    __ghs_ramend    = ENDADDR(.bss);
    __ghs_bssstart = ADDR(.bss);
        
}
