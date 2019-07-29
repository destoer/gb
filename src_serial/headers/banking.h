#pragma once
#include "cpu.h"

void do_ram_bank_change_mbc1(Cpu *cpu, int data);
void do_change_hi_rom_bank_mbc1(Cpu *cpu, int data);
void banking_unused(Cpu *cpu,uint16_t address,int data);
void do_change_rom_ram_mode(Cpu *cpu,uint16_t address,int data);
void mbc5_ram_bank_change(Cpu *cpu,uint16_t address,int data);
void mbc3_ram_bank_change(Cpu *cpu,uint16_t address,int data);
void mbc1_banking_change(Cpu *cpu,uint16_t address,int data);
void do_change_hi_rom_bank_mbc5(Cpu *cpu,uint16_t address,int data);
void do_change_lo_rom_bank_mbc5(Cpu *cpu,uint16_t address,int data);
void do_change_rom_bank_mbc3(Cpu *cpu,uint16_t address,int data);
void do_change_lo_rom_bank_mbc2(Cpu *cpu,uint16_t address,int data);
void do_change_lo_rom_bank_mbc1(Cpu *cpu, uint16_t address, int data);
void do_ram_bank_enable_mbc2(Cpu *cpu,uint16_t address,int data);
void do_ram_bank_enable(Cpu *cpu, uint16_t address, int data);
void cache_banking_ptrs(Cpu *cpu);