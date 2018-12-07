
#pragma once
#include <stdbool.h>
#include "lib.h"
// flags
		// first 4 bits are never set
#define Z 7 // zero flag 7th bit of F register is set
#define N 6 // subtraciton flag 6th bit of F register is set
#define H 5 // half carry flag 5th bit of F register is set
#define C 4 // carry flag 4th bit of F register is set

// in terms of machine cycles
// and not clock cycles 1 machine cycle = 4 clock cycles
// taken from blargs intstr_timing test
static const int mcycles[] = 
{	
	1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
	0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
	2,3,2,2,1,1,2,1,2,2,2,2,1,1,2,1,
	2,3,2,2,3,3,3,1,2,2,2,2,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,2,2,2,2,2,0,2,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,3,3,4,3,4,2,4,2,4,3,0,3,6,2,4,
	2,3,3,0,3,4,2,4,2,4,3,0,3,0,2,4,
	3,3,2,0,0,4,2,4,4,1,4,0,0,0,2,4,
	3,3,2,1,0,4,2,4,3,2,4,1,0,0,2,4
};

static const int cbmcycles[] =
{
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2
};

//registers
typedef union
{
	
	struct 
	{
		uint8_t lb; // low  order byte/register
		uint8_t hb; // high order byte/register
	};
	uint16_t reg;
}Register;




typedef struct 
{
	Register af;  // F register is the flags register
	Register bc;
	Register de;
	Register hl;
	Register sp; // stack pointer	
	uint16_t pc;
	int tacfreq; // frequency at which tima increases
	bool timerenable; // is tima enabled?
	uint8_t *mem;
	bool interrupt_enable; // affected by di and ei instr
	int scanline_counter;
	uint8_t screen[X][Y][3];
} Cpu;

// hb = high order byte
// lb = low order byte
// refer to gb cpu manual power up sequence for details
// some inits excluded as whole memory has been set to 0 instead
// of random values


Cpu init_cpu(void); // returns an initial cpu state
inline int step_cpu(Cpu * cpu, uint8_t * rom); // perform a single cpu instr
inline void update_timers(Cpu *cpu, int cycles); // update the cpu timers
inline void do_interrupts(Cpu *cpu);
inline void update_graphics(Cpu *cpu, int cycles);