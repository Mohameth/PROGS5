/*
Armator - simulateur de jeu d'instruction ARMv5T ï¿½ but pï¿½dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Gï¿½nï¿½rale GNU publiï¿½e par la Free Software
Foundation (version 2 ou bien toute autre version ultï¿½rieure choisie par vous).

Ce programme est distribuï¿½ car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spï¿½cifique. Reportez-vous ï¿½ la
Licence Publique Gï¿½nï¿½rale GNU pour plus de dï¿½tails.

Vous devez avoir reï¿½u une copie de la Licence Publique Gï¿½nï¿½rale GNU en mï¿½me
temps que ce programme ; si ce n'est pas le cas, ï¿½crivez ï¿½ la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
ï¿½tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bï¿½timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hï¿½res
*/
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

int check_cond_failed(uint8_t flag_N,uint8_t flag_Z,uint8_t flag_V,uint8_t flag_C,uint8_t cond) {
	switch (cond) {
		case 0:
			if (flag_Z) return 0;
			break;
		case 1:
			if (!flag_Z) return 0;
			break;
		case 2:
			if (flag_C) return 0;
			break;
		case 3:
			if (!flag_C) return 0;
			break;
		case 4:
			if (flag_N) return 0;
			break;
		case 5:
			if (!flag_N) return 0;
			break;
		case 6:
			if (flag_V) return 0;
			break;
		case 7:
			if (!flag_V) return 0;
			break;
		case 8:
			if (flag_C && !flag_Z) return 0;
			break;
		case 9:
			if (!flag_C && flag_Z) return 0;
			break;
		case 10:
			if ((flag_N && flag_V) || (!flag_N && !flag_V)) return 0;
			break;
		case 11:
			if ((flag_N && !flag_V) || (!flag_N && flag_V)) return 0;
			break;
		case 12:
			if ( !flag_Z && ((flag_N && flag_V) || (!flag_N && !flag_V))) return 0;
			break;
		case 13:
			if ((flag_Z) || (flag_N && !flag_V) || (!flag_N && flag_V)) return 0;
			break;
		case 14:
			return 0;
			break;
		case 15:
			return 1;
			break;

		default:
			printf("ERREUR: Mauvaise instruction \n");
			return 0;
	};

	return 1;
}

/**
 * Maj du shifter operand selon le flag I (Immediate) --> Doc p. 446
 * Parametres : pointeur sur le shifter_operand, flags Carry et Immediate
 * Retourne : shifter_carry_out indiquant
 * Effet de bord : Modifie shifter_operand
 **/
uint8_t update_shifter_operand(arm_core p, uint8_t Rdest, uint16_t *shifter_operand, uint8_t I, uint8_t flag_C) {
	uint8_t shifter_carry_out;

	if (!I) {
		//*shifter_operand = arm_read_usr_register(p, *shifter_operand);
		//shifter_carry_out = flag_C;

		uint8_t r = (*shifter_operand>>4)&1;
		if (r) {
			uint8_t RS = (*shifter_operand>>8) & 0xF;
			uint8_t shift = (*shifter_operand>>5) & 3;
			uint8_t RM = arm_read_usr_register(p, (*shifter_operand) & 0xF);

			uint8_t RS4 = RS;
			uint8_t RS7 = (Rdest<<4) | (RS);

			if (shift == 0) {
				if (RS7 == 0) {
					*shifter_operand = RM;
					shifter_carry_out = flag_C;
				} else if (RS7 < 32) {
					*shifter_operand = RM << RS7;
					shifter_carry_out = (RM >> (32-RS7)) & 1;
				} else if (RS7 == 32) {
					*shifter_operand = 0;
					shifter_carry_out = RM & 1;
				} else {
					*shifter_operand = 0;
					shifter_carry_out = 0;
				}
			} else if (shift == 1) {
				if (RS7 == 0) {
					*shifter_operand = RM;
					shifter_carry_out = flag_C;
				} else if (RS7 < 32) {
					*shifter_operand = RM >> RS7;
					shifter_carry_out = (RM >> (RS7-1)) & 1;
				} else if (RS7 == 32) {
					*shifter_operand = 0;
					shifter_carry_out = (RM>>31) & 1;
				} else {
					*shifter_operand = 0;
					shifter_carry_out = 0;
				}
			} else if (shift == 2) {
				if (RS7 == 0) {
					*shifter_operand = RM;
					shifter_carry_out = flag_C;
				} else if (RS7 < 32) {
					*shifter_operand = asr(RM,RS7);
					shifter_carry_out = (RM >> (RS7-1)) & 1;
				} else{
					if ((RM>>31 & 1)) {
						*shifter_operand = 0;
						shifter_carry_out = (RM>>31) & 1;
					} else {
						*shifter_operand = 0xFFFF;
						shifter_carry_out = (RM>>31) & 1;
					}

				}
			} else if (shift == 3) {
				if (RS7 == 0) {
					*shifter_operand = RM;
					shifter_carry_out = flag_C;
				} else if (RS4 == 0) {
					*shifter_operand = RM;
					shifter_carry_out = (RM >> 31) & 1;
				} else{
					*shifter_operand = asr(RM,RS4);
					shifter_carry_out = (RM >> (RS4-1)) & 1;
				}
			}
		} else {
			uint8_t shift_imm = (*shifter_operand>>7) & 0x1F;
			uint8_t shift = (*shifter_operand>>5) & 3;
			uint8_t RM = arm_read_usr_register(p, (*shifter_operand) & 0xF);

			if (shift == 0) {
				if (shift_imm==0) {
					*shifter_operand = RM;
					shifter_carry_out = flag_C;
				} else {
					//OK
					*shifter_operand = RM << shift_imm;
					shifter_carry_out = (RM >> (32 - shift_imm)) & 1;
				}
			} else if (shift == 1) {
				if (shift_imm==0) {
					*shifter_operand = 0;
					shifter_carry_out = (RM >> 31) & 1;
				} else {
					//OK
					*shifter_operand = RM  >> shift_imm;
					shifter_carry_out = (RM >> (shift_imm - 1)) & 1;
				}
			} else if (shift == 2) {
				if (shift_imm==0) {
					if ((RM<<31)&1==0) {
						*shifter_operand = 0;
						shifter_carry_out = (RM >> 31) & 1;
					} else {
						*shifter_operand = 0xFFFF;
						shifter_carry_out = (RM >> 31) & 1;
					}
				} else {
					*shifter_operand =  asr(RM,shift_imm);
					shifter_carry_out = (RM >> (shift_imm - 1)) & 1;
				}
			} else if (shift == 3) {
				if (shift_imm==0) {
					
				} else {
					*shifter_operand = ror(RM, shift_imm);
					shifter_carry_out = (RM >> (shift_imm - 1)) & 1;
				}
			}
		}

	}
	else {
		uint8_t ri = *shifter_operand >> 8 & 0xF;
		*shifter_operand = ror(*shifter_operand & 0xFF, ri*2);

		if (ri == 0) {
			shifter_carry_out = flag_C;
		} else {
			shifter_carry_out = (*shifter_operand >> 31) & 1;
		}
	}

	return shifter_carry_out;
}




//
int traitement_AR(arm_core p, uint32_t mot, uint8_t flag_C ){

	uint8_t masque4bits = 15; uint16_t masque12bits = 4095;
	uint8_t I = (mot>>25)&1;
	uint8_t S = (mot>>20)&1;
		/** Code of the current instruction **/
	uint8_t opCode 	= (uint8_t) ((mot >> 21) & masque4bits);
	/** Registers addresses **/
	uint8_t Rsource = (uint8_t) ((mot >> 16) & masque4bits);
	uint8_t Rdest 	= (uint8_t) ((mot >> 12) & masque4bits);
	uint16_t shifter_operand = (uint16_t) (mot & masque12bits);

	/** Update of the shifter_operand if not immediate value **/
	uint8_t shifter_carry_out = update_shifter_operand(p, Rdest, &shifter_operand, I, flag_C);

	/** Selecion of the instruction encoded in opCode **/
	switch (opCode) {
		case 0:
			return and_instr(p, Rsource, Rdest, shifter_operand, shifter_carry_out, S);
			break;
		case 1:
			return xor_instr(p, Rsource, Rdest, shifter_operand, shifter_carry_out, S);
			break;
		case 2:
			return sub_instr(p, Rsource, Rdest, shifter_operand, S);
			break;
		case 3:
			return rsb_instr(p, Rsource, Rdest, shifter_operand, S);
			break;
		case 4:
			return add_instr(p, Rsource, Rdest, shifter_operand, S);
			break;
		case 5:
			return adc_instr(p, Rsource, Rdest, shifter_operand, flag_C, S);
			break;
		case 6:
			return sbc_instr(p, Rsource, Rdest, shifter_operand, flag_C, S);
			break;
		case 7:
			return rsc_instr(p, Rsource, Rdest, shifter_operand, flag_C, S);
			break;
		// TEST & COMPARE
		case 8:
			return tst_instr(p, Rsource, shifter_operand, shifter_carry_out);
			break;
		case 9:
			return teq_instr(p, Rsource, shifter_operand, shifter_carry_out);
			break;
		case 10:
			return cmp_instr(p, Rsource, shifter_operand);
			break;
		case 11:
			return cmn_instr(p, Rsource, shifter_operand);
			break;
		case 12:
			return orr_instr(p, Rsource, Rdest, shifter_operand, shifter_carry_out, S);
			break;
		case 13:
			return mov_instr(p, Rdest, shifter_operand, shifter_carry_out, S);
			break;
		case 14:
			return bic_instr(p, Rsource, Rdest, shifter_operand, shifter_carry_out, S);
			break;
		case 15:
			return mvn_instr(p, Rdest, shifter_operand, shifter_carry_out, S);
			break;

		default:
			printf("ERREUR: Mauvaise instruction \n");
	}
	return 1;
}

int traitement_LS(){return 0;}

int traitement_BR(arm_core p, uint32_t mot){
	arm_branch(p,mot);


	return 0;
}

int traitement_SPE(){ return 0;}















int select_execute_instruction(arm_core p, uint32_t mot, uint8_t flag_N, uint8_t flag_Z, uint8_t flag_V, uint8_t flag_C) {

	uint8_t typeop = (mot >>26)&3;
	switch (typeop){
		case 0: return traitement_AR(p, mot, flag_C);break;
		case 1: return traitement_LS();break;
		case 2: return traitement_BR(p, mot);break;
		case 3: return traitement_SPE();break;
	}

	return 1;
}

static int arm_execute_instruction(arm_core p) {
	uint32_t mot = arm_read_cpsr(p);
	arm_fetch(p, &mot);

	/** Condition of execution of the instruction **/
	uint8_t cond = (uint8_t) ((mot >> 28) & 15);

	/** FLAGS **/
	uint32_t cpsr = arm_read_cpsr(p);

	uint8_t flag_N = (cpsr >> N) & 1;
	uint8_t flag_Z = (cpsr >> Z) & 1;
	uint8_t flag_V = (cpsr >> V) & 1;
	uint8_t flag_C = (cpsr >> C) & 1;

	if (check_cond_failed(flag_N, flag_Z, flag_V, flag_C, cond)) {
		return 1;
	}

	return select_execute_instruction(p, mot, flag_N, flag_Z, flag_V, flag_C);
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result)
        arm_exception(p, result);
    return result;
}
