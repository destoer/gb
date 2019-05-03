#include "cpu.h"
#include "stdint.h"
#pragma once
uint8_t read_mem(uint16_t address, Cpu *cpu);
void write_mem(Cpu *cpu,uint16_t address,int data);

// direct memory reads (actions done by the program should not use these)
uint8_t read_oam(uint16_t address, Cpu *cpu);
uint8_t read_vram(uint16_t address, Cpu *cpu);
uint8_t read_io(uint16_t address, Cpu *cpu);
void write_io(Cpu *cpu,uint16_t address, int data);