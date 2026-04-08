DEFAULTS {

    heap_reserve = 1M
    stack_reserve = 512K

}
//
// Program layout for PIC and PID built programs.
//
//

SECTIONS
{

//
// The text segment
//

    .picbase                     0x00000000 :
    .text                                   :
    .syscall                                :
    .intercall                              :
    .interfunc                              :
    .fixaddr                                :
    .fixtype                                :
    .rodata                                 :   
    .secinfo                                :
    
//
// The data segment
//

    .pidbase                      0x00000000 :
    .sdabase                                 :
    .sbss                                    :
    .sdata                                   :
    .data                                    :
    .bss                                     :
    .ghcovfz			       CLEAR :
    .ghcovcz			       CLEAR :
    .ghcovdz			       CLEAR :
    .heap                   ALIGN(16) PAD( heap_reserve +
	    // Add space for call-graph profiling if used:
	    (isdefined(__ghs_indgcount)?(2000+(sizeof(.text)/2)):0) +
	    // Add estimated space for call-count profiling if used:
	    (isdefined(__ghs_indmcount)?10000:0) )
	    				     :
    .stack      ALIGN(16) PAD(stack_reserve) :
}
