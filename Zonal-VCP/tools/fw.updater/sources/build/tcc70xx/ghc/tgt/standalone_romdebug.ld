
MEMORY {

    // Reserve 0x8000 bytes for use by target-resident code or debug monitor
    dram_rsvd1   : ORIGIN = 0x02000000, LENGTH = 0x8000
    dram_memory  : ORIGIN = .,	 	LENGTH = 16M-0x8000
    dram_rsvd2   : ORIGIN = ., 	        LENGTH = 0

    flash_rsvd1  : ORIGIN = 0x00000000, LENGTH = 0
    flash_memory : ORIGIN = .,          LENGTH = 16M
    flash_rsvd2  : ORIGIN = .,          LENGTH = 0

}
DEFAULTS {

    heap_reserve = 1M
    stack_reserve = 512K

}
//
// Program layout for starting in ROM, copying data to RAM, and continuing to
// execute out of ROM.  Some small debugging library functions are located in
// RAM to allow MULTI to set software breakpoints rather than hardware.
//

SECTIONS
{

//
// ROM SECTIONS
//

    .text 					        : > flash_memory
    .ROM.syscall 		          ROM(.syscall)	: > .
    .intercall					        : > .
    .interfunc  				        : > .

    .rodata 					        : > .
    .sdata2 					        : > .

    .secinfo 					        : > .
    .fixaddr 					        : > .
    .fixtype 					        : > .

    .CROM.sdata  	                   CROM(.sdata) : > .
    .CROM.multibreak 		      CROM(.multibreak)	: > . 
    .CROM.data 	                            CROM(.data) : > .

//
// RAM SECTIONS
//

    .sdabase 					        : > dram_memory
    .sbss 					        : > .
    .sdata 					        : > .

    // Allow small hook functions to execute out of RAM where
    // MULTI can set software breakpoints for debugging.
    .multibreak						: {
    	libind.a(indmultibreak.o)(.text)
	libdbmem.a(indmultibreak.o)(.text)
    	libind.a(indrnerr_wrap.o)(.text)
	libdbmem.a(indrnerr_wrap.o)(.text)
    }							  > .
    .syscall					        : > .

    .data 					        : > .
    .bss 					        : > .
    .ghcovfz					  CLEAR : > .
    .ghcovcz					  CLEAR : > .
    .ghcovdz					  CLEAR : > .
    .heap                   ALIGN(16) PAD( heap_reserve +
	    // Add space for call-graph profiling if used:
	    (isdefined(__ghs_indgcount)?(2000+(sizeof(.text)/2)):0) +
	    // Add estimated space for call-count profiling if used:
	    (isdefined(__ghs_indmcount)?10000:0) )
	    						: > .
    .stack                 ALIGN(16) PAD(stack_reserve) : > .


//
// These special symbols mark the bounds of RAM and ROM memory.
// They are used by the MULTI debugger.
//
    __ghs_romstart  = MEMADDR(flash_rsvd1);
    __ghs_romend    = MEMENDADDR(flash_rsvd2);
    __ghs_ramstart  = MEMADDR(dram_rsvd1);
    __ghs_ramend    = MEMENDADDR(dram_rsvd2);

//
// These special symbols mark the bounds of RAM and ROM images of boot code.
// They are used by the GHS startup code (_start and __ghs_ind_crt0).
//
    __ghs_rombootcodestart  = ADDR(.text);
    __ghs_rombootcodeend    = ENDADDR(.fixtype);
    __ghs_rambootcodestart  = 0;
    __ghs_rambootcodeend    = 0;

//
// This special symbol marks the the first address executed after the
// ROM to RAM copy is complete.  It is used by the MULTI debugger.
//
    __ghs_after_romcopy = __ghs_ind_crt1;

//
// This symbol points to the ROM copy of __dotsyscall (which is in RAM).
// It overrides the weak symbol in startup code, which is called before RAM
// has been initialized.  This symbol is used by the MULTI debugger.
//
    __ghs_initial_dotsyscall = ADDR(.ROM.syscall);
}


// Redirect calls to __rnerr Runtime Error Checking function to a wrapper
// function located in RAM so we can set a software breakpoint on it. 
OPTION("-wrap __rnerr")

// Pull in alternate _multiend definitions to move into RAM for
// Command Line Procedure Calls.
OPTION("-u _multiend_ram")
