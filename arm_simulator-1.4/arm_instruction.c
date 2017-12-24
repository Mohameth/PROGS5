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
*
* MAJ des flags
*
*/


/**
* met a jour un bit d'une suite de bit donnee
* parametres : res est l'ensemble de bits consideres, num_bit est le numero du bit a modifier, set_bit est la nouvelle valeure a ajouter.
* retourne l'ensemble de bits
* effets de bords : aucun
*/
uint32_t set_1_bit(uint32_t res, uint8_t num_bit, uint8_t set_bit) {
	uint32_t masque = 1;
	masque = ~(masque << num_bit);
	res = res & masque;		//forcage à 0 du bit numbit

	return res | (set_bit << num_bit);
}

/**
*
*
*
*/
void update_flags(arm_core p, uint8_t Rdest, uint8_t S, uint8_t flag_C, uint8_t flag_V) {
	if ((S == 1) && (Rdest == 15)) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}
	} else if (S == 1) {
		/*uint8_t	new_flag_N = (arm_read_usr_register(p, Rdest) >> 31) & 1;
		uit8_t	new_flag_Z = (arm_read_usr_register(p, Rdest) == 0);*/

		uint32_t new_CPSR = arm_read_cpsr(p);
		set_1_bit(new_CPSR, N, (arm_read_usr_register(p, Rdest) >> 31) & 1); //N Flag = Rd[31]
		set_1_bit(new_CPSR, Z, (arm_read_usr_register(p, Rdest) == 0));//Z Flag = Rd == 0
		if (flag_C != 0xFF) set_1_bit(new_CPSR, C, flag_C);
		if (flag_V != 0xFF) set_1_bit(new_CPSR, V, flag_V);

		arm_write_cpsr(p, new_CPSR);
	}
}

/** INSTRUCTIONS TODO: Move functions below to instruction.c **/

int and_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) {
	//Doc. p159
	uint32_t value = arm_read_usr_register(p, Rsource) & shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	update_flags(p, S, Rdest, shifter_carry_out, 0xFF);

	return 0;
}

int xor_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) {
	//Doc. p183
  uint32_t value = arm_read_usr_register(p, Rsource) ^ shifter_operand;
  arm_write_usr_register(p, Rdest, value);

	update_flags(p, S, Rdest, shifter_carry_out, 0xFF);

  return 0;
}
int sub_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S) {
  //Doc. p358
	uint32_t value = arm_read_usr_register(p, Rsource) - shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	//NOT BorrowFrom(Rn - shifter_operand) --> NOT shifter_operand > Rn
	//TODO: flag_V=OverflowFrom(Rn - shifter_operand)
	update_flags(p, S, Rdest, ~(shifter_operand > arm_read_register(p, Rsource)), 0xFF);

  return 0;
}
int rsb_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S) {
  //Doc. p256
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource);
	arm_write_usr_register(p, Rdest, value);

	//NOT BorrowFrom(shifter_operand - Rn) --> NOT Rn > shifter_operand
	//TODO: flag_V=OverflowFrom(shifter_operand - Rn)
	update_flags(p, S, Rdest, ~(arm_read_register(p, Rsource) > shifter_operand), 0xFF);

	return 0;
}
int add_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S) {
  //Doc. p156
	uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	//TODO: flag_C=CarryFrom(Rn + shifter_operand)
	//TODO: flag_V=OverflowFrom(Rn + shifter_operand)
	update_flags(p, S, Rdest, 0xFF, 0xFF);

	return 0;
}
int adc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S) {
  //Doc. p154
  uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand + flag_C;
	arm_write_usr_register(p, Rdest, value);

  //Test du carry flag avec r2 = shifter_operand
	/*if (r2 < 4294967295 - so) { // if (r2>2^32-1)
		//uint32_t flag_C  = 1 >> C;
		cpsr = cpsr | (1 << C);
		arm_write_cpsr(p, cpsr);
	} else {
		cpsr = cpsr & ~(1 << C);
		arm_write_cpsr(p, cpsr);
	}*/

	//TODO: flag_C=CarryFrom(Rn + shifter_operand + flag_C)
	//TODO: flag_V=OverflowFrom(Rn + shifter_operand + flag_C)
	update_flags(p, S, Rdest, 0xFF, 0xFF);

	return 0;
}


/**
* soustraction avec flag carry (c)
* arguments : p l'armcore considere, le registre source, le registre destination, l'operande shift, la valeur du flag c, la valeur du flag s.
* retourne : 0 SSI l'operation s'est bien passee, 1 sinon.
* effets de bord : si s==1 alors il y a maj des flags ZNCV.
*
**/
int sbc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S) {
	//Doc p.275
  uint32_t value = arm_read_usr_register(p, Rsource) - shifter_operand - ~(flag_C);
	arm_write_usr_register(p, Rdest, value);

	//TODO: flag_C = NOT BorrowFrom(Rn - shifter_operand - NOT(flag_C))
	//TODO: flag_V = OverflowFrom(Rn - shifter_operand - NOT(flag_C))
	/*uint32_t max32bits = 0xFFFFFFFF;
	uint8_t	new_flag_V = max32bits < (arm_read_register(p, Rdest) + ) & 1;//Attention, à revoir*/
	update_flags(p, S, Rdest, 0xFF, 0xFF);

	return 0;
}

//
int rsc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S) {
  //Doc. p267
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource) - ~(flag_C);
	arm_write_usr_register(p, Rdest, value);

	//TODO: flag_C = NOT BorrowFrom(shifter_operand - Rn - NOT(flag_C))
	//TODO: flag_V = OverflowFrom(shifter_operand - Rn - NOT(flag_C))
	update_flags(p, S, Rdest, 0xFF, 0xFF);

	return 0;
}

//
int orr_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) {
  //Doc. p235
  uint32_t value = arm_read_usr_register(p, Rsource) | shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	update_flags(p, S, Rdest, shifter_carry_out, 0xFF);

  return 0;
}

int mov_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) {
  //Doc. p218
	uint32_t value = shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	//flag_C = value - (4294967295) > 0; (value>2^32-1)
	update_flags(p, S, Rdest, shifter_carry_out, 0xFF);

	return 0;
}

//
int bic_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) {
	//Doc. p162
	uint32_t value = arm_read_usr_register(p, Rsource) & ~shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	update_flags(p, S, Rdest, shifter_carry_out, 0xFF);

	return 0;
}

int mvn_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) {
	//Doc. p232
	uint32_t value = ~shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	//flag_C = value - (4294967295) > 0; (value>2^32-1)
	update_flags(p, S, Rdest, shifter_carry_out, 0xFF);

	return 0;
}

/**
 * Maj du shifter operand selon le flag I (Immediate) --> Doc p. 446
 * Parametres : pointeur sur le shifter_operand, flags Carry et Immediate
 * Retourne : shifter_carry_out indiquant
 * Effet de bord : Modifie shifter_operand
 **/
uint8_t update_shifter_operand(arm_core p, uint16_t *shifter_operand, uint8_t I, uint8_t flag_C) {
	uint8_t shifter_carry_out;

	if (~I) {
		*shifter_operand = arm_read_usr_register(p, *shifter_operand);
		shifter_carry_out = flag_C;
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
	uint8_t shifter_carry_out = update_shifter_operand(p, &shifter_operand, I, flag_C);

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
