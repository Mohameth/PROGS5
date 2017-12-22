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
* met � jour un bit d'une suite de bit donnee
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

//uint32_t maj_flags(int Z, int N, int C){
	// tester chaque possibilite
//}

/*
* Maj du flag Z
* parametres : p est l'armcore considere, cpsr est l'ensemble de bits consideres, Rdest est le registre qui contient la valeure nouvellement ajoutee (sur laquelle sera evaluee le flag)
* retourne : le nouvel ensemble de flags
* effet de bords : aucun
*/

uint32_t maj_Z(arm_core p, uint32_t cpsr, uint8_t Rdest){

	return set_1_bit(cpsr, Z, (arm_read_usr_register(p, Rdest)==0));
}

/*
* Maj du flag N
* parametres : p est l'armcore considere, cpsr est l'ensemble de bits consideres, Rdest est le registre qui contient la valeure nouvellement ajoutee (sur laquelle sera evaluee le flag)
* retourne : le nouvel ensemble de flags
* effet de bords : aucun
*/

uint32_t maj_N(arm_core p, uint32_t cpsr, uint8_t Rdest){

	return set_1_bit(cpsr, N, (arm_read_register(p, Rdest) >> 31) & 1);
}

/*
* Maj du flag C
* parametres : p est l'armcore considere, cpsr est l'ensemble de bits consideres, Rdest est le registre qui contient la valeure nouvellement ajoutee (sur laquelle sera evaluee le flag), shifter_operand est la deuxieme operand, flag_C est l'actuel flag C.
* retourne : le nouvel ensemble de flags
* effet de bords : aucun
* auteurs : Corentin, Mohameth
*/

uint32_t maj_C(arm_core p, uint32_t cpsr, uint8_t Rdest, uint32_t shifter_operand, uint8_t flag_C){

	return set_1_bit(cpsr, C, (arm_read_register(p, Rsource)-shifter_operand-~(flag_C) >= 0));
}


/** INSTRUCTIONS TODO: Move functions below to instruction.c **/

int and_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t I, uint8_t S) {
	if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            uint8_t shifter_carry_out;
            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }

	uint32_t value = arm_read_usr_register(p, Rsource) & shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	if ((S == 1) && (Rdest == 15)) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}
	} else if (S == 1) {
		uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
		uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
		uint8_t	new_flag_C = shifter_carry_out;


		uint32_t new_CPSR = arm_read_cpsr(p);
		set_1_bit(new_CPSR, N, new_flag_N);
		set_1_bit(new_CPSR, Z, new_flag_Z);
		set_1_bit(new_CPSR, C, new_flag_C);
		//V UNAFFECTED

		arm_write_cpsr(p, new_CPSR);
	}

	return 0;
}

int xor_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
            if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
            else {
                uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
                shifter_operand = ror(shifter_operand & 0xFF, ri*2);

                uint8_t shifter_carry_out;
                if (ri == 0) {
                    shifter_carry_out = flag_C;
                } else {
                    shifter_carry_out = (shifter_operand >> 31) & 1;
                }
            }

            uint32_t value = arm_read_usr_register(p, Rsource) ^ shifter_operand;
            arm_write_usr_register(p, Rdest, value);

            if ((S == 1) && (Rdest == 15)) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}
            } else if (S == 1) {
                    uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
                    uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
                    uint8_t	new_flag_C = shifter_carry_out;


                    uint32_t new_CPSR = arm_read_cpsr(p);
                    set_1_bit(new_CPSR, N, new_flag_N);
                    set_1_bit(new_CPSR, Z, new_flag_Z);
                    set_1_bit(new_CPSR, C, new_flag_C);
                    //V UNAFFECTED

                    arm_write_cpsr(p, new_CPSR);
            }

            return 0;
}
int sub_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
        //Doc. p358
        if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            uint8_t shifter_carry_out;
            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }
	uint32_t value = arm_read_usr_register(p, Rsource) - shifter_operand;
	arm_write_usr_register(p, Rdest, value);

	if ((S == 1) && (Rdest == 15)) {
            if(arm_current_mode_has_spsr(p)) {
                    arm_write_cpsr(p, arm_read_spsr(p));
            }
        } else if (S == 1) {
                uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
                uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
                uint8_t	new_flag_C = (~(arm_read_register(p, Rsource)-shifter_operand >= 0)) & 1;// flag_C = NOT BorrowFrom(Rn - shifter_operand)
                //TODO: flag_V=OverflowFrom(Rn - shifter_operand)

                uint32_t new_CPSR = arm_read_cpsr(p);
                set_1_bit(new_CPSR, N, new_flag_N);
                set_1_bit(new_CPSR, Z, new_flag_Z);
                set_1_bit(new_CPSR, C, new_flag_C);
                //TODO: flag_V


                arm_write_cpsr(p, new_CPSR);
        }

        return 0;
}
int rsb_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
        //Doc. p256
        if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            uint8_t shifter_carry_out;
            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }
	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource);
	arm_write_usr_register(p, Rdest, value);

        if ((S == 1) && (Rdest == 15)) {
            if(arm_current_mode_has_spsr(p)) {
                    arm_write_cpsr(p, arm_read_spsr(p));
            }
        } else if (S == 1) {
                uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
                uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
                uint8_t	new_flag_C = (~(shifter_operand - arm_read_register(p, Rsource) >= 0)) & 1;// flag_C = NOT BorrowFrom(shifter_operand - Rn)
                //TODO: flag_V=OverflowFrom(shifter_operand - Rn)

                uint32_t new_CPSR = arm_read_cpsr(p);
                set_1_bit(new_CPSR, N, new_flag_N);
                set_1_bit(new_CPSR, Z, new_flag_Z);
                set_1_bit(new_CPSR, C, new_flag_C);
                //TODO: flag_V


                arm_write_cpsr(p, new_CPSR);
        }

	return 0;
}
int add_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t I) {
        //Doc. p156
        if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            uint8_t shifter_carry_out;
            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }
	uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand;
	arm_write_usr_register(p, Rdest, value);

        if ((S == 1) && (Rdest == 15)) {
            if(arm_current_mode_has_spsr(p)) {
                    arm_write_cpsr(p, arm_read_spsr(p));
            }
        } else if (S == 1) {
                uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
                uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);

                //TODO: flag_C=CarryFrom(Rn + shifter_operand)
                //TODO: flag_V=OverflowFrom(Rn + shifter_operand)

                uint32_t new_CPSR = arm_read_cpsr(p);
                set_1_bit(new_CPSR, N, new_flag_N);
                set_1_bit(new_CPSR, Z, new_flag_Z);
                //TODO: flag_C
                //TODO: flag_V


                arm_write_cpsr(p, new_CPSR);
        }

	return 0;
}
int adc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t I) {
        //Doc. p154
	if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            uint8_t shifter_carry_out;
            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }
        uint32_t value = arm_read_usr_register(p, Rsource) + shifter_operand + flag_C;
	arm_write_usr_register(p, Rdest, value);

	uint32_t cpsr = arm_read_cpsr(p);


        //Test du carry flag avec r2 = shifter_operand
	/*if (r2 < 4294967295 - so) { // if (r2>2^32-1)
		//uint32_t flag_C  = 1 >> C;
		cpsr = cpsr | (1 << C);
		arm_write_cpsr(p, cpsr);
	} else {
		cpsr = cpsr & ~(1 << C);
		arm_write_cpsr(p, cpsr);
	}*/

        if ((S == 1) && (Rdest == 15)) {
            if(arm_current_mode_has_spsr(p)) {
                    arm_write_cpsr(p, arm_read_spsr(p));
            }
        } else if (S == 1) {
                uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
                uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);

                //TODO: flag_C=CarryFrom(Rn + shifter_operand + flag_C)
                //TODO: flag_V=OverflowFrom(Rn + shifter_operand + flag_C)

                uint32_t new_CPSR = arm_read_cpsr(p);
                set_1_bit(new_CPSR, N, new_flag_N);
                set_1_bit(new_CPSR, Z, new_flag_Z);
                //TODO: flag_C
                //TODO: flag_V


                arm_write_cpsr(p, new_CPSR);
        }

	return 0;
}


/**
* soustraction avec flag carry (c)
* arguments : p l'armcore considere, le registre source, le registre destination, l'operande shift, la valeur du flag c, la valeur du flag s.
* retourne : 0 SSI l'operation s'est bien passee, 1 sinon.
* effets de bord : si s==1 alors il y a maj des flags ZNCV.
*
**/
int sbc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C, uint8_t I, uint8_t S) {
	//Doc p.275
        if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            //  Inutile dans sbc
            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }

        uint32_t value = arm_read_usr_register(p, Rsource) - shifter_operand - ~(flag_C);

	arm_write_usr_register(p, Rdest, value);
	if ((S == 1) && (Rdest == 15)) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}
	} else if (S == 1) {
		uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
		uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
                //TODO: flag_C = NOT BorrowFrom(Rn - shifter_operand - NOT(flag_C))
		uint8_t	new_flag_C = (arm_read_register(p, Rsource)-shifter_operand-~(flag_C) >= 0);
		/*uint32_t max32bits = 0xFFFFFFFF;
		uint8_t	new_flag_V = max32bits < (arm_read_register(p, Rdest) + ) & 1;//Attention, à revoir*/

		uint32_t new_CPSR = arm_read_cpsr(p);
		set_1_bit(new_CPSR, N, new_flag_N);
		set_1_bit(new_CPSR, Z, new_flag_Z);
		set_1_bit(new_CPSR, C, new_flag_C);
		//set_1_bit(new_CPSR, V, new_flag_V);

		arm_write_cpsr(p, new_CPSR);
	}

	return 0;
}

//
int rsc_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand, uint8_t flag_C) {
        //Doc. p267
        if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }

	uint32_t value = shifter_operand - arm_read_usr_register(p, Rsource) - ~(flag_C);
	arm_write_usr_register(p, Rdest, value);

        if ((S == 1) && (Rdest == 15)) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}
	} else if (S == 1) {
		uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
		uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
                //TODO: flag_C = NOT BorrowFrom(shifter_operand - Rn - NOT(flag_C))
		//TODO: flag_V = OverflowFrom(shifter_operand - Rn - NOT(flag_C))

		uint32_t new_CPSR = arm_read_cpsr(p);
		set_1_bit(new_CPSR, N, new_flag_N);
		set_1_bit(new_CPSR, Z, new_flag_Z);
                //TODO: flag_C
                //TODO: flag_V

		arm_write_cpsr(p, new_CPSR);
	}

	return 0;
}

//
int orr_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {
        //Doc. p235
        if (~I) shifter_operand = arm_read_usr_register(p, shifter_operand);
        else {
            uint8_t ri = shifter_operand >> 8 & 0xF;//Doc. p446
            shifter_operand = ror(shifter_operand & 0xFF, ri*2);

            if (ri == 0) {
                shifter_carry_out = flag_C;
            } else {
                shifter_carry_out = (shifter_operand >> 31) & 1;
            }
        }

        uint32_t value = arm_read_usr_register(p, Rsource) | shifter_operand;
	arm_write_usr_register(p, Rdest, value);

        if ((S == 1) && (Rdest == 15)) {
		if(arm_current_mode_has_spsr(p)) {
			arm_write_cpsr(p, arm_read_spsr(p));
		}
	} else if (S == 1) {
		uint8_t	new_flag_N = (arm_read_register(p, Rdest) >> 31) & 1;
		uint8_t	new_flag_Z = (arm_read_register(p, Rdest) == 0);
                uint8_t	new_flag_C = shifter_carry_out;

		uint32_t new_CPSR = arm_read_cpsr(p);
		set_1_bit(new_CPSR, N, new_flag_N);
		set_1_bit(new_CPSR, Z, new_flag_Z);
                set_1_bit(new_CPSR, C, new_flag_C);
                //V UNAFFECTED

		arm_write_cpsr(p, new_CPSR);
	}

        return 0;
}

int mov_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand, uint8_t I, uint8_t flag_S) {
        //Doc. p218
	uint16_t ri = shifter_operand >> 8 & 0xF;
	uint32_t so = ror(shifter_operand & 0xFF, ri*2);

	if (I) {
		arm_write_usr_register(p, Rdest, so);
	}else {
		uint32_t value;
		value = arm_read_usr_register(p,so);

		arm_write_usr_register(p, Rdest, value);
	}

	if (flag_S && Rdest) {
		if (arm_current_mode_has_spsr(p)){
			uint32_t spsr = arm_read_spsr(p);
			arm_write_cpsr(p, spsr);
		} else {
			// UNPREDICTABLE
			return 1;
		}
	} else if (flag_S == 1){
		uint32_t cpsr = arm_read_cpsr(p);
		uint32_t value = arm_read_usr_register(p, Rdest);

		//uint8_t flag_N = (value >> N) & 1;
		cpsr = maj_N(p, cpsr, Rdest);
		//uint8_t flag_Z = value == 0;
		cpsr = maj_Z(p, cpsr, Rdest);
		uint8_t flag_V = (cpsr >> V) & 1;
		//uint8_t flag_C = 0;
		if (value - (4294967295) > 0) { // if (r2>2^32-1)
			flag_C = 1;
		} else {
			flag_C = 0;
		}

		/*clr_bit(cpsr, Z);
		clr_bit(cpsr, N);
		clr_bit(cpsr, C);
		clr_bit(cpsr, V);
		if (flag_Z)
			set_bit(cpsr, Z);
		if (flag_N)
			set_bit(cpsr, N);
		if (flag_C)
			set_bit(cpsr, C);
		if (flag_V)
			set_bit(cpsr, V);*/

		/*set_1_bit(cpsr, Z, flag_Z);
		set_1_bit(cpsr, N, flag_N);
		set_1_bit(cpsr, C, flag_C);*/
		set_1_bit(cpsr, V, flag_V);

		arm_write_cpsr(p,cpsr);
	}

	return 0;
}

//
int bic_instr(arm_core p, uint8_t Rsource, uint8_t Rdest, uint16_t shifter_operand) {return 1;}

int mvn_instr(arm_core p, uint8_t Rdest, uint16_t shifter_operand, uint8_t I, uint8_t flag_S) {

	uint16_t ri = shifter_operand >> 8 & 0xF;
	uint32_t so = ror(shifter_operand & 0xFF, ri*2);

	if (I) {
		arm_write_usr_register(p, Rdest, ~so);
	}else {
		uint32_t value;
		value = arm_read_usr_register(p,so);

		arm_write_usr_register(p, Rdest, ~value);
	}

	if (flag_S && Rdest) {
		if (arm_current_mode_has_spsr(p)){
			uint32_t spsr = arm_read_spsr(p);
			arm_write_cpsr(p, spsr);
		} else {
			// UNPREDICTABLE
			return 1;
		}
	} else if (flag_S == 1){
		uint32_t cpsr = arm_read_cpsr(p);
		uint32_t value = arm_read_usr_register(p, Rdest);

		uint8_t flag_N = (value >> N) & 1;
		uint8_t flag_Z = value == 0;
		uint8_t flag_V = (cpsr >> V) & 1;
		uint8_t flag_C = 0;
		if (value - (4294967295) > 0) { // if (r2>2^32-1)
			flag_C = 1;
		} else {
			flag_C = 0;
		}

		clr_bit(cpsr, Z);
		clr_bit(cpsr, N);
		clr_bit(cpsr, C);
		clr_bit(cpsr, V);
		if (flag_Z)
			set_bit(cpsr, Z);
		if (flag_N)
			set_bit(cpsr, N);
		if (flag_C)
			set_bit(cpsr, C);
		if (flag_V)
			set_bit(cpsr, V);

		arm_write_cpsr(p,cpsr);
	}

	return 0;
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

	uint8_t flag_S = (uint8_t) ((mot >> 20) & 1);

/** Selecion of the instruction encoded in opCode **/
	switch (opCode) {
		case 0:
			return and_instr(p, Rsource, Rdest, shifter_operand, I, S);
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
			return add_instr(p, Rsource, Rdest, shifter_operand, I);
			break;
		case 5:
			return adc_instr(p, Rsource, Rdest, shifter_operand, flag_C, I);
			break;
		case 6:
			return sbc_instr(p, Rsource, Rdest, shifter_operand, flag_C, I, S);
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
			return mov_instr(p, Rdest, shifter_operand, I, flag_S);
			break;
		case 14:
			return bic_instr(p, Rsource, Rdest, shifter_operand);
			break;
		case 15:
			return mvn_instr(p, Rdest, shifter_operand, I, flag_S);
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
