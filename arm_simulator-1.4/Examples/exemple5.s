.global main
.text
main:
    mov r1, #0x0
    adc r0, r0, r1
    mov r1, #0x0
    swi 0x123456
