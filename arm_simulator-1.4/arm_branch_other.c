/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>


int b_bl_instr(arm_core p, uint32_t ins) {

    /*int i;
    int j;*/

    uint8_t L = (ins>>24)&1;

    int si = (ins & 0xFFFFFF);

    uint32_t pc = arm_read_usr_register(p, 15);

    if (L) {
        arm_write_usr_register(p, 14, pc);
    }

    int SE = 0;

    uint8_t b24 =  (si>>23) & 1;
    if (b24) {
        SE = (si | 0x3F000000) << 2;
    } else {
        SE = si << 2;
    }

    pc = pc + (SE);
    arm_write_usr_register(p,15,pc);
    return 0;
}

int arm_branch(arm_core p, uint32_t ins) {
    // conditions � faire
    if (get_bit(ins,25))
        return b_bl_instr(p,ins);

    return UNDEFINED_INSTRUCTION;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        /* Here we implement the end of the simulation as swi 0x123456 */
        if ((ins & 0xFFFFFF) == 0x123456)
            exit(0);
        return SOFTWARE_INTERRUPT;
    }
    return UNDEFINED_INSTRUCTION;
}

int mrs_instr(arm_core p, uint32_t ins) {
    uint8_t R = get_bit(ins,22);
    uint8_t Rd = get_bits(ins, 15, 12);

    uint32_t v;

    if (R) {
        v = arm_read_spsr(p);
    } else {
        v = arm_read_cpsr(p);
    }

    arm_write_usr_register(p, Rd, v);

    return 0;
}



int arm_miscellaneous(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
