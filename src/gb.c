#include "headers/cpu.h"
#include "headers/banking.h"
#include "headers/joypad.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void boot_bios(Cpu *cpu) 
{
	//test code to boot a bios	
		
	memset(cpu->io,0,0x100);
	cpu->af.reg = 0;
	cpu->de.reg = 0;
	cpu->sp  = 0;
	cpu->bc.reg = 0;
	cpu->hl.reg = 0;
	
	FILE *fpbin = fopen("dmg_boot.bin","rb");	
	fseek(fpbin,0,SEEK_END);
	int len = ftell(fpbin);
	rewind(fpbin);
	fread(cpu->rom_mem,1,len,fpbin);
	fclose(fpbin);
	cpu->pc = 0x0;

}


void init_sdl(Cpu *cpu)
{
	/* sdl setup */
	

	// initialize our window
	cpu->window = SDL_CreateWindow("GEMBOY",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,X*2,Y*2,SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // crashes without this on windows?
	
	// set a render for our window
	cpu->renderer = SDL_CreateRenderer(cpu->window, -1, SDL_RENDERER_ACCELERATED);

	cpu->texture = SDL_CreateTexture(cpu->renderer,
		SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, X, Y);
	memset(cpu->screen ,255,Y * X *  4 * sizeof(uint8_t));	
}

void handle_input(Cpu *cpu) 
{
	SDL_Event event;
	
		// handle input
	while(SDL_PollEvent(&event))
	{	
		switch(event.type) // <-- this needs factoring out of main
		{
	
			case SDL_WINDOWEVENT:
			{
				if(event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					SDL_SetWindowSize(cpu->window,event.window.data1, event.window.data2);
				}
				break;
			}
		
	
			case SDL_QUIT:
			{
					
				#ifdef LOGGER
				fclose(cpu->logger);
				#endif
					
					
				// save the ram and load it later
				// should do detection on the save battery
				char *savename = calloc(cpu->romname_len+5,1);
				strcpy(savename,cpu->rom_name);
					
				strcat(savename,"sv");
					
				FILE *fp = fopen(savename,"wb");
				if(fp == NULL)
				{
					printf("Error opening save file %s for saving\n",savename);
					free(savename);
					goto done;
				}
					
					
				fwrite(cpu->ram_banks,sizeof(uint8_t),(0x2000*cpu->rom_info.noRamBanks),fp);	
				free(savename);
				fclose(fp);
				#ifdef LOGGER // close the log file
					fclose(cpu->fp);
				#endif				
				done: // skip saving 
				// should clean up our state here too 
				SDL_DestroyRenderer(cpu->renderer);
				SDL_DestroyWindow(cpu->window);
				SDL_QuitSubSystem(SDL_INIT_EVERYTHING);
				SDL_Quit();
					
				puts("Emulator shutting down...");
					

				// clean up
				if(cpu->ram_banks != NULL)
				{
					free(cpu->ram_banks);
				}
				free(cpu->rom_mem);
				exit(0);
			}	
			
			case SDL_KEYDOWN:
			{
				switch(event.key.keysym.sym) // <--- could remove as repeated code
				{
					case SDLK_a: key_pressed(4,cpu); break;
					case SDLK_s: key_pressed(5,cpu); break;
					case SDLK_RETURN: key_pressed(7,cpu); break;
					case SDLK_SPACE: key_pressed(6,cpu); break;
					case SDLK_RIGHT: key_pressed(0,cpu); break;
					case SDLK_LEFT: key_pressed(1,cpu); break;
					case SDLK_UP: key_pressed(2,cpu);break;
					case SDLK_DOWN: key_pressed(3,cpu); break;
						

						
						
					#ifdef DEBUG
					case SDLK_p:
					{
						// enable the debug console by setting a breakpoint at this pc
						cpu->breakpoint = cpu->pc;
						break;
					}

					case SDLK_l:
					{
						// we are switching off 
						// we should drop the audio
						if(cpu->speed_up)
						{
							SDL_ClearQueuedAudio(1);
						}
							
						cpu->speed_up = !cpu->speed_up;
						break;
					}

					#endif	
				}
				break;
			}
			
			case SDL_KEYUP:
			{
				switch(event.key.keysym.sym)
				{
					case SDLK_a: key_released(4,cpu); break;
					case SDLK_s: key_released(5,cpu); break;
					case SDLK_RETURN: key_released(7,cpu); break;
					case SDLK_SPACE: key_released(6,cpu); break;
					case SDLK_RIGHT: key_released(0,cpu); break;
					case SDLK_LEFT: key_released(1,cpu); break;
					case SDLK_UP: key_released(2,cpu); break;
					case SDLK_DOWN: key_released(3,cpu);break;
						

					case SDLK_0: // save state
					{
						puts("Saved state!");
						char *savestname = calloc(cpu->romname_len+5,1);
						strcpy(savestname,cpu->rom_name);
		
						strcat(savestname,"svt");
							

						FILE *savstate = fopen(savestname,"wb");
						free(savestname);
						if(savstate == NULL)
						{
							puts("Failed to save state!");
							break;
						}
							
							
						fwrite(cpu,sizeof(Cpu),1,savstate);
						fwrite(cpu->ram_banks,1,0x2000*cpu->rom_info.noRamBanks,savstate);
						fclose(savstate);
						break;
					}
						
					// should do validation on our file so the user knows it is not compatible
					// rather than just letting it trash the cpu state
					case SDLK_9: // load sate
					{
						puts("Loaded state!");
						char *savestname = calloc(cpu->romname_len+5,1);
						strcpy(savestname,cpu->rom_name);
		
						strcat(savestname,"svt");
							

						FILE *savstate = fopen(savestname,"rb");
						free(savestname);
							
						// free our pointer as we will have to reallocate them 
						// as our saved struct probably contains invalid pointers
						free(cpu->rom_mem); // loaded below
						free(cpu->ram_banks);
						
						struct Memory_table memory_table[MEMORY_TABLE_SIZE];
							
						// save the memory table and recopy it in 
						// so that function pointers aernt loaded from an untrusted source
						memcpy(memory_table,cpu->memory_table,MEMORY_TABLE_SIZE*sizeof(struct Memory_table));
							
						// save our sdl pointers 
						SDL_Window * window = cpu->window;
						SDL_Renderer * renderer = cpu->renderer;
						SDL_Texture * texture = cpu->texture;
							
						fread(cpu,sizeof(Cpu),1,savstate);
						memcpy(cpu->memory_table,memory_table,MEMORY_TABLE_SIZE*sizeof(struct Memory_table));
						cpu->ram_banks = calloc(0x2000 * cpu->rom_info.noRamBanks,1); // ram banks
							
						cpu->window = window;
						cpu->renderer = renderer;
						cpu->texture = texture;
							
				
						fread(cpu->ram_banks,1,0x2000*cpu->rom_info.noRamBanks,savstate);
						cpu->rom_mem = load_rom(cpu->rom_name); 
						cache_banking_ptrs(cpu); // old bank pointers are invalid recache them
						fclose(savstate);
						break;
					}
				}
				break;
			}
		}
	}	
}


void load_save(Cpu *cpu)
{
	// check for a sav batt but for now we just copy the damb thing
	
	// should be copied back into the ram banks not the memory where
	// its accessed from

	char *savename = calloc(cpu->romname_len+5,1);
	strcat(savename,cpu->rom_name);
	
	strcat(savename,"sv");
				
	FILE *fp = fopen(savename,"rb");
	
	// if file doesn't exist just ignore it
	if(fp != NULL)
	{
		fread(cpu->ram_banks,sizeof(uint8_t),(0x2000*cpu->rom_info.noRamBanks),fp);
		fclose(fp);
	}
	
	free(savename);
	savename = NULL;
	fp = NULL;	
}