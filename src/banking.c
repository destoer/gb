#include "headers/cpu.h"
#include "headers/lib.h"
#include <stdbool.h>
#include <stdint.h>

void do_ram_bank_enable(Cpu * cpu,uint16_t address, uint8_t data)
{
	if(cpu->rom_info.mbc2)
	{
		if(is_set(address,4)) return;
	}
	
	uint8_t test_data = data & 0xf;
	if(test_data == 0xa)
	{
		cpu->enable_ram = true;
	}
	else if(test_data == 0x0)
	{
		cpu->enable_ram = false;
	}
}


void do_change_lo_rom_bank(Cpu *cpu,uint8_t data)
{
	if(cpu->rom_info.mbc2)
	{
		cpu->currentrom_bank = data & 0xf;
		if(cpu->currentrom_bank == 0) cpu->currentrom_bank++;
		return;
	}
	
	uint8_t lower5 = data & 31;
	cpu->currentrom_bank &= 224; // turn off bits lower than 5
	cpu->currentrom_bank |= lower5;
	if(cpu->currentrom_bank == 0) cpu->currentrom_bank++;
}

void do_change_hi_rom_bank(Cpu *cpu, uint8_t data)
{
	// turn off upper 3 bits of curent bank
	cpu->currentrom_bank &= 31;
	
	// turn off the lower 5 bits
	data &= 224;
	cpu->currentrom_bank |= data;
	if(cpu->currentrom_bank == 0) cpu->currentrom_bank++;
}


void do_ram_bank_change(Cpu *cpu, uint8_t data)
{
	cpu->currentram_bank = data & 0x3;
}


void do_change_rom_ram_mode(Cpu *cpu, uint8_t data)
{
	uint8_t new_data = data & 0x1;
	cpu->rom_banking = (new_data == 0)?true:false;
	if(cpu->rom_banking)
	{
		cpu->currentram_bank = 0;
	}
}
