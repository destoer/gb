
// memory accesses (READ THE PANCDOCS ON MEMORY MAP)
// may need access to information structs

// needs ones related to banking
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
void write_mem(Cpu *cpu,uint16_t address,int data);
uint8_t read_mem(uint16_t address, Cpu *cpu);
void do_dma(Cpu *cpu, uint8_t data);

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
		if(status <= 1)
		{
			cpu->oam[address & 0xff] = data;
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
				cpu->io[IO_TIMA] = 0;
				return;
			}
			
			

			// nr 10
			case IO_NR10:
			{
				cpu->io[IO_NR10] = data | 128;
				return;
			}


			// nr 30
			case IO_NR30:
			{
				cpu->io[IO_NR30] = (data & 128) | 127;
				return;
			}

			// nr 32
			case IO_NR32:
			{
				cpu->io[IO_NR32] = data | 159;
				return;
			}

			
			// nr 34
			case IO_NR34:
			{
				cpu->io[IO_NR34] = data | (16 + 32 + 8);
				return;
			}
			
			
			// nr 41	
			case 0x20:
			{
				cpu->io[0x20] = data | 192;
				return;
			}

			// nr 44
			case IO_NR44:
			{
				cpu->io[IO_NR44] = data | 63;
				return;
			}

			


			// nr 52 // bits 0-3 read only 7 r/w 4-6 unused
			case IO_NR52:
			{
				cpu->io[IO_NR52] = cpu->io[IO_NR52] & 0xf;
				cpu->io[IO_NR52] |= 112;
				cpu->io[IO_NR52] |= data & 128; 
				return;
			}


			case IO_LCDC: // lcdc
			{
				cpu->io[IO_LCDC] = data;
				if(!is_lcd_enabled(cpu) && (data & 0x80)) // lcd switched off this write
				{
					cpu->scanline_counter = 0; // counter is reset
					cpu->io[IO_LY] = 0; // reset ly
					cpu->io[IO_STAT] &= ~3;
					cpu->io[IO_STAT] |= 128;	
				}
				return;

			}


			// lcd stat <-- writes can trigger intterupts?
			case IO_STAT:
			{
				// delete writeable bits
				cpu->io[IO_STAT] &= 7;
				
				// set the w bits with the value
				cpu->io[IO_STAT] |= data & ~7;
			
				// set unused bit
				cpu->io[IO_STAT] |= 0x80;

				// update the stat state
				update_stat(cpu);
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
	if(dma_address < 0xe000)
	{
		for(int i = 0; i < 0xA0; i++)
		{
			write_mem(cpu,0xfe00+i, read_mem(dma_address+i,cpu));
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
			return cpu->vram[address - 0x8000];
		}
		
		return 0xff; // return ff if you cant read
		
	}

	// oam is accesible during mode 0-1
	else if(address >= 0xfe00 && address <= 0xfe9f)
	{
		uint8_t status = read_mem(0xff41,cpu);
		status &= 3; // get just the mode
		if(status <= 1)
		{
			return cpu->oam[address & 0xff];
		}
		
		return 0xff; // cant read so return ff
	}
	
	// io mem
	else if(address >= 0xff00)
	{	

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
			
			

			
			// sound regs
			
			case IO_SB: // <-- stub for failed transfer 
			{
				return cpu->io[IO_SB];
			}
			
			// nr 11 only 7 and 6 readable
			case IO_NR11:
			{
				return (cpu->io[IO_NR11] & (128 + 64)) | (0xff-(128+64));
			}
			
			// write only
			case 0x13:
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
	
	#ifdef DEBUG
	else
	{
		printf("unhandled read %x\n",address);
		exit(1);
	}
	#endif
	return 0xff;
}

