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
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"

int arm_load_store(arm_core p, uint32_t ins) {
    	uint32_t RN_adr;
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
	uint8_t *byte;
	uint32_t *mot;
	int boolean;
	uint8_t shift_T = (uint8_t) ((ins >> 5) & 3);
	uint8_t shift_im = (uint8_t) ((ins >> 7) & 31);
	uint8_t RM_bit = (uint8_t) ((ins) & 31);
	uint32_t offset_reg;
	if(L_bit){

		if(I_bit){
			switch(shift_T){

				case 0 :  

					offset_reg = arm_read_usr_register(p,RM_bit)<<shift_im;
					if(U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;				
					
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;	
					}
					if(B_bit){
						boolean = arm_read_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_read_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);



				case 1 :

					offset_reg = arm_read_usr_register(p,RM_bit)>>shift_im;
					if(U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;				
					
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;	
					}
					if(B_bit){
						boolean = arm_read_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_read_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);




				case 2 :

					/*offset_reg = arm_read_usr_register(p,RM_bit)>>shift_im;*/

					offset_reg= asr(offset_reg, shift_im);	
					if(U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;				
					
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;	
					}
					if(B_bit){
						boolean = arm_read_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_read_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);



				case 3 :

					offset_reg=arm_read_usr_register(p,RM_bit);
					/*for(int i=0;i<shift_im;i++;){
						if(offset_reg % 2 == 0){
							offset_reg = offset_reg >> 1;
						}else{
							offset_reg = offset_reg >> 1;
							offset_reg = offset_reg + (1<<32);
						}}*/


							offset_reg= ror( offset_reg, shift_im);



					if (U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;
					}if(B_bit){
						boolean = arm_read_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_read_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);
					
			}
		}else{ 
			if(P_bit){
			
				if(U_bit){
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr + offset;				
				
				}else{
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr - offset;	
				}
				if(B_bit){
					boolean = arm_read_byte(p, temp, byte);
					&mot=&byte
					arm_write_usr_register(p, RD_bit, &mot);
				}else{
					boolean = arm_read_word(p, temp, mot);
					arm_write_usr_register(p, RD_bit, &mot);
				}
				if(W_bit){
					arm_write_usr_register(p, RN_bit, temp);
				}

			}
			else{
				if(U_bit){
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr + offset;				
				
				}else{
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr - offset;	
				}
				if(B_bit){
					boolean = arm_read_byte(p, RN_adr, byte);
					&mot=&byte
					arm_write_usr_register(p, RD_bit, &mot);
				}else{
					boolean = arm_read_word(p, RN_adr, mot);
					arm_write_usr_register(p, RD_bit, &mot);
				}
				arm_write_usr_register(p, RN_bit, temp);
			}	

		}




	}else{

		if(I_bit){
			switch(shift_T){

				case 0 :  

					offset_reg = arm_read_usr_register(p,RM_bit) << shift_im;
					if(U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;				
					
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;	
					}
					if(B_bit){
						boolean = arm_read_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_read_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);



				case 1 :

					offset_reg = arm_read_usr_register(p,RM_bit)>>shift_im;
					if(U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;				
					
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;	
					}
					if(B_bit){
						boolean = arm_write_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_write_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);




				case 2 :

					offset_reg = arm_read_usr_register(p,RM_bit)>>shift_im;
					if(U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;				
					
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;	
					}
					if(B_bit){
						boolean = arm_write_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_write_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);



				case 3 :

					offset_reg=arm_read_usr_register(p,RM_bit);
					for(int i=0;i<shift_im;i++;){
						if(offset_reg % 2 == 0){
							offset_reg = offset_reg >> 1;
						}else{
							offset_reg = offset_reg >> 1;
							offset_reg = offset_reg + (1<<32);
						}}
					if (U_bit){
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr + offset_reg;
					}else{
						RN_adr=arm_read_usr_register(p,RN_bit);
						temp = RN_adr - offset_reg;
					}if(B_bit){
						boolean = arm_write_byte(p, temp, byte);
						&mot=&byte
						arm_write_usr_register(p, RD_bit, &mot);
					}else{
						boolean = arm_write_word(p, temp, mot);
						arm_write_usr_register(p, RD_bit, &mot);
					}
					arm_write_usr_register(p, RN_bit, temp);
					
			}
		}else{ 
			if(P_bit){
			
				if(U_bit){
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr + offset;				
				
				}else{
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr - offset;	
				}
				if(B_bit){
					boolean = arm_write_byte(p, temp, byte);
					&mot=&byte
					arm_write_usr_register(p, RD_bit, &mot);
				}else{
					boolean =arm_write_word(p, temp, mot);
					arm_write_usr_register(p, RD_bit, &mot);
				}
				if(W_bit){
					arm_write_usr_register(p, RN_bit, temp);
				}

			}
			else{
				if(U_bit){
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr + offset;				
				
				}else{
					RN_adr=arm_read_usr_register(p,RN_bit);
					temp = RN_adr - offset;	
				}
				if(B_bit){
					boolean = arm_write_byte(p, RN_adr, byte);
					&mot=&byte
					arm_write_usr_register(p, RD_bit, &mot);
				}else{
					boolean = arm_write_word(p, RN_adr, mot);
					arm_write_usr_register(p, RD_bit, &mot);
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
int arm_load_store_multiple(arm_core p, uint32_t ins) {

	uint8_t 23_24_bit = (uint8_t) ((ins >> 23) & 1);	
	uint16_t register_list=(uint8_t) ((ins) & 65536);
	uint8_t RN_bit = (uint8_t) ((ins >> 16) & 15);
	uint8_t Cond_bit = (uint8_t) ((ins >> 28) & 15);
	uint8_t S_bit = (uint8_t) ((ins >> 22) & 1);
	uint8_t L_bit = (uint8_t) ((ins >> 20) & 1);
	uint8_t W_bit = (uint8_t) ((ins >> 21) & 1);
	uint32_t start_address;
	uint32_t end_address;
	switch(23_24_bit){
		case 0:/*Decrement after*/
			if(L_bit){
				start_address =arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4)+4;
				end_address = arm_read_usr_register(p,RN_bit);
				if(W_bit){
					RN_bit= arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4);	
				}
			}else{
			
			}
		case 1:/*Increment after*/
			if(L_bit){
				start_address =arm_read_usr_register(p,RN_bit);
				end_address =  arm_read_usr_register(p,RN_bit)+(number_of_registers(register_list)*4)-4;
				if(W_bit){
					RN_bit= arm_read_usr_register(p,RN_bit)+(number_of_registers(register_list)*4);	
				}
			}else{
			
			}

		case 2:/*Decrement before*/
			if(L_bit){
				start_address =arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4);
				end_address = arm_read_usr_register(p,RN_bit)-4;
				if(W_bit){
					RN_bit= arm_read_usr_register(p,RN_bit)-(number_of_registers(register_list)*4);	
				}
			}else{
			
			}

			case 3:/*Increment before*/
			if(L_bit){
				start_address =arm_read_usr_register(p,RN_bit)+4;
				end_address = arm_read_usr_register(p,RN_bit)+(number_of_registers(register_list)*4);
				if(W_bit){
					RN_bit= arm_read_usr_register(p,RN_bit)+(number_of_registers(register_list)*4);	
				}
			}else{
			
			}

	}


	return 0;

}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
