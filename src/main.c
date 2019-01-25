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
#include <SDL2/SDL.h>
//#include "D:/projects/gameboy/sdllib/include/SDL2/SDL.h" 



// fix dma timings 

// fix ppu window 
// cause links awakening is scrolling the thing
// when it shouldunt be affected

// fps limiting may not be accurate and may need fixing later
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


// alot of the writemem and readme inside
// the memory and ppu functions can likely be removed 
// along with the opcode fetches
// need to fix mbc2 (or maybye just battery detection)
// either way f1 race aint saving 



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
	cpu.rom_info.filename = calloc(strlen(argv[1])+1,sizeof(char));
	strcpy(cpu.rom_info.filename,argv[1]);
	
	
	
	
	// load the first banks in (implement bank switches later)
	
	memcpy(cpu.mem,cpu.rom_mem,0x8000); // memcpy the first 2 banks in
	
	
	
	
	// check for a sav batt but for now we just copy the damb thing
	
	// should be copied back into the ram banks not the memory where
	// its accessed from
	
	char *savename = calloc(strlen(argv[1])+5,1);
	strcpy(savename,argv[1]);
	
	strcat(savename,"sv");
				
	FILE *fp = fopen(savename,"r");
	
	// if file doesn't exist just ignore it
	if(fp != NULL)
	{
		
		
		fread(cpu.ram_banks,sizeof(uint8_t),(0x2000*cpu.rom_info.noRamBanks),fp);
		fclose(fp);
		
	}
	
	free(savename);
	savename = NULL;
	fp = NULL;
	
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
				
				fp = fopen(savename,"w");
				
				
				
				fwrite(cpu.ram_banks,sizeof(uint8_t),(0x2000*cpu.rom_info.noRamBanks),fp);
				
				
				//fwrite(&cpu.mem[0xa000],1,0x2000,fp);
				
				free(savename);
				fclose(fp);
				
				// should clean up our state here too 
				SDL_DestroyRenderer(renderer);
				SDL_DestroyWindow(window);
				SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
				SDL_Quit();
				
				puts("Emulator shutting down...");
				

				// clean up
				free(cpu.rom_info.filename);
				free(cpu.mem);
				free(cpu.ram_banks);
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
			
			if(cpu.di) // di
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
			
			
			if(cpu.halt) // halt  <-- bugged as hell 
			{
				
				cpu.halt = false;

				uint8_t req = cpu.mem[0xff0f]; // req ints 
				uint8_t enabled = cpu.mem[0xffff]; // enabled interrutps
				
				//printf("halt req: %x, enabled: %x, ime: %x\n",req,enabled,cpu.interrupt_enable);
				
				
				// halt bug
				// halt state not entered and the pc fails to increment for
				// one instruction read 
				
				// appears to detect when it happens but does not emulate the behavior properly
				
				if( (cpu.interrupt_enable == false) &&  (req & enabled & 0x1f) != 0)
				{
					//puts("HALT BUG");
					cpu.halt_bug = true;
					//printf("halt bug over\n");
				}

				
				// normal halt
				
				else 
				{		
					/*printf("ly = %x\n",read_mem(0xff44,&cpu));
					printf("div = %x\n",cpu.mem[DIV]);
					printf("tima = %x\n",cpu.mem[TIMA]);
					printf("lcdc = %x\n",cpu.mem[0xff41]);
					printf("if = %x\n",cpu.mem[0xff0f]);
					printf("ime = %x\n",cpu.interrupt_enable);
					printf("ie = %x\n",cpu.mem[0xffff]);
					printf("stat = %x\n",cpu.mem[0xff41]); // lcd stat
					printf("lyc = %x\n",cpu.mem[0xff45]);
					cpu_state(&cpu);
					print_flags(&cpu);
					*/
					//puts("normal halt");
					while( ( req & enabled & 0x1f) == 0)
					{
						cycles = 1; // just go a cycle at a time
						cycles_this_update += cycles;
						update_timers(&cpu,cycles); // <--- update timers 
						//printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
						update_graphics(&cpu,cycles); // handle the lcd emulation
							
						req = cpu.mem[0xff0f];
						enabled = cpu.mem[0xffff];
					}
					//puts("interrupted halt");
					do_interrupts(&cpu); // handle interrupts
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

		//SDL_Delay(time_left());
		next_time += screen_ticks_per_frame;
	}

}
