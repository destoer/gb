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
uint8_t read_rom_bank_zero(Cpu *cpu, uint16_t addr);
uint8_t read_rom_bank(Cpu *cpu, uint16_t address);
void do_dma(Cpu *cpu, uint8_t data);
uint8_t read_mem_vram(Cpu *cpu, uint16_t address);
uint8_t read_cart_ram(Cpu *cpu, uint16_t address);
uint8_t read_wram_low(Cpu *cpu, uint16_t address);
uint8_t read_wram_high(Cpu *cpu, uint16_t address);
uint8_t read_mem_hram(Cpu *cpu, uint16_t address);

void write_hram(Cpu *cpu, uint16_t address, int data);
void write_wram_high(Cpu *cpu, uint16_t address, int data);
void write_wram_low(Cpu *cpu, uint16_t address, int data);
void write_cart_mem(Cpu *cpu, uint16_t address, int data);
void write_vram_mem(Cpu *cpu, uint16_t address ,int data);
void write_banking(Cpu *cpu, uint16_t address, int data);

uint8_t read_rom_bank4(Cpu *cpu, uint16_t address);
uint8_t read_rom_bank5(Cpu *cpu, uint16_t address);
uint8_t read_rom_bank6(Cpu *cpu, uint16_t address);
uint8_t read_rom_bank7(Cpu *cpu, uint16_t address);