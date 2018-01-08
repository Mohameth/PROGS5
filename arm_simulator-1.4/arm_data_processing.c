/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bâtiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hères
*/
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
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
	res = res & masque;		//forcage Ã  0 du bit numbit

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
	uint8_t	new_flag_V = max32bits < (arm_read_register(p, Rdest) + ) & 1;//Attention, Ã  revoir*/
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

