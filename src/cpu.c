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
#include "headers/memory.h"




uint8_t read_mem(uint16_t address, Cpu *cpu);
bool is_set(uint8_t reg, uint8_t bit);
int set_clock_freq(Cpu *cpu);
void service_interrupt(Cpu *cpu,int interrupt);


// fix interrupt timing

// WORK on sound emulation 1st make sure unused bits are properly handled

// implement internal timer behaviour 

// setup a fresh cpu state and return it to main
Cpu init_cpu(void) // <--- memory should be randomized on startup
{	
	Cpu cpu;

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


	cpu.io[0x10] = 0x80;
	cpu.io[0x11] = 0xBF;	
	cpu.io[0x12] = 0xF3;
	cpu.io[0x14] = 0xBF;
	cpu.io[0x16] = 0x3F;
	cpu.io[0x19] = 0xBF;
	cpu.io[0x1A] = 0x7F;
	cpu.io[0x1B] = 0xFF;
	cpu.io[0x1C] = 0x9F;
	cpu.io[0x1E] = 0xBF;
	cpu.io[0x20] = 0xFF;
	cpu.io[0x23] = 0xBF;
	cpu.io[0x24] = 0x77;
	cpu.io[0x25] = 0xF3;
	cpu.io[0x26] = 0xF1;
	cpu.io[0x40] = 0x91;
	cpu.io[0x47] = 0xFC;
	cpu.io[0x48] = 0xFF;
	cpu.io[0x49] = 0xFF;
	
	
	
	
	// init unused hwio
	cpu.io[0x03] = 0xff;
	cpu.io[0x08] = 0xff;
	cpu.io[0x09] = 0xff;
	cpu.io[0x0a] = 0xff;
	cpu.io[0x0b] = 0xff;
	cpu.io[0x0c] = 0xff;
	cpu.io[0x0d] = 0xff;
	cpu.io[0x0e] = 0xff;
	cpu.io[0x15] = 0xff;
	cpu.io[0x1f] = 0xff;
	cpu.io[0x27] = 0xff;
	cpu.io[0x28] = 0xff;
	cpu.io[0x29] = 0xff;
	cpu.io[0x20] = 0xff;

	
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
	uint8_t req = cpu->io[IO_IF];
	set_bit(req,interrupt);
	cpu->io[IO_IF] = req;
}



void do_interrupts(Cpu *cpu)
{
	//int cycles = 0;
	// if interrupts are enabled
	if(cpu->interrupt_enable)
	{	
		// get the set requested interrupts
		uint8_t req = cpu->io[IO_IF];
		// checked that the interrupt is enabled from the ie reg 
		uint8_t enabled = cpu->io[IO_IE];
		
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
						//cycles += 5;
					}
				}
			}
		}
	}
	//if(cycles > 0)
	//{
		//update_timers(cpu,cycles);
	//}
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








void write_word(Cpu *cpu,uint16_t address,int data) // <- verify 
{
	write_mem(cpu,address+1,((data & 0xff00) >> 8));
	write_mem(cpu,address,(data & 0x00ff));
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
		cpu->io[IO_DIV]++; // inc the div timer 
						
	}
	
	// if timer is enabled <--- edge case with timer is failing the timing test
	if(is_set(cpu->io[IO_TMC],2))
	{	
		cpu->timer_counter += cycles;
		
		int threshold = set_clock_freq(cpu);
		
		// update tima register cycles have elapsed
		while(cpu->timer_counter >= threshold)
		{
			cpu->timer_counter -= threshold;
			
			// about to overflow
			if(cpu->io[IO_TIMA] == 255)
			{	
				cpu->io[IO_TIMA] = cpu->io[IO_TMA]; // reset to value in tma
				request_interrupt(cpu,2); // timer overflow interrupt
			}
			
			else
			{
				cpu->io[IO_TIMA]++;
			}
		}
	}
}

int set_clock_freq(Cpu *cpu)
{
	uint8_t freq = cpu->io[IO_TMC] & 0x3;
	
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
