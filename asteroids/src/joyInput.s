
		preserve8 ; Indicate the code here preserve
                  ; 8 byte stack alignment
        area |.text|, code, readonly ; Start of CODE area
        export joyPressed
        entry
        
joyPressed  function

	;r0 used to identify input pressed (see above)
	;r1 used to identify if input should be press/release or hold

;IOBASE EQU 0x20098000
;P5base EQU 0xA0
P5Base EQU 0x200980A0 ;P5 is at offset 0xA0 on IOBASE

dir  EQU 0x00
mask EQU 0x10
pin  EQU 0x14
set  EQU 0x18
clr  EQU 0x1c

;JUP  		EQU 1<<2
;JDOWN  	EQU 1<<1
;JLEFT  	EQU 1<<0
;JRIGHT  	EQU	1<<4
;JCENTER 	EQU 1<<3

	PUSH {R2,R3,LR}		
	MOV r1,r0			;register 0 holds parameter, move input value to r1
	LDR r2,=P5Base		;store P5Base address in r2

	LDR r3,[r2,#pin]	;read state of the pin at the base address
	
	LSL r0,r1			;shift bytes left by number specified in register
	AND	r0,r3			;AND register 3 with shifted bytes to return joy state 
;	register 0 holds return value
	POP {R2,R3,LR}
	BX LR
	ENDFUNC
    END
