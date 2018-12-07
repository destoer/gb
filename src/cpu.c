#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "headers/cpu.h"
#include "headers/disass.h"
#include "headers/lib.h"

uint8_t read_mem(int address, uint8_t *mem);
bool is_set(uint8_t reg, uint8_t bit);

// need to do the lcd emulation now
// follow through on the graphics emulation
// chap on codeslinger and then comment it all 
// and refactor it


// <----
// one of the half carry implementations is not working
// in the cpu the half carry impl is bugged
// think the bit carry checking is not properly implemented

// setup a fresh cpu state and return it to main
Cpu init_cpu(void) // <--- memory should be randomized on startup
{	
	Cpu cpu;
	cpu.mem = calloc(0x10000, sizeof(uint8_t)); // main memory
	cpu.af.reg = 0x01B0;
	cpu.bc.reg = 0x0013;
	cpu.de.reg = 0x00D8;
	cpu.hl.reg = 0x014d;
	cpu.sp.reg = 0xFFFE;
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
	cpu.pc = 0x100; // reset at 0x100
	cpu.tacfreq = 256; // number of machine cycles till update
	cpu.timerenable = true;
	cpu.scanline_counter = 456;
	return cpu;
}



void request_interrupt(Cpu * cpu,int interrupt)
{
	// set the interrupt flag to signal
	// an interrupt request
	uint8_t flag = read_mem(0xff0f,cpu->mem);
	flag = set_bit(flag,interrupt);
	write_mem(cpu,0xff0f, flag); // write the flag back with the bit set
	//puts("Interrupt sucessfully requested");
}



void do_interrupts(Cpu *cpu)
{
	
	// if interrupts are enabled
	if(cpu->interrupt_enable)
	{	
		// get the set requested interrupts
		uint8_t flag = read_mem(cpu,0xff0f);
		// checked that the interrupt is enabled from the ie reg 
		uint8_t enabled = read_mem(cpu,0xffff);
		
		if(flag > 0)
		{
			// priority for servicing starts at interrupt 0
			for(int i = 0; i < 5; i++)
			{
				// check that the particular interrupt is enabled
				if(is_set(enabled,i))
				{
					service_interrupt(i);
				}
			}
		}
	}
}

inline void service_interrupt(Cpu *cpu,int interrupt)
{
	cpu->interrupt_enable = false; // disable interrupts now one is serviced
	
	// reset the bit of in the if to indicate it has been serviced
	uint8_t req = read_mem(cpu,0xff0f);
	req = deset_bit(req,interrupt);
	write_mem(cpu,0xff04,req);
	
	// push the current pc on the stack to save it
	// it will be pulled off by reti or ret later
	write_stackw(cpu,cpu->pc);
	
	// set the program counter to the start of the
	// interrupt handler for the request interrupt
	
	switch(interrupt)
	{
		// interrupts are one less than listed in cpu manual
		// as our bit macros work from bits 0-7 not 1-8
		case 0: cpu->pc = 0x40; break;
		case 1: cpu->pc = 0x48; break;
		case 2: cpu->pc = 0x50; break;
		case 4: cpu->pc = 0x60; break;
	}
}


// memory accesses (READ THE PANCDOCS ON MEMORY MAP)
// may need access to information structs

// needs ones related to banking

void write_mem(Cpu *cpu,int address,int data)
{

	// ECHO ram also writes in ram
	if( (address >= 0xE000) && (address < 0xFE00))
	{
		cpu->mem[address] = data;
		cpu->mem[address-0x2000] = data;
	}
	
	
	// two below need imeplementing 
	
	// vram can only be accesed at mode 0-2
	if(address >= 0x8000 && address <= 0x9fff)
	{
		uint8_t status = read_mem(0xff41,cpu->mem);
		status &= 3; // get just the mode
		if(status <= 3)
		{
			cpu->mem[address] = data;
		}
	}

	// oam is accesible during mode 0-1
	if(address >= 0xfe00 && address <= 0xfe9f)
	{
		uint8_t status = read_mem(0xff41,cpu->mem);
		status &= 3; // get just the mode
		if(status <= 1)
		{
			cpu->mem[address] = data;
		}
	}

	
	// restricted 
	else if( (address >= 0xFEA0) && (address < 0xFEFF) )
	{

	}
	
	// update the timer freq
	else if(address == TMC)
	{
		
		// timer is set to CLOCKSPEED/freq/4 (4 to convert to machine cycles)
		if(cpu->tacfreq != cpu->mem[TMC] & 3)
		{
			switch(cpu->mem[TMC] & 3)
			{
				case 0: cpu->tacfreq = 256; break; // freq 4096
				case 1: cpu->tacfreq = 4; break; //freq 262144
				case 2: cpu->tacfreq = 16; break; // freq 65536
				case 3: cpu->tacfreq = 64; break; // freq 16382
			}
		}
		cpu->mem[address] = data;
	}
	
	// writing anything to DIV resets it to zero
	else if(address == DIV)
	{
		cpu->mem[DIV] = 0;
	}
	
	// reset ly if the game tries writing to it
	else if (address == 0xFF44)
	{
		cpu->mem[address] = 0;
	} 
	
	else if(address == 0xff46) // dma reg perform a dma transfer
	{
		data *= 100; // source address = data * 100
		// transfer is from 0xfe00 to 0xfea0
		for(int i = 0; i < 0xA0; i++)
		{
			write_mem(cpu,0xfe00+i, read_mem(address+i,cpu->mem));
		}
	}
	
	
	// unrestricted
	else
	{
		cpu->mem[address] = data;
	}
}


// needs reads related to banking after tetris
// also needs the vram related stuff 
uint8_t read_mem(int address, uint8_t *mem)
{
	return mem[address]; // just a stub for now implement later
}



// checked memory reads
uint16_t read_word(int address, uint8_t *mem)
{
	return read_mem(address,mem) | (read_mem(address+1,mem) << 8);
}

// implement the memory stuff for stack minips


void write_stack(Cpu *cpu, uint8_t data)
{
	write_mem(cpu,cpu->sp.reg--,data); // write to stack
	// and decrement stack pointer

}


void write_stackw(Cpu *cpu,uint16_t data)
{
	write_stack(cpu,(data & 0xff00) >> 8);
	write_stack(cpu,(data &0x00ff));
	
}

uint8_t read_stack(Cpu *cpu)
{
	cpu->sp.reg += 1;
	uint8_t data = read_mem(cpu->sp.reg,cpu->mem);
	return data;
}


uint16_t read_stackw(Cpu *cpu)
{
	return read_stack(cpu) | (read_stack(cpu) << 8);
}


// globals fix later
int divcounter = 64;
int timercounter = 256; // we are doing this in machine cycles




//void update_graphics(Cpu *cpu, int cycles) todo



// updates the timers
void update_timers(Cpu *cpu, int cycles)
{
	
	// divider reg in here for convenience
	divcounter -= cycles;
	if(divcounter <= 0)
	{
		divcounter = 64; // inc rate
		cpu->mem[DIV]++; // inc the div timer <- hopefully overflow behavior works
						// if it doesent we will have to to zero it manually
	}
	
	// if timer is enabled
	if(cpu->timerenable)
	{	
		timercounter -= cycles;
		
		// update tima register cycles have elapsed
		if(timercounter <= 0)
		{
			timercounter = cpu->tacfreq;
			
			// about to overflow
			if(cpu->mem[TIMA] == 255)
			{	
				cpu->mem[TIMA] = cpu->mem[TMA]; // reset to value in tma
				//puts("Timer overflow!");
				request_interrupt(cpu,2); // timer overflow interrupt
			}
			
			else
			{
				cpu->mem[TIMA]++;
			}
		}
	}
}



// flag helper functions


void set_zero(Cpu *cpu, uint8_t reg)
{
	
	// set the zero flag
	if(reg == 0)
	{
		set_bit(cpu->af.lb,Z);
	}
	
	else
	{
		deset_bit(cpu->af.lb,Z);
	}
}





// consider functioning off the flag operations
// for setting and desetting

// decrement
void dec(Cpu *cpu, uint8_t reg)
{
	reg -= 1;
	// set the negation flag
	set_bit(cpu->af.lb,N);
	
	// set the zero flag
	set_zero(cpu,reg);
	
	// check the carry <-- check this works
	if((((reg+1 & 0xf) + (-1 & 0xf)) & 0x10) == 0x10)
	{
		deset_bit(cpu->af.lb,H);
	}
	
	else
	{
		set_bit(cpu->af.lb,H);	
	}
	
}



void inc(Cpu *cpu,uint8_t reg)
{
	reg += 1;
	
	// reset negative flag
	deset_bit(cpu->af.lb,N);
	
	// test the zero flag
	set_zero(cpu,reg);
	
	

	// test carry from bit 3
	// set the half carry if there is
	if((((reg-1&0xf) + (1&0xf))&0x10) == 0x10)
	{
		set_bit(cpu->af.lb,H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);
	}
}


// not setting the correct flags
// debug

void bit(Cpu *cpu, uint8_t reg, uint8_t bit)
{
	
	if(reg & (1 << bit)) // if bit set
	{
		deset_bit(cpu->af.lb,Z);
		
	}
	
	else // if bit b of r is 0
	{
		set_bit(cpu->af.lb,Z);
	}
	
	// set half carry
	set_bit(cpu->af.lb,H);
	
	// deset negative
	deset_bit(cpu->af.lb,N);
	
}





// the register's bits are shifted left. 
// The carry value is put into 0th bit of the register, 
// and the leaving 7th bit is put into the carry.

// <--- optimize after and compare code
// there is definitely a quick way to do this

uint8_t rl(Cpu *cpu, uint8_t reg, uint8_t shift)
{
	bool cond = reg & (1 << 7); // cache if 7 bit is set
	
	// deset negative
	deset_bit(cpu->af.lb,N);

	
	// deset half carry
	deset_bit(cpu->af.lb,H);
	

	

	// perform the rotation
	
	// shift the register left
	reg <<= 1;
	
	// bit 0 gets bit of carry flag
	reg |= (cpu->af.lb & (1 << C)) >> C; // bit 0 of reg gains carry bit
	
	// Carry flag gets bit 7 of reg
	if(cond)
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	

	
	// test zero flag set
	set_zero(cpu,reg);
	
	return reg;
}	

// may want to use a different register size...
// for the return value 
uint8_t sub(Cpu *cpu, uint8_t reg, uint8_t num)
{
	// set negative flag
	set_bit(cpu->af.lb,N);

	if(reg == num)
	{
		set_bit(cpu->af.lb,Z);
	}
	
	else
	{
		deset_bit(cpu->af.lb,Z);
	}
	
	// check the carry <-- check this works
	// opossite to the book?
	if((( (reg & 0xf) - (num & 0xf) ) < 0))
	{
		set_bit(cpu->af.lb,H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);	
	}
	
	
	if(num > reg) // <--- if flags fail on a sub instruction this is a likely candidate
	{
		set_bit(cpu->af.lb,C); // set the carry
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	return reg - num;
}

// potentially need the rominfo too but not needed yet
int step_cpu(Cpu * cpu, uint8_t * rom)
{
	
	// one byte immediate
	// instrucitons may need to be signed? eg ld a, (ff00 + n)
	
	// read an opcode and inc the program counter
	uint8_t opcode = read_mem(cpu->pc++,cpu->mem);
	int8_t operand;
	uint8_t cbop;
	//print cpu state and disassemble the opcode
	//cpu_state(cpu);
	//disass_8080(opcode, cpu);
	
	// decode and execute 
	//(opcode loads may need to go through the read mem funcion at some point)
	switch(opcode)
	{
		case 0x0: // nop
			break;
		
		case 0x4: // inc b
			inc(cpu, cpu->bc.hb++);
			break;
			
		case 0x5: // dec b
			dec(cpu, cpu->bc.hb--);
			//print_flags(cpu);
			break;
			
		case 0x6: // ld b, n
			cpu->bc.hb = read_mem(cpu->pc++,cpu->mem);
			break;
		
		
		case 0xc: // inc c
			inc(cpu,cpu->bc.lb++);
			break;
		
		case 0xd: // dec c
			dec(cpu, cpu->bc.lb--);
			break;
		
		
		case 0xe: // ld c, n
			cpu->bc.lb = read_mem(cpu->pc++,cpu->mem);
			break;

		case 0x11: // ld de, nn
			cpu->de.reg = read_word(cpu->pc,cpu->mem);
			cpu->pc += 2;
			break;

		case 0x13: // inc de
			cpu->de.reg += 1;
			break;
		
		case 0x15: // dec d
			dec(cpu, cpu->de.hb--);
			break;
		
		//case 0x16: <-- the screen scroll is finished here
		// somethign is wrong with our cpu writes or gpu implemation
		
		
		case 0x17: // rla (rotate left through carry flag) 
			cpu->af.hb = rl(cpu,cpu->af.hb,1);
			break;
			
		case 0x18: // jr n
			operand = read_mem(cpu->pc++, cpu->mem);
			cpu->pc += operand;
			break;
			
		case 0x1a: // ld a,(de) <--- fix memory reading / writing
			cpu->af.hb = read_mem(cpu->de.reg,cpu->mem);
			break;
			
		case 0x1d: // dec e
			dec(cpu, cpu->de.lb--);
			break;
			
		case 0x1e: // ld e, n
			cpu->de.lb = read_mem(cpu->pc++, cpu->mem);
			break;
		
		case 0x20: // jr nz, n
			operand = read_mem(cpu->pc++, cpu->mem);
			if(!is_set(cpu->af.lb, Z))
			{
				cpu->pc += operand;
			}
			break;
			
		case 0x21: // ld hl, nn
			cpu->hl.reg = read_word(cpu->pc,cpu->mem);
			cpu->pc += 2;
			break;
		
		case 0x22: // ldi (hl), a
			write_mem(cpu,cpu->hl.reg++,cpu->af.hb);
			break;
		
		case 0x23: // inc hl
			cpu->hl.reg += 1; // increment hl
			break;
		
		case 0x24: // inc h
			inc(cpu,cpu->hl.hb++);
			break;
		
		case 0x28: // jr z, n
			operand = read_mem(cpu->pc++, cpu->mem);
			if(is_set(cpu->af.lb, Z))
			{
				cpu->pc += operand;
			}
			break;
		
		case 0x2e: // ld l, n
			cpu->hl.lb = read_mem(cpu->pc++, cpu->mem);
			break;
		
		case 0x31: // ld sp, nn
			cpu->sp.reg = read_word(cpu->pc,cpu->mem);
			cpu->pc += 2;
			break;
		
		case 0x32: // ldd (hl), a // <--- check memory was written properly
			write_mem(cpu,cpu->hl.reg--,cpu->af.hb);
			break;
		
		case 0x3d: // dec a
			dec(cpu,cpu->af.hb--);
			break;
			
		
		case 0x3e: // ld a, n
			cpu->af.hb = read_mem(cpu->pc++, cpu->mem);
			break;
		
		case 0x4f: // ld c,a
			cpu->bc.lb = cpu->af.hb;
			break;
		
		case 0x57: // ld d, a
			cpu->de.hb = cpu->af.hb;
			break;
		
		case 0x67: // ld h, a 
			cpu->hl.hb = cpu->af.hb;
			break;
		
		case 0x77: // ld (hl), a 
			write_mem(cpu,cpu->hl.reg,cpu->af.hb);
			break;
		
		case 0x7b: // ld a, e
			cpu->af.hb = cpu->de.lb;
			break;
		
		
		case 0x7c: // ld a, h
			cpu->af.hb = cpu->hl.hb;
			break;
		
		
		case 0x90: // sub b
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->bc.hb);
			//cpu_state(cpu); print_flags(cpu); exit(1);
			break;
		
		
		// shortcut case end up with just zero flag being set
		case 0xaf: // xor a, a
			cpu->af.reg = 128; 
			break;
		
		case 0xc1: // pop bc
			cpu->bc.reg = read_stackw(cpu);
			break;
		
		case 0xc3: // jump
			cpu->pc = read_word(cpu->pc,cpu->mem);
			break;
		
		
		case 0xc5: // push bc 
			write_stackw(cpu,cpu->bc.reg);
			break;
		
		
		case 0xc9: // ret 
			cpu->pc = read_stackw(cpu);
			break;
		
		// return early from here and return the cycles
		// from the cb prefix table
		case 0xcb: // multi len opcode (cb prefix)
		
			
			cbop = read_mem(cpu->pc++, cpu->mem); // fetch the opcode
			
			// figure out instruction bits and decode fields
			// unless u want 100s of switch statements 
			// ^ not going with above may resort to later
			switch(cbop)
			{
				
				case 0x11: // rl c (rotate left)
					cpu->bc.lb = rl(cpu,cpu->bc.lb,1);
					break;
				
				case 0x7c: // bit 7, h
					bit(cpu, cpu->hl.hb, 7);
					break;
				
				default:
					fprintf(stderr, "[cpu] Unknown CB opcode: %x\n", cbop);
					cpu_state(cpu);
					print_flags(cpu);
					exit(1);
			}
			
			return cbmcycles[cbop]; // update machine cylces for cb prefix
			break;
		
		
		case 0xCD: // call nn <-- verify
			write_stackw(cpu,cpu->pc+2);
			cpu->pc = read_word(cpu->pc, cpu->mem);
			break;

		case 0xE0: // ld (ff00+n),a
			write_mem(cpu,0xff00+read_mem(cpu->pc++,cpu->mem),cpu->af.hb);
			break;

		case 0xE2: // LD ($FF00+C),A
			write_mem(cpu,0xff00 + cpu->bc.lb, cpu->af.hb);
			break;

			
		case 0xea: // ld (nnnn), a
			write_mem(cpu,load_word(cpu->pc,cpu->mem),cpu->af.hb);
			cpu->pc += 2;
			break;
		
		case 0xF0: // ld a, (ff00+n)
			cpu->af.hb = read_mem(0xff00+read_mem(cpu->pc++, cpu->mem),cpu->mem);
			break;
		
		
		case 0xFE: // cp a, n (do a sub and discard result)
			sub(cpu,cpu->af.hb,read_mem(cpu->pc++,cpu->mem));
		//	cpu_state(cpu);
		//	print_flags(cpu); exit(1);
			break;
			
		
		default:
			fprintf(stderr, "[cpu] Unknown opcode: %x\n", opcode);
			cpu_state(cpu);
			print_flags(cpu);
			exit(1);
		
	}
	
    return mcycles[opcode]; // update the machine cycles		
}