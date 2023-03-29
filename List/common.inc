defaultSize:    equ 16

struc List
.arrayPtr:  resq 1
.currentLength:  resd 1 ;index of first free space in array
.allocatedMem:   resd 1 ;in bytes (not elements)
.elementSize:    resb 1
endstruc
