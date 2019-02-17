#include "cpu.h"
#include "stdint.h"
#pragma once
uint8_t read_mem(uint16_t address, Cpu *cpu);
void write_mem(Cpu *cpu,uint16_t address,int data);

