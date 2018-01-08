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
#ifndef __ARM_DATA_PROCESSING_H__
#define __ARM_DATA_PROCESSING_H__
#include <stdint.h>
#include "arm_core.h"

int arm_data_processing_shift(arm_core p, uint32_t ins);
int arm_data_processing_immediate_msr(arm_core p, uint32_t ins);

uint32_t set_1_bit(uint32_t res, uint8_t num_bit, uint8_t set_bit);
void update_flags(arm_core p, uint8_t Rdest, uint8_t S, uint8_t flag_C, uint8_t flag_V);
uint8_t borrowFrom(uint32_t operande1, uint32_t operande2);
uint8_t carryFrom(uint32_t operande1, uint32_t operande2);
uint8_t overflowFrom(uint32_t operande1, uint32_t operande2, uint32_t result, uint8_t operation);


int and_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S);
int xor_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) ;
int sub_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S);
int rsb_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S);
int add_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t S);
int adc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S);
int sbc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S);
int rsc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t S);
int orr_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S);
int mov_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S);
int bic_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S) ;
int mvn_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand, uint8_t shifter_carry_out, uint8_t S);

#endif
