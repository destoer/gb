#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

// supress error warnings for banking function ptrs
#define UNUSED(X) (void)(X)

// rtc enabled for mbc3
#define RTC_ENABLED -1

// sound sample size
#define SAMPLE_SIZE 1024

// cpu clock speed
#define CLOCKSPEED 4194304

// timers
#define IO_TIMA 0x05
#define IO_TMA 0x06
#define IO_TMC 0x07 
#define IO_DIV 0X04

// gb is 160x144 pixels
#define X 160
#define Y 144

// read a word from memory (bypasses the read_memory function)
// only use when memory reads dont have side affects 
uint16_t load_word(uint16_t pc, uint8_t *mem);



// if this done work (likely)
// change back function names and uncomment hte old macro

// set a bit 
#define set_bit(dest,bit) ((dest) |= (1 << bit))
//uint8_t set_bit(uint8_t num,uint8_t bit);

// deset (that is set to zero a bit)
#define deset_bit(dest,bit) ((dest) &= ~(1 << bit)) 
//uint8_t deset_bit(uint8_t num,uint8_t bit);



inline bool is_set(int reg, int bit)
{
	return ((reg >> bit) & 1);
}



inline uint8_t val_bit(uint8_t data, int position)
{
	uint8_t mask = 1 << position ;
	return ( data & mask ) ? 1 : 0 ;
}


#ifdef DEBUG
void binary(int number);
#endif



typedef struct // struct for holding sprites on a scanline
{
	int index;
	int x_pos;
} Obj;



#define TILE 0
#define TILE_CGBD 3 // priority over everything when set in tile attr
#define SPRITE_ZERO 1
#define SPRITE_ONE 2

// NEED new prioritys implemented for cgb

typedef struct 
{
	int colour_num;
	int source;
	int cgb_pal;
	bool scx_a; // should adjust for scx
	
} Pixel_Obj;




// i/o reg constants only use when performing direct access to the io array(use the actual address otherwhise)
#define IO_NR10 0x10
#define IO_NR11 0x11
#define IO_NR12 0x12
#define IO_NR13 0x13
#define IO_NR14 0x14
#define IO_NR21 0x16
#define IO_NR22 0x17
#define IO_NR23 0x18
#define IO_NR24 0x19
#define IO_NR30 0x1a
#define IO_NR31 0x1b
#define IO_NR32 0x1c
#define IO_NR33 0x1d
#define IO_NR34 0x1e
#define IO_NR41 0x20
#define IO_NR42 0x21
#define IO_NR43 0x22
#define IO_NR44 0x23
#define IO_NR50 0x24
#define IO_NR51 0x25
#define IO_NR52 0x26
#define IO_LY 0x44
#define IO_WX 0x4b
#define IO_WY 0x4a
#define IO_SCY 0x42
#define IO_SPEED 0x4D
#define IO_VBANK 0x4f
#define IO_BGPI 0x68
#define IO_BGPD 0x69
#define IO_SPPI 0x6a
#define IO_SPPD 0x6b
#define IO_LCDC 0x40
#define IO_STAT 0x41
#define IO_LYC 0x45
#define IO_DMA 0x46
#define IO_IF 0x0f
#define IO_IE 0xff
#define IO_JOYPAD 0x00
#define IO_SB 0x01
#define IO_SC 0x02

#define IO_SCX 0x43
#define IO_SVBK 0x70

// cgb dma 
#define IO_HDMA1 0x51
#define IO_HDMA2 0x52

#define IO_HDMA3 0x53
#define IO_HDMA4 0x54

#define IO_HDMA5 0x55


// potentially add constants for unused bits




// header guarded logger

#ifdef LOGGER
	#define write_log(cpu,fmt,...) write_log_func(cpu,fmt,__VA_ARGS__)
#else
	#define write_log(cpu,fmt,...) ;
#endif


#ifdef LOGGER
void write_log_func(Cpu *cpu,const char *fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	vfprintf(cpu->logger,fmt,args);
	va_end(args);
}
#endif