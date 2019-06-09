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

void do_change_rom_ram_mode(Cpu *cpu,uint16_t address,int data);
void do_ram_bank_change(Cpu *cpu,int data);
void do_change_hi_rom_bank_mbc1(Cpu *cpu,int data);
void do_change_lo_rom_bank(Cpu *cpu,uint16_t address,int data);
void do_ram_bank_enable(Cpu *cpu,uint16_t address,int data);
void do_ram_bank_change_mbc1(Cpu *cpu, int data);



void cache_banking_ptrs(Cpu *cpu)
{
	cpu->current_bank_ptr4 = &cpu->rom_mem[(cpu->currentrom_bank*0x4000)];
	cpu->current_bank_ptr5 = &cpu->rom_mem[0x1000 + (cpu->currentrom_bank*0x4000)];
	cpu->current_bank_ptr6 = &cpu->rom_mem[0x2000 + (cpu->currentrom_bank*0x4000)];
	cpu->current_bank_ptr7 = &cpu->rom_mem[0x3000 +(cpu->currentrom_bank*0x4000)];		
}

// ram bank enables 0x0000-0x1fff
// every mbc other than 2
void do_ram_bank_enable(Cpu *cpu, uint16_t address, int data)
{
	UNUSED(address);

	// no ram banks dont enable it
	if(cpu->rom_info.noRamBanks == 0)
	{
		return;
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

// just mbc2
void do_ram_bank_enable_mbc2(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	if(cpu->rom_info.mbc2)
	{
		if(is_set(address,4))
		{
			return;
		}
	}

	do_ram_bank_enable(cpu,address,data);
}

// ram bank change 
// 0x2000 - 0x3fff

// mbc1
void do_change_lo_rom_bank_mbc1(Cpu *cpu, uint16_t address, int data)
{
	UNUSED(address);
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
	cache_banking_ptrs(cpu);
}

//mbc2
void do_change_lo_rom_bank_mbc2(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	cpu->currentrom_bank = data & 0xf;
	if(cpu->currentrom_bank == 0) cpu->currentrom_bank++;
	cache_banking_ptrs(cpu);
	return;
}

// mbc3
void do_change_rom_bank_mbc3(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
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
	cache_banking_ptrs(cpu);
}


// mbc5
void do_change_lo_rom_bank_mbc5(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	cpu->currentrom_bank &= 0x100;
	cpu->currentrom_bank |= data;
				
	if(cpu->currentrom_bank >= cpu->rom_info.noRomBanks)
	{
		//printf("[BANKING] Attempted to set a rom bank greater than max %d\n",cpu->currentrom_bank);
		cpu->currentrom_bank %= cpu->rom_info.noRomBanks;
		//printf("new rom bank %d\n",cpu->currentrom_bank);
		//exit(1);
	}			
	// bank zero actually acceses bank 0
	cache_banking_ptrs(cpu);
}


//mbc5 (9th bit) (03000 - 0x3fff)
void do_change_hi_rom_bank_mbc5(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	cpu->currentrom_bank &= 0xff;
	cpu->currentrom_bank |= (data & 1) << 8; // 9th bank bit
	if(cpu->currentrom_bank >= cpu->rom_info.noRomBanks)
	{
		//printf("[BANKING] Attempted to set a rom bank greater than max %d\n",cpu->currentrom_bank);
		cpu->currentrom_bank %= cpu->rom_info.noRomBanks;
		//printf("new rom bank %d\n",cpu->currentrom_bank);
		//exit(1);
	}
	cache_banking_ptrs(cpu);
}


// 0x4000 - 0x5fff

// mbc1
void mbc1_banking_change(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	if(cpu->rom_banking)
	{
		do_change_hi_rom_bank_mbc1(cpu,data);
	}
	
	else
	{
		do_ram_bank_change_mbc1(cpu,data);
	}
}

// mbc3
void mbc3_ram_bank_change(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	// change the ram bank
	// if ram bank is greater than 0x3 disable writes
	cpu->currentram_bank = data;
			
	if(cpu->currentram_bank > 3)
	{
		cpu->currentram_bank = RTC_ENABLED;
	}	
	
	else if(cpu->rom_info.noRamBanks == 0)
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
// mbc5
void mbc5_ram_bank_change(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
	cpu->currentram_bank = data & 0xf;
	
	if(cpu->currentram_bank >= cpu->rom_info.noRamBanks)
	{
		cpu->currentram_bank %= cpu->rom_info.noRamBanks;	
	}	
}

// 0x6000-0x7fff
// mbc1
void do_change_rom_ram_mode(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(address);
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

// ideally a well behaved game wont call this but it might happen
void banking_unused(Cpu *cpu,uint16_t address,int data)
{
	UNUSED(cpu); UNUSED(address); UNUSED(data);
	return;
}




// both below are called by mbc1_banking_change

// mbc1 high bank change
void do_change_hi_rom_bank_mbc1(Cpu *cpu, int data)
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
	

	cache_banking_ptrs(cpu);
	//printf("hi change %x\n",cpu->currentrom_bank);
	
}

// mbc1 ram bank change
void do_ram_bank_change_mbc1(Cpu *cpu, int data)
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
