
#pragma once
#include <stdbool.h>
#include "lib.h"
#include "rom.h"
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



// for use when a conditional is taken
// return mtcycles[opcode];
static const int mtcycles[] =
{
     1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
     0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
     3,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
     3,3,2,2,3,3,3,1,3,2,2,2,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     2,2,2,2,2,2,0,2,1,1,1,1,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
     5,3,4,4,6,4,2,4,5,4,4,0,6,6,2,4,
     5,3,4,0,6,4,2,4,5,4,4,0,6,0,2,4,
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
	uint16_t sp; // stack pointer	
	uint16_t pc;
	int tacfreq; // frequency at which tima increases
	uint8_t *mem; // main cpu mem 
	uint8_t *rom_mem; // rom 
	bool interrupt_enable; // affected by di and ei instr
	int scanline_counter;
	uint8_t screen[Y][X][4]; // <--- need an extra one for this format?
	uint8_t joypad_state; // has state of held down buttons
	
	// banking
	
	uint8_t *ram_banks; // 4 banks max
	uint8_t currentram_bank; // currently selected ram bank
	uint8_t currentrom_bank; // currently selected rom bank
	bool rom_banking; // is rom banking enabled
	bool enable_ram; // is ram banking enabled
	
	RomInfo rom_info;

	// debugger vars
	
	int breakpoint;
	int memw_breakpoint;
	int memr_breakpoint;
	bool step;
	
	// bools used to inform cpu of special instrucitons occuring
	bool ei;
	bool di;
	bool halt;
	bool halt_bug;
	// timers
	int timer_counter;
	int div_counter;
	
} Cpu;

// hb = high order byte
// lb = low order byte
// refer to gb cpu manual power up sequence for details
// some inits excluded as whole memory has been set to 0 instead
// of random values


Cpu init_cpu(void); // returns an initial cpu state
void update_timers(Cpu *cpu, int cycles); // update the cpu timers
void do_interrupts(Cpu *cpu);
void update_graphics(Cpu *cpu, int cycles);
uint8_t read_mem(uint16_t address, Cpu *cpu);
bool is_set(uint8_t reg, uint8_t bit);
uint16_t read_word(int address, Cpu *cpu);
void write_mem(Cpu *cpu,uint16_t address,int data);
void write_word(Cpu *cpu,uint16_t address,int data);
void write_stack(Cpu *cpu, uint8_t data);
void write_stackw(Cpu *cpu,uint16_t data);
uint8_t read_stack(Cpu *cpu);
uint16_t read_stackw(Cpu *cpu);
void request_interrupt(Cpu * cpu,int interrupt);
