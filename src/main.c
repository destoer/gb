#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "headers/cpu.h"
#include "headers/rom.h"
#include "headers/lib.h"
#include "headers/ppu.h"
#include "headers/joypad.h"
#include "headers/opcode.h"
#include "headers/debug.h"

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

// Kirby dream land 2 is broken now as well investigate... likely related to di / ei order again... (outright emulator lockup)
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
	
	printf("Cpu located at %p\n",&cpu);
	
	cpu.rom_info = parse_rom(cpu.rom_mem); // get rom info out of the header
	if(cpu.rom_info.noRamBanks > 0)
	{
		cpu.ram_banks = calloc(0x2000 * cpu.rom_info.noRamBanks,sizeof(uint8_t)); // ram banks
	}

	else 
	{
		// dont allow any access
		cpu.ram_banks = NULL;
	}
/*
	test code to boot a bios	
		
	memset(cpu.io,0,0x100);
	cpu.af.reg = 0;
	cpu.de.reg = 0;
	cpu.sp  = 0;
	cpu.bc.reg = 0;
	cpu.hl.reg = 0;
	
	FILE *fpbin = fopen("dmg_boot.bin","rb");	


	fread(cpu.rom_mem,1,256,fpbin);

	fclose(fpbin);

	cpu.pc = 0x0;
*/
	// check for a sav batt but for now we just copy the damb thing
	
	// should be copied back into the ram banks not the memory where
	// its accessed from
	const int romname_len = strlen(argv[1]);
	char *savename = calloc(romname_len+5,1);
	strcat(savename,argv[1]);
	
	strcat(savename,"sv");
				
	FILE *fp = fopen(savename,"rb");
	
	// if file doesn't exist just ignore it
	if(fp != NULL)
	{
		fread(cpu.ram_banks,sizeof(uint8_t),(0x2000*cpu.rom_info.noRamBanks),fp);
		fclose(fp);
	}
	
	free(savename);
	savename = NULL;
	fp = NULL;
	

	
	/* sdl setup */
	
	// setup sdl
	 SDL_Event event;
	
	// initialize our window
	SDL_Window * window = SDL_CreateWindow("GEMBOY",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,X*2,Y*2,0);
	
	// set a render for our window
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Texture * texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, X, Y);
	memset(cpu.screen ,255,Y * X *  4 * sizeof(uint8_t));
	
	#ifdef DEBUG
		bool speed_up = false;
	#endif
	

	
	for(;;)
	{
		

		
		const int fps = 60; // approximation <--- could use float at 59.73
		const int screen_ticks_per_frame = 1000 / fps;
		// what it should be but ok
		
		next_time = SDL_GetTicks() + screen_ticks_per_frame;
		
		
		
		// handle input
		while(SDL_PollEvent(&event))
		{	
			if(event.type == SDL_QUIT) // <-- get saving backing up at regular intervals 
			{
				// save the ram and load it later
				// should do detection on the save battery
				savename = calloc(romname_len+5,1);
				strcpy(savename,argv[1]);
				
				strcat(savename,"sv");
				
				fp = fopen(savename,"wb");
				if(fp == NULL)
				{
					printf("Error opening save file %s for saving\n",savename);
					free(savename);
					goto done;
				}
				
				
				fwrite(cpu.ram_banks,sizeof(uint8_t),(0x2000*cpu.rom_info.noRamBanks),fp);
				
				

				
				free(savename);
				fclose(fp);
				
				done: // skip saving 
				// should clean up our state here too 
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
				SDL_Quit();
				
				puts("Emulator shutting down...");
				

				// clean up

				if(cpu.ram_banks != NULL)
				{
					free(cpu.ram_banks);
				}
				free(cpu.rom_mem);
				return 0;
			}	
			
			if(event.type == SDL_KEYDOWN)
			{
				int key = -1;
				switch(event.key.keysym.sym) // <--- could remove as repeated code
				{
					case SDLK_a: key = 4; break;
					case SDLK_s: key = 5; break;
					case SDLK_RETURN: key = 7; break;
					case SDLK_SPACE: key = 6; break;
					case SDLK_RIGHT: key = 0; break;
					case SDLK_LEFT: key = 1; break;
					case SDLK_UP: key = 2; break;
					case SDLK_DOWN: key = 3; break;
					
					
					#ifdef DEBUG
					case SDLK_p:
					{
						// enable the debug console by setting a breakpoint at this pc
						cpu.breakpoint = cpu.pc;
						break;
					}

					case SDLK_l:
					{
						speed_up = !speed_up;
						break;
					}

					#endif
					

					
					
					
				}
				if(key != -1)
				{
					key_pressed(key,&cpu);
				}
			}
			
			else if(event.type == SDL_KEYUP)
			{
				int key = -1;
				switch(event.key.keysym.sym)
				{
					case SDLK_a: key = 4; break;
					case SDLK_s: key = 5; break;
					case SDLK_RETURN: key = 7; break;
					case SDLK_SPACE: key = 6; break;
					case SDLK_RIGHT: key = 0; break;
					case SDLK_LEFT: key = 1; break;
					case SDLK_UP: key = 2; break;
					case SDLK_DOWN: key = 3; break;
					
					// stating prone to crashing atm
					// seems to work fine before an emulator reboot..
					case SDLK_0: // save state
					{
						puts("Saved state!");
						char *savestname = calloc(romname_len+5,1);
						strcpy(savestname,argv[1]);
	
						strcat(savestname,"svt");
						

						FILE *savstate = fopen(savestname,"wb");
						free(savestname);
						if(savstate == NULL)
						{
							puts("Failed to save state!");
							break;
						}
						
						
						fwrite(&cpu,sizeof(Cpu),1,savstate);
						fwrite(cpu.ram_banks,1,0x2000*cpu.rom_info.noRamBanks,savstate);
						fclose(savstate);
						break;
					}
					
					// should do validation on our file so the user knows it is not compatible
					// rather than just letting it trash the cpu state
					case SDLK_9: // load sate
					{
						puts("Loaded state!");
						char *savestname = calloc(romname_len+5,1);
						strcpy(savestname,argv[1]);
	
						strcat(savestname,"svt");
						

						FILE *savstate = fopen(savestname,"rb");
						free(savestname);
						
						// free our pointer as we will have to reallocate them 
						// as our saved struct probably contains invalid pointers
						free(cpu.rom_mem); // loaded below
						free(cpu.ram_banks);
						
						fread(&cpu,sizeof(Cpu),1,savstate);
						
						cpu.ram_banks = calloc(0x2000 * cpu.rom_info.noRamBanks,sizeof(uint8_t)); // ram banks
						
						
			
						fread(cpu.ram_banks,1,0x2000*cpu.rom_info.noRamBanks,savstate);
						cpu.rom_mem = load_rom(argv[1]); 
						fclose(savstate);
						break;
					}
				}
				if(key != -1)
				{
					key_released(key,&cpu);
				}
			}
		}
		
		
		
		// number of cycles for a full screen redraw
		const int MAXCYCLES = (16725); // 17556 was the old one 
		int cycles_this_update = 0;	
		while(cycles_this_update < MAXCYCLES)
		{
			int cycles = step_cpu(&cpu); // will  exec the instruction tick timers gfx apu etc
			cycles_this_update += cycles;

			// now done in the opcode execution
			
			do_interrupts(&cpu); // handle interrupts 
			
			// now we need to test if an ei or di instruction
			// has just occured if it has step a cpu instr and then 
			// perform the requested operation and set the ime flag
			
			
			if(cpu.ei) // ei
			{
				cpu.ei = false;
				cycles = step_cpu(&cpu);
				// we have done an instruction now set ime
				// needs to be just after the instruction service
				// but before we service interrupts
				if(!cpu.di) // if the next instruction executed is a di no interrupts should be enabled
				{
					cpu.interrupt_enable = true;
				}
				
				else
				{
					cpu.di = false; // turn di off so it doesent immediately trigger 
				}
				
				cycles_this_update += cycles;
				do_interrupts(&cpu); // handle interrupts 
			}
			
			if(cpu.di) // di
			{
				cpu.di = false;
				cpu.interrupt_enable = false; // di should disable immediately unlike ei!
			}
			
			
			
			// this will make the cpu stop executing instr
			// until an interrupt occurs and wakes it up 
			
			
			if(cpu.halt) // halt occured in prev instruction
			{
			
				cpu.halt = false;

				uint8_t req = cpu.io[IO_IF]; // requested ints 
				uint8_t enabled = cpu.io[IO_IE]; // enabled interrutps
		
				// halt bug
				// halt state not entered and the pc fails to increment for
				// one instruction read 
				
				// appears to detect when it happens but does not emulate the behavior properly
				

				
				if( (cpu.interrupt_enable == false) &&  (req & enabled & 0x1f) != 0)
				{
					cpu.halt_bug = true;
				}

				
				// not sure what defined behaviour is here
				else if(enabled == 0)
				{
	
				}
				
				// normal halt
				
				else 
				{
					while( ( req & enabled & 0x1f) == 0) // <--- needs debugger access or a bailout condition
					{
						// just go a cycle at a time
						cycles_this_update += 1;
						// just tick it
						update_timers(&cpu,1); // <--- update timers 
						update_graphics(&cpu,1); // handle the lcd emulation
											// may need to tick dma here....
												
						req = cpu.io[IO_IF];
						enabled = cpu.io[IO_IE];
					}
					do_interrupts(&cpu); // handle interrupts
				}
			}	
		}
		
		
		
		// do our screen blit
		SDL_UpdateTexture(texture, NULL, &cpu.screen,  4 * X * sizeof(uint8_t));
		//SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);


		// delay to keep our emulator running at the correct speed
		// if in debug mode the l key toggles speedup
		#ifdef DEBUG
		if(speed_up)
		{
			//SDL_Delay(time_left() / 8);
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
