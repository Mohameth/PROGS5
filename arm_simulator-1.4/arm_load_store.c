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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"


int arm_load_store(arm_core p, uint32_t ins) {
	//L_bit Distinguishes between a Load (L==1) and a Store instruction (L==0).
	//B_bit Distinguishes between an unsigned byte (B==1) and a word (B==0) access
	//I, P, U, W Are bits that distinguish between different types of <addressing_mode>
	//TODO: T_BIT

	uint32_t temp;
	uint8_t P_bit = (uint8_t) ((ins >> 24) & 1);
	uint8_t U_bit = (uint8_t) ((ins >> 23) & 1);
	uint8_t W_bit = (uint8_t) ((ins >> 21) & 1);
	uint8_t I_bit = (uint8_t) ((ins >> 25) & 1);
	uint8_t L_bit = (uint8_t) ((ins >> 20) & 1);
	uint8_t B_bit = (uint8_t) ((ins >> 22) & 15);
	uint8_t RN_bit = (uint8_t) ((ins >> 16) & 15);
	uint8_t RD_bit = (uint8_t) ((ins >> 12) & 1);
	uint16_t offset = (uint16_t) ((ins) & 4095);/*12^2-1*/
	uint8_t shift_T = (uint8_t) ((ins >> 5) & 3);
	uint8_t shift_im = (uint8_t) ((ins >> 7) & 31);
	uint8_t RM_bit = (uint8_t) ((ins) & 31);
	uint32_t offset_reg;

	uint32_t RN_adr = arm_read_usr_register(p,RN_bit);

	if(L_bit){//load page 194
		if(I_bit){
			switch(shift_T){//page 460

				case 0 :
					offset_reg = arm_read_usr_register(p,RM_bit)<<shift_im;
					if(U_bit){
						temp = RN_adr + offset_reg;
					}else{
						temp = RN_adr - offset_reg;
					}

					if(B_bit){
						uint8_t choco;
						arm_read_byte(p, temp, &choco);
						arm_write_usr_register(p, RD_bit, choco);
					}else{
						uint32_t choco;
						arm_read_word(p, temp, &choco);
					if(RD_bit ==15){
						arm_write_usr_register(p,15 ,choco & 0xFFFFFFFE);
					}else{
						arm_write_usr_register(p, RD_bit, choco);
					}
				}
					arm_write_usr_register(p, RN_bit, temp);
					break;

				case 1 :
					offset_reg = arm_read_usr_register(p,RM_bit)>>shift_im;
						if(U_bit){
							temp = RN_adr + offset_reg;
						}else{
							temp = RN_adr - offset_reg;
						}

					if(B_bit){
						uint8_t choco;
						arm_read_byte(p, temp, &choco);
						arm_write_usr_register(p, RD_bit, choco);
					}else{
						uint32_t choco;
						arm_read_word(p, temp, &choco);

					if(RD_bit ==15){
						arm_write_usr_register(p,15 ,choco & 0xFFFFFFFE);
					}else{
						arm_write_usr_register(p, RD_bit, choco);
					}
				}

					arm_write_usr_register(p, RN_bit, temp);
					break;

				case 2 :
					offset_reg= asr(RM_bit, shift_im);
					if(U_bit){
						temp = RN_adr + offset_reg;
					}else{
						temp = RN_adr - offset_reg;
					}

					if(B_bit){
						uint8_t choco;
						arm_read_byte(p, temp, &choco);
						arm_write_usr_register(p, RD_bit, choco);
					}else{
						uint32_t choco;
						arm_read_word(p, temp, &choco);

						if(RD_bit ==15){
							arm_write_usr_register(p,15 ,choco & 0xFFFFFFFE);
						}else{
							arm_write_usr_register(p, RD_bit, choco);
						}
					}

					arm_write_usr_register(p, RN_bit, temp);
					break;

				case 3 :

					offset_reg=arm_read_usr_register(p,RM_bit);
					offset_reg= ror( RM_bit, shift_im);
					if(U_bit){
						temp = RN_adr + offset_reg;
					}else{
						temp = RN_adr - offset_reg;
					}

					if(B_bit) {
						uint8_t choco;
						arm_read_byte(p, temp, &choco);
						arm_write_usr_register(p, RD_bit, choco);
					} else {
						uint32_t choco;
						arm_read_word(p, temp, &choco);
					if(RD_bit ==15) {
						arm_write_usr_register(p,15 ,choco & 0xFFFFFFFE);
					} else {
						arm_write_usr_register(p, RD_bit, choco);
					}
					arm_write_usr_register(p, RN_bit, temp);
			}
		}
		}else{

			if(P_bit){
				if(U_bit){
					temp = RN_adr + offset;
				}else{
					temp = RN_adr - offset;
				}
				if(B_bit){
					uint8_t choco;
					arm_read_byte(p, temp, &choco);
					arm_write_usr_register(p, RD_bit, choco);
				}else{
					uint32_t choco;
					arm_read_word(p, temp, &choco);

					if(RD_bit ==15){
						arm_write_usr_register(p,15 ,choco & 0xFFFFFFFE);
					}else{
						arm_write_usr_register(p, RD_bit, choco);
					}
				}
					if(W_bit){
					arm_write_usr_register(p, RN_bit, temp);
				}
			}else{
				if(U_bit){
					temp = RN_adr + offset;
				}else{
					temp = RN_adr - offset;
				}
				if(B_bit){
					uint8_t choco;
					arm_read_byte(p, temp, &choco);
					arm_write_usr_register(p, RD_bit, choco);
				}else{
					uint32_t choco;
					arm_read_word(p, temp, &choco);

					if(RD_bit ==15){
						arm_write_usr_register(p,15 ,choco & 0xFFFFFFFE);
					}else{
						arm_write_usr_register(p, RD_bit, choco);
					}
				}
					arm_write_usr_register(p, RN_bit, temp);
			}

		}

	}else {//store page 345

		if(I_bit){

			switch(shift_T){
				case 0: ;
				case 1: ;
				case 2: ;
				case 3: ;
			}

		}else{
			if(P_bit){
				if(U_bit){
					temp = RN_adr + offset;
				}else{
					temp = RN_adr - offset;
				}
				if(B_bit){
					uint8_t choco = arm_read_usr_register(p, RD_bit) & 0xFF;
					arm_write_byte(p, temp, choco);
				}else{
					uint32_t choco = arm_read_usr_register(p, RD_bit);
					arm_write_usr_register(p, temp, choco);
				}

				if(W_bit){
					arm_write_usr_register(p, RN_bit, temp);
				}
			}else{
				if(U_bit){
					temp = RN_adr + offset;
				}else{
					temp = RN_adr - offset;
				}
				if(B_bit){
					uint8_t choco = arm_read_usr_register(p, RD_bit) & 0xFF;
					arm_write_byte(p, temp, choco);
				}else{
					uint32_t choco = arm_read_usr_register(p, RD_bit);
					arm_write_usr_register(p, temp, choco);
				}
					arm_write_usr_register(p, RN_bit, temp);
			}
		}
	}
	return 0;
}


int number_of_registers(uint16_t bit){
	int i,cpt=0;
	for(i=1;i<16;i++){
		if((bit & 1)%2 == 0){
			bit= bit >> 1;
		}else{
			cpt++;
			bit= bit >> 1;

		}
	}
	return cpt;


}

void init_addresses(arm_core p, uint32_t * start_address, uint32_t * end_address, uint8_t bit_23, uint8_t RN_bit, uint16_t register_list) {
	//doc page
	switch(bit_23){
		case 0:/*Decrement	ent after*/
			*start_address =arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4)+4;
			*end_address = arm_read_usr_register(p,RN_bit);
			break;
		case 1:/*Increment after*/
			*start_address =arm_read_usr_register(p,RN_bit);
			*end_address =  arm_read_usr_register(p,RN_bit)+(number_of_registers(register_list)*4)-4;
			break;
		case 2:/*Decrement before*/
			*start_address =arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4);
			*end_address = arm_read_usr_register(p,RN_bit)-4;
			break;
		case 3:/*Increment before*/
			*start_address =arm_read_usr_register(p,RN_bit)+4;
			*end_address = arm_read_usr_register(p,RN_bit)+(number_of_registers(register_list)*4);
			break;

		}
}


int arm_load_store_multiple(arm_core p, uint32_t ins) {//page 143
	//TODO: T_BIT
	uint8_t bit_23 = (uint8_t) ((ins >> 23) & 2);
	uint16_t register_list=(uint8_t) ((ins) & 65536);
	uint8_t RN_bit = (uint8_t) ((ins >> 16) & 15);
	//uint8_t Cond_bit = (uint8_t) ((ins >> 28) & 15);
	//uint8_t S_bit = (uint8_t) ((ins >> 22) & 1);
	uint8_t L_bit = (uint8_t) ((ins >> 20) & 1);
	uint8_t W_bit = (uint8_t) ((ins >> 21) & 1);
	uint8_t RD_bit = (uint8_t) ((ins >> 12) & 1);

	uint32_t start_address;
	uint32_t end_address;
	uint32_t address;
	uint32_t value;

	init_addresses(p, &start_address, &end_address, bit_23, RN_bit, register_list);
	if(W_bit){
		RN_bit= arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4);
	}

	if(L_bit){//load page
		address=start_address;
		for (int i=0;i<15;i++) {
			if((register_list >> i) & 1) {
				arm_write_usr_register(p, arm_read_usr_register(p, i), arm_read_word(p, address, &value));
				address = address + 4;
			}
		}
		if((register_list >> 15) & 1){
			arm_write_usr_register(p, value, arm_read_word(p,address,&value));
			arm_write_usr_register(p, 15, value & 0xFFFFFFFE);
		}
		if(end_address!=address-4){
			return 1;
		}

	} else {//store page
		address=start_address;
		for (int i=0;i<15;i++){
			if((register_list >> i) & 1){
				arm_write_word(p, value, arm_read_usr_register(p, arm_read_usr_register(p, RD_bit)));
				address = address + 4;
			}
		}
	}
		if(end_address!=address-4){
			return 1;
		}
	return 0;

}



int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
