.global main
.text
main:
    mov r1, #0x1
    adc r0, r0, r1
    mov r1, #0x2
    swi 0x123456
