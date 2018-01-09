.global main
.text
main:
	mov r0, #16
	mov r0, r0, LSR #4
	mov r1, #1

    tst r0, r1

    swi 0x123456
.data
