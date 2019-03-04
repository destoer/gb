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
#ifdef __linux__
	#include <SDL2/SDL.h>
#elif _WIN32
	#include "D:/projects/gameboy/sdllib/include/SDL2/SDL.h" 
	//#include "E:/projects/gameboy/sdllib/include/SDL2/SDL.h"
#endif 

static int next_time;

uint32_t time_left(void)
{
	int now;
	
	now = SDL_GetTicks();
	if(next_time <= now)
		return 0;
	else 
		return next_time - now;
}
/* TODO */
// implement the internal timer
// then get mem timing tests passing but breaks a couple games remove the last traces of read_mem from opcode.c tomorrow
// and do it for stack + jp + call + ret behavior
// Kirby dream land 2 is broken now as well investigate... likely related to di / ei order again... (outright emulator lockup)
// then the ppu ones <---

int main(int argc, char *argv[])
{
	
	// validate the args
	if(argc != 2)
	{
		fprintf(stderr, "[ERROR] usage: %s <rom to open>",argv[0]);
		exit(1);
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
	
	
	



	// check for a sav batt but for now we just copy the damb thing
	
	// should be copied back into the ram banks not the memory where
	// its accessed from
	
	char *savename = calloc(strlen(argv[1])+5,1);
	strcpy(savename,argv[1]);
	
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
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// initialize our window
	SDL_Window * window = SDL_CreateWindow("GEMBOY",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,X,Y,0);
	
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
		const int screen_ticks_per_frame = 1000 / fps; // <-- no idea why it is the four times 
		// what it should be but ok
		
		next_time = SDL_GetTicks() + screen_ticks_per_frame;
		
		
		
		// handle input
		while(SDL_PollEvent(&event))
		{	
			if(event.type == SDL_QUIT) // <-- get saving backing up at regular intervals 
			{
				// save the ram and load it later
				// should do detection on the save battery
				savename = calloc(strlen(argv[1])+5,1);
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
				
				done:
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
				}
				if(key != -1)
				{
					key_released(key,&cpu);
				}
			}
		}
		
		
		
		// number of cycles for a full screen redraw
		const int MAXCYCLES = (17556);
		int cycles_this_update = 0;	
		while(cycles_this_update < MAXCYCLES)
		{
			int cycles = step_cpu(&cpu);
			cycles_this_update += cycles;

			// now done in the opcode execution
			//update_timers(&cpu,cycles); // <--- update timers 
			//update_graphics(&cpu,cycles); // handle the lcd emulation
			
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
				cpu.interrupt_enable = true;
				cycles_this_update += cycles;
				// now done in the opcode execution
				//update_timers(&cpu,cycles); // <--- update timers 
				//update_graphics(&cpu,cycles); // handle the lcd emulation
				do_interrupts(&cpu); // handle interrupts <-- not sure what should happen here		
				
			}
			
			if(cpu.di) // di
			{
				cpu.di = false;
				cycles = step_cpu(&cpu);
				// we have executed another instruction now deset ime
				cpu.interrupt_enable = false;
				cycles_this_update += cycles;
				// now done in the opcode execution
				//update_timers(&cpu,cycles); // <--- update timers 
				//update_graphics(&cpu,cycles); // handle the lcd emulation
				do_interrupts(&cpu); // handle interrupts <-- what should happen here?
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
					while( ( req & enabled & 0x1f) == 0)
					{
						// just go a cycle at a time
						cycles_this_update += 1;
						// just tick it
						update_timers(&cpu,1); // <--- update timers 
						update_graphics(&cpu,1); // handle the lcd emulation
							
						req = cpu.io[IO_IF];
						enabled = cpu.io[IO_IE];
					}
					do_interrupts(&cpu); // handle interrupts
				}	
			} 
		}
		
		// do our screen blit
		SDL_UpdateTexture(texture, NULL, &cpu.screen,  4 * X * sizeof(uint8_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);


		// delay to keep our emulator running at the correct speed
		// if in debug mode the l key toggles speedup
		#ifdef DEBUG
		if(speed_up)
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
