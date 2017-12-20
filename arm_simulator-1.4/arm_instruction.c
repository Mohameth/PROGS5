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
/** INSTRUCTIONS TODO: Move functions below to instruction.c **/

int and_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
	uint32_t value = arm_read_usr_register(p, Rsource) & shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	return 1;
}
int xor_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
	uint32_t value = arm_read_usr_register(p, Rsource) ^ shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	return 1;
}
int sub_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
	uint32_t value = arm_read_usr_register(p, Rsource) - shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	return 1;
}
int rsb_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource);
	arm_write_usr_register(p, Rdest, value);

	return 1;
}
int add_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
	uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	return 1;
}
int adc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C) {
	uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand + flag_C;
	arm_write_usr_register(p, Rdest, value);

	return 1;
}

int sbc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C) {
	uint32_t value = arm_read_usr_register(p, Rsource)- shifter_operand - ~(flag_C);
	arm_write_usr_register(p, Rdest, value);

	return 1;
}

int rsc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C) {
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource) - ~(flag_C);
	arm_write_usr_register(p, Rdest, value);

	return 1;
}
int orr_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {return 0;}
int mov_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand) {
	arm_write_usr_register(p, Rdest, shifter_operand);

	return 1;
}
int bic_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {return 0;}
int mvn_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand) {
	arm_write_usr_register(p, Rdest, ~shifter_operand);

	return 1;
}


int select_execute_instruction(arm_core p, uint32_t mot, uint8_t flag_N, uint8_t flag_Z, uint8_t flag_V, uint8_t flag_C) {

	/** MASQUES **/
	uint8_t masque4bits = 15; uint16_t masque12bits = 4095;

	/** Code of the current instruction **/
	uint8_t opCode 	= (uint8_t) ((mot >> 21) & masque4bits);

	/** Registers addresses **/
	uint8_t Rsource = (uint8_t) ((mot >> 16) & masque4bits);
	uint8_t Rdest 	= (uint8_t) ((mot >> 12) & masque4bits);
	uint16_t shifter_operand = (uint16_t) (mot & masque12bits);

	/** Selecion of the instruction encoded in opCode **/
	switch (opCode) {
		case 0:
			return and_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 1:
			return xor_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 2:
			return sub_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 3:
			return rsb_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 4:
			return add_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 5:
			return adc_instr(p, Rsource, Rdest, shifter_operand, flag_C);
			break;
		case 6:
			return sbc_instr(p, Rsource, Rdest, shifter_operand, flag_C);
			break;
		case 7:
			return rsc_instr(p, Rsource, Rdest, shifter_operand, flag_C);
			break;
		/* TEST & COMPARE
		case 8:
			return tst_instr(p, Rsource, Rdest, shifter_operand, mot);//Flags will be updated
			break;
		case 9:
			return teq_instr(p, Rsource, Rdest, shifter_operand, mot);//Flags will be updated
			break;
		case 10:
			return cmp_instr(p, Rsource, Rdest, shifter_operand, mot);//Flags will be updated
			break;
		case 11:
			return cmn_instr(p, Rsource, Rdest, shifter_operand, mot);//Flags will be updated
			break;
		*/
		case 12:
			return orr_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 13:
			return mov_instr(p, Rdest, shifter_operand);
			break;
		case 14:
			return bic_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 15:
			return mvn_instr(p, Rdest, shifter_operand);
			break;

		default:
			printf("ERREUR: Mauvaise instruction \n");
	}

	return 0;
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
		return 0;
	}

	return select_execute_instruction(p, mot, flag_N, flag_Z, flag_V, flag_C);
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (!result)
        arm_exception(p, result);
    return result;
}
