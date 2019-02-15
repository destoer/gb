#pragma once
#include "cpu.h"

void do_change_rom_ram_mode(Cpu *cpu, uint8_t data);
void do_ram_bank_change(Cpu *cpu, uint8_t data);
void do_change_hi_rom_bank(Cpu *cpu, uint8_t data);
void do_change_lo_rom_bank(Cpu *cpu,uint8_t data);
void do_ram_bank_enable(Cpu * cpu,uint16_t address, uint8_t data);
void handle_banking(uint16_t address, uint8_t data,Cpu *cpu);