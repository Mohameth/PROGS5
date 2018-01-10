.global main
.text
main:
	MRS r1, CPSR
    swi 0x123456
.data
