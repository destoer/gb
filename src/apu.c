
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
	
	if(is_set(cpu->io[IO_NR52],2))
	{
		cpu->wave_period -= cycles;
		
		// reload timer and goto the next sample in the wave table
		if(cpu->wave_period <= 0)
		{
			if(cpu->wave_nibble == 0) // goto the 2nd nibble
			{
				cpu->wave_nibble = 1;
			}
			
			
			// goto next sample
			else 
			{
				cpu->wave_idx++;
				// loop back around the index
				if(cpu->wave_idx == 16)
				{
					cpu->wave_idx = 0;
				}
				cpu->wave_nibble = 0; // reset the nibble
			}

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
				printf("Error unknown step %d!\n",cpu->sequencer_step);
				exit(1);
			}
		}
		
		
		// reset the cycles
		cpu->sequencer_cycles = 0;
	}
}