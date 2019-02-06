#pragma once

#include "cpu.h"
#include <stdint.h>
#include <stdbool.h>

void set_zero(Cpu *cpu, uint8_t reg);
void dec(Cpu *cpu, uint8_t reg);
void inc(Cpu *cpu,uint8_t reg);
void bit(Cpu *cpu, uint8_t reg, uint8_t bit);
void and(Cpu *cpu, uint8_t num);
uint8_t rl(Cpu *cpu, uint8_t reg);
void sub(Cpu *cpu,  uint8_t num);
void cp(Cpu *cpu,uint8_t num);
void sbc(Cpu *cpu, uint8_t num);
void add(Cpu *cpu, uint16_t num);
uint16_t addi(Cpu *cpu,uint16_t reg, int8_t num);
void adc(Cpu *cpu,uint8_t num);
uint16_t addw(Cpu *cpu, uint16_t reg, uint16_t num);
void or(Cpu *cpu,uint8_t val);
uint8_t swap(Cpu *cpu, uint8_t num);
void xor(Cpu *cpu,uint8_t num);
void binary(int number);
uint8_t sla(Cpu *cpu,uint8_t reg);
uint8_t srl(Cpu *cpu, uint8_t reg);
uint8_t sra(Cpu *cpu,uint8_t reg);
uint8_t rr(Cpu *cpu, uint8_t reg);
uint8_t rrc(Cpu *cpu,uint8_t reg);
uint8_t rlc(Cpu *cpu, uint8_t reg);

