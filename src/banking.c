#include "headers/cpu.h"
#include "headers/lib.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// get the last test on mbc1 working
// fix windows saving!?


// think the high bank change is not being done correctly

// do it based on cpu manual 
// fix mbc2 ram

// implement mbc3 timer <--- now

// mbc3 may not be enabling and disabling ram properly? 

void do_change_rom_ram_mode(Cpu *cpu, uint8_t data);
void do_ram_bank_change(Cpu *cpu, uint8_t data);
void do_change_hi_rom_bank(Cpu *cpu, uint8_t data);
void do_change_lo_rom_bank(Cpu *cpu,uint8_t data);
void do_ram_bank_enable(Cpu * cpu,uint16_t address, uint8_t data);

void handle_banking(uint16_t address, uint8_t data,Cpu *cpu)
{
	// do ram enabling 
	if(address < 0x2000)
	{
		if(cpu->rom_info.mbc1 || cpu->rom_info.mbc2 || cpu->rom_info.mbc3)
		{
			do_ram_bank_enable(cpu,address,data);
		}
	}
	
	// do rom or ram bank change
	else if(address >= 0x2000 && address < 0x4000)
	{
		if(cpu->rom_info.mbc1 || cpu->rom_info.mbc2)
		{
			do_change_lo_rom_bank(cpu,data);
		}

		else if(cpu->rom_info.mbc3)
		{
			cpu->currentrom_bank = data & 127;
			cpu->currentrom_bank &= 127;
			
			if(cpu->currentrom_bank >= cpu->rom_info.noRomBanks)
			{
				//printf("[BANKING] Attempted to set a rom bank greater than max %d\n",cpu->currentrom_bank);
				cpu->currentrom_bank %= cpu->rom_info.noRomBanks;
				//printf("new rom bank %d\n",cpu->currentrom_bank);
				//exit(1);
			}
			
			if(cpu->currentrom_bank == 0) cpu->currentrom_bank = 1;
		}
	}
	
	else if((address >= 0x4000) && (address < 0x6000))
	{
		// no rambank in mbc2 use rambank 0
		if(cpu->rom_info.mbc1) 
		{
			if(cpu->rom_banking)
			{
				do_change_hi_rom_bank(cpu,data);
			}
				
			else
			{
				do_ram_bank_change(cpu,data);
			}	
		}
		
		
		else if(cpu->rom_info.mbc3)
		{
			// change the ram bank
			// if ram bank is greater than 0x3 disable writes
			cpu->currentram_bank = data;
			if(cpu->rom_info.noRamBanks == 0)
			{
				cpu->currentram_bank = 0;
			}
	
			else if(cpu->currentram_bank <= 3 && cpu->currentram_bank >= cpu->rom_info.noRamBanks)
			{
				//printf("[BANKING] Attempted to set a  ram bank greater than max %d\n",cpu->currentram_bank);
				cpu->currentram_bank %= cpu->rom_info.noRamBanks;
				//exit(1);
			}
			
			//puts("MBC3 ram change");	
		}
	}
	
	// this changes wether we want to rom or ram bank
	// for the above
	else if((address >= 0x6000 && address < 0x8000))
	{
		if(cpu->rom_info.mbc1)
		{
			do_change_rom_ram_mode(cpu,data);
		}	
	}	
}

void do_ram_bank_enable(Cpu * cpu,uint16_t address, uint8_t data)
{
	

	
	if(cpu->rom_info.mbc2)
	{
		if(is_set(address,4))
		{
			return;
		}
	}
	
	uint8_t test_data = data & 0xf;
	
	
	if(test_data == 0xa)
	{
		cpu->enable_ram = true;
	}
	
	// any number will work that aint 0xa
	else
	{
		cpu->enable_ram = false;
	}
	
	//printf("Ram enabled = %d\n",cpu->enable_ram);
	
}


void do_change_lo_rom_bank(Cpu *cpu,uint8_t data)
{
	
	
	if(cpu->rom_info.mbc2)
	{
		cpu->currentrom_bank = data & 0xf;
		if(cpu->currentrom_bank == 0) cpu->currentrom_bank++;
		return;
	}
	
	uint8_t lower5 = data & 31; // get lower 5 bits 
	cpu->currentrom_bank &= 224; // turn off bits lower than 5
	cpu->currentrom_bank |= lower5;
	
	
	if(cpu->currentrom_bank == 0 || cpu->currentrom_bank == 0x20 
	|| cpu->currentrom_bank == 0x40 || cpu->currentrom_bank == 0x60)
	{
		cpu->currentrom_bank += 1;
	}
	
	if(cpu->currentrom_bank >= cpu->rom_info.noRomBanks)
	{
		//printf("[BANKING] Attempted to set a rom bank greater than max %d\n",cpu->currentrom_bank);
		cpu->currentrom_bank %= cpu->rom_info.noRomBanks;
		//printf("new rom bank %d\n",cpu->currentrom_bank);
		//exit(1);
	}

	//printf("lo change %x\n",cpu->currentrom_bank);
	
}

void do_change_hi_rom_bank(Cpu *cpu, uint8_t data)
{
	
	cpu->currentrom_bank &= 0x1f;
	
	data &= 0xe0;
	
	cpu->currentrom_bank |= data;
	
	if(cpu->currentrom_bank == 0 || cpu->currentrom_bank == 0x20 
	|| cpu->currentrom_bank == 0x40 || cpu->currentrom_bank == 0x60)
	{
		cpu->currentrom_bank += 1;
	}
	
	
	// not sure what the defined behaviour is here
	if(cpu->currentrom_bank >= cpu->rom_info.noRomBanks)
	{
		//printf("[BANKING] Attempted to set a rom bank greater than max %d\n",cpu->currentrom_bank);
		cpu->currentrom_bank %= cpu->rom_info.noRomBanks;
		//printf("new rom bank %d\n",cpu->currentrom_bank);
		//exit(1);
	}
	


	
	//printf("hi change %x\n",cpu->currentrom_bank);
	
}


void do_ram_bank_change(Cpu *cpu, uint8_t data)
{
	cpu->currentram_bank = data & 0x3;
	
	
	if(cpu->rom_info.noRamBanks <= 1)
	{
		cpu->currentram_bank = 0;
	}
	
	if(cpu->currentram_bank > cpu->rom_info.noRamBanks)
	{
		//printf("[BANKING] Attempted to set a  ram bank greater than max %d\n",cpu->currentram_bank);
		cpu->currentram_bank %= cpu->rom_info.noRamBanks;
		//exit(1);
	}
	
}


void do_change_rom_ram_mode(Cpu *cpu, uint8_t data)
{
	// may need to and data with 1
	data &= 0x1;
	
	if(data == 1)
	{
		// enable ram banking mode
		cpu->rom_banking = false;
	}
	
	else
	{
		// enable rom banking mode
		cpu->rom_banking = true;
	}
	
	
	if(cpu->rom_banking)
	{
		cpu->currentram_bank = 0;
	}
}
