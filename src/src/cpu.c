#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "headers/cpu.h"
#include "headers/disass.h"
#include "headers/lib.h"
#include "headers/banking.h"
#include "headers/instr.h"
#include "headers/debug.h"
#include "headers/ppu.h"




uint8_t read_mem(uint16_t address, Cpu *cpu);
bool is_set(uint8_t reg, uint8_t bit);
int set_clock_freq(Cpu *cpu);
void service_interrupt(Cpu *cpu,int interrupt);


// fix interrupt timing

// WORK on sound emulation 1st make sure unused bits are properly handled



// setup a fresh cpu state and return it to main
Cpu init_cpu(void) // <--- memory should be randomized on startup
{	
	Cpu cpu;
	cpu.mem = calloc(0x10000, sizeof(uint8_t)); // main memory
	cpu.currentram_bank = 0;
	cpu.currentrom_bank = 1; // all ways at least one
	cpu.rom_banking = true; // default
	cpu.enable_ram = false;
	cpu.rtc_enabled = false;
	//different values for the test!?
	cpu.af.reg = 0x01B0; // <-- correct values
	cpu.bc.reg = 0x0013;
	cpu.de.reg = 0x00D8;
	cpu.hl.reg = 0x014d;
	cpu.sp = 0xFFFE;
	cpu.cycles = 0; // cycles from interrupts etc

	cpu.mem[0xFF10] = 0x80;
	cpu.mem[0xFF11] = 0xBF;	
	cpu.mem[0xFF12] = 0xF3;
	cpu.mem[0xFF14] = 0xBF;
	cpu.mem[0xFF16] = 0x3F;
	cpu.mem[0xFF19] = 0xBF;
	cpu.mem[0xFF1A] = 0x7F;
	cpu.mem[0xFF1B] = 0xFF;
	cpu.mem[0xFF1C] = 0x9F;
	cpu.mem[0xFF1E] = 0xBF;
	cpu.mem[0xFF20] = 0xFF;
	cpu.mem[0xFF23] = 0xBF;
	cpu.mem[0xFF24] = 0x77;
	cpu.mem[0xFF25] = 0xF3;
	cpu.mem[0xFF26] = 0xF1;
	cpu.mem[0xFF40] = 0x91;
	cpu.mem[0xFF47] = 0xFC;
	cpu.mem[0xFF48] = 0xFF;
	cpu.mem[0xFF49] = 0xFF;
	
	
	
	
	// init unused hwio
	cpu.mem[0xff03] = 0xff;
	cpu.mem[0xff08] = 0xff;
	cpu.mem[0xff09] = 0xff;
	cpu.mem[0xff0a] = 0xff;
	cpu.mem[0xff0b] = 0xff;
	cpu.mem[0xff0c] = 0xff;
	cpu.mem[0xff0d] = 0xff;
	cpu.mem[0xff0e] = 0xff;
	cpu.mem[0xff15] = 0xff;
	cpu.mem[0xff1f] = 0xff;
	cpu.mem[0xff27] = 0xff;
	cpu.mem[0xff28] = 0xff;
	cpu.mem[0xff29] = 0xff;
	cpu.mem[0xff20] = 0xff;

	
	cpu.pc = 0x100; // reset at 0x100
	cpu.tacfreq = 256; // number of machine cycles till update
	cpu.div_counter = 0;
	cpu.scanline_counter = 0;
	cpu.signal = false;
	cpu.joypad_state = 0xff;
	cpu.interrupt_enable = false;
	
	cpu.di = false;
	cpu.ei = false;
	cpu.halt = false;
	cpu.halt_bug = false;
	
	#ifdef DEBUG
	// debugging vars 
	cpu.breakpoint = 0x100;
	cpu.memr_breakpoint = -1;
	cpu.memw_breakpoint = -1;
	cpu.memr_value = -1;
	cpu.memw_value = -1;
	#endif


	return cpu;
}



void request_interrupt(Cpu * cpu,int interrupt)
{
	// set the interrupt flag to signal
	// an interrupt request
	uint8_t req = cpu->mem[0xff0f];
	set_bit(req,interrupt);
	cpu->mem[0xff0f] = req;
}



void do_interrupts(Cpu *cpu)
{
	
	// if interrupts are enabled
	if(cpu->interrupt_enable)
	{	
		// get the set requested interrupts
		uint8_t req = cpu->mem[0xff0f];
		// checked that the interrupt is enabled from the ie reg 
		uint8_t enabled = cpu->mem[0xffff];
		
		if(req > 0)
		{
			// priority for servicing starts at interrupt 0
			for(int i = 0; i < 5; i++)
			{
				
				// if requested
				if(is_set(req,i))
				{
					// check that the particular interrupt is enabled
					if(is_set(enabled,i))
					{
						service_interrupt(cpu,i);
						cpu->cycles += 5; // 5 for interrupt dispatch
					}
				}
			}
		}
	}
	update_timers(cpu,cpu->cycles);
	cpu->cycles = 0;
}

void service_interrupt(Cpu *cpu,int interrupt)
{
	
	
	cpu->interrupt_enable = false; // disable interrupts now one is serviced
		
	// reset the bit of in the if to indicate it has been serviced
	uint8_t req = read_mem(0xff0f,cpu);
	deset_bit(req,interrupt);
	write_mem(cpu,0xff0f,req);
		
	// push the current pc on the stack to save it
	// it will be pulled off by reti or ret later
	write_stackw(cpu,cpu->pc);

		
	// set the program counter to the start of the
	// interrupt handler for the request interrupt
		
	switch(interrupt)
	{
		// interrupts are one less than listed in cpu manual
		// as our bit macros work from bits 0-7 not 1-8
		case 0: cpu->pc = 0x40;  break; //vblank
		case 1: cpu->pc = 0x48; break; //lcd-state 
		case 2: cpu->pc = 0x50; break; // timer 
		case 3: cpu->pc = 0x58; break; //serial (not fully implemented)
		case 4: cpu->pc = 0x60; break; // joypad
		default: printf("Invalid interrupt %d at %x\n",interrupt,cpu->pc); exit(1);
	}	
}


// memory accesses (READ THE PANCDOCS ON MEMORY MAP)
// may need access to information structs

// needs ones related to banking



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

	
	// ECHO ram also writes in ram
	else if( (address >= 0xE000) && (address < 0xFE00))
	{
		cpu->mem[address] = data;
		cpu->mem[address-0x2000] = data;
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
			cpu->mem[address] = data;
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
			cpu->mem[address] = data;
		}
		return;
	}

	
	// restricted 
	else if( (address >= 0xFEA0) && (address < 0xFEFF) )
	{
		return;
	}
	



	// update the timer freq (tac in gb docs)
	else if(address == TMC)
	{
		
		// timer is set to CLOCKSPEED/freq/4 (4 to convert to machine cycles)
		// <--- needs verifying
		uint8_t currentfreq = cpu->mem[TMC] & 3;
		cpu->mem[TMC] = data | 248;
		
		if(currentfreq != (data & 3) )
		{
			cpu->timer_counter = set_clock_freq(cpu);
		}
		return;
	}
	

	

/*	// serial control (stub)
	else if(address == 0xff02)
	{
		data |= 2; // <-- for dmg 
		cpu->mem[address] = data | (~0x83);
		deset_bit(cpu->mem[0xff02],7); // <- cheat to time it out immediately lol
		request_interrupt(cpu,3);
		cpu->mem[0xff01] = 0xff;		
		return; 
	}
*/
	//---------------------------
	// unused hwio
	
	// unused
	else if(address == 0xff03)
	{
		cpu->mem[address] = 0xff;
		return;
	}
	
	// unused
	else if(address >= 0xff08 && address <= 0xff0e )
	{
		cpu->mem[address] = 0xff;
		return;
	}
	
	
	// unused
	else if(address == 0xff15 )
	{
		cpu->mem[address] = 0xff;
		return;
	}
	

	// unused
	else if(address == 0xff1f)
	{
		cpu->mem[address] = 0xff;
		return;
	}
	
	
	// unused
	else if(address == 0xff27 )
	{
		cpu->mem[address] = 0xff;
		return;
	}
	
	
	// unused
	else if(address == 0xff28)
	{
		cpu->mem[address] = 0xff;
		return;
	}
	
	// unused
	else if(address == 0xff29)
	{
		cpu->mem[address] = 0xff;
		return;
	}	
	
	
		// unused
	else if(address >= 0xff4c && address <= 0xff7f)
	{
		cpu->mem[address] = 0xff;
		return;
	}
	


	// div and tima share the same internal counter
	// should account for this internal behavior
	else if(address == DIV)
	{
		cpu->mem[DIV] = 0;
		cpu->mem[TIMA] = 0;
		return;
	}
	
	

	// nr 10
	else if(address == 0xff10)
	{
		cpu->mem[address] = data | 128;
		return;
	}


	// nr 30
	else if(address == 0xff1a)
	{
		cpu->mem[address] = (data & 128) | 127;
		return;
	}

	// nr 32
	else if(address == 0xff1c)
	{
		cpu->mem[address] = data | 159;
		return;
	}

	
	// nr 34
	else if(address == 0xff1e)
	{
		cpu->mem[address] = data | (16 + 32 + 8);
	}
	
	
	// nr 41	
	else if(address == 0xff20)
	{
		cpu->mem[address] = data | 192;
		return;
	}

	// nr 44
	else if(address == 0xff23)
	{
		cpu->mem[address] = data | 63;
		return;
	}

	


	// nr 52 // bits 0-3 read only 7 r/w 4-6 unused
	else if(address == 0xff26)
	{
		cpu->mem[address] = cpu->mem[0xff26] & 0xf;
		cpu->mem[0xff26] |= 112;
		cpu->mem[0xff26] |= data & 128; 
		return;
	}



	// lcd stat <-- writes can trigger intterupts?
	else if(address == 0xff41)
	{
		cpu->mem[address] = (cpu->mem[address] & 0x7) | ( (data & (248)) | 128); // bit 7 unused 0-2 read only
		
		
		return;
	}

	// block ly writes
	else if (address == 0xFF44)
	{
		return;
	} 
	
	
	// implement timing on dma and page boundary checking
	else if(address == 0xff46) // dma reg perform a dma transfer
	{
		cpu->mem[0xff46] = data; // write to the dma reg
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
		return;
	}
	
	
	else if(address == 0xff0f)
	{
		cpu->mem[address] = data | (128 + 64 + 32); // top 3 bits allways on
		return;
	}
	


	
	// unrestricted
	else
	{
		cpu->mem[address] = data;
	}
}


void write_word(Cpu *cpu,uint16_t address,int data) // <- verify 
{
	write_mem(cpu,address+1,((data & 0xff00) >> 8));
	write_mem(cpu,address,(data & 0x00ff));
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


	// are we reading from a rom bank
	if((address >= 0x4000) && (address <= 0x7FFF))
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
	
	

	// vram can only be accesed at mode 0-2
	else if(address >= 0x8000 && address <= 0x9fff)
	{
		uint8_t status = read_mem(0xff41,cpu);
		status &= 3; // get just the mode
		if(status <= 2)
		{
			return cpu->mem[address];
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
			return cpu->mem[address];
		}
		
		return 0xff; // cant read so return ff
	}
	
	
	
	else if(address == 0xff00) // joypad control reg <-- used for sgb command packets too
	{
		

		
		// read from mem
		uint8_t req = cpu->mem[0xff00];
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
	
	else if(address == 0xff01) // <-- stub for failed transfer 
	{
		return cpu->mem[0xff01];
	}
	
	// nr 11 only 7 and 6 readable
	else if(address == 0xff11)
	{
		return (cpu->mem[address] & (128 + 64)) | (0xff-(128+64));
	}
	
	// write only
	else if(address == 0xff13)
	{
		return 0xff;
	}
	
	// nr 14 only 6 is readable
	else if(address == 0xff14)
	{
		return (cpu->mem[address] & (64)) | (0xff-64);
	}
	
	// nr 21 only bits 6-7 are r 
	else if(address == 0xff16)
	{
		return (cpu->mem[address] & (128 + 64)) | (0xff-(128+64));		
	}
	
	// nr 23 write only
	else if(address == 0xff18)
	{
		return 0xff;
	}
	
	// nr 24 only bit 6 can be read 
	else if(address == 0xff19)
	{
		return (cpu->mem[address] & (64)) | (0xff-64);	
	}
	
	// nr 30 only bit 7
	else if(address == 0xff1a)
	{
		return (cpu->mem[address] & (128)) | (0xff-128);	
	}
	
	// nr 31 <-- unsure
	else if(address == 0xff1b)
	{
		return 0xff;
	}
	
	// nr 32 6-5 r
	else if(address == 0xff1c)
	{
		return (cpu->mem[address] & (64 + 32)) | (0xff-(64+32));
	}
	
	// nr33 write only
	else if(address == 0xff1d)
	{
		return 0xff;
	}
	
	// nr 34 6 r
	else if(address == 0xff1e)
	{
		return (cpu->mem[address] & (64)) | (0xff-64);
	}
	
	// nr 41
	// requires additional handling
	else if(address == 0xff20)
	{
		return 0xff;
	}


	// nr 44 bit 6
	else if(address == 0xff23)
	{
		return (cpu->mem[address] & (64)) | (0xff-64);		
	}
	
	// heck knows unsure
	else if(address >= 0xff2a && address <= 0xff2f)
	{
		return 0xff;
	}
	
	// cgb vram bank unused on dmg
	else if(address == 0xff4f)
	{
		return 0xff;
	}
	
	else
	{
		return cpu->mem[address]; // just a stub for now implement later
	}
}



// checked memory reads
uint16_t read_word(int address, Cpu *cpu)
{
	return read_mem(address,cpu) | (read_mem(address+1,cpu) << 8);
}

// implement the memory stuff for stack minips


void write_stack(Cpu *cpu, uint8_t data)
{
	cpu->sp -= 1; // dec stack pointer before writing mem for push
	write_mem(cpu,cpu->sp,data); // write to stack

}


void write_stackw(Cpu *cpu,uint16_t data)
{
	write_stack(cpu,(data & 0xff00) >> 8);
	write_stack(cpu,(data &0x00ff));
}

uint8_t read_stack(Cpu *cpu)
{
	
	uint8_t data = read_mem(cpu->sp,cpu);
	cpu->sp += 1;
	return data;
}


uint16_t read_stackw(Cpu *cpu)
{
	return read_stack(cpu) | (read_stack(cpu) << 8);
}











// implement internal timer behavior with div and tima
// being one reg 

// updates the timers
void update_timers(Cpu *cpu, int cycles)
{
	
	// divider reg in here for convenience
	cpu->div_counter += cycles;
	if(cpu->div_counter > 64)
	{
		cpu->div_counter = 0; // inc rate
		cpu->mem[DIV]++; // inc the div timer 
						
	}
	
	// if timer is enabled <--- edge case with timer is failing the timing test
	if(is_set(cpu->mem[TMC],2))
	{	
		cpu->timer_counter += cycles;
		
		int threshold = set_clock_freq(cpu);
		
		// update tima register cycles have elapsed
		while(cpu->timer_counter >= threshold)
		{
			cpu->timer_counter -= threshold;
			
			// about to overflow
			if(cpu->mem[TIMA] == 255)
			{	
				cpu->mem[TIMA] = cpu->mem[TMA]; // reset to value in tma
				request_interrupt(cpu,2); // timer overflow interrupt
			}
			
			else
			{
				cpu->mem[TIMA]++;
			}
		}
	}
}

int set_clock_freq(Cpu *cpu)
{
	uint8_t freq = cpu->mem[TMC] & 0x3;
	
	int newfreq = 0;
	
	switch(freq)
	{				
		case 0: newfreq = 256; break; // freq 4096
		case 1: newfreq = 4; break; //freq 262144
		case 2: newfreq = 16; break; // freq 65536
		case 3: newfreq = 64; break; // freq 16382
		default: puts("Invalid freq"); exit(1);
	}
	
	return  newfreq;
}
