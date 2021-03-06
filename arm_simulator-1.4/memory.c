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
#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
    size_t size;
    uint8_t *memo;
    int is_big_endian;
};

memory memory_create(size_t size, int is_big_endian) {

    memory mem= malloc(sizeof(struct memory_data)) ;
    mem->memo = malloc(sizeof(uint8_t)*4*size);
    mem->size= size;
    mem->is_big_endian = is_big_endian;
    return mem;
}

size_t memory_get_size(memory mem) {

    return mem->size;
}

void memory_destroy(memory mem) {
    free (mem->memo);
    free (mem);
}

/**
*
* @param value la valeur, taille la taille de value (16 ou 32)
*
*/

int reverse_endianess(uint32_t value, uint8_t taille){
  int resultat = 0;
  int sizeOf = sizeof(uint32_t);
  if(taille == 8||16||32){   // la taille est correcte.
	if(taille == 16){
    //uint16_t value ;

		//value = (uint16_t) value ;
		sizeOf = sizeof(uint16_t);
	}
	if(taille == 8){
		value = (uint8_t) value ;
		sizeOf = sizeof(uint8_t);
	}
        char *source, *destination;
        int i;
        source = (char *) &value;
        destination = ((char *) &resultat)+sizeOf;
        for (i=0; i<sizeOf; i++)
        *(--destination) = *(source++);

  } else{	// la taille est incorrecte
		resultat = -1 ;
  }
  return resultat ;
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    if (address < 0 || address/4> mem->size){ return -1;}

    *value = mem->memo[address];

    if (!mem->is_big_endian){
         *value = reverse_endianess(*value, 8);
    }
    return 0;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value) {

    if (address < 0 || address/4> mem->size || address%4 > 2){ return -1;}


    *value = ((mem->memo[address] ) << 8)+(mem->memo[address+1]);
    if (!mem->is_big_endian){
        *value = reverse_endianess(*value, 16);
    }

    return 0;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
    if (address < 0 || address/4> mem->size || address % 4 > 0){ return -1;}
    if (mem->is_big_endian){
        *value = ((mem->memo[address]) << 24)+((mem->memo[address+1]) << 16)+((mem->memo[address+2] ) << 8)+(mem->memo[address+3]);
    } else {
        *value = ((mem->memo[address]) << 24)+((mem->memo[address+1]) << 16)+((mem->memo[address+2] ) << 8)+(mem->memo[address+3]);
        *value = reverse_endianess(*value, 32);
    }
    return 0;
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    if (address < 0 || address/4> mem->size){ return -1;}
    if (!mem->is_big_endian){
    	value = reverse_endianess(value, 8);
    }
  	mem->memo[address] = value;
    return 0;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {
    if (address < 0 || address/4> mem->size || address % 4 > 2)
        { return -1;}
    if (!mem->is_big_endian){
    	value = reverse_endianess(value, 16);
    }

    mem->memo[address] = value >> 8;
    mem -> memo[address+1] = value & 255;
    return 0;

}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {
    if (address < 0 || address/4> mem->size || address % 4 > 0)
        { return -1;}
    if (!mem->is_big_endian){
        value = reverse_endianess(value, 32);
    }

     mem->memo[address] = value>>24;
     mem -> memo[address+1] = (value >> 16) & 255;
     mem -> memo[address+2] = (value >> 8) & 255;
     mem -> memo[address+3] = value & 255;
     return 0;

}
