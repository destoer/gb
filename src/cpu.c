#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "headers/cpu.h"
#include "headers/disass.h"
#include "headers/opcode.h"
#include "headers/lib.h"
#include "headers/banking.h"
#include "headers/instr.h"
#include "headers/debug.h"
#include "headers/ppu.h"
#include "headers/memory.h"
#include "headers/apu.h"

void cycle_tick(Cpu *cpu,int cycles);

uint8_t read_mem(uint16_t address, Cpu *cpu);
int set_clock_freq(Cpu *cpu);
void service_interrupt(Cpu *cpu,int interrupt);


// fix interrupt timing

// WORK on sound emulation 1st make sure unused bits are properly handled

// implement internal timer behaviour 

// setup a fresh cpu state and return it to main
Cpu init_cpu(void) // <--- memory should be randomized on startup
{	
	Cpu cpu;

	memset(&cpu, 0, sizeof(Cpu));


	
	#ifdef LOGGER
	cpu.logger = fopen("log.txt","w");
	if(cpu.logger == NULL)
	{
		puts("Could not open log file for logging");
		exit(1);
	}
	#endif
	
	cpu.currentram_bank = 0;
	cpu.currentrom_bank = 1; // all ways at least one
	cpu.rom_banking = true; // default
	cpu.enable_ram = false;
//	cpu.rtc_enabled = false;


	// dmg boot reg values 
	// need alternate ones for when we switch to cgb
	cpu.af.reg = 0x01B0; 
	cpu.bc.reg = 0x0013;
	cpu.de.reg = 0x00D8;
	cpu.hl.reg = 0x014d;
	cpu.sp = 0xFFFE;


	cpu.io[0x10] = 0x80;
	cpu.io[0x11] = 0xBF;	
	cpu.io[0x12] = 0xF3;
	cpu.io[0x14] = 0xBF;
	cpu.io[0x16] = 0x3F;
	cpu.io[0x19] = 0xBF;
	cpu.io[0x1A] = 0x7F;
	cpu.io[0x1B] = 0xFF;
	cpu.io[0x1C] = 0x9F;
	cpu.io[0x1E] = 0xBF;
	cpu.io[0x20] = 0xFF;
	cpu.io[0x23] = 0xBF;
	cpu.io[0x24] = 0x77;
	cpu.io[0x25] = 0xF3;
	cpu.io[0x26] = 0xF1;
	cpu.io[0x40] = 0x91;
	cpu.io[0x47] = 0xFC;
	cpu.io[0x48] = 0xFF;
	cpu.io[0x49] = 0xFF;
	
	
	
	
	// init unused hwio
	cpu.io[0x03] = 0xff;
	cpu.io[0x08] = 0xff;
	cpu.io[0x09] = 0xff;
	cpu.io[0x0a] = 0xff;
	cpu.io[0x0b] = 0xff;
	cpu.io[0x0c] = 0xff;
	cpu.io[0x0d] = 0xff;
	cpu.io[0x0e] = 0xff;
	cpu.io[0x15] = 0xff;
	cpu.io[0x1f] = 0xff;
	cpu.io[0x27] = 0xff;
	cpu.io[0x28] = 0xff;
	cpu.io[0x29] = 0xff;
	cpu.io[0x20] = 0xff;

	
	cpu.pc = 0x100; // reset at 0x100

	cpu.scanline_counter = 0;
	cpu.current_line = 0;
	cpu.signal = false;
	cpu.joypad_state = 0xff;
	cpu.interrupt_enable = false;
	
	cpu.di = false;
	cpu.ei = false;
	cpu.halt = false;
	cpu.halt_bug = false;
	
	cpu.oam_dma_active = false;
	cpu.hblank = false;
	cpu.tile_ready = false;
	cpu.x_cord = 0;
	cpu.pixel_count = 0;
	cpu.tile_cord = 0;
	cpu.sprite_drawn = false;
	cpu.window_start = false;
	cpu.ppu_cyc = 0;
	cpu.ppu_scyc = 0;

	
	#ifdef DEBUG
	// debugging vars 
	cpu.breakpoint = 0x100;
	cpu.memr_breakpoint = -1;
	cpu.memw_breakpoint = -1;
	cpu.memr_value = -1;
	cpu.memw_value = -1;
	#endif


	
	
	// sound 
	cpu.sequencer_step = 0;
	cpu.sound_enabled = true;
	cpu.sweep_enabled = false;
	


	// init sdl sound 
	// Set up SDL audio spec
	cpu.audio_spec.freq = 44100;
	cpu.audio_spec.format = AUDIO_F32SYS;
	cpu.audio_spec.channels = 2;
	cpu.audio_spec.samples = SAMPLE_SIZE;	
	cpu.audio_spec.callback = NULL; // we will use SDL_QueueAudio()  rather than 
	cpu.audio_spec.userdata = NULL; // using a callback :)

	cpu.audio_buf_idx = 0;
	cpu.down_sample_cnt = 23;
	
	
	
	//const int ndev = SDL_GetNumAudioDevices(0);
	
	printf("no of audio devices: %d\n",SDL_GetNumAudioDevices(0));
	
/*	bool success = false;
	int dev = -1;
	for(int i = 0; i < ndev; i++)
	{
		char *device = SDL_GetAudioDeviceName(i,0);
		printf("device %d: %s\n",i,device);
		//dev = SDL_OpenAudioDevice(NULL,0,&cpu.audio_spec,NULL,0);
		dev = SDL_OpenAudioDevice(device,0,&cpu.audio_spec,NULL,0);
		if(dev >= 0)
		{
			printf("opened device %d(%s)!\n",dev,SDL_GetAudioDeviceName(dev,0));
			success = true; break;
		}
	}

	printf("success = %s\n", success ? "true" : "false");
	
	//if(SDL_OpenAudio(&cpu.audio_spec, NULL) < 0)
		
	if(!success)
	{
		fprintf(stderr, "Could not open audio %s", SDL_GetError());
		exit(1);
	}

	// enable playback
	SDL_PauseAudioDevice(dev, 0);
*/

	// using the legacy interface
	
	int dev = 1;
	
	SDL_OpenAudio(&cpu.audio_spec,NULL);
	SDL_PauseAudio(0);
	
	printf("opened device %d(%s)!\n",dev,SDL_GetAudioDeviceName(dev,0));
	
	printf("Device state: ");
    switch (SDL_GetAudioDeviceStatus(dev))
    {
        case SDL_AUDIO_STOPPED: printf("stopped\n"); break;
        case SDL_AUDIO_PLAYING: printf("playing\n"); break;
        case SDL_AUDIO_PAUSED: printf("paused\n"); break;
        default: printf("???"); break;
    }

	cpu.initial_sample =  SDL_GetQueuedAudioSize(dev);

	printf("Queued audio: %d!\n",cpu.initial_sample);
	
	//cpu.fp = fopen("audio.pcm","wb+");

	
	
	// init the cgb stuff
	memset(cpu.bg_pal,0xff,0x40);
	memset(cpu.sp_pal,0xff,0x40);


	// init our table our memory pointers
	/* read */
	cpu.memory_table[0x0].read_memf = read_rom_bank_zero; // 0x0000
	cpu.memory_table[0x1].read_memf = read_rom_bank_zero; // 0x1000
	cpu.memory_table[0x2].read_memf = read_rom_bank_zero; // 0x2000
	cpu.memory_table[0x3].read_memf = read_rom_bank_zero; // 0x3000
	cpu.memory_table[0x4].read_memf = read_rom_bank4; // 0x4000
	cpu.memory_table[0x5].read_memf = read_rom_bank5; // 0x5000
	cpu.memory_table[0x6].read_memf = read_rom_bank6; // 0x6000
	cpu.memory_table[0x7].read_memf = read_rom_bank7; // 0x7000
	cpu.memory_table[0x8].read_memf = read_mem_vram; // 0x8000
	cpu.memory_table[0x9].read_memf = read_mem_vram; // 0x9000
	cpu.memory_table[0xa].read_memf = read_cart_ram; // 0xa000
	cpu.memory_table[0xb].read_memf = read_cart_ram; // 0xb000
	cpu.memory_table[0xc].read_memf = read_wram_low; // 0xc000
	cpu.memory_table[0xd].read_memf = read_wram_high; //0xd000
	cpu.memory_table[0xe].read_memf = read_wram_low; // 0xe000 (echo ram)
	cpu.memory_table[0xf].read_memf = read_mem_hram;
	/* write */
	// 0x7 - 0x8 is initialized in init_banking_pointers
	cpu.memory_table[0x8].write_memf = write_vram_mem; // 0x8000
	cpu.memory_table[0x9].write_memf = write_vram_mem; // 0x9000
	cpu.memory_table[0xa].write_memf = write_cart_mem; // 0xa000
	cpu.memory_table[0xb].write_memf = write_cart_mem; // 0xb000
	cpu.memory_table[0xc].write_memf = write_wram_low; // 0xc000
	cpu.memory_table[0xd].write_memf = write_wram_high; //0xd000
	cpu.memory_table[0xe].write_memf = write_wram_low; // 0xe000 (echo ram)
	cpu.memory_table[0xf].write_memf = write_hram;	   // 0xf000
	
	
	
	return cpu;
}

void init_banking_pointers(Cpu *cpu)
{
	
	cache_banking_ptrs(cpu);
	
	// mbc1 rom
	if(cpu->rom_info.mbc1)
	{
		cpu->memory_table[0x0].write_memf = do_ram_bank_enable;
		cpu->memory_table[0x1].write_memf = do_ram_bank_enable;
		cpu->memory_table[0x2].write_memf = do_change_lo_rom_bank_mbc1;
		cpu->memory_table[0x3].write_memf = do_change_lo_rom_bank_mbc1;
		cpu->memory_table[0x4].write_memf = mbc1_banking_change;
		cpu->memory_table[0x5].write_memf = mbc1_banking_change; 
		cpu->memory_table[0x6].write_memf = do_change_rom_ram_mode;
		cpu->memory_table[0x7].write_memf = do_change_rom_ram_mode;
		return;
	}
	
	// mbc2 rom
	else if(cpu->rom_info.mbc2)
	{
		cpu->memory_table[0x0].write_memf = do_ram_bank_enable_mbc2;
		cpu->memory_table[0x1].write_memf = do_ram_bank_enable_mbc2;
		// this is shared with mbc1
		cpu->memory_table[0x2].write_memf = do_change_lo_rom_bank_mbc1;
		cpu->memory_table[0x3].write_memf = do_change_lo_rom_bank_mbc1;
		
		// no ram bank changing
		cpu->memory_table[0x4].write_memf = banking_unused;
		cpu->memory_table[0x5].write_memf = banking_unused;
		
		// as is the high banking for mbc2
		cpu->memory_table[0x6].write_memf = banking_unused;
		cpu->memory_table[0x7].write_memf = banking_unused;
		return;
	}
	
	// mbc3 rom
	else if(cpu->rom_info.mbc3)
	{
		cpu->memory_table[0x0].write_memf = do_ram_bank_enable;
		cpu->memory_table[0x1].write_memf = do_ram_bank_enable;
		cpu->memory_table[0x2].write_memf = do_change_rom_bank_mbc3 ;
		cpu->memory_table[0x3].write_memf = do_change_rom_bank_mbc3;
		cpu->memory_table[0x4].write_memf = mbc3_ram_bank_change;
		cpu->memory_table[0x5].write_memf = mbc3_ram_bank_change;
		cpu->memory_table[0x6].write_memf = banking_unused;
		cpu->memory_table[0x7].write_memf = banking_unused;
		return;
	}
	
	// mbc5 rom
	else if(cpu->rom_info.mbc5)
	{
		cpu->memory_table[0x0].write_memf = do_ram_bank_enable;
		cpu->memory_table[0x1].write_memf = do_ram_bank_enable;
		cpu->memory_table[0x2].write_memf = do_change_lo_rom_bank_mbc5;
		cpu->memory_table[0x3].write_memf = do_change_hi_rom_bank_mbc5;
		cpu->memory_table[0x4].write_memf = mbc5_ram_bank_change;
		cpu->memory_table[0x5].write_memf = mbc5_ram_bank_change;
		cpu->memory_table[0x6].write_memf = banking_unused;
		cpu->memory_table[0x7].write_memf = banking_unused;		
		return;
	}
	
	// zero in cart type is rom only 
	// if aint this then somethign is wrong
	else if(cpu->rom_info.cartType != 0)
	{
		printf("invalid rom type in init_banking_pointers!");
		exit(1);
	}
	
	
	// rom only 
	for(int i = 0; i < 8; i++)
	{
		cpu->memory_table[i].write_memf = banking_unused; // banking not used on rom only
	}
	
}

// needs accuracy improvement with the precise interrupt 
// timings to pass ie_push

void request_interrupt(Cpu * cpu,int interrupt)
{
	// set the interrupt flag to signal
	// an interrupt request
	uint8_t req = cpu->io[IO_IF];
	set_bit(req,interrupt);
	cpu->io[IO_IF] = req;
}



void do_interrupts(Cpu *cpu)
{
	// if interrupts are enabled
	if(cpu->interrupt_enable)
	{	
		// get the set requested interrupts
		uint8_t req = cpu->io[IO_IF];
		// checked that the interrupt is enabled from the ie reg 
		uint8_t enabled = cpu->io[IO_IE];
		
		if(req > 0)
		{
			// priority for servicing starts at interrupt 0
			for(int i = 0; i < 5; i++)
			{
				// if requested & is enabled
				if(is_set(req,i) && is_set(enabled,i))
				{
					service_interrupt(cpu,i);
					cycle_tick(cpu,5); // every interrupt service costs 5 M cycles <-- break this up tomorrow
					break;
				}
			}
		}
	}
}

void service_interrupt(Cpu *cpu,int interrupt)
{
	write_log(cpu,"Interrupt serviced at: %x, interrupt req: %x\n",cpu->pc,interrupt);
	
	cpu->interrupt_enable = false; // disable interrupts now one is serviced
		
	// reset the bit of in the if to indicate it has been serviced
	uint8_t req = cpu->io[IO_IF];
	deset_bit(req,interrupt);
	cpu->io[IO_IF] = req;
		
	// push the current pc on the stack to save it
	// it will be pulled off by reti or ret later
	write_stackw(cpu,cpu->pc);

		
	// set the program counter to the start of the
	// interrupt handler for the request interrupt
		
	switch(interrupt)
	{
		// interrupts are one less than listed in cpu manual
		// as our bit macros work from bits 0-7 not 1-8
		case 0: cpu->pc = 0x40;  break; //vblank
		case 1: cpu->pc = 0x48; break; //lcd-state 
		case 2: cpu->pc = 0x50; break; // timer 
		case 3: cpu->pc = 0x58; break; //serial (not fully implemented)
		case 4: cpu->pc = 0x60; break; // joypad
		default: printf("Invalid interrupt %d at %x\n",interrupt,cpu->pc); exit(1);
	}	
}




// writes



void write_wordt(Cpu *cpu,uint16_t address,int data) // <- verify 
{
	write_memt(cpu,address+1,((data & 0xff00) >> 8));
	write_memt(cpu,address,(data & 0x00ff));
}

void write_memt(Cpu *cpu,uint16_t address,int data)
{
	write_mem(cpu,address,data);
	cycle_tick(cpu,1); // update cycles for memory write
}

void write_word(Cpu *cpu,uint16_t address,int data) // <- verify 
{
	write_mem(cpu,address+1,((data & 0xff00) >> 8));
	write_mem(cpu,address,(data & 0x00ff));
}


uint8_t read_iot(uint16_t address, Cpu *cpu)
{
	uint8_t val = read_io(address,cpu);
	cycle_tick(cpu,1);
	return val;
}



// checked memory reads
uint16_t read_word(int address, Cpu *cpu)
{
	return read_mem(address,cpu) | (read_mem(address+1,cpu) << 8);
}


uint8_t read_memt(int address, Cpu *cpu)
{
	uint8_t val = read_mem(address,cpu);
	cycle_tick(cpu,1); // update cycles for memory read
	return val;
}


uint16_t read_wordt(int address, Cpu *cpu)
{
	return read_memt(address,cpu) | (read_memt(address+1,cpu) << 8);
}

void write_iot(Cpu *cpu,uint16_t address, int data)
{
	write_io(cpu,address,data);
	cycle_tick(cpu,1);
}


// implement the memory stuff for stack minips


void write_stack(Cpu *cpu, uint8_t data)
{
	cpu->sp -= 1; // dec stack pointer before writing mem for push
	write_mem(cpu,cpu->sp,data); // write to stack

}

void write_stackt(Cpu *cpu, uint8_t data)
{
	cpu->sp -= 1; // dec stack pointer before writing mem for push
	write_memt(cpu,cpu->sp,data); // write to stack

}

void write_stackw(Cpu *cpu,uint16_t data)
{
	write_stack(cpu,(data & 0xff00) >> 8);
	write_stack(cpu,(data &0x00ff));
}

void write_stackwt(Cpu *cpu,uint16_t data)
{
	write_stackt(cpu,(data & 0xff00) >> 8);
	write_stackt(cpu,(data & 0x00ff));
}

uint8_t read_stack(Cpu *cpu)
{
	
	uint8_t data = read_mem(cpu->sp,cpu);
	cpu->sp += 1;
	return data;
}

uint8_t read_stackt(Cpu *cpu)
{
	
	uint8_t data = read_memt(cpu->sp,cpu);
	cpu->sp += 1;
	return data;
}


uint16_t read_stackw(Cpu *cpu)
{
	return read_stack(cpu) | (read_stack(cpu) << 8);
}


uint16_t read_stackwt(Cpu *cpu)
{
	return read_stackt(cpu) | (read_stackt(cpu) << 8);
}

void cycle_tick(Cpu *cpu,int cycles)
{

	// if in double speed oam dma
	// should operate at double speed
	int factor = cpu->is_double ? 2 : 1;

	// timers act at constant speed
	update_timers(cpu,cycles*4); 

	if(cpu->oam_dma_active)
	{
		tick_dma(cpu, cycles*factor);
	}
	

	// in double speed mode gfx and apu should operate at half
	update_graphics(cpu,(cycles*4) / factor); // handle the lcd emulation
	tick_apu(cpu,(cycles*4) / factor); // advance the apu state

}





// implement internal timer behavior with div and tima
// being one reg 

// updates the timers
void update_timers(Cpu *cpu, int cycles)
{
	
	int sound_bit = cpu->is_double? 13 : 12;

	bool sound_bit_set = is_set(cpu->internal_timer,sound_bit);
				

	// if our bit is deset and it was before (falling edge)
	// and the timer is enabled of course
	if(is_set(cpu->io[IO_TMC],2))
	{
		uint8_t freq = cpu->io[IO_TMC] & 0x3;

		static const int freq_arr[4] = {9,3,5,7};

		int bit = freq_arr[freq];

		bool bit_set = is_set(cpu->internal_timer,bit);

		cpu->internal_timer += cycles;

		if(!is_set(cpu->internal_timer,bit) && bit_set)
		{

			// timer about to overflow
			if(cpu->io[IO_TIMA] == 255)
			{	
				cpu->io[IO_TIMA] = cpu->io[IO_TMA]; // reset to value in tma
				request_interrupt(cpu,2); // timer overflow interrupt
						
			}
					
			else
			{
			cpu->io[IO_TIMA]++;
			}
		}
		// we repeat this here because we have to add the cycles
		// at the proper time for the falling edge det to work
		// and we dont want to waste time handling the falling edge
		// for the timer when its off
		if(!is_set(cpu->internal_timer,sound_bit) && sound_bit_set)
		{
			advance_sequencer(cpu); // advance the sequencer
		}
	}

	// falling edge for sound clk which allways ticks
	// done when timer is off 
	// (cycles should only ever be added once per function call)
	else 
	{
		cpu->internal_timer += cycles;
		if(!is_set(cpu->internal_timer,sound_bit) && sound_bit_set)
		{
				advance_sequencer(cpu); // advance the sequencer
		}
	}
}






// this needs work
void tick_dma(Cpu *cpu, int cycles)
{
	// lie so we can freely access memory 
	cpu->oam_dma_active = false;

	if(cpu->oam_dma_index > 0xa0)
	{
		cpu->oam_dma_index += cycles;
		if( cpu->oam_dma_index >= 0xa2) // now its done 
		{
			return;
		}	
	}

	
	for(int i = 0; i < cycles; i++)
	{
		
		//write_mem(cpu,(0xfe00+cpu->oam_dma_index), read_mem((cpu->oam_dma_address+cpu->oam_dma_index),cpu));
		cpu->oam_dma_index += 1; // go to next one
		// We are done with our dma
		if(cpu->oam_dma_index > 0xa0) 
		{ 
			//cpu->oam_dma_active = false;
			
			// get ready to add cycles as 2 extra are used for start / stop
			cycles -= i+1;
		}
	}
	
	if(cpu->oam_dma_index > 0xa0)
	{
		cpu->oam_dma_index += cycles;
		if( cpu->oam_dma_index >= 0xa2) // now its done 
		{
			cpu->oam_dma_active = false;
			return;
		}
		
	}
	
	cpu->oam_dma_active = true;  // continue the dma
}
