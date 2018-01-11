.global main
.text
   
main:
    mov r0, #0xFF0
    mov r1, #2

    add r2, r0, r1		//8
    sub r3, r0, r1		//2
    swi 0x123456
.data