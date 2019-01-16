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
#include "D:/projects/gameboy/sdllib/include/SDL2/SDL.h" 


// fix timings on when conditionals are taken
// fix the halt bug 
// fix joypad <--- think its working now appears to be a seperate issue
// fix issue where tetris loops upon hitting <- key
// use different tables in opcodes for successful branches 
// fix the div and tima reg as they may be broken

// fps limiting may not be accurate and may need fixing later
static uint32_t next_time;

uint32_t time_left(void)
{
	uint32_t now;
	
	now = SDL_GetTicks();
	if(next_time <= now)
		return 0;
	else 
		return next_time - now;
}


// alot of the writemem and readme inside
// the memory and ppu functions can likely be removed 
// along with the opcode fetches

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
	
	
	
	cpu.rom_info = parse_rom(cpu.rom_mem); // get rom info out of the header
	
	
	
	
	
	// load the first banks in (implement bank switches later)
	
	memcpy(cpu.mem,cpu.rom_mem,0x8000); // memcpy the first 2 banks in
	
	
	
	
	// check for a sav batt but for now we just copy the damb thing
	char *savename = calloc(strlen(argv[1])+5,1);
	strcpy(savename,argv[1]);
	
	strcat(savename,"sv");
				
	FILE *fp = fopen(savename,"r");
	
	// if file doesn't exist just ignore it
	if(fp != NULL)
	{
		fread(&cpu.mem[0xa000],1,0x2000,fp);
		fclose(fp);
	}
	
	free(savename);
	
	
/*//----------------------------------------------------------	
	// memcpy our boot rom over this (we gonna find the bugs)
	// read in the rom
	FILE *fp = fopen("dmg_boot.bin", "rb");
	
	if(fp == NULL)
	{
		fprintf(stderr, "Failed to open file: %s","dmg_bot.bin");
		exit(1);
	}

	fseek(fp, 0, SEEK_END); // get file size and set it to len
	int len = ftell(fp); // get length of file
	printf("Rom length is %d (bytes)\n", len );
	rewind(fp); // go to start of file
	
	
	uint8_t *rom = malloc(len * sizeof(uint8_t));
	
	fread(rom, sizeof(uint8_t), len, fp); // load file into array buffer
	
	
	memcpy(cpu.mem,rom,len);
	cpu.pc = 0;
	cpu.de.reg = 0;
	cpu.af.reg = 0;
	cpu.sp = 0;
	cpu.bc.reg = 0;
	
//-------------------------------------------------------------------- */
	
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
	
	

	
	
	for(;;)
	{
		

		
		const uint32_t fps = 60; // approximation <--- could use float at 59.73
		const uint32_t screen_ticks_per_frame = 1000 / fps; // <-- no idea why it is the four times 
		// what it should be but ok
		
		next_time = SDL_GetTicks() + screen_ticks_per_frame;
		
		
		
		// handle input
		while(SDL_PollEvent(&event))
		{	
			if(event.type == SDL_QUIT)
			{
				// save the ram and load it later
				// should do detection on the save battery
				char *savename = calloc(strlen(argv[1])+5,1);
				strcpy(savename,argv[1]);
				
				strcat(savename,"sv");
				
				FILE *fp = fopen(savename,"w");
				
				fwrite(&cpu.mem[0xa000],1,0x2000,fp);
				
				free(savename);
				fclose(fp);
				
				// should clean up our state here too 
				
				SDL_Quit();
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
					
					

					case SDLK_p:
					{
						// enable the debug console by setting a breakpoint at this pc
						cpu.breakpoint = cpu.pc;
					}
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
		
		
		
		
		const int MAXCYCLES = (69905 / 4);
		int cycles_this_update = 0;	
		while(cycles_this_update < MAXCYCLES)
		{
			
			
			int cycles = step_cpu(&cpu);			
			cycles_this_update += cycles;
			update_timers(&cpu,cycles); // <--- update timers 
			//printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
			update_graphics(&cpu,cycles); // handle the lcd emulation
			do_interrupts(&cpu); // handle interrupts <-- verify it works
			
			
			// now we need to test if an ei or di instruction
			// has just occured if it has step a cpu instr and then 
			// perform the requested operation and set the ime flag
			
			// refactor as the code looks messy this way but use the general idea
			
			//luckily all the tricky opcodes are of length one
			// so hardcoding the size if fine
			//uint8_t last_opcode = read_mem(cpu.pc-1,&cpu);
			
			if(cpu.ei) // ei
			{
				cpu.ei = false;
				cycles = step_cpu(&cpu);
				cycles_this_update += cycles;
				update_timers(&cpu,cycles); // <--- update timers 
				//printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
				update_graphics(&cpu,cycles); // handle the lcd emulation
				do_interrupts(&cpu); // handle interrupts <-- verify it works
				
				// we have done an instruction now set ime
				// may need to be just after the instruction service
				// but before we service interrupts
				
				cpu.interrupt_enable = true;
			}
			
			else if(cpu.di) // di
			{
				cpu.di = false;
				cycles = step_cpu(&cpu);
				cycles_this_update += cycles;
				update_timers(&cpu,cycles); // <--- update timers 
				//printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
				update_graphics(&cpu,cycles); // handle the lcd emulation
				do_interrupts(&cpu); // handle interrupts <-- verify it works
				
				// we have executed another instruction now deset ime
				
				cpu.interrupt_enable = false;
			}
			
			
			
			// this will make the cpu stop executing instr
			// until an interrupt occurs and wakes it up 
			
			
			else if(cpu.halt) // halt  <-- bugged as hell 
			{
				//exit(1);
				cpu.halt = false;
				/*puts("Cpu halted");
				cpu_state(&cpu);
				print_flags(&cpu);
				printf("int flag %x : ie: %x\n",cpu.mem[0xff0f],cpu.mem[0xffff]);
				printf("ime: %x\n",cpu.interrupt_enable);
				*/
				//for(;;) { }
				
				
				
				//cpu.pc -= 1;
				bool imeoff = false;
				bool interruptnotfired = true;
				uint8_t req = cpu.mem[0xff0f]; // req ints
				uint8_t enabled = cpu.mem[0xffff]; // enabled interrutps
				
				

				
				// halt bug
				// halt state not entered and the pc fails to increment for
				// one instruction read 
				
				// appears to detect when it happens but does not emulate the behavior properly
				
				if( cpu.interrupt_enable == false &&  (req & enabled) != 0)
				{
					puts("HALT BUG");
					cpu.halt_bug = true;
					//printf("halt bug over\n");
				}
				
				// normal
				else
				{
					// while we have not serviced an interrupt
					while(interruptnotfired) 
					{
						cycles = 1; // just go a cycle at a time
						cycles_this_update += cycles;
						update_timers(&cpu,cycles); // <--- update timers 
						//printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
						update_graphics(&cpu,cycles); // handle the lcd emulation
						
						uint8_t req = cpu.mem[0xff0f];
						uint8_t enabled = cpu.mem[0xffff];
						

						if(req > 0)
						{
						
							for(int i = 0; i < 5; i++)
							{
								
								// if requested
								if(is_set(req,i))
								{
									// check that the particular interrupt is enabled
									if(is_set(enabled,i))
									{
										interruptnotfired = false; // interrupt ready to be fired
									}
								}
							}
						}
						// if ime is off it should just wake up the cpu and then resume normal execution
						
						do_interrupts(&cpu); // handle interrupts 
					}
				}
				
			} 


		}
		
		cycles_this_update = 0;
		SDL_UpdateTexture(texture, NULL, &cpu.screen,  4 * X * sizeof(uint8_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		//memset(cpu.screen,255,X*Y*4);
		//puts("stub"); // need to add frame limiting 
		//puts(

		SDL_Delay(time_left());
		next_time += screen_ticks_per_frame;
	}

}
