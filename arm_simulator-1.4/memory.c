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
#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
    size_t size;
    uint8_t *memo;
    int is_big_endian;
};

memory memory_create(size_t size, int is_big_endian) {

    memory mem= malloc(sizeof( memory)) ;
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

int reverse_endianess(int value)
{
  int resultat = 0;
  char *source, *destination;
  int i;

  source = (char *) &value;
  destination = ((char *) &resultat)+sizeof(int);
  for (i=0; i<sizeof(int); i++)
      *(--destination) = *(source++);
  return resultat;
}

uint16_t reverse_endianess16(uint16_t value)
{
  uint16_t resultat = 0;
  char *source, *destination;
  int i;

  source = (char *) &value;
  destination = ((char *) &resultat)+sizeof(uint16_t);
  for (i=0; i<sizeof(uint16_t); i++)
      *(--destination) = *(source++);
  return resultat;
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    if (address < 0 || address/4> mem->size){ return -1;}
    *value = mem->memo[address];
    return 0;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value) {

    if (address < 0 || address/4> mem->size || address%4 > 2){ return -1;}
    if (mem->is_big_endian){
        *value = ((mem->memo[address] ) << 8)+(mem->memo[address+1]);
    } else {
        *value = ((mem->memo[address] ) << 8)+(mem->memo[address+1]);
        *value = reverse_endianess16(*value);
    }
    return 0;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value) {
    if (address < 0 || address/4> mem->size || address % 4 > 0){ return -1;}
    if (mem->is_big_endian){
        *value = ((mem->memo[address]) << 24)+((mem->memo[address+1]) << 16)+((mem->memo[address+2] ) << 8)+(mem->memo[address+3]);
    } else {
        *value = ((mem->memo[address]) << 24)+((mem->memo[address+1]) << 16)+((mem->memo[address+2] ) << 8)+(mem->memo[address+3]);
        *value = reverse_endianess(*value);
    }
    return 0;
}

int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    if (address < 0 || address/4> mem->size){ return -1;}
    if (mem->is_big_endian){
        mem->memo[address] = value;
    }
    else{
       
        mem -> memo[address] = value;
    }
    return 0;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value) {
    if (address < 0 || address/4> mem->size || address % 4 > 2)
        { return -1;}
    if (mem->is_big_endian){
        mem->memo[address] = value>>8;
        mem -> memo[address+1] = value & 255;
    }
    else{
        value = reverse_endianess16(value);
        mem->memo[address] = value>>8;
        mem -> memo[address+1] = value & 255;
    }
        return 0;

}

int memory_write_word(memory mem, uint32_t address, uint32_t value) {
    if (address < 0 || address/4> mem->size || address % 4 > 0)
        { return -1;}
    if (mem->is_big_endian){
        mem->memo[address] = value>>24;
        mem -> memo[address+1] = (value >> 16) & 255;
        mem -> memo[address+2] = (value >> 8) & 255;
        mem -> memo[address+3] = value & 255;
    }
    else{
        value = reverse_endianess(value);
        mem->memo[address] = value>>24;
        mem -> memo[address+1] = value >> 16 & 255;
        mem -> memo[address+2] = value >> 8 & 255;
        mem -> memo[address+3] = value & 255;
    }
        return 0;

}

