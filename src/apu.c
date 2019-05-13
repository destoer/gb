
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "headers/cpu.h"


// sound is very broken atm need to get trigger test passing
// need to refactor it to use an internal represenation
// so it is much less of a pain to work with

// internal counter loaded upon write with 64-data  or 256-data for wave channel


// general idea seems fine but not the timing 
// investigate the timer and frame sequencer
// sweep etc 


// this works fine for now but implement properly with the desc off the reddit post
// after finishing up this part and function off the length ticking to a separate part of tick_apu
// https://www.reddit.com/r/EmuDev/comments/5gkwi5/gb_apu_sound_emulation/
// ^ look at the github links for help


/* Handle the channel dac

Channel DAC

Each channel has a 4-bit digital-to-analog convertor (DAC). This converts the input value to a proportional output voltage. An input of 0 generates -1.0 and an input of 15 generates +1.0, using arbitrary voltage units.

DAC power is controlled by the upper 5 bits of NRx2 (top bit of NR30 for wave channel). If these bits are not all clear, the DAC is on, otherwise it's off and outputs 0 volts. Also, any time the DAC is off the channel is kept disabled (but turning the DAC back on does NOT enable the channel)

*/


uint16_t get_wave_freq(Cpu *cpu)
{
	uint8_t lo = cpu->io[IO_NR33];
	uint8_t hi = cpu->io[IO_NR34] & 0x3;
	
	uint16_t freq = (hi << 8) | lo;
	
	return freq;
}


// handle ticking the length counters performed every other step
void tick_lengthc(Cpu *cpu)
{
	// This should be called every 4096 cycles
	for(int i = 0; i < 4; i++)
	{
		if(cpu->square[i].length_enabled)
		{
			// tick the length counter if zero deset it
			if(!--cpu->square[i].lengthc)
			{
				deset_bit(cpu->io[IO_NR52],i);
			}
		}
	}
}





uint16_t calc_freqsweep(Cpu *cpu)
{
	
	uint16_t shadow_shifted = cpu->sweep_shadow >> (cpu->io[IO_NR10] &  0x7);
	if(is_set(cpu->io[IO_NR10],3)) // test bit 3 of nr10 if its 1 take them away
	{
		cpu->sweep_calced = true; // calc done using negation
		return cpu->sweep_shadow - shadow_shifted;
	}
	
	// else add them
	return cpu->sweep_shadow + shadow_shifted;
}


// calc the freq sweep and do the overflow checking
// disable chan if it overflows
void do_freqsweep(Cpu *cpu)
{
	// dont calc the freqsweep if sweep peroid is zero or its disabled
	if(!cpu->sweep_enabled || !cpu->sweep_period ) { return; }
	
	uint16_t temp = calc_freqsweep(cpu);
	
	// if it is <= 2047 and shift is non 0
	// write it back to freq regs
	// and then do the calc again without writing the value back
	if(temp > 0x7ff)
	{
		cpu->sweep_timer = 0;
		deset_bit(cpu->io[IO_NR52],0);
		cpu->sweep_enabled = false;
		return;
		
	}
	
	// sweep shift not zero (can be assumed its <= 0x7ff for first calc now)
	if(cpu->io[IO_NR10] &  0x7)
	{
		// write back to shadow
		cpu->sweep_shadow = temp;
		
		// write back low 8
		cpu->io[IO_NR13] = cpu->sweep_shadow & 0xff;

		
		// and high 3
		cpu->io[IO_NR14] &= ~0x7; // mask bottom 3
		cpu->io[IO_NR14] |= (cpu->sweep_shadow >> 8) & 0x7; // and write them out
		
		
		// reperform the calc + overflow check (but dont write back)
		temp = calc_freqsweep(cpu);
		
		if(temp > 0x7ff)
		{
			cpu->sweep_timer = 0;
			deset_bit(cpu->io[IO_NR52],0);
			cpu->sweep_enabled = false;
		}	
	}
}

// not sure how the sweep timer is meant to be used?
// or what the sweep period is this is likely what is not being 
// done correctly....


// is the sweep timer how long the sweep lasts or how long until one is done?

// clock sweep generator for square 1
void clock_sweep(Cpu *cpu)
{
	
	//if(!cpu->sweep_period) { return; }
	
	
	// tick the sweep timer calc when zero
	if(cpu->sweep_timer && !--cpu->sweep_timer)
	{
		// if elapsed do the actual "sweep"
		do_freqsweep(cpu);
		
		// and reload the timer of course
		// does this use the value present during the trigger event 
		// or is it reloaded newly?
		cpu->sweep_timer = cpu->sweep_period;
		if(cpu->sweep_period == 0) cpu->sweep_timer = 8;  // see obscure behaviour		
	}
	
}

// currently this just ticks the length counter
// we want to use the proper stepping from 0-8
void tick_apu(Cpu *cpu, int cycles)
{
	// 4096((4194304 / 256 ) /4) should dec the internal counter every <-- cycles
	
	// dec the wave channel timer :) 
	// just cheat for now
	static float wave_vol = 0;
	
	// handle wave ticking
	if(is_set(cpu->io[IO_NR52],2))
	{
		cpu->wave_period -= cycles;
		
		// reload timer and goto the next sample in the wave table
		if(cpu->wave_period <= 0)
		{
			cpu->wave_idx  = (cpu->wave_idx + 1) & 0x1f;

			// dac is enabled
			if(is_set(cpu->io[IO_NR30],7))
			{
				int pos = cpu->wave_idx / 2;
				uint8_t byte = cpu->io[0x30 + pos];
				
				if(!(cpu->wave_idx & 0x1)) // access the high nibble first
				{
					byte >>= 4;
				}
				byte &= 0xf;
				
				uint8_t vol = cpu->io[IO_NR32] & 0x3;
				
				if(vol) byte >>= vol - 1;
				else byte = 0;
				
				wave_vol = byte;
				//puts("filling queue!");
			}
			
			else  wave_vol = 0;
			
			
			// reload the timer
			// period (2048-frequency)*2
			uint16_t freq = get_wave_freq(cpu);
			cpu->wave_period = ((2048-freq)*2) / 4; // may need to be / 4 for M cycles					
		}
		
		
		
		
		
	}
	
	

	// so every 8192 cycles the sequencer will increment
	// when it does every other step it will clock our length counters 
	// we will then use a switch and determine what to do 
	// could also use function pointers but this is likely better
	
	
	
	// first we need to tick the frame sequencer
	
	cpu->sequencer_cycles += cycles;
	
	// now if 8192 T cycles (2048 M cycles) have elapsed we need to inc the step
	// and perform the action for the current step
	
	if(cpu->sequencer_cycles >= 2048)
	{

		// go to the next step
		cpu->sequencer_step += 1;
		
		if(cpu->sequencer_step == 8)
		{
			cpu->sequencer_step = 0; // loop back around the steps
		}
		
		// switch and perform the function required for our step
		switch(cpu->sequencer_step)
		{
			case 0: // clock the length counters
			{
				tick_lengthc(cpu);
				break;
			}
			
			case 1: break; // do nothing

			
			case 2: // sweep generator + lengthc
			{
				clock_sweep(cpu);
				tick_lengthc(cpu);
				break;
			}
			
			case 3: break; // do nothing

			
			case 4: // clock lengthc
			{
				tick_lengthc(cpu);
				break;
			}
			
			case 5: break; // do nothing
			
			case 6:  // clock the sweep generator + lengthc
			{
				clock_sweep(cpu);
				tick_lengthc(cpu);
				break;
			}
			
			case 7: break; //clock the envelope 
			
			default:
			{
				printf("Error unknown sequencer step %d!\n",cpu->sequencer_step);
				exit(1);
			}
		}
		
		
		// reset the cycles
		cpu->sequencer_cycles = 0;
	}
	
	
	
	// handle audio output 
	
	if(!--cpu->down_sample_cnt)
	{
		
		//printf("Filling audio queue! %x\n",cpu->audio_buf_idx);
		
		cpu->down_sample_cnt = 95; // may need adjusting for m cycles 
		
		
		
		float bufferin0 = 0;
		float bufferin1 = 0;
		
		// left output
		int volume = (128 * (cpu->io[IO_NR50] & 0x3) ) / 7;
		// just mix wave for now 
	
		//if(is_set(cpu->io[IO_NR50],3))
		{
			if(is_set(cpu->io[IO_NR52],2) && is_set(cpu->io[IO_NR51],2))
			{
				bufferin1 = wave_vol / 100;
				printf("%f\n",bufferin1);
				SDL_MixAudioFormat((Uint8*)&bufferin0,(Uint8*)&bufferin1,AUDIO_F32SYS,sizeof(float),volume);
				cpu->audio_buf[cpu->audio_buf_idx++] = bufferin0;
			}
			
			
			
		}
		
		
		// right output
		volume = 128 * (cpu->io[IO_NR50 & 122] >> 5) / 7;
		printf("vol: %d\n",volume);
		
		//printf("%x\n",cpu->io[IO_NR50]);
		//if(is_set(cpu->io[IO_NR50],7))
		{
			if(is_set(cpu->io[IO_NR52],2) && is_set(cpu->io[IO_NR51],6))
			{
				bufferin1 = wave_vol / 100;
				printf("%f\n",bufferin1);
				//bufferin1 = 0.5f;
				SDL_MixAudioFormat((Uint8*)&bufferin0,(Uint8*)&bufferin1,AUDIO_F32SYS,sizeof(float),volume);
				cpu->audio_buf[cpu->audio_buf_idx++] = bufferin0;
			}
			
			
				
		}
	}
	
	if(cpu->audio_buf_idx >= 1024) // dont know what function to call to actually play said buffer... 
	{
		//printf("Playing audio! :P\n");
		cpu->audio_buf_idx = 0;
		
		static const SDL_AudioDeviceID dev = 55;

		// delay execution and let the que drain to approx a frame
		while(SDL_GetQueuedAudioSize(dev) > (1024 *  sizeof(float)))
		{
			printf("%d\n",SDL_GetQueuedAudioSize(dev)); // somehow the audio size keeps fluctuating when it should just go down
			SDL_Delay(100);
		}
		
		//printf("Playing audio! :D\n");
		//exit(1);
		SDL_QueueAudio(dev,cpu->audio_buf,1024*sizeof(float));
	}
	
	
}