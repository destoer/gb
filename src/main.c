#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "headers/cpu.h"
#include "headers/rom.h"
#include "headers/lib.h"
#include "headers/ppu.h"
#include "D:/projects/gameboy/sdllib/include/SDL2/SDL.h" 




int main(int argc, char *argv[])
{
	
	// validate the args
	if(argc != 2)
	{
		fprintf(stderr, "[ERROR] usage: %s <rom to open>",argv[0]);
		exit(1);
	}
	
	
	// bios booting tes
	Cpu cpu;
	memset(&cpu,0,sizeof(Cpu));
	
	cpu.mem = calloc(0x10000, sizeof(uint8_t)); // main memory
	cpu.tacfreq = 256; 
	cpu.timerenable = true;
	// read in the bios
	FILE *fp = fopen(argv[1], "rb");
	
	if(fp == NULL)
	{
		fprintf(stderr, "Failed to open file: %s",argv[1]);
		exit(1);
	}

	fseek(fp, 0, SEEK_END); // get file size and set it to len
	int len = ftell(fp); // get length of file
	printf("Rom length is %d (bytes)\n", len );
	rewind(fp); // go to start of file
	
	
	uint8_t *rom = malloc(len * sizeof(uint8_t));
	
	fread(rom, sizeof(uint8_t), len, fp); // load file into array buffer
	fclose(fp); // done with rom now

	
	uint8_t *rom2 = load_rom("Tetris (World).gb"); // load the rom in 
	
	memcpy(cpu.mem,rom2,0x8000); // memcpy first bank in
	
	
	memcpy(cpu.mem,rom,len); // overwrite with the boot code
	
	
	
	
	
	
	/* sdl setup */
	
	// setup sdl
	 SDL_Event event;
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// initialize our window
	SDL_Window * window = SDL_CreateWindow("Gameboy",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,X,Y,0);
	
		// set a render for our window
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture * texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, X, Y);
	memset(cpu.screen ,255,Y * X *  3 * sizeof(uint8_t));
	
	
	
	for(;;)
	{
		const int MAXCYCLES = 69905;
		int cycles_this_update = 0;
		
		while(cycles_this_update < MAXCYCLES)
		{
			int cycles = step_cpu(&cpu,rom);
			cycles_this_update += cycles;
			update_timers(&cpu,cycles); // <--- update timers 
			//printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
			update_graphics(&cpu,cycles); // handle the lcd emulation
			do_interrupts(&cpu); // handle interrupts <-- verify it works
		}
		
		cycles_this_update = 0;
		SDL_UpdateTexture(texture, NULL,cpu.screen, Y * sizeof(uint8_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		//puts("stub"); // need to add frame limiting 
		//puts(
	}
	
	
	
/*	
	uint8_t *rom = load_rom(argv[1]); // read the rom into a buffer
	
	RomInfo romInfo = parse_rom(rom); // get rom info out of the header
	
	Cpu cpu = init_cpu(); // initalize the cpu
	
	// load the first bank in (implement bank switches later)
	
	if(romInfo.cartType == 0) // rom only
	{
		memcpy(cpu.mem,rom,0x8000); // memcpy the first bank in
	}
	
	else
	{
		fprintf(stderr, "Cart type not implemented\n");
		return 1;
	}
	
	uint8_t screen[160][144][3]; // 160x144 resolution color val stored at each one
	
	// setup sdl
	 SDL_Event event;
	if(SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// initialize our window
	SDL_Window * window = SDL_CreateWindow("Gameboy",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,Y,X,0);
	
		// set a render for our window
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture * texture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, Y, X);
	memset(screen ,255,Y * X *  3 * sizeof(uint8_t));
	
	// begin the main emulation loop
	// get things like timers and cycles working from the get go 
	
	int totalcycles;
	
	for(;;)
	{
		
		// 0x2817 is first graphics draw 
		int cycles = step_cpu(&cpu,rom);
		update_timers(&cpu,cycles); // <--- update timers 
		printf("tima: %d, div: %d\n",cpu.mem[TIMA], cpu.mem[DIV]);
		
		// handle lcd
		SDL_UpdateTexture(texture, NULL,screen, Y * sizeof(uint8_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
		{
			SDL_DestroyTexture(texture);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
			return 0;
		}
		// render the screen
	}
*/
}
