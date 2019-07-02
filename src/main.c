 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "headers/cpu.h"
#include "headers/rom.h"
#include "headers/lib.h"
#include "headers/ppu.h"
#include "headers/joypad.h"
#include "headers/opcode.h"
#include "headers/debug.h"
#include "headers/apu.h"
#include "headers/memory.h"
#include "headers/gb.h"


// colors are off in the oracle games 
// appaers to be due to a bug in the fetcher

// sound needs improving 
// and the fetcher needs to be redone
// (linear tile fetch) & (refactor the fifo so we aint doing a ton of memcpys)

static int next_time;

uint32_t time_left(void)
{
	int now;
	
	now = SDL_GetTicks();
	if(next_time <= now)
	{
		return 0;
	}
	
	else
	{
		return next_time - now;
	}
}


/* TODO */

// <----- Work on sound support pass test 04 

// Kirby dream land 2 is broken now as well investigate... (outright emulator lockup inside halt)
// then the ppu ones <---
// implement oam bug and look at memory blocking during dma transfer
// fix enable interrupt and disable interrupt timing (does not handle rapid toggles)


// F-1 race 
// 1ece not reached
// seems the most easy to investigate

// kirby's dream land two also broken by memory accuracy?



// scx timings need to be done for the ppu 

// todo
// sound -> sgb -> cgb
// pass sound trigger test



// in need of a refactor and profiling as its unoptimised atm
// the interpreter could be doing some basic optimisaitons
// http://blargg.8bitalley.com/nes-emu/6502.html <-- this gives a good idea of the type of stuff
// ^ that can be done

// eg defering flag calcs
// (a ^ b ^ result) & 0x10 for half carry

// a big one is that sound vars should be kept in an internal representation
// as all the manual bit twiddling is slow as hell

// and doing stuff like opimising busy waits in loops

// not ticking until the device state will change 


// i.e not wriitg out the value of div to memory 
// and just returning it directly in read_mem from the internal timer


// optimsing some instrs out to contants (i.e xor a, a)
// not ticking mid instruciton when it is not required
// i.e instrucitons that 


// optimising the instr.c functiosn that the instrucitons 
// actually use
// the ppu likely has quite a few slowdowns now
// optimisng banking with a function pointer
// should probably be dumping ram banks on the stack as they are small enough

int main(int argc, char *argv[])
{
	
	// validate the args
	if(argc != 2)
	{
		fprintf(stderr, "[ERROR] usage: %s <rom to open>",argv[0]);
		exit(1);
	}

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	
	Cpu cpu = init_cpu(); // initalize the cpu
	cpu.rom_mem = load_rom(argv[1]); // read the rom into a buffer

#ifdef CGB	// gate cgb mode as it is still very buggy
	// detect if the gameboy is in CGB mode 
	
	switch(cpu.rom_mem[0x143])
	{
		case 0x80: cpu.is_cgb = true; break; // add options to run cgb in dmg1
		case 0xc0: cpu.is_cgb = true; break;
		default: cpu.is_cgb = false; break;
	}
	
	// set the cgb initial registers 
	if(cpu.is_cgb)
	{
	
		puts("CGB MODE!");

		cpu.af.reg = 0x1180;
		cpu.bc.reg = 0x0000;
		cpu.de.reg = 0xff56;
		cpu.hl.reg = 0x000d;
		cpu.sp = 0xfffe;
	}
	


	else
	{
		puts("DMG MODE!");
	}
#endif
	
	cpu.rom_info = parse_rom(cpu.rom_mem); // get rom info out of the header
	init_banking_pointers(&cpu); // now we have rom info we can parse the banking ptrs
	if(cpu.rom_info.noRamBanks > 0)
	{
		cpu.ram_banks = calloc(0x2000 * cpu.rom_info.noRamBanks,sizeof(uint8_t)); // ram banks
	}

	else 
	{
		// dont allow any access
		cpu.ram_banks = NULL;
	}


	cpu.romname_len  = strlen(argv[1]);
	strncpy(cpu.rom_name,argv[1],255); 
	
	load_save(&cpu);
	
	init_sdl(&cpu);
	
	for(;;)
	{
		const int fps = 60; // approximation <--- could use float at 59.73
		const int screen_ticks_per_frame = 1000 / fps;
		// what it should be but ok
		
		next_time = SDL_GetTicks() + screen_ticks_per_frame;
		
		
		handle_input(&cpu);
		
		while(!cpu.new_vblank) // exec until a vblank hits
		{
			step_cpu(&cpu); // will  exec the instruction tick timers gfx apu etc
			
			
			// now done in the opcode execution
			
			do_interrupts(&cpu); // handle interrupts 
			
			// now we need to test if an ei or di instruction
			// has just occured if it has step a cpu instr and then 
			// perform the requested operation and set the ime flag
			
			handle_instr_effects(&cpu);
		}
		
	
	
		// do our screen blit
		SDL_UpdateTexture(cpu.texture, NULL, &cpu.screen,  4 * X * sizeof(uint8_t));
		SDL_RenderCopy(cpu.renderer, cpu.texture, NULL, NULL);
		SDL_RenderPresent(cpu.renderer);


		
		// dont draw until next vblank
		cpu.new_vblank = false;

		// delay to keep our emulator running at the correct speed
		// if in debug mode the l key toggles speedup
		#ifdef DEBUG
		if(cpu.speed_up)
		{
			SDL_Delay(time_left() / 8);
		}

		else
		{
			SDL_Delay(time_left());
		}

		#else
		
		
		SDL_Delay(time_left());
		
		#endif		
		next_time += screen_ticks_per_frame;
	}

}
