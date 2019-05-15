#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "headers/cpu.h"
#include "headers/disass.h"
#include "headers/lib.h"
#include "headers/banking.h"
#include "headers/instr.h"
#include "headers/debug.h"
#include "headers/ppu.h"
#include "headers/apu.h"










void write_mem(Cpu *cpu,uint16_t address,int data);
uint8_t read_mem(uint16_t address, Cpu *cpu);
void do_dma(Cpu *cpu, uint8_t data);


void write_oam(Cpu *cpu, uint16_t address, int data)
{
	cpu->oam[address & 0xff] = data;	
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
				
			// timer is set to CLOCKSPEED/freq/4 (4 to convert to machine cycles)
			// <--- needs verifying
			uint8_t currentfreq = cpu->io[IO_TMC] & 3;
			cpu->io[IO_TMC] = data | 248;
				
			if(currentfreq != (data & 3) )
			{
				cpu->timer_counter = set_clock_freq(cpu);
			}
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
				cpu->io[0x30 + (cpu->wave_idx / 2)] = data;
				return;
			}
			
			else // if its off allow "free reign" over it
			
			{
				cpu->io[address & 0xff] = data;
				return;
			}
		}

		
		// unused
		case 0x4c ... 0x7f:
		{
			cpu->io[address & 0xff] = 0xff;
			return;
		}
			


		// div and tima share the same internal counter
		// should account for this internal behavior
		case IO_DIV:
		{
			cpu->io[IO_DIV] = 0;
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
					
					
				cpu->io[IO_NR12] = data;
			}
			return;
		}
			
		// nr 13
		case IO_NR13:
		{
			if(cpu->sound_enabled)
			{
				cpu->io[IO_NR13] = data;
			}
			return;
		}
			
		// nr 14
		case IO_NR14: 
		{
			if(cpu->sound_enabled)
			{
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
						
						
						
					// Handle the trigger events for the frequency sweep
						
					/*
					Square 1's frequency is copied to the shadow register.
					The sweep timer is reloaded.
					The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
					If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
					*/
					
					cpu->sweep_calced = false; // indicates no sweep freq calcs have happened since trigger	
						
					// Copy the freq to the shadow reg
					cpu->sweep_shadow =  cpu->io[IO_NR13]; // lower 8
					cpu->sweep_shadow |= (data & 0x7) << 8; // upper 3
						
						
					// reload the sweep timer
					cpu->sweep_period = (cpu->io[IO_NR10] >> 4) & 7;
					cpu->sweep_timer = cpu->sweep_period;
					if(cpu->sweep_period == 0) cpu->sweep_timer = 8; // see obscure behavior
					
						
						
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
						// we aernt going to use it if it fails 
						// as it will get disabled immediately
						//cpu->sweep_shadow = calc_freqsweep(cpu);
						//if(cpu->sweep_shadow > 0x7ff)
						uint16_t tmp = calc_freqsweep(cpu);
						if(tmp  > 0x7ff)
						{
							cpu->sweep_timer = 0;
							deset_bit(cpu->io[IO_NR52],0);
							cpu->sweep_enabled = false;	
						}
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
			}
			return;
		}
			
		// nr 24
		case IO_NR24: // <--- does not pass trigger test
		{	
			if(cpu->sound_enabled)
			{
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
			}
			return;
		}

			
		// nr 34
		case IO_NR34:
		{
			if(cpu->sound_enabled)
			{
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
					cpu->wave_idx = 0; // reset the wave index
					cpu->wave_nibble = 0;
					// reload the wave peroid 
					// period (2048-frequency)*2
					uint16_t freq = get_wave_freq(cpu);
					cpu->wave_period = ((2048-freq)*2) / 4; // may need to be / 4 for M cycles
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
			cpu->io[IO_NR52] &= 0xf; //  0xf (cant emulate the bottom bits for now mask them out for now to make this simple)
			cpu->io[IO_NR52] |= 112;
			
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
			
		// default for hram
		default:
		{	
			cpu->io[address & 0xff] = data;
			return;
		}
	}
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


/*	// only allow reads from hram or from dma reg 
	if(cpu->oam_dma_active)
	{
			if(address >= 0xFF80 && address <= 0xFFFE)
			{
				// allow the read to hram
				
			}
			
			// dma reg
			else if(address == 0xff46)
			{
				do_dma(cpu,address); // restart dma
				cpu->io[IO_DMA] = data;
			}
			
			// block do nothing
			return;
	}
*/

	// handle banking 
	if(address < 0x8000)
	{
		handle_banking(address,data,cpu);
		return;
	}
			


	else if(address >= 0xa000 && address < 0xc000)
	{
		// if ram enabled
		if(cpu->enable_ram && cpu->currentram_bank <= 3)
		{
			uint16_t new_address = address - 0xa000;


			//printf("write ram bank: %x : %x\n",cpu->currentram_bank,data);
			cpu->ram_banks[new_address + (cpu->currentram_bank * 0x2000)] = data;
			return;
		}
	}


	// work ram
	else if((address >= 0xc000) && (address <= 0xdfff))
	{
		cpu->wram[address - 0xc000] = data;
		return;
	}

	
	// ECHO ram also writes in ram
	else if( (address >= 0xE000) && (address < 0xFE00))
	{
		cpu->wram[address - 0xe000] = data;
		return;
	}
	
	// two below need imeplementing 
	
	// vram can only be accesed at mode 0-2
	else if(address >= 0x8000 && address <= 0x9fff)
	{
		uint8_t status = read_mem(0xff41,cpu);
		status &= 3; // get just the mode
		if(status <= 2)
		{
			cpu->vram[address - 0x8000] = data;
		}
		return;
	}

	// oam is accesible during mode 0-1
	else if(address >= 0xfe00 && address <= 0xfe9f)
	{
		uint8_t status = read_mem(0xff41,cpu);
		status &= 3; // get just the mode
		// should be blocked during dma but possibly not to the ppu
		if(status <= 1 && !cpu->oam_dma_active)
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
	

	// io
	else if(address >= 0xff00)
	{
		write_io(cpu,address,data);
	}
	


	#ifdef DEBUG
	// unhandled write
	else
	{
		printf("unhandled write %x\n",address);
		exit(1);
	}
	#endif
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
		/*for(int i = 0; i < 0xA0; i++)
		{
			write_oam(cpu,0xfe00+i, read_mem(dma_address+i,cpu)); 	
		}
		*/
		// start here <-----
		
		// check ranges and assign the correct array and offset to the pointer
		// so it can be passed to memcpy
		
		
		uint8_t *dma_src;
	
		if(dma_address < 0x4000)
		{
			dma_src = &cpu->rom_mem[dma_address];
		}
	
		// are we reading from a rom bank
		else if((dma_address >= 0x4000) && (dma_address <= 0x7FFF))
		{
			uint16_t new_address = dma_address - 0x4000;
			dma_src = &cpu->rom_mem[new_address + (cpu->currentrom_bank*0x4000)];
		}

		
		// vram can only be accesed at mode 0-2
		else if(dma_address >= 0x8000 && dma_address <= 0x9fff)
		{
			dma_src = &cpu->vram[dma_address - 0x8000];
		}
		
		
		
		// are we reading from a ram bank
		else if((dma_address >= 0xa000) && (dma_address <= 0xbfff))
		{
		
			// is ram enabled (ram bank of -1 indicates rtc reg)
			if(cpu->enable_ram && cpu->currentram_bank <= 3)
			{
				uint16_t new_address = dma_address - 0xa000;
				dma_src = &cpu->ram_banks[new_address + (cpu->currentram_bank * 0x2000)];
			}
			
			else {puts("dma from locked ram bank!"); exit(1); }
			
			/*else // may require handling for it being blocked
				   // but i cant imagine a game doing this.....
			{
				return 0xff;
			}*/
		}
		
		// work ram
		else if((dma_address >= 0xc000) && (dma_address <= 0xdfff))
		{
			dma_src = &cpu->wram[dma_address - 0xc000];
		}
	
	
		memcpy(cpu->oam,dma_src,0xA0);	
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
	return cpu->vram[address - 0x8000];
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
				return cpu->io[0x30 + (cpu->wave_idx / 2)];
			}
			
			
			else // return normally
			
			{
				return cpu->io[address & 0xff];
			}
		}	

		
		// cgb vram bank unused on dmg
		case 0x4f:
		{
			return 0xff;
		}

		// default for hram
		default:
		{	
			return cpu->io[address & 0xff];
		}
	}
}

// needs reads related to banking after tetris
// also needs the vram related stuff 
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


/*	// only allow reads from hram or from dma reg 
	if(cpu->oam_dma_active)
	{
			if(address >= 0xFF80 && address <= 0xFFFE)
			{
				return cpu->io[address & 0xff];
			}
			
			// dma reg
			else if(address == 0xff46)
			{
				return cpu->io[IO_DMA];
			}
			
			// block do nothing
			return 0xff;
	}
*/
	// rom bank 0
	if(address < 0x4000)
	{
		return cpu->rom_mem[address];
	}


	// are we reading from a rom bank
	else if((address >= 0x4000) && (address <= 0x7FFF))
	{
		uint16_t new_address = address - 0x4000;
		return cpu->rom_mem[new_address + (cpu->currentrom_bank*0x4000)];
	}
		
	
	// are we reading from a ram bank
	else if((address >= 0xa000) && (address <= 0xbfff))
	{
	
		// is ram enabled (ram bank of -1 indicates rtc reg)
		if(cpu->enable_ram && cpu->currentram_bank <= 3)
		{
			uint16_t new_address = address - 0xa000;
			return cpu->ram_banks[new_address + (cpu->currentram_bank * 0x2000)];
		}
		
		else
		{
			return 0xff;
		}
	}
	

	// work ram
	else if((address >= 0xc000) && (address <= 0xdfff))
	{
		return cpu->wram[address - 0xc000];
	}

	
	// ECHO ram 
	else if( (address >= 0xE000) && (address < 0xFE00))
	{
		return cpu->wram[address - 0xe000];
	}	

	// vram can only be accesed at mode 0-2
	else if(address >= 0x8000 && address <= 0x9fff)
	{
		uint8_t status = read_mem(0xff41,cpu);
		status &= 3; // get just the mode
		if(status <= 2)
		{
			return read_vram(address,cpu);
		}
		
		return 0xff; // return ff if you cant read
		
	}

	// oam is accesible during mode 0-1
	else if(address >= 0xfe00 && address <= 0xfe9f)
	{
		uint8_t status = read_mem(0xff41,cpu);
		status &= 3; // get just the mode
		if(status <= 1 && !cpu->oam_dma_active) // cant access during dma? but ppu should?
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
	
	// io mem
	else if(address >= 0xff00)
	{	
		return read_io(address, cpu);
	}
	
	#ifdef DEBUG
	else
	{
		printf("unhandled read %x\n",address);
		exit(1);
	}
	#endif
	return 0xff;
}

