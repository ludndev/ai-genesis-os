; kernel_entry.asm
[bits 32]
[extern _kernel_main] ; Define calling point. Must have same name as kernel.c 'kernel_main' function
call _kernel_main ; Calls the C function. The linker will know where it is placed in memory
jmp $
