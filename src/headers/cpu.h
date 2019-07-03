#pragma once
#include <stdbool.h>
#include <stdio.h>
#include "lib.h"
#include "rom.h"
// flags
		// first 4 bits are never set
#define Z 7 // zero flag 7th bit of F register is set
#define N 6 // subtraciton flag 6th bit of F register is set
#define H 5 // half carry flag 5th bit of F register is set
#define C 4 // carry flag 4th bit of F register is set

/*
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
*/


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



// should have a bool for the channel being enabled
// as it is better to cache it
typedef struct {
	int lengthc;
	bool length_enabled;
	int period; // how long before we move to the next sample
	float output; // output of the channel
	int volume; // should have envelope regs here
	int volume_load;
	int duty; // current duty 
	int duty_idx; // current index into duty
	int freq; // current frequency
	int env_period; // current timer
	int env_load; // cached period
	bool env_enabled; // disabled when it ticks over or under
} Sqaure;

// how many cycles a frame is in single speed mode
#define MAXCYC_SINGLE_SPEED 16725

// how many cycles in double speed mode
#define MAXCYC_DOUBLE_SPEED MAXCYC_SINGLE_SPEED * 2

struct CPU;

struct Memory_table
{
	uint8_t (*read_memf)(struct CPU *cpu, uint16_t addr);
	void (*write_memf) (struct CPU *cpu, uint16_t addr, int data);	
};


struct CPU
{
	Register af;  // F register is the flags register
	Register bc;
	Register de;
	Register hl;
	uint16_t sp; // stack pointer	
	uint16_t pc;


	

	// memory
	uint8_t vram[2][0x2000]; // 2nd array is cgb vram
	uint8_t wram[0x1000]; // 1st cgb ram bank used for 2nd half in dmg mode
	uint8_t oam[0xA0];
	uint8_t io[0x100]; // hram and io registers

	
	struct Memory_table memory_table[0x10]; // table of function pointers for memory
	
	uint8_t *rom_mem; // rom 
	bool interrupt_enable; // affected by di and ei instr
	
	// ppu
	int scanline_counter;
	uint8_t screen[Y][X][4]; // <--- need an extra one for this format?
	bool signal;
	int current_line;
	int mode; // current ppu mode
	bool new_vblank;

	
	// fetcher
	bool hblank;
	int x_cord; // current x cord of the ppu
	Pixel_Obj ppu_fifo[168]; // pixel fifo for the line
	int pixel_idx;

	uint8_t ppu_cyc; // how far for a tile fetch is
	uint8_t ppu_scyc; // how far along a sprite fetch is
	int pixel_count; // how many pixels are in the fifo
	Pixel_Obj fetcher_tile[8];
	int tile_cord;
	bool tile_ready; // is the tile fetch ready to go into the fio 
	Obj objects_priority[10]; // sprites for the current scanline
	int no_sprites; // how many sprites
	bool sprite_drawn;
	bool window_start;
	bool x_scroll_tick;
	int scx_cnt;
	
	uint8_t joypad_state; // has state of held down buttons
	// banking
	
	uint8_t *ram_banks; // 4 banks max
	int currentram_bank; // currently selected ram bank
	int currentrom_bank; // currently selected rom bank
	bool rom_banking; // is rom banking enabled
	bool enable_ram; // is ram banking enabled
//	bool rtc_enabled; // useless until the rtc is implemented!
	RomInfo rom_info;
	// pointer to the current banks range is on the end
	uint8_t *current_bank_ptr4;
	uint8_t *current_bank_ptr5;
	uint8_t *current_bank_ptr6;
	uint8_t *current_bank_ptr7;

	// ------------- sound ------------------------
	
	bool sound_enabled; // is sound enabled in nr52 or not?
	int sequencer_step; // keeps track of the current set goes from 0-8 and loops back around
	Sqaure square[4]; // holds common variables for sqaure channels

	
	
	// Sweep
	bool sweep_enabled;
	uint16_t sweep_shadow;
	int sweep_period;
	int sweep_timer;
	bool sweep_calced;
	

	/*
	FF22 - NR43 - Channel 4 Polynomial Counter (R/W)
	The amplitude is randomly switched between high and low at the given frequency. A higher frequency will make the noise to appear 'softer'.
	When Bit 3 is set, the output will become more regular, and some frequencies will sound more like Tone than Noise.
	  Bit 7-4 - Shift Clock Frequency (s)
	  Bit 3   - Counter Step/Width (0=15 bits, 1=7 bits)
	  Bit 2-0 - Dividing Ratio of Frequencies (r)
	*/

	// NOISE CHANNEL
	int clock_shift;
	int counter_width;
	int divisor_idx; // indexes into divisors table
	uint16_t shift_reg; // 15 bit reg

	
	
	
	// SDL SOUND
	SDL_AudioSpec audio_spec;
	float audio_buf[SAMPLE_SIZE];
	int audio_buf_idx; // how fill is the buffer
	int down_sample_cnt; // counter used to down sample
	int initial_sample;
	FILE *fp;
	
	// rtc 

	// sdl gfx 
	SDL_Window * window;
	SDL_Renderer * renderer;
	SDL_Texture * texture;
	
	
	
	
	// debugger vars
	#ifdef DEBUG
	int breakpoint;
	int memw_breakpoint;
	int memw_value;
	int memr_breakpoint;
	int memr_value;
	bool step;
	bool speed_up;
	#endif
	
	
	#ifdef LOGGER
	FILE *logger;
	#endif
	
	// bools used to inform cpu of special instrucitons occuring
	bool ei;
	bool di;
	bool halt;
	bool halt_bug;
	
	
	// timer
	uint16_t internal_timer;
	bool timer_reloading;
	
	// is a oam dma active
	bool oam_dma_active;
	uint16_t oam_dma_address;
	int oam_dma_index; // how far along the dma transfer we are
	
	
	
	// CGB
	bool is_cgb;
	int cgb_ram_bank_num;
	uint8_t cgb_ram_bank[7][0x1000]; // switchable work ram bank 
	int vram_bank; // what cgb vram bank are we in?
	bool is_double; // cpu is in double speed mode!
	uint8_t bg_pal[0x40]; // bg palette data
	uint8_t sp_pal[0x40]; // sprite pallete data 
	int sp_pal_idx;
	int bg_pal_idx; // index into the bg pal (entry takes two bytes)
	
	int hdma_len; // length to transfer on a  gdma
	int hdma_len_ticked; // how many total dma transfers we have done
	int dma_src;
	int dma_dst;
	bool hdma_active;
	
	
	char rom_name[256];
	int romname_len;
	
};

typedef struct CPU Cpu;


// hb = high order byte
// lb = low order byte
// refer to gb cpu manual power up sequence for details
// some inits excluded as whole memory has been set to 0 instead
// of random values


Cpu init_cpu(void); // returns an initial cpu state
void init_banking_pointers(Cpu *cpu); // init pointers for banking
void update_timers(Cpu *cpu, int cycles); // update the cpu timers
void do_interrupts(Cpu *cpu);
void update_graphics(Cpu *cpu, int cycles);
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
void write_iot(Cpu *cpu,uint16_t address, int data);
uint8_t read_iot(uint16_t address, Cpu *cpu);
uint8_t read_stackt(Cpu *cpu);
uint16_t read_stackwt(Cpu *cpu);
void tick_dma(Cpu *cpu, int cycles);
