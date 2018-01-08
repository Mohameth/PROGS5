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
	res = res & masque;		//forcage a� 0 du bit numbit

	return res | (set_bit << num_bit);
}

/**
* met � jour les flags Z et N. C et V restent a modifier en local (car variables)
* parametres : Rdest est le registre de destination, S est l'indicateur de mise a jour des flags, flag_C et flag_V sont les flags C et V actuels.
* retour : aucun
* effet de bord : cpsr mis a jour
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

/**
* Calcule s'il y a un borrow
* param : operande1 et operande2, les deux operandes
* return 1 ssi il y a borrow, 0 sinon
* effet de bord : aucun
*/

uint8_t borrowFrom(uint32_t operande1, uint32_t operande2) {
	return operande1 < operande2;
}

/**
* Calcule s'il y a un carry
* param : operande1 et operande2, les deux operandes
* return 1 ssi il y a carry, 0 sinon
* effet de bord : aucun
*/


uint8_t carryFrom(uint32_t operande1, uint32_t operande2) {
	uint32_t max = ~0;
	int temp = max-operande1;
		
	return operande2>temp;
}

/**
* Calcule s'il y a un overflow
* param : operande1 et operande2 les deux operandes, result le resultat constate et operation le type d'operation : 1 pour l'adition 0 pour la soustraction. 
* return 1 ssi il y a overflow, 0 sinon
* effet de bord : aucun
*/

uint8_t overflowFrom(uint32_t operande1, uint32_t operande2, uint32_t result, uint8_t operation) {
	if (operation == 1) { //Addition
		return (((operande1 >> 31) == (operande2 >> 31)) && (operande1 >> 31) == result >> 31); 	} else { //Soustraction
		return ((operande1 >> 31) != (operande2 >> 31)) && ((operande1 >> 31) != (result >> 31));
	}
}

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
	//flag_V=OverflowFrom(Rn - shifter_operand)
	update_flags(p, S, Rdest, ~borrowFrom(arm_read_register(p, Rsource), shifter_operand), overflowFrom(arm_read_register(p, Rsource), shifter_operand, value, 0));

  return 0;
}
int rsb_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S) {
  //Doc. p256
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource);
	arm_write_usr_register(p, Rdest, value);

	//NOT BorrowFrom(shifter_operand - Rn) --> NOT Rn > shifter_operand
	//flag_V=OverflowFrom(shifter_operand - Rn)
	update_flags(p, S, Rdest, ~borrowFrom(shifter_operand, arm_read_register(p, Rsource)), overflowFrom(shifter_operand, arm_read_register(p, Rsource), value, 0));

	return 0;
}
int add_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S) {
  //Doc. p156
	uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	//flag_C=CarryFrom(Rn + shifter_operand)
	//flag_V=OverflowFrom(Rn + shifter_operand)
	update_flags(p, S, Rdest, carryFrom(arm_read_register(p, Rsource), shifter_operand), overflowFrom(shifter_operand, arm_read_register(p, Rsource), value, 1));

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

	//flag_C=CarryFrom(Rn + shifter_operand + flag_C)
	//flag_V=OverflowFrom(Rn + shifter_operand + flag_C)
	update_flags(p, S, Rdest, carryFrom(arm_read_register(p, Rsource), shifter_operand + flag_C), overflowFrom(shifter_operand + flag_C, arm_read_register(p, Rsource), value, 1));

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

	//flag_C = NOT BorrowFrom(Rn - shifter_operand - NOT(flag_C))
	//flag_V = OverflowFrom(Rn - shifter_operand - NOT(flag_C))

	update_flags(p, S, Rdest, ~(borrowFrom(arm_read_register(p, Rsource), shifter_operand - ~flag_C)), overflowFrom(arm_read_register(p, Rsource), shifter_operand - ~flag_C, value, 0));

	return 0;
}

//
int rsc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S) {
  //Doc. p267
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource) - ~(flag_C);
	arm_write_usr_register(p, Rdest, value);

	//flag_C = NOT BorrowFrom(shifter_operand - Rn - NOT(flag_C))
	//flag_V = OverflowFrom(shifter_operand - Rn - NOT(flag_C))
	update_flags(p, S, Rdest, ~(borrowFrom(shifter_operand - ~flag_C, arm_read_register(p, Rsource))), overflowFrom(shifter_operand - ~flag_C, arm_read_register(p, Rsource), value, 0));

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

