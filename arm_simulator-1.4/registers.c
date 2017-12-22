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
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>

struct registers_data {

	uint32_t *regs;
	uint32_t cpsr;
};

registers registers_create() {
    registers r = malloc(sizeof(registers));
    r-> regs = malloc(sizeof(uint32_t)*15);
    return r;
}

void registers_destroy(registers r) {
	free(r->regs);
	free(r);
}

uint8_t get_mode(registers r) {
    return 0;
} 

int current_mode_has_spsr(registers r) {
    return 0;
}

int in_a_privileged_mode(registers r) {
    return 0;
}

uint32_t read_register(registers r, uint8_t reg) {
    uint32_t value = 0;
	value = read_usr_register(r,reg);
    return value;
}

uint32_t read_usr_register(registers r, uint8_t reg) {
    uint32_t value=0;
    value = r->regs[reg];
    return value;
}

uint32_t read_cpsr(registers r) {
    uint32_t value=0;
    value = r->cpsr;
    return value;
}

uint32_t read_spsr(registers r) {
    uint32_t value=0;
    return value;
}

void write_register(registers r, uint8_t reg, uint32_t value) {
	write_usr_register(r,reg, value);
}

void write_usr_register(registers r, uint8_t reg, uint32_t value) {
	r->regs[reg] = value;
}

void write_cpsr(registers r, uint32_t value) {
	r->cpsr = value;
}

void write_spsr(registers r, uint32_t value) {
}	