#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/lib.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// implement oam bug 

// flag helper functions


// add sub adc and sbc can have a accessed directely and not returned out
// note if we use sub we will have to to change cp to use a different function
// that does not access the value (probably for the best)


// not triggered look for a slight variation for this 
// cause currently it is just eating cycles not getting triggered
bool waitloop_bit(Cpu *cpu, bool cond, int bit);

// believe that this is used alot
bool waitloop_and_read(Cpu *cpu, bool cond, int bit);


void set_zero(Cpu *cpu, uint8_t reg)
{
	// set the zero flag
	if(!reg)
	{
		set_bit(cpu->af.lb,Z);
	}
}





// consider functioning off the flag operations
// for setting and desetting

// decrement
void dec(Cpu *cpu, uint8_t reg)
{
	reg -= 1;
	
	// preserve carry
	cpu->af.lb &= (1 << C);
	
	// set the negation flag
	set_bit(cpu->af.lb,N);

	set_zero(cpu,reg);

	// check the carry 
	if(!(((((reg+1) & 0xf) + (-1 & 0xf)) & 0x10) == 0x10))
	{
		set_bit(cpu->af.lb,H);
	}
	
}



void inc(Cpu *cpu,uint8_t reg)
{
	reg += 1;
	
	// preserve carry
	cpu->af.lb &= (1 << C);
	
	if(!reg)
	{
		set_bit(cpu->af.lb,Z);
	}	
	// test carry from bit 3
	// set the half carry if there is
	if(((((reg-1)&0xf) + (1&0xf))&0x10) == 0x10)
	{
		set_bit(cpu->af.lb,H);
	}
}


// not setting the correct flags
// debug
void bit(Cpu *cpu, uint8_t reg, uint8_t bit)
{
	cpu->af.lb &= (1 << C); // preserve Carry
	if(!is_set(reg,bit)) // if bit set
	{
		set_bit(cpu->af.lb,Z);
		
	}
	
	// set half carry
	set_bit(cpu->af.lb,H);	
}



// logical and  and n with a
void and(Cpu *cpu, uint8_t num)
{
	// deset negative and carry
    cpu->af.lb = 0;
	
	// set half carry
    set_bit(cpu->af.lb,H);

	// set if result is zero 
	cpu->af.hb &= num;
	set_zero(cpu,cpu->af.hb);
}


// the register's bits are shifted left. 
// The carry value is put into 0th bit of the register, 
// and the leaving 7th bit is put into the carry.

// <--- optimize after and compare code
// there is definitely a quick way to do this

uint8_t rl(Cpu *cpu, uint8_t reg)
{
	bool cond = reg & (1 << 7); // cache if 7 bit is set
	

	
	// perform the rotation
	// shift the register left
	reg <<= 1;
	
	// bit 0 gets bit of carry flag
	reg |= (cpu->af.lb & (1 << C)) >> C; // bit 0 of reg gains carry bit
	
	// deset neagtive and half carry 
	cpu->af.lb = 0;	
	
	// Carry flag gets bit 7 of reg
	if(cond)
	{
		set_bit(cpu->af.lb,C);
	}
	set_zero(cpu,reg);
	
	return reg;
}	

// may want to use a different register size...
// for the return value 
void sub(Cpu *cpu,  uint8_t num)
{
	// deset every flag 
	cpu->af.lb = 0;
	
	// set negative flag
	set_bit(cpu->af.lb,N);

	if(cpu->af.hb == num)
	{
		set_bit(cpu->af.lb,Z);
	}

	// check the carry <-- check this works
	// opossite to the book?
	if((( (int)(cpu->af.hb & 0xf) - (int)(num & 0xf) ) < 0))
	{
		set_bit(cpu->af.lb,H);
	}

	if(num > cpu->af.hb) // <--- if flags fail on a sub instruction this is a likely candidate
	{
		set_bit(cpu->af.lb,C); // set the carry
	}

	cpu->af.hb -= num;
}

void cp(Cpu *cpu,uint8_t num)
{
	// deset every flag 
	cpu->af.lb = 0;
	
	// set negative flag
	set_bit(cpu->af.lb,N);

	if(cpu->af.hb == num)
	{
		set_bit(cpu->af.lb,Z);
	}
	
	// check the carry <-- check this works
	// opossite to the book?
	if((( (int)(cpu->af.hb & 0xf) - (int)(num & 0xf) ) < 0))
	{
		set_bit(cpu->af.lb,H);
	}

	if(num > cpu->af.hb) // <--- if flags fail on a sub instruction this is a likely candidate
	{
		set_bit(cpu->af.lb,C); // set the carry
	}

}	

void sbc(Cpu *cpu, uint8_t num)
{	

	uint8_t reg = cpu->af.hb;

	int carry = is_set(cpu->af.lb,C) ? 1 : 0;
	
	int result = reg - num - carry;
	
	// clear all flags
	cpu->af.lb = 0;
	set_bit(cpu->af.lb,N); // set negative
	
	if(result < 0)
	{
		set_bit(cpu->af.lb,C);
	}
	
	if((reg & 0x0f) - (num & 0x0f) - carry < 0)
	{
		set_bit(cpu->af.lb,H);
	}
	
	cpu->af.hb = result;
	set_zero(cpu,cpu->af.hb);
}

void add(Cpu *cpu, uint8_t num)
{
	// reset every flag 
	cpu->af.lb = 0;
	
	// test carry from bit 3
	// set the half carry if there is
	if((((cpu->af.hb&0xf) + (num&0xf))&0x10) == 0x10)
	{
		set_bit(cpu->af.lb,H);
	}
	
	// check carry from bit 7 <--- could be wrong 
	if(cpu->af.hb + num > 255)
	{
		set_bit(cpu->af.lb, C);
	}
		
	cpu->af.hb += num;
	set_zero(cpu,cpu->af.hb);
}

// for the sp add opcodes <-- unsure
uint16_t addi(Cpu *cpu,uint16_t reg, int8_t num)
{
	cpu->af.lb = 0; // reset flags
	// test carry from bit 3
	// set the half carry if there is
	if( (( (int)(reg&0xf) + (int)(num&0xf)) & 0x10) == 0x10)
	{
		set_bit(cpu->af.lb,H);
	}
	
	if( (( (int)(reg & 0xff) + (int)(num&0xff) ) & 0x100) == 0x100)
	{
		set_bit(cpu->af.lb,C);
	}	
	
	reg += num;	
	return reg;
}
// add n + carry flag to a 
void adc(Cpu *cpu,uint8_t num)
{
	uint8_t reg = cpu->af.hb;
	
	int carry = is_set(cpu->af.lb,C) ? 1 : 0;
	
	int result = reg + num + carry;
	
	cpu->af.lb = 0; // reset all flags
	
	if(result > 0xff)
	{
		set_bit(cpu->af.lb,C);
	}	
	
	if((reg & 0x0f) + (num & 0x0f) + carry > 0x0f)
	{
		set_bit(cpu->af.lb,H);
	}
		
	cpu->af.hb = result;
	set_zero(cpu,cpu->af.hb);
}
	

uint16_t addw(Cpu *cpu, uint16_t reg, uint16_t num) 
{
	cpu->af.lb &= (1 << Z); // only preserve the Z flag 

	// check for carry from bit 11
	if((((reg & 0x0fff) + (num&0x0fff)) & 0x1000) == 0x1000)
	{
		set_bit(cpu->af.lb, H);
	}
	
	// check for full carry 
	if(reg + num > 0xffff )
	{
		set_bit(cpu->af.lb,C);
	}
	
	reg += num;
	
	return reg;
}


void or(Cpu *cpu,uint8_t val)
{
	cpu->af.hb |= val;
	cpu->af.lb = 0; // reset all flags
	set_zero(cpu,cpu->af.hb);
}


// swap upper and lower nibbles 
uint8_t swap(Cpu *cpu, uint8_t num)
{
	// reset flags register
	cpu->af.lb = 0;
	
	if(!num)
	{
		set_bit(cpu->af.lb,Z);
	}

	return ((num & 0x0f) << 4 | (num & 0xf0) >> 4);
}

void xor(Cpu *cpu,uint8_t num)
{
	// reset flags register
	cpu->af.lb = 0;
	cpu->af.hb ^= num;
	set_zero(cpu,cpu->af.hb);
}




// shift left into carry deset bit 1
uint8_t sla(Cpu *cpu,uint8_t reg)
{
	cpu->af.lb = 0;
	bool cond = reg & (1 << 7); // cache if 7 bit is set

	reg <<= 1;
	
	// deset bit one
	deset_bit(reg,0);

	set_zero(cpu,reg);
	
	if(cond)
	{
		set_bit(cpu->af.lb,C);
	}
		
	return reg;
}


// shift left into carry deset bit 7
uint8_t sra(Cpu *cpu,uint8_t reg)
{

	cpu->af.lb = 0;
	
	bool cond = is_set(reg,0);// cache if 0 bit is set
	bool set = is_set(reg,7);
	
	reg >>= 1;
	
	if(set)
	{
		set_bit(reg,7);
	}
	
	if(cond)
	{
		set_bit(cpu->af.lb,C);
	}
	
	set_zero(cpu,reg);
	
	return reg;
}

// can probably remove duplication here lol 

uint8_t srl(Cpu *cpu, uint8_t reg)
{
	cpu->af.lb = 0;
	if(is_set(reg,0))
	{
		set_bit(cpu->af.lb, C);
	}
	
	reg >>= 1;

	set_zero(cpu,reg);
	
	return reg;
}





uint8_t rr(Cpu *cpu, uint8_t reg)
{
	
	
	bool set = is_set(reg,0);
	
	reg >>= 1;
	
	
	// bit 7 gets carry 
	if(is_set(cpu->af.lb,C))
	{
		set_bit(reg, 7);
	}
	
	else 
	{
		deset_bit(reg,7);
	}
	
	cpu->af.lb = 0;
	
	// carry gets bit 0
	if(set)
	{
		set_bit(cpu->af.lb,C);
	}
	
	set_zero(cpu,reg);
	
	return reg;
}	



uint8_t rrc(Cpu *cpu,uint8_t reg)
{
	
	bool set = is_set(reg,0);
	
	cpu->af.lb = 0;

	reg >>= 1;
	
	if(set)
	{
		set_bit(cpu->af.lb,C);
		set_bit(reg,7);
	}
	set_zero(cpu,reg);
	return reg;
}


uint8_t rlc(Cpu *cpu, uint8_t reg)
{

	bool set = is_set(reg,7);
		
	reg <<= 1;
	
	cpu->af.lb = 0;
	
	if(set)
	{
		set_bit(cpu->af.lb,C);
		set_bit(reg,0);
	}	
	set_zero(cpu,reg);
	return reg;
}


void jr(Cpu *cpu)
{
	int8_t operand = read_memt(cpu->pc++, cpu);
	cycle_tick(cpu,1); // internal delay
	cpu->pc += operand;	
}

void jr_cond(Cpu *cpu,bool cond, int bit)
{
	int8_t operand = read_memt(cpu->pc++, cpu);
	if(is_set(cpu->af.lb, bit) == cond)
	{
		cycle_tick(cpu,1); // internal delay
		cpu->pc += operand;
		
#ifdef WAITLOOP_OPT		
		// waitloop detection
		if(operand < 0)
		{
		
			if(waitloop_bit(cpu,cond,bit) && operand == -1)
			{
				return;
			}
			
		}
#endif	
	}

	
}


bool waitloop_and_read(Cpu *cpu, bool cond, int bit)
{
	// shortcuts wait loops for instruction sequences like this
	/*
		loop:
			ld r, (nnnn)
			and r, nn
			jr z, loop
	
	*/
	
	UNUSED(cpu);
	return bit & cond;
}

// waitloop detection for the bit instr
// returns wether or not the waitloop of this manner was detected
bool waitloop_bit(Cpu *cpu, bool cond, int bit)
{
	/* shorcuts wait loops with sequences like this
		(testing for a bit being set)
		loop:
			bit x, (hl)
			jr z, loop
	*/
			
	// lets handle a very simple kind of busy loop 
	// suppose the operand jumps to 1 instr behind
	// with jr z, nnnn
	// and performs bit x, (hl)
	// this means that it is waiting for the bit to be set
			
	// we will rather than run instructions count the ammount of cycles
	// for the whole loop and tick them until the bit changes then pass control back 
			
	// this works fine but we have to check it more throughly and handle the bit setting generally
	// as well as this we have to handle interrupts being fired at each tick we need to check if 
	// an enabled interrupt has fired and pass back control 
	// it will eventually return here if the memory has not changed in the mean time
			
	if(!(cond && bit == Z)) // jr Z nnnn
	{
		return false;
	}
	
	int cb = read_mem(cpu->pc,cpu);
	
	// not a cb opcode means it cant be bit instr return 
	// may handle more cases in the future
	if(cb != 0xcb)
	{
		return false;
	}
	
	int opcode = read_mem(cpu->pc+1,cpu);
			
	/* ((opcode & 0x6) == 0x6)) (hl) is used 
		((opcode & 0x40 ) == 0x40)  bit opcode
		0x38 for the bit that the instr tests
	*/
	if(!((opcode & 0x46) == 0x46)) // bit x, (hl)
	{
		return false;
	}


	// conditons have been met lets shorcut the waitloop

	// mem[hl] cache
	const uint8_t *mem_cell = get_direct_mem_access(cpu,cpu->hl.reg);

	int test_bit = (opcode & 0x38) >> 3;
	int enabled = cpu->io[IO_IE];
	puts("WAITLOOP DETECTED!");
	int cycles = 5; // 5 m cycles for the bit op + the jr op 
	// while the condition is not true and no interrupts have fired
	while(!is_set(*mem_cell,test_bit) && !(enabled & (cpu->io[IO_IF] & 0xf)))
	{
		cycle_tick(cpu,cycles);
	}
	// now the value has changed we can pass back the control
	// of the cpu

	return true;
}