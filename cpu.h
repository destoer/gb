
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
	//uint8_t *mem; // main cpu mem 


	// memory
	uint8_t vram[0x2000];
	uint8_t wram[0x2000];
	uint8_t oam[0x100];
	uint8_t io[0x100]; // hram and io registers

	uint8_t *rom_mem; // rom 
	bool interrupt_enable; // affected by di and ei instr
	
	// ppu
	int scanline_counter;
	uint8_t screen[Y][X][4]; // <--- need an extra one for this format?
	uint8_t screenp[Y][X];
	bool signal;
	
	
	// fetcher
	bool hblank;
	int x_cord; // current x cord of the ppu
	uint8_t ppu_fifo[16];
	uint8_t ppu_cyc; // how far for a tile / sprite fetch it is 
	int pixel_count; // how many pixels are in the fifo
	uint8_t fetcher_tile[8];
	uint8_t fetcher_sprite[8];
	int tile_cord;
	bool tile_ready; // is the tile fetch ready to go into the fio 
	bool sprite_ready; // is the sprite ready to go into the fifo
	
	uint8_t joypad_state; // has state of held down buttons
	// banking
	
	uint8_t *ram_banks; // 4 banks max
	int currentram_bank; // currently selected ram bank
	int currentrom_bank; // currently selected rom bank
	bool rom_banking; // is rom banking enabled
	bool enable_ram; // is ram banking enabled
	bool rtc_enabled;
	RomInfo rom_info;


	// ------------- sound ------------------------
	
	bool sound_enabled; // is sound enabled in nr52 or not?
/*	
	int nr1_lengthc; //  NR1 internal length counter
	int nr1_cyc; // nr1 cycle counter (ticks to 4096) then incs the internal length counter
	
	
	int nr2_lengthc; //  NR2 internal length counter
	int nr2_cyc; // nr2 cycle counter (ticks to 4096) then incs the internal length counter
	
	
	int nr3_lengthc; //  NR3 internal length counter
	int nr3_cyc; // nr3 cycle counter (ticks to 4096) then incs the internal length counter
	
	
	int nr4_lengthc; //  NR4 internal length counter
	int nr4_cyc; // nr4 cycle counter (ticks to 4096) then incs the internal length counter
*/


	int sequencer_cycles; // keeps track of cycles for frame sequencer when it his 8192 inc the seq step
	int sequencer_step; // keeps track of the current set goes from 0-8 and loops back around

	// length counters
	int nr1_lengthc; //  NR1 internal length counter
	int nr2_lengthc; //  NR2 internal length counter
	int nr3_lengthc; //  NR3 internal length counter
	int nr4_lengthc; //  NR4 internal length counter
	
	
	
	// Sweep
	bool sweep_enabled;
	int sweep_shadow;
	int sweep_period;
	int sweep_timer;
	// rtc 

	
	
	
	
	
	// debugger vars
	#ifdef DEBUG
	int breakpoint;
	int memw_breakpoint;
	int memw_value;
	int memr_breakpoint;
	int memr_value;
	bool step;
	#endif
	// bools used to inform cpu of special instrucitons occuring
	bool ei;
	bool di;
	bool halt;
	bool halt_bug;
	
	
	// timers
	int timer_counter;
	int div_counter;
	uint16_t internal_timer;
	bool timer_reloading;
	
	// is a oam dma active
	bool oam_dma_active;
	uint16_t oam_dma_address;
	int oam_dma_index; // how far along the dma transfer we are
	
	
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
bool is_set(uint8_t reg, uint8_t bit);
uint16_t read_word(int address, Cpu *cpu);
void write_word(Cpu *cpu,uint16_t address,int data);
void write_stack(Cpu *cpu, uint8_t data);
void write_stackw(Cpu *cpu,uint16_t data);
uint8_t read_stack(Cpu *cpu);
uint16_t read_stackw(Cpu *cpu);
void request_interrupt(Cpu * cpu,int interrupt);
int set_clock_freq(Cpu *cpu);
void cycle_tick(Cpu *cpu,int cycles);
uint16_t read_wordt(int address, Cpu *cpu);
void write_wordt(Cpu *cpu,uint16_t address,int data);
uint8_t read_memt(int address, Cpu *cpu);
void write_memt(Cpu *cpu,uint16_t address,int data);
void write_stackwt(Cpu *cpu,uint16_t data);
void write_stackt(Cpu *cpu,uint8_t data);
uint8_t read_stackt(Cpu *cpu);
uint16_t read_stackwt(Cpu *cpu);
void tick_dma(Cpu *cpu, int cycles);