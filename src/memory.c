#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "headers/cpu.h"
#include "headers/disass.h"
#include "headers/lib.h"
#include "headers/banking.h"
#include "headers/instr.h"
#include "headers/debug.h"
#include "headers/ppu.h"
#include "headers/apu.h"




uint8_t read_oam(uint16_t address, Cpu *cpu);
uint8_t read_vram(uint16_t address, Cpu *cpu);
uint8_t read_io(uint16_t address, Cpu *cpu);
void write_io(Cpu *cpu,uint16_t address, int data);
uint8_t read_rom_bank_zero(Cpu *cpu, uint16_t addr);
void do_dma(Cpu *cpu, uint8_t data);
uint8_t read_vram(uint16_t address, Cpu *cpu);
uint8_t read_mem(uint16_t address, Cpu *cpu);
void write_mem(Cpu *cpu,uint16_t address,int data);

uint8_t read_rom_bank_zero(Cpu *cpu, uint16_t address)
{
	return cpu->rom_mem[address];	
}



// cached ptrs for each section of the banked rom 
// in 0x1000 segments
uint8_t read_rom_bank4(Cpu *cpu, uint16_t address)
{
	uint16_t addr = address & 0xfff;
	return cpu->current_bank_ptr4[addr];
}

uint8_t read_rom_bank5(Cpu *cpu, uint16_t address)
{
	uint16_t addr = address & 0xfff;
	return cpu->current_bank_ptr5[addr];
}

uint8_t read_rom_bank6(Cpu *cpu, uint16_t address)
{
	uint16_t addr = address & 0xfff;
	return cpu->current_bank_ptr6[addr];
}

uint8_t read_rom_bank7(Cpu *cpu, uint16_t address)
{
	uint16_t addr = address & 0xfff;
	return cpu->current_bank_ptr7[addr];
}


// vram can only be accesed at mode 0-2
// between 0x8000 - 0x9fff
	
uint8_t read_mem_vram(Cpu *cpu, uint16_t address)
{
	if(cpu->mode <= 2)
	{
		return read_vram(address,cpu);
	}
		
	return 0xff; // return ff if you cant read
}		

// read out of cart ram 
// 0xa000 - 0xbfff	
uint8_t read_cart_ram(Cpu *cpu, uint16_t address)
{	
	// is ram enabled (ram bank of -1 indicates rtc reg)
	if(cpu->enable_ram && cpu->currentram_bank != RTC_ENABLED)
	{
		uint16_t new_address = address - 0xa000;
		return cpu->ram_banks[new_address + (cpu->currentram_bank * 0x2000)];
	}
		
	else
	{
		return 0xff;
	}	
}


// WRAM
// 0xc000
uint8_t read_wram_low(Cpu *cpu, uint16_t address)
{	
	uint16_t addr = address & 0xfff;
	return cpu->wram[addr];
}

// 0xd000
uint8_t read_wram_high(Cpu *cpu, uint16_t address)
{
	uint16_t addr = address & 0xfff;
	// on dmg the work ram bank is fixed!
	return cpu->cgb_ram_bank[cpu->cgb_ram_bank_num][addr];
}

// reads from 0xf000
uint8_t read_mem_hram(Cpu *cpu, uint16_t address)
{
	// io mem
	if(address >= 0xff00)
	{	
		return read_io(address, cpu);
	}	
	
	else if(address >= 0xf000 && address <= 0xfdff)
	{
		uint16_t addr = address & 0xfff;
		// on dmg the work ram bank is fixed!
		return cpu->cgb_ram_bank[cpu->cgb_ram_bank_num][addr];
	}	
	
	// oam is accesible during mode 0-1
	else if(address >= 0xfe00 && address <= 0xfe9f)
	{
		if(cpu->mode <= 1 && !cpu->oam_dma_active) // cant access during dma? but ppu should?
		{
			return read_oam(address,cpu);
		}
		
		return 0xff; // cant read so return ff
	}

	// restricted 
	else if( (address >= 0xFEA0) && (address <= 0xFEFF) )
	{
		return 0xff;
	}


	// else 
	printf("fell through at hram: %x\n",address);
	exit(1);
	
}


uint8_t read_mem(uint16_t address, Cpu *cpu)
{
	#ifdef DEBUG
	// read breakpoint
	if(address == cpu->memr_breakpoint)
	{
		int break_backup = cpu->memr_breakpoint;
		cpu->memr_breakpoint = -1;
		if(cpu->memr_value == -1 || cpu->memr_value == read_mem(address,cpu))
		{
			cpu->memr_breakpoint = break_backup;
			printf("read breakpoint (%x)!\n",cpu->memr_breakpoint);
			enter_debugger(cpu);
		}
		cpu->memr_breakpoint = break_backup;
	}
	#endif	
	
	return cpu->memory_table[(address & 0xf000) >> 12].read_memf(cpu,address);
}


void write_oam(Cpu *cpu, uint16_t address, int data)
{
	cpu->oam[address & 0xff] = data;	
}


void start_gdma(Cpu *cpu)
{
	
	//puts("GDMA!");
	//exit(1);


	uint16_t source = cpu->dma_src;
	
	uint16_t dest = cpu->dma_dst | 0x8000;
	
	// hdma5 stores how many 16 byte incremnts we have to transfer
	int len = ((cpu->io[IO_HDMA5] & 0x7f) + 1) * 0x10;

	
	// find out how many cycles we tick but for now just copy the whole damb thing 
	
	for(int i = 0; i < len; i++)
	{
		write_mem(cpu,dest+i,read_mem(source+i,cpu));
	}

	cycle_tick(cpu,8*(len / 0x10)); // 8 M cycles for each 10 byte block

	
}


// needs reindentation
void write_io(Cpu *cpu,uint16_t address, int data)
{
	#ifdef DEBUG
	// write breakpoint
	if(address == cpu->memw_breakpoint)
	{
		if(cpu-> memw_value == -1 || cpu->memw_value == data)
		{
			printf("Write breakpoint (%x)!\n",cpu->memw_breakpoint);
			printf("data %x\n",data);
			enter_debugger(cpu);
		}
	}
	#endif
		
	switch(address & 0xff)
	{ 
		// update the timer freq (tac in gb docs)
		case IO_TMC:
		{
			cpu->io[IO_TMC] = data | 248;
			return;
		}
			

			
		case IO_SC:
		{
			cpu->io[IO_SC] = (data | 0x7e);
			return;
		}


		/*	// serial control (stub)
			case IO_SC
			{
				data |= 2; // <-- for dmg 
				cpu->io[IO_SC] = data | (~0x83);
				deset_bit(cpu->io[IO_SC],7); // <- cheat to time it out immediately lol
				request_interrupt(cpu,3);
				cpu->mem[IO_SC] = 0xff;		
				return; 
			}
		*/
			//---------------------------
			// unused hwio
			
		// unused
		case 0x03:
		{
			cpu->io[0x03] = 0xff;
			return;
		}
			
		// unused
		//else if(address >= 0xff08 && address <= 0xff0e )
		case 0x08 ... 0x0e:
		{
			cpu->io[address & 0xff] = 0xff;
			return;
		}
			
			
		// unused
		case 0x15:
		{
			cpu->io[0x15] = 0xff;
			return;
		}
			

		// unused
		case 0x1f:
		{
			cpu->io[0x1f] = 0xff;
			return;
		}
			
			
		// unused
		case 0x27:
		{
			cpu->io[0x27] = 0xff;
			return;
		}
			
			
		// unused
		case 0x28:
		{
			cpu->io[0x29] = 0xff;
			return;
		}
			
		// unused
		case 0x29:
		{
			cpu->io[0x29] = 0xff;
			return;
		}	
			
		case 0x30 ... 0x3f: // what is the behavior of the write test?
		{
			
			// if wave is on write to current byte <-- finish accuracy later
			if(is_set(cpu->io[IO_NR52],2))
			{
				cpu->io[0x30 + (cpu->square[2].duty_idx / 2)] = data;
				return;
			}
			
			else // if its off allow "free reign" over it
			
			{
				cpu->io[address & 0xff] = data;
				return;
			}
		}

		
	/*	// unused // fix after cgb
		case 0x4c ... 0x7f:
		{
			cpu->io[address & 0xff] = 0xff;
			return;
		}
	*/		


		// div and tima share the same internal counter
		// should account for this internal behavior
		case IO_DIV:
		{
			cpu->internal_timer = 0;
			return;
		}
			
			

		// sound registers
			
		// nr 10
		case IO_NR10:
		{
			if(cpu->sound_enabled)
			{
				// if we have used a sweep calc in negation mode 
				// since the last trigger turn the channel off
				if(is_set(cpu->io[IO_NR10],3) && !is_set(data,3))
				{
					if(cpu->sweep_calced)
					{
						deset_bit(cpu->io[IO_NR52],0);
					}
				}
				
				cpu->io[IO_NR10] = data | 128;
				cpu->sweep_period = (cpu->io[IO_NR10] >> 4) & 7;
			}
			return;
		}

			
		// nr 11
		case IO_NR11:
		{
			if(cpu->sound_enabled)
			{
				// bottom 6 bits are length data 
				// set the internal counter to 64 - bottom 6 bits of data
				cpu->square[0].lengthc = 64 - (data & 63);

				cpu->square[0].duty = (data >> 6) & 0x3;	
				cpu->io[IO_NR11] = data;
			}
			return;
		}
			
		// nr 12
		case IO_NR12:
		{
			if(cpu->sound_enabled)
			{
					
				// if the top 5 bits are deset (channel dac)
				// disable the channel
				if((data & 248) == 0)
				{
					deset_bit(cpu->io[IO_NR52],0);
				}
				cpu->square[0].volume_load = (data >> 4) & 0xf;
				cpu->square[0].volume = cpu->square[0].volume_load;
				cpu->square[0].env_load = data & 0x3;					
					
				cpu->io[IO_NR12] = data;
			}
			return;
		}
			
		// nr 13
		case IO_NR13:
		{
			if(cpu->sound_enabled)
			{
				cpu->square[0].freq &= ~0xff;
				cpu->square[0].freq |= data;
				cpu->io[IO_NR13] = data;
			}
			return;
		}
			
		// nr 14
		case IO_NR14: 
		{
			if(cpu->sound_enabled)
			{
				cpu->square[0].freq &= 0xff;
			 	cpu->square[0].freq |= (data & 0x7) << 8;

				// Trigger event
				// if the data is set to 7 it should enable Sound for nr1
				// in nr52
				if(is_set(data,7))
				{	
					set_bit(cpu->io[IO_NR52],0); // enable channel
						
						
					// if the length counter is 0 it should be loaded with max upon a trigger event
					if(cpu->square[0].lengthc == 0)
					{
						cpu->square[0].lengthc = 64;
						
						// disable the chan 
						// if the value enables the length this will cause an extra tick :P
						deset_bit(cpu->io[IO_NR14],6); 
					}
						
					// reload period and reset duty
					cpu->square[0].period = ((2048 - cpu->square[0].freq))*4; 
					cpu->square[0].env_period = cpu->square[0].env_load;
					cpu->square[0].duty_idx = 0; // reset duty					
					cpu->square[0].volume = cpu->square[0].volume_load;
					cpu->square[0].env_enabled = true;
						
					// Handle the trigger events for the frequency sweep
						
					/*
					Square 1's frequency is copied to the shadow register.
					The sweep timer is reloaded.
					The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
					If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
					*/
					
					cpu->sweep_calced = false; // indicates no sweep freq calcs have happened since trigger	
						
					// Copy the freq to the shadow reg
					//cpu->sweep_shadow =  cpu->io[IO_NR13]; // lower 8
					//cpu->sweep_shadow |= (data & 0x7) << 8; // upper 3
					cpu->sweep_shadow = cpu->square[0].freq;	
						
					// reload the sweep timer
					cpu->sweep_period = (cpu->io[IO_NR10] >> 4) & 7;
					cpu->sweep_timer = cpu->sweep_period;
					if(cpu->sweep_period == 0) // period of zero treated as 8 
					{
						cpu->sweep_timer = 8; // see obscure behavior
					}					
						
						
					// if sweep period or shift are non zero set the internal flag 
					// else clear it
					// 0x77 is both shift and peroid
					if(cpu->io[IO_NR10] &  0x77)
					{
						cpu->sweep_enabled = true;
					}
						
					else 
					{
						cpu->sweep_enabled = false;
					}
						
						
					//exit(1);
						
						
					// if the sweep shift is non zero 
					// perform the overflow check and freq calc immediately 
					if((cpu->io[IO_NR10] & 0x7))
					{
						calc_freqsweep(cpu);
					}
				}
					
					
				// if previously clear and now is enabled 
				// + next step doesent clock, clock the length counter
				if(!is_set(cpu->io[IO_NR14],6) && is_set(data,6) && !(cpu->sequencer_step & 1))
				{
					// if not zero decrement
					if(cpu->square[0].lengthc != 0)
					{	
						// decrement and if now zero and there is no trigger 
						// switch the channel off
						if(!--cpu->square[0].lengthc)
						{
							if(is_set(data,7)) 
							{ 
								// if we have hit a trigger it should be max len - 1
								cpu->square[0].lengthc = 0x3f;
							}
								
							else
							{
								deset_bit(cpu->io[IO_NR52],0);
							}
								
						}	
					}	
				}
					
					
				// after all the trigger events have happend
				// if the dac is off switch channel off
				if((cpu->io[IO_NR12] & 248) == 0) // if dac  disabled
				{
					deset_bit(cpu->io[IO_NR52],0); // turn nr1 off 
				}
					
					
				cpu->io[IO_NR14] = data;
				cpu->square[0].length_enabled = is_set(data,6);
			}
			return;
		}
			
			
		// nr21
		case IO_NR21:
		{
			if(cpu->sound_enabled)
			{
					
				// bottom 6 bits are length data 
				// set the internal counter to 64 - bottom 6 bits of data
				cpu->square[1].lengthc = 64 - (data & 63);	
				cpu->square[1].duty = (data >> 6) & 0x3;
				cpu->io[IO_NR21] = data;
			}
			return;
		}
			
		// nr 22
		case IO_NR22:
		{
			if(cpu->sound_enabled)
			{
					
				// if the top 5 bits are deset (channel dac)
				// disable the channel
				if((data & 248) == 0)
				{
					deset_bit(cpu->io[IO_NR52],1);
				}

				cpu->square[1].volume_load = (data >> 4) & 0x7;
				cpu->square[1].volume = cpu->square[1].volume_load;
				cpu->square[1].env_load = data & 0x3;
				cpu->io[IO_NR22] = data;
			}
			return;
		}
		
		// nr 23
		case IO_NR23:
		{
			if(cpu->sound_enabled)
			{
				cpu->io[IO_NR23] = data;
				cpu->square[1].freq = (cpu->square[1].freq & 0x700) | data;
			}
			return;
		}
			
		// nr 24
		case IO_NR24: // <--- does not pass trigger test
		{	
			if(cpu->sound_enabled)
			{

				cpu->square[1].freq = (cpu->square[1].freq & 0xff) | ((data & 0x7) << 8);


				// Trigger event
				// if the data is set to 7 it should enable Sound for nr2
				// in nr52
				if(is_set(data,7))
				{
					set_bit(cpu->io[IO_NR52],1); // enable channel
				
					// if the length counter is 0 it should be loaded with max upon a trigger event
					if(cpu->square[1].lengthc == 0)
					{
						cpu->square[1].lengthc = 64;
							
						// disable the chan 
						// if the value enables the length this will cause an extra tick :P
						deset_bit(cpu->io[IO_NR24],6); 
					}
					// reload period and reset duty
					cpu->square[1].volume = cpu->square[1].volume_load;
					cpu->square[1].period = ((2048 - cpu->square[1].freq))*4; 
					cpu->square[1].env_period = cpu->square[1].env_load;
					cpu->square[1].duty_idx = 0; // reset duty
					cpu->square[1].env_enabled = true;
				}
					
					
				// if previously clear and now is enabled 
				// + next step doesent clock, clock the length counter
				if(!is_set(cpu->io[IO_NR24],6) && is_set(data,6) && !(cpu->sequencer_step & 1))
				{
					// if not zero decrement
					if(cpu->square[1].lengthc != 0)
					{	
						// decrement and if now zero and there is no trigger 
						// switch the channel off
						if(!--cpu->square[1].lengthc)
						{
							if(is_set(data,7)) 
							{
								// if we have hit a trigger it should be max len - 1
								cpu->square[1].lengthc = 0x3f;
							}
								
							else
							{
								deset_bit(cpu->io[IO_NR52],1); // disable square 2
							}	
						}	
					}
				}
					
					
				// after all the trigger events have happened
				// if the dac is off switch channel off
				if((cpu->io[IO_NR22] & 248) == 0) // if dac  disabled
				{
					deset_bit(cpu->io[IO_NR52],1);
				}
					
					
				cpu->io[IO_NR24] = data;
				cpu->square[1].length_enabled = is_set(data,6);
			}
			return;
		}
			
		// nr 30
		case IO_NR30:
		{
			if(cpu->sound_enabled)
			{
					
				// if the top bit is deset (special for nr3) (channel dac)
				// disable the channel
				if(!is_set(data,7))
				{
					deset_bit(cpu->io[IO_NR52],2);	
				}
					
				cpu->io[IO_NR30] = data | 127;
			}
			return;
		}
			
		// nr 31
		case IO_NR31:
		{
			if(cpu->sound_enabled)
			{
				// whole of the nr31 is length for the wave channel
				cpu->square[2].lengthc = 256 - data;
					
				cpu->io[IO_NR31] = data;
			}
			return;
				
		}	
			

		// nr 32
		case IO_NR32:
		{
			if(cpu->sound_enabled)
			{
				cpu->square[2].volume_load = (data >> 5) & 0x3;
				cpu->square[2].volume = cpu->square[2].volume_load;
				cpu->io[IO_NR32] = data | 159;
			}
			return;
		}

		// nr 33
		case IO_NR33:
		{
			if(cpu->sound_enabled)
			{
				cpu->io[IO_NR33] = data;
				cpu->square[2].freq = (cpu->square[2].freq & ~0xff) | data;
			}
			return;
		}

			
		// nr 34
		case IO_NR34:
		{
			if(cpu->sound_enabled)
			{
				cpu->square[2].freq = (cpu->square[2].freq & 0xff) | ((data & 0x7) << 8);



				// Trigger event
				// if the data is set to 7 it should enable Sound for nr2
				// in nr52
				if(is_set(data,7))
				{
					set_bit(cpu->io[IO_NR52],2); // enable channel
						
					// if the length counter is 0 it should be loaded with max upon a trigger event
					if(cpu->square[2].lengthc == 0)
					{
						cpu->square[2].lengthc = 256;
							
						// disable the chan 
						// if the value enables the length this will cause an extra tick :P
						deset_bit(cpu->io[IO_NR34],6); 
					}
					
					
					
					// wave stuff
					cpu->square[2].duty_idx = 0; // reset the wave index
					// reload the wave peroid 
					// period (2048-frequency)*2
					cpu->square[2].period = ((2048 - cpu->square[2].freq)*2);
					cpu->square[2].volume = cpu->square[2].volume_load;
				}
					
					
				// if previously clear and now is enabled 
				// + next step doesent clock, clock the length counter
				if(!is_set(cpu->io[IO_NR34],6) && is_set(data,6) && !(cpu->sequencer_step & 1))
				{
					// if not zero decrement
					if(cpu->square[2].lengthc != 0)
					{	
						// decrement and if now zero and there is no trigger 
						// switch the channel off
						if(!--cpu->square[2].lengthc)
						{
							if(is_set(data,7)) 
							{
								// if we have hit a trigger it should be max len - 1
								cpu->square[2].lengthc = 255;
							}
								
							else
							{
								deset_bit(cpu->io[IO_NR52],2); // disable square 2
							}	
						}	
					}
				}					
					
				
					
				cpu->square[2].length_enabled = is_set(data,6);

	
			
				cpu->io[IO_NR34] = data | (16 + 32 + 8);
				if(!is_set(cpu->io[IO_NR30],7)) 
				{
					deset_bit(cpu->io[IO_NR52],2);
				}
			}
			return;
		}
			
			
		// nr 41	
		case IO_NR41:
		{
			if(cpu->sound_enabled)
			{
					
				// bottom 6 bits are length data 
				// set the internal counter to 64 - bottom 6 bits of data
				cpu->square[3].lengthc = 64 - (data & 63);	
				cpu->io[IO_NR41] = data | 192;
			}
			return;
		}
			
		// nr 42
		case IO_NR42:
		{
			if(cpu->sound_enabled)
			{
					
				// if the top 5 bits are deset (channel dac)
				// disable the channel
				if((data & 248) == 0)
				{
					deset_bit(cpu->io[IO_NR52],3);
				}
				cpu->square[3].volume_load = (data >> 4) & 0x7;
				cpu->square[3].volume = cpu->square[3].volume_load;
				cpu->square[3].env_load = data & 0x3;
				cpu->io[IO_NR42] = data;
			}
			return;
		}
			

		// NR 43
		case IO_NR43:
		{
			if(cpu->sound_enabled)
			{
				cpu->io[IO_NR43] = data;
				cpu->divisor_idx = data & 0x7;
				cpu->counter_width = is_set(data,3);
				cpu->clock_shift = (data >> 4) & 0xf;
			}
			return;
		}			
			
			
		// nr 44
		case IO_NR44:
		{
			if(cpu->sound_enabled)
			{
				// if the data is set to 7 it should enable Sound for nr4
				// in nr52
				if(is_set(data,7))
				{

					set_bit(cpu->io[IO_NR52],3);
						
						
					// if the length counter is 0 it should be loaded with max upon a trigger event
					if(cpu->square[3].lengthc == 0)
					{
						cpu->square[3].lengthc = 64;
						deset_bit(cpu->io[IO_NR44],6);
					}
					// reload period and reset duty
					cpu->square[3].volume = cpu->square[3].volume_load;
					cpu->square[3].period = ((2048 - cpu->square[3].freq))*4;
					cpu->square[3].env_period = cpu->square[3].env_load;
					cpu->square[3].env_enabled = true;	

					// noise channel stuff
					cpu->square[3].period = (divisors[cpu->divisor_idx] << cpu->clock_shift);
					cpu->shift_reg = 0x7fff;
		
				}
					
				// if previously clear and now is enabled 
				// + next step doesent clock, clock the length counter
				if(!is_set(cpu->io[IO_NR44],6) && is_set(data,6) && !(cpu->sequencer_step & 1))
				{
					// if not zero decrement
					if(cpu->square[3].lengthc != 0)
					{
						
							
						// decrement and if now zero and there is no trigger 
						// switch the channel off
						if(!--cpu->square[3].lengthc)
						{
							if(is_set(data,7)) 
							{
								// if we have hit a trigger it should be max len - 1
								cpu->square[3].lengthc = 0x3f;
							}
								
							else
							{
								deset_bit(cpu->io[IO_NR52],3); // disable square 4
							}	
						}	
					}
				}
					
				if((cpu->io[IO_NR42] & 248) == 0) // if dac  disabled
				{
					deset_bit(cpu->io[IO_NR52],3);
				}
					
				cpu->square[3].length_enabled = is_set(data,6);
				cpu->io[IO_NR44] = data | 63;
			}
			return;
		}

		// nr 50
		case IO_NR50:
		{
			if(cpu->sound_enabled)
			{
				cpu->io[IO_NR50] = data;
			}
			return;
		}

			
		case IO_NR51:
		{
			if(cpu->sound_enabled)
			{
				cpu->io[IO_NR51] = data;
			}
			return;
		}			
			
		// nr 52 // bits 0-3 read only 7 r/w 4-6 unused
		case IO_NR52:
		{
			cpu->io[IO_NR52] |= 112;
			
			if(is_set(cpu->io[IO_NR52] ^ data,7)) // if we are going from on to off reset the sequencer
			{
				cpu->sequencer_step = 0;
			}



			// if we have disabled sound we should
			// zero all the registers (nr10-nr51) 
			// and lock writes until its on
			if(!is_set(data,7))
			{
				// set nr10-nr51 regs to 0
				for(int i = 0x10; i < 0x26; i++)
				{
					write_io(cpu,i,0);	
				} 
				
				cpu->io[IO_NR52] = 112; // need to write the unused bits and just zero everything else
	
				// now lock writes
				cpu->sound_enabled = false;

			}
			
			else // its enabled
			{
				cpu->sound_enabled = true;
				cpu->io[IO_NR52] |= 0x80; // data had 0x80 so write back
			}
			
			return;
		}


		case IO_LCDC: // lcdc
		{
			if(!is_set(data,7) && is_lcd_enabled(cpu)) // lcd switched off this write
			{
				cpu->scanline_counter = 0; // counter is reset
				cpu->current_line = 0; // reset ly
				cpu->io[IO_STAT] &= ~3; // mode 0
				cpu->mode = 0;
			}
			
			if(is_set(data,7) && !is_lcd_enabled(cpu))
			{
				cpu->scanline_counter = 0;
				cpu->io[IO_STAT] |= 2; // mode 2?
				cpu->mode = 2;
			}
			
			cpu->io[IO_LCDC] = data;
			return;
		}


		// lcd stat <-- writes can trigger interrupts?
		case IO_STAT:
		{
			// delete writeable bits
			cpu->io[IO_STAT] &= 7;
				
			// set the w bits with the value
			cpu->io[IO_STAT] |= data & ~7;
			
			// set unused bit
			cpu->io[IO_STAT] |= 0x80;
			return;
		}

		// block ly writes
		case IO_LY:
		{
			return;
		} 
			
			
		// implement timing on dma and page boundary checking
		case IO_DMA: // dma reg perform a dma transfer //<-- may need seperate handling in a do_dma
		{
			do_dma(cpu,data);
			return;
		}
				
		case IO_IF:
		{
			cpu->io[IO_IF] = data | (128 + 64 + 32); // top 3 bits allways on
			return;
		}
		

		// cgb regs
		
		// cgb ram bank number
		case IO_SVBK:
		{
			if(cpu->is_cgb)
			{
				cpu->cgb_ram_bank_num = data & 0x7;
				
				// bank 0 is same as accessing bank 1
				if(cpu->cgb_ram_bank_num == 0)
				{
					cpu->cgb_ram_bank_num = 1;
				}
				
				// index it
				cpu->cgb_ram_bank_num -= 1;
				
				cpu->io[IO_SVBK] = data | 248;
				
			}
			
			else
			{
				cpu->io[IO_SVBK] = data;
			}
			
			return;	
		}

		case IO_SPEED:
		{
			// not cgb return ff 
			if(!cpu->is_cgb)
			{
				cpu->io[address & 0xff] = 0xff;
				return;
			}

			cpu->io[IO_SPEED] = (cpu->io[IO_SPEED] & 0x80) | (data & 0x1) | 0x7e;
			return;
		}

		case IO_VBANK: // what vram bank are we writing to?
		{
			// not cgb return data
			if(!cpu->is_cgb)
			{
				cpu->io[address & 0xff] = data;
				return;
			}
			
			if(cpu->is_cgb)
			{
				cpu->vram_bank = data & 1;
				cpu->io[IO_VBANK] = data | 254;
			}
			return;
		}

		case IO_BGPI:
		{
			// not cgb return ff 
			if(!cpu->is_cgb)
			{
				cpu->io[address & 0xff] = data;
				return;
			}
			
			cpu->bg_pal_idx = data &  0x3f;
			cpu->io[IO_BGPI] = data | 0x40;
			return;
		}

		case IO_BGPD: // finish later 
		{
			// not cgb return ff 
			if(!cpu->is_cgb)
			{
				cpu->io[address & 0xff] = data;
				return;
			}
			
			if(cpu->mode <= 1)
			{
				cpu->bg_pal[cpu->bg_pal_idx] = data; 
			}
			if(is_set(cpu->io[IO_BGPI],7)) // increment on a write 
			{
				cpu->bg_pal_idx = (cpu->bg_pal_idx + 1) & 0x3f;
				cpu->io[IO_BGPI] &= ~0x3f;
				cpu->io[IO_BGPI] |= cpu->bg_pal_idx;
			}
			return;
		}

		case IO_SPPI: // sprite pallete index
		{
			// not cgb return ff 
			if(!cpu->is_cgb)
			{
				cpu->io[address & 0xff] = data;
				return;
			}			
			
			cpu->sp_pal_idx = data & 0x3f;
			cpu->io[IO_SPPI] = data | 0x40;
			return;
		}		
		
		case IO_SPPD: // sprite pallete data
		{
			// not cgb return ff 
			if(!cpu->is_cgb)
			{
				cpu->io[address & 0xff] = data;
				return;
			}			
			
			// only in hblank and vblank
			if(cpu->mode <= 1)
			{
				cpu->sp_pal[cpu->sp_pal_idx] = data; 
			}
			if(is_set(cpu->io[IO_SPPI],7)) // increment on a write 
			{
				cpu->sp_pal_idx = (cpu->sp_pal_idx + 1) & 0x3f;
				cpu->io[IO_SPPI] &= ~0x3f;
				cpu->io[IO_SPPI] |= cpu->sp_pal_idx;
			}
			return;
		}
	
	
		// specifies src byte dest of dma
		case IO_HDMA1:
		{
			cpu->dma_src &= 0xff;
			cpu->dma_src |= data << 8;
			cpu->io[IO_HDMA1] = data;
			return;
		}
		
		// lo byte dma src
		case IO_HDMA2:
		{
			data &= 0xf0;
			cpu->dma_src &= ~0xff;
			cpu->dma_src |= data;
			cpu->io[IO_HDMA2] = data;
			return;
		}
		
		
		// high byte dma dst
		case IO_HDMA3:
		{
			data &= 0x1f;
			cpu->dma_dst &= 0xff;
			cpu->dma_dst |= data << 8;
			cpu->io[IO_HDMA3] = data;
			return;
		}
		
		// low byte dma dst
		case IO_HDMA4:
		{
			data &= 0xf0;
			cpu->dma_dst &= ~0xff;
			cpu->dma_dst |= data;
			cpu->io[IO_HDMA4] = data;
			return;
		}
	
		// cgb dma start
		case IO_HDMA5:
		{
			cpu->io[IO_HDMA5] = data;
			// if data is zero do a gdma
			// bit 1 will start a hdma during hblank
			
			// writing 0 to bit 7 terminates a hdma transfer
			if(!is_set(data,7))
			{
				cpu->hdma_active = false;
			}
			
			else // start a hdma
			{
				//puts("unhandled gdma!");
				// number of 16 byte incremnts to transfer
				cpu->hdma_len = (data & 0x7f)+1;
				cpu->hdma_len_ticked = 0;
				cpu->hdma_active = true;
			}
			
			if(is_set(data,0)) 
			{
				start_gdma(cpu);
			}
			return;
		}
		
		// default for hram
		default:
		{	
			cpu->io[address & 0xff] = data;
			return;
		}
	}
}





// 0x8000 - 0x9fff
void write_vram_mem(Cpu *cpu, uint16_t address ,int data)
{
	// vram can only be accesed at mode 0-2
	
	if(cpu->mode <= 2)
	{
		uint16_t addr = address - 0x8000;
		// bank is allways zero in dmg mode
		cpu->vram[cpu->vram_bank][addr] = data;	
	}
	return;
}

// 0xa000 - 0xbfff
void write_cart_mem(Cpu *cpu, uint16_t address, int data)
{
	// if ram enabled
	if(cpu->enable_ram && cpu->currentram_bank != RTC_ENABLED)
	{
		uint16_t new_address = address - 0xa000;
		//printf("write ram bank: %x : %x\n",cpu->currentram_bank,data);
		cpu->ram_banks[new_address + (cpu->currentram_bank * 0x2000)] = data;
		return;
	}
}


// WRAM
// 0xc000 - 0xcfff
void write_wram_low(Cpu *cpu, uint16_t address, int data)
{
	// same on both dmg and cgb
	uint16_t addr = address & 0xfff;
	cpu->wram[addr] = data;
	return;
}

// 0xd000 - 0xdfff
void write_wram_high(Cpu *cpu, uint16_t address, int data)
{
	uint16_t addr = address & 0xfff;
	// on dmg the work ram bank is fixed!
	cpu->cgb_ram_bank[cpu->cgb_ram_bank_num][addr] = data;
	return;	
}

void write_hram(Cpu *cpu, uint16_t address, int data)
{
	// io
	if(address >= 0xff00)
	{
		write_io(cpu,address,data);
		return;
	}
	
	else if(address >= 0xf000 && address <= 0xfdff)
	{
		uint16_t addr = address & 0xfff;
		// on dmg the work ram bank is fixed!
		cpu->cgb_ram_bank[cpu->cgb_ram_bank_num][addr] = data;
		return;
	}

	// oam is accesible during mode 0-1
	else if(address >= 0xfe00 && address <= 0xfe9f)
	{
		// should be blocked during dma but possibly not to the ppu
		if(cpu->mode <= 1 && !cpu->oam_dma_active)
		{
			write_oam(cpu,address,data);
		}
		return;
	}	
	
	// restricted 
	else if( (address >= 0xFEA0) && (address <= 0xFEFF) )
	{
		return;
	}
	printf("write fall through: %x, %x!\n",address,data);
	exit(1);
}


void write_mem(Cpu *cpu,uint16_t address,int data)
{

	#ifdef DEBUG
	// write breakpoint
	if(address == cpu->memw_breakpoint)
	{
		if(cpu-> memw_value == -1 || cpu->memw_value == data)
		{
			printf("Write breakpoint (%x)!\n",cpu->memw_breakpoint);
			printf("data %x\n",data);
			enter_debugger(cpu);
		}
	}
	#endif
	
	cpu->memory_table[(address & 0xf000) >> 12].write_memf(cpu,address,data);
}

// returns a pointer directly to the memory at the requested address 
// used internally by the emulator when we want to poll a section of memory 

// need to optimise with switch on each 0x1000 range like read_mem

uint8_t *get_direct_mem_access(Cpu *cpu, uint16_t address)
{
	uint8_t *mem_ptr = NULL;
	
	
	switch((address & 0xf000) >> 12)
	{
		// rom bank 0
		case 0 ... 3:
		{
			mem_ptr = &cpu->rom_mem[address];
			break;
		}
		
		// rom bank x
		case 4 ... 7:
		{
			uint16_t new_address = address - 0x4000;
			mem_ptr = &cpu->rom_mem[new_address + (cpu->currentrom_bank*0x4000)];
			break;
		}
		
		// vram
		case 8 ... 9:
		{
			mem_ptr = &cpu->vram[cpu->vram_bank][address - 0x8000];
			break;
		}
		
		// cartridge ram banks
		case 0xa ... 0xb:
		{
			// is ram enabled (ram bank of -1 indicates rtc reg)
			if(cpu->enable_ram && cpu->currentram_bank != RTC_ENABLED)
			{
				uint16_t new_address = address - 0xa000;
				mem_ptr = &cpu->ram_banks[new_address + (cpu->currentram_bank * 0x2000)];
			}
			
			// may need to handle ram being locked		

			break;
		}
		
		// work ram
		case 0xc ... 0xd:
		{
			uint16_t addr = address & 0xfff;
			if(!cpu->is_cgb)
			{
				mem_ptr = &cpu->wram[addr];
			}
				
			else 
			{
				
				if(address >= 0xc000 && address <= 0xcfff)
				{
					mem_ptr = &cpu->wram[addr];
				}
					
				// 0xd000-0xdfff
				else
				{
					mem_ptr = &cpu->cgb_ram_bank[cpu->cgb_ram_bank_num][addr];
				}	
			}
			break;
		}
		
		// echo ram
		case 0xe:
		{
			mem_ptr = &cpu->wram[address & 0xfff];
			break;
		}
		
		// io
		
		case 0xf:
		{
			mem_ptr = &cpu->io[address & 0xfff];
			break;
		}
		
		
		default:
		{
			printf("INVALID DIRECT MEM ACCESS AT %X\n", address);
			exit(1);			
		}
	}
	
	return mem_ptr;
}


void do_dma(Cpu *cpu, uint8_t data)
{
	cpu->io[IO_DMA] = data; // write to the dma reg
	uint16_t dma_address = data  << 8;
	// transfer is from 0xfe00 to 0xfea0
			
	/*// must be page aligned revisit later
	if(dma_address % 0x100) return;		
	*/

	// source must be below 0xe000
	// tick immediatly but keep the timing
	if(dma_address < 0xe000)
	{
		// old implementaiton
		for(int i = 0; i < 0xA0; i++)
		{
			write_oam(cpu,0xfe00+i, read_mem(dma_address+i,cpu)); 	
		}
		
/*
		//optimised but likely error prone version...
		uint8_t *dma_src = get_direct_mem_access(cpu,dma_address);
		memcpy(cpu->oam,dma_src,0xA0);	
*/	
	}

	// tell the emulator to start ticking the dma transfer
	// source must be below 0xe000 <-- playing like hell fix later
	
	// technically dma should take a cycle before it starts
	
	if(dma_address < 0xe000)
	{
		cpu->oam_dma_active = true; // indicate a dma is active and to lock memory
		cpu->oam_dma_address = dma_address; // the source address
		cpu->oam_dma_index = 0; // how far along the dma transfer we are
	}
	

}





uint8_t read_vram(uint16_t address, Cpu *cpu)
{
	
	uint16_t addr = address - 0x8000;
	
	// onlly accessible between mode 0-2
	if(cpu->mode <= 2)
	{
		// in dmg mode the bank will allways be zero
		return cpu->vram[cpu->vram_bank][addr];	
	}
	
	return 0xff;
}

uint8_t read_oam(uint16_t address, Cpu *cpu)
{
	return cpu->oam[address & 0xff];
}


uint8_t read_io(uint16_t address, Cpu *cpu)
{
	#ifdef DEBUG
	// read breakpoint
	if(address == cpu->memr_breakpoint)
	{
		int break_backup = cpu->memr_breakpoint;
		cpu->memr_breakpoint = -1;
		if(cpu->memr_value == -1 || cpu->memr_value == read_mem(address,cpu))
		{
			cpu->memr_breakpoint = break_backup;
			printf("read breakpoint (%x)!\n",cpu->memr_breakpoint);
			enter_debugger(cpu);
		}
		cpu->memr_breakpoint = break_backup;
	}
	#endif
	
	switch(address & 0xff)
	{

		case 0x00: // joypad control reg <-- used for sgb command packets too
		{
				

				
			// read from mem
			uint8_t req = cpu->io[IO_JOYPAD];
			// we want to test if bit 5 and 4 are set 
			// so we can determine what the game is interested
			// in reading
						
				
			// read out dpad 
			if(!is_set(req,4))
			{
				return ( (req & 0xf0) | (cpu->joypad_state & 0x0f) );
			}
			// read out a b sel start 
			else if(!is_set(req,5))
			{
				return ( (req & 0xf0) | ((cpu->joypad_state >> 4) & 0xf ) );
			}		
				
			return 0xff; // return all unset
		}	
			
			
		case IO_DIV:
		{
			// div register is upper 8 bits of the internal timer
			return (cpu->internal_timer & 0xff00) >> 8;
		}
		
		// sound regs
			
		case IO_SB: // <-- stub for failed transfer 
		{
			return cpu->io[IO_SB];
		}
		
		case IO_LY:
		{
			return cpu->current_line;
		}
			
		// nr 11 only 7 and 6 readable
		case IO_NR11:
		{
			return (cpu->io[IO_NR11] & (128 + 64)) | (0xff-(128+64));
		}
			
		// write only
		case IO_NR13:
		{
			return 0xff;
		}
			
		// nr 14 only 6 is readable
		case IO_NR14:
		{
			return (cpu->io[IO_NR14] & (64)) | (0xff-64);
		}
			
		// nr 21 only bits 6-7 are r 
		case IO_NR21:
		{
			return (cpu->io[IO_NR21] & (128 + 64)) | (0xff-(128+64));		
		}
			
		// nr 23 write only
		case IO_NR23:
		{
			return 0xff;
		}
			
		// nr 24 only bit 6 can be read 
		case IO_NR24:
		{
			return (cpu->io[IO_NR24] & (64)) | (0xff-64);	
		}
			
		// nr 30 only bit 7
		case IO_NR30:
		{
			return (cpu->io[IO_NR30] & (128)) | (0xff-128);	
		}
			
		// nr 31 <-- unsure
		case IO_NR31:
		{
			return 0xff;
		}
			
		// nr 32 6-5 r
		case IO_NR32:
		{
			return (cpu->io[IO_NR32] & (64 + 32)) | (0xff-(64+32));
		}
			
		// nr33 write only
		case IO_NR33:
		{
			return 0xff;
		}
			
		// nr 34 6 r
		case IO_NR34:
		{
			return (cpu->io[IO_NR34] & (64)) | (0xff-64);
		}
			
		// nr 41
		// requires additional handling
		case IO_NR41:
		{
			return 0xff;
		}

			// nr 44 bit 6
		case IO_NR44:
		{
			return (cpu->io[IO_NR44] & (64)) | (0xff-64);		
		}
			
			// heck knows unsure
		case 0x2a ... 0x2f:
		{
			return 0xff;
		}
		
		case 0x30 ... 0x3f:
		{
			
			if(is_set(cpu->io[IO_NR52],2)) // wave channel on return current sample
			{
				return cpu->io[0x30 + (cpu->square[2].duty_idx / 2)];
			}
			
			
			else // return normally
			
			{
				return cpu->io[address & 0xff];
			}
		}	

		
		
		// CGB
		
		
		
		// cgb vram bank unused on dmg
		case IO_VBANK:
		{
			return cpu->io[IO_VBANK];
		}

		
		case IO_BGPD: 
		{
			return cpu->bg_pal[cpu->bg_pal_idx];
		}
		
		case IO_SPPD:
		{
			return cpu->sp_pal[cpu->sp_pal_idx];
		}

		
		case IO_HDMA5: // verify
		{
			if(!cpu->hdma_active)
			{
				return 0xff;
			}
			
			// return the lenght tick left
			else 
			{
				return (cpu->hdma_len - 1);
			}
		}
		
		
		// default for hram
		default:
		{	
			return cpu->io[address & 0xff];
		}
	}
}
