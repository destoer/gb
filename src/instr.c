#include "headers/cpu.h"
#include "headers/lib.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// flag helper functions


// add sub adc and sbc can have a accessed directely and not returned out
// note if we use sub we will have to to change cp to use a different function
// that does not access the value

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
	if(((((reg+1) & 0xf) + (-1 & 0xf)) & 0x10) == 0x10)
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
	if(((((reg-1)&0xf) + (1&0xf))&0x10) == 0x10)
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
	
	if(is_set(reg,bit)) // if bit set
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



// logical and 
// and n with a
// may need a implemation now specific to a 
// reg later
void and(Cpu *cpu, uint8_t num)
{
	// deset negative
	deset_bit(cpu->af.lb,N);
	
	// deset negative
	deset_bit(cpu->af.lb,C);
	
	// set half carry 
	set_bit(cpu->af.lb,H);
	
	
	// set if result is zero 
	
	cpu->af.hb &= num;
	
	if(cpu->af.hb == 0)
	{
		set_bit(cpu->af.lb,Z);
	}
	
	else
	{
		deset_bit(cpu->af.lb,Z);
	}
	
	
}



// the register's bits are shifted left. 
// The carry value is put into 0th bit of the register, 
// and the leaving 7th bit is put into the carry.

// <--- optimize after and compare code
// there is definitely a quick way to do this

uint8_t rl(Cpu *cpu, uint8_t reg)
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
	if((( (int)(reg & 0xf) - (int)(num & 0xf) ) < 0))
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


uint8_t sbc(Cpu *cpu, uint8_t reg, uint8_t num)
{	
	int carry = is_set(cpu->af.lb,C) ? 1 : 0;
	
	int result = reg - num - carry;
	
	if(result < 0)
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	if((reg & 0x0f) - (num & 0x0f) - carry < 0)
	{
		set_bit(cpu->af.lb,H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);
	}
	
	//uint8_t finalres = (uint8_t)result;
	set_bit(cpu->af.lb,N);
	set_zero(cpu,result);
	
	return result;
}

uint8_t add(Cpu *cpu, uint8_t reg, uint16_t num)
{
	deset_bit(cpu->af.lb, N); // reset negative
	
	// test carry from bit 3
	// set the half carry if there is
	if((((reg&0xf) + (num&0xf))&0x10) == 0x10)
	{
		set_bit(cpu->af.lb,H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);
	}
	
	
	
	// check carry from bit 7 <--- could be wrong 
	if(reg + num > 255)
	{
		set_bit(cpu->af.lb, C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	
	reg += num;
	
	set_zero(cpu,reg);
	return reg;
}

// for the sp add opcodes <-- unsure
uint16_t addi(Cpu *cpu,uint16_t reg, int8_t num)
{
	deset_bit(cpu->af.lb, N); // reset negative
	deset_bit(cpu->af.lb, Z); // and zero
	// test carry from bit 3
	// set the half carry if there is
	if( (( (int)(reg&0xf) + (int)(num&0xf)) & 0x10) == 0x10)
	{
		set_bit(cpu->af.lb,H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);
	}
	
	
	
	if( (( (int)(reg & 0xff) + (int)(num&0xff) ) & 0x100) == 0x100)
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	reg += num;
	

	
	return reg;
}
// add n + carry flag to a 
uint8_t adc(Cpu *cpu,uint8_t reg, uint8_t num)
{
	
	
	int carry = is_set(cpu->af.lb,C) ? 1 : 0;
	
	int result = reg + num + carry;
	
	if(result > 0xff)
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	if((reg & 0x0f) + (num & 0x0f) + carry > 0x0f)
	{
		set_bit(cpu->af.lb,H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);
	}
	
	//uint8_t finalres = (uint8_t)result;
	deset_bit(cpu->af.lb,N);
	set_zero(cpu,result);
	return result;
}
	

uint16_t addw(Cpu *cpu, uint16_t reg, uint16_t num) // this is likely faulty
{
	deset_bit(cpu->af.lb,N); // deset the negative flag
	
	
	
	// check for carry from bit 11 <-- unsure 
	if((((reg & 0x0fff) + (num&0x0fff)) & 0x1000) == 0x1000)
	{
		set_bit(cpu->af.lb, H);
	}
	
	else
	{
		deset_bit(cpu->af.lb,H);
	}
	
	
	
	// check for full carry (unsure)
	if(reg + num > 0xffff )
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	reg += num;
	
	return reg;
}


void or(Cpu *cpu,uint8_t val)
{
	cpu->af.hb |= val;
	cpu->af.lb = 0; // reset all flags
	
	set_zero(cpu,cpu->af.hb); // set zero if result is zero
}


// swap upper and lower nibbles 
uint8_t swap(Cpu *cpu, uint8_t num)
{
	// reset flags register
	cpu->af.lb = 0;
	
	set_zero(cpu,num); // swapping it still makes it 0

	return ((num & 0x0f) << 4 | (num & 0xf0) >> 4);
}

void xor(Cpu *cpu,uint8_t num)
{
	// reset flags register
	cpu->af.lb = 0;
	
	
	cpu->af.hb ^= num;
	
	set_zero(cpu,cpu->af.hb); // check for zero
}

void binary(int number)
{
	int counter = 1;
	int mask = 128;
	while(counter <= 8)
	{
		putchar(number & mask? '1' : '0');
		number = number << 1;
		if(counter % 8 == 0)
		{
			putchar(' ');
		}
		counter++;
	}
}


// shift left into carry deset bit 1
uint8_t sla(Cpu *cpu,uint8_t reg)
{
	deset_bit(cpu->af.lb,N);
	deset_bit(cpu->af.lb,H);
	
	bool cond = reg & (1 << 7); // cache if 7 bit is set
	
	
	reg <<= 1;
	
	
	// deset bit one
	deset_bit(reg,0);
	
	
	
	set_zero(cpu,reg); // set zero state
	
	
	if(cond)
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	
	return reg;
}


// shift left into carry deset bit 7
uint8_t sra(Cpu *cpu,uint8_t reg)
{
	deset_bit(cpu->af.lb,N);
	deset_bit(cpu->af.lb,H);
	
	bool cond = is_set(reg,0);// cache if 0 bit is set
	bool set = is_set(reg,7);
	
	reg >>= 1;
	
	if(set)
	{
		set_bit(reg,7);
	}
	
	
	
	
	
	set_zero(cpu,reg); // set zero state
	
	
	if(cond)
	{
		set_bit(cpu->af.lb,C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	
	return reg;
}

// can probably remove duplication here lol 

uint8_t srl(Cpu *cpu, uint8_t reg)
{
	deset_bit(cpu->af.lb,N);
	deset_bit(cpu->af.lb,H);

	if(is_set(reg,0))
	{
		set_bit(cpu->af.lb, C);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
	}
	
	reg >>= 1;
	
	deset_bit(reg,7); // even needed lol?
	
	set_zero(cpu,reg);
	
	return reg;
}





uint8_t rr(Cpu *cpu, uint8_t reg)
{
	
	
	bool set = is_set(reg,0);
	
	
	// deset negative
	deset_bit(cpu->af.lb,N);

	
	// deset half carry
	deset_bit(cpu->af.lb,H);
	
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
	
	

	
	// carry gets bit 0
	if(set)
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



uint8_t rrc(Cpu *cpu,uint8_t reg)
{
	deset_bit(cpu->af.lb,N);
	deset_bit(cpu->af.lb,H);
	
	bool set = is_set(reg,0);


	reg >>= 1;
	
	if(set)
	{
		set_bit(cpu->af.lb,C);
		set_bit(reg,7);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
		deset_bit(reg,7);
	}
	
	set_zero(cpu,reg);
	
	return reg;
}


uint8_t rlc(Cpu *cpu, uint8_t reg)
{
	deset_bit(cpu->af.lb,N);
	deset_bit(cpu->af.lb,H);
	
	bool set = is_set(reg,7);
	
	
			
	reg <<= 1;
	
	if(set)
	{
		set_bit(cpu->af.lb,C);
		set_bit(reg,0);
	}
	
	else
	{
		deset_bit(cpu->af.lb,C);
		deset_bit(reg,0);		
	}
	
			
	set_zero(cpu,reg);
	
	return reg;

}
