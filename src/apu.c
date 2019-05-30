
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "headers/cpu.h"
#include "headers/apu.h"


// done in terms of T cycles

// need to pass the final two tests
// square one sounds too high pitched (pokemon red intro star (and lasts too long) )
// likely envelope or the freq sweep (more likely) not working as intended

// also when it stays on the frequency does not change (sweep is off but its not disabled)


/* Handle the channel dac

Channel DAC

Each channel has a 4-bit digital-to-analog convertor (DAC). This converts the input value to a proportional output voltage. An input of 0 generates -1.0 and an input of 15 generates +1.0, using arbitrary voltage units.

DAC power is controlled by the upper 5 bits of NRx2 (top bit of NR30 for wave channel). If these bits are not all clear, the DAC is on, otherwise it's off and outputs 0 volts. Also, any time the DAC is off the channel is kept disabled (but turning the DAC back on does NOT enable the channel)

*/


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





// performs the frequency sweep
// calc + overflow check

uint16_t calc_freqsweep(Cpu *cpu)
{
	
	uint16_t shadow_shifted = cpu->sweep_shadow >> (cpu->io[IO_NR10] &  0x7);
	uint16_t result;
	if(is_set(cpu->io[IO_NR10],3)) // test bit 3 of nr10 if its 1 take them away
	{
		cpu->sweep_calced = true; // calc done using negation
		result = cpu->sweep_shadow - shadow_shifted;
	}
	
	// else add them
	else
	{
		result = cpu->sweep_shadow + shadow_shifted;
	}

	// result is greater than 0x7ff (disable the channel)
	if(result > 0x7ff)
	{
		cpu->sweep_timer = 0;
		deset_bit(cpu->io[IO_NR52],0);
		cpu->sweep_enabled = false;	
	}

	return result;
}


// calc the freq sweep and do the overflow checking
// disable chan if it overflows
void do_freqsweep(Cpu *cpu)
{
	// dont calc the freqsweep if sweep peroid is zero or its disabled
	if(!cpu->sweep_enabled || !cpu->sweep_period ) { return; }
	
	uint16_t temp = calc_freqsweep(cpu);
	
	// sweep shift not zero 
	if( (cpu->io[IO_NR10] &  0x7) && temp <= 0x7ff)
	{
		// write back to shadow
		cpu->sweep_shadow = temp;
		
		// write back low 8
		cpu->io[IO_NR13] = cpu->sweep_shadow & 0xff;

		
		// and high 3
		cpu->io[IO_NR14] &= ~0x7; // mask bottom 3
		cpu->io[IO_NR14] |= (cpu->sweep_shadow >> 8) & 0x7; // and write them out
		
		// also back to our internal cached freq
		cpu->square[0].freq = cpu->sweep_shadow;		

		// reperform the calc + overflow check (but dont write back)
		calc_freqsweep(cpu);		
	}
}


// clock sweep generator for square 1
void clock_sweep(Cpu *cpu)
{
	//if(!is_set(cpu->io[IO_NR52],0)) return;
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
		if(cpu->sweep_period == 0)
		{ 
			cpu->sweep_timer = 8;  
		}		
	}
	
}


void clock_envelope_channel(Cpu *cpu, int idx)
{
	int vol = cpu->square[idx].volume;
	if(!--cpu->square[idx].env_period && cpu->square[idx].env_enabled)
	{
		int addr;
		switch(idx) // find the reg we are reading to find if sweep is up or down
		{
			case 0: addr = IO_NR12; break;
			case 1: addr = IO_NR22; break;
			case 3: addr = IO_NR42; break;
			default:
			{
				puts("invalid envelope sweep up!");
				exit(1);
			}
		}
		
		if(is_set(cpu->io[addr],3)) // sweep up
		{
			vol += 1;
		}

		else // sweep down
		{
			vol -= 1;
		}

		if(vol >= 0 && vol <= 15) // if vol is between 0 and 15 it is updated
		{
			cpu->square[idx].volume = vol;
		}

		else
		{
			cpu->square[idx].env_enabled = false;
		}

		// reload the period
		if(!cpu->square[idx].env_load) // timers treat period of zero as 8
		{
			cpu->square[idx].env_period = 8; 
		}

		else
		{
			cpu->square[idx].env_period = cpu->square[idx].env_load;
		}
	}	
}

void clock_envelope(Cpu *cpu)
{
	clock_envelope_channel(cpu,0);
	clock_envelope_channel(cpu,1);
	clock_envelope_channel(cpu,3);
}


// it appears the sequencer is clocked by the internal div register
// http://gbdev.gg8.se/wiki/articles/Timer_Obscure_Behaviour
// find out how later (i think its when bit 12 falls)
// or (13 in double speed mode)
void advance_sequencer(Cpu *cpu)
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
			tick_lengthc(cpu);
			clock_sweep(cpu);
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
			tick_lengthc(cpu);
			clock_sweep(cpu);
			break;
		}
			
		case 7:
		{ 
			clock_envelope(cpu);
			break; //clock the envelope 
		}
		default:
		{
			printf("Error unknown sequencer step %d!\n",cpu->sequencer_step);
			exit(1);
		}
	}	
}

// ticks the channel period for channels
// 1 AND 2 ONLY 
void tick_period_square(Cpu *cpu, int idx, int cycles)
{
	cpu->square[idx].period -= cycles;
	// idx 0 use nr12 channel 1 use nr12
	int addr = idx ? IO_NR22 : IO_NR12; 
	if(cpu->square[idx].period <= 0)
	{
		// advance the duty
		cpu->square[idx].duty_idx = (cpu->square[idx].duty_idx + 1) & 0x7;
		cpu->square[idx].period = ((2048 - cpu->square[idx].freq)) *4; 

		// if channel and dac is enabled
		if(is_set(cpu->io[IO_NR52],idx) && (cpu->io[addr] & 248))
		{
			cpu->square[idx].output = cpu->square[idx].volume;
		}
		
		else
		{
			cpu->square[idx].output = 0;
		}		

		// if the duty is on a low posistion there is no output
		// (vol is multiplied by duty but its only on or off)
		if(!duty[cpu->square[idx].duty][cpu->square[idx].duty_idx])
		{
			cpu->square[idx].output = 0;
		}
	}	
}


void tick_apu(Cpu *cpu, int cycles)
{
	// apu is off do nothing
	if(!is_set(cpu->io[IO_NR52],7)) return;


	// so every 8192 cycles the sequencer will increment
	// when it does every other step it will clock our length counters 
	// we will then use a switch and determine what to do 
	// could also use function pointers but this is likely better
	
	// first we need to tick the frame sequencer
	cpu->sequencer_cycles += cycles;
	
	// now if 8192 T cycles (2048 M cycles) have elapsed we need to inc the step
	// and perform the action for the current step
	
	if(cpu->sequencer_cycles >= 8192)
	{
		advance_sequencer(cpu);
			
		// reset the cycles
		cpu->sequencer_cycles = 0;
	}




	// tick the output of all the channels
	

	// refactor to use a function for first two squares

	// square 1
	tick_period_square(cpu,0,cycles);


	// square 2
	tick_period_square(cpu,1,cycles);

	
	
	// handle wave ticking (square 3)	
	cpu->square[2].period -= cycles;
		
	// reload timer and goto the next sample in the wave table
	if(cpu->square[2].period <= 0)
	{
		// duty is the wave table index for wave channel 
		cpu->square[2].duty_idx  = (cpu->square[2].duty_idx + 1) & 0x1f; 

		// dac is enabled
		if(is_set(cpu->io[IO_NR30],7) && is_set(cpu->io[IO_NR52],2))
		{
			int pos = cpu->square[2].duty_idx / 2;
			uint8_t byte = cpu->io[0x30 + pos];
				
			if(!is_set(cpu->square[2].duty_idx,0)) // access the high nibble first
			{
				byte >>= 4;
			}
				
			byte &= 0xf;
				
			if(cpu->square[2].volume)
			{
				byte >>= cpu->square[2].volume - 1;
			}
				
			else
			{
				byte = 0;
			}
			cpu->square[2].output = byte;
			
		}
			
		else
		{ 
			cpu->square[2].output = 0;
		}	
			
		// reload the timer
		// period (2048-frequency)*2 (in cpu cycles)
		cpu->square[2].period = ((2048 - cpu->square[2].freq)*2);				
	}	


	// NOISE <-- does not sound correct in places
	// square 4
	cpu->square[3].period -= cycles; // polynomial counter

	if(cpu->square[3].period <= 0)
	{
		// "The noise channel's frequency timer period is set by a base divisor shifted left some number of bits. "
		cpu->square[3].period = (divisors[cpu->divisor_idx] << cpu->clock_shift); 

		// bottom two bits xored and reg shifted right
		int result = cpu->shift_reg & 0x1;
		cpu->shift_reg >>= 1;
		result ^= cpu->shift_reg & 0x1;

		// result placed in high bit (15 bit reg)
		cpu->shift_reg |=  (result << 14);

		if(cpu->counter_width) // in width mode
		{
			// also put result in bit 6
			deset_bit(cpu->shift_reg,6);
			cpu->shift_reg |= result << 6;
		} 

		// if lsb NOT SET
		// put output
		if(is_set(cpu->io[IO_NR52],3) && (cpu->io[IO_NR42] & 248) && !is_set(cpu->shift_reg,0))
		{

			cpu->square[3].output = cpu->square[3].volume;
		}
	
		else 
		{
			cpu->square[3].output = 0;
		}

	}


	
	
	
	
	// handle audio output 
#ifdef SOUND	
	if(!--cpu->down_sample_cnt)
	{
		
		//printf("Filling audio queue! %x\n",cpu->audio_buf_idx);
		
		// while our counts are in T cycles the update function is called 
		// ever M cycle so its 23
		cpu->down_sample_cnt = 23; // may need adjusting for m cycles (95)
		
		if(cpu->is_double) // if in double speed the function is called twice as often 
		{					// so to adjust the down sample count must be double
			cpu->down_sample_cnt *= 2;
		}

		
		float bufferin0 = 0;
		float bufferin1 = 0;
		
		// left output
		int volume = (128 *(cpu->io[IO_NR50] & 7)) / 7 ;
		// just mix wave for now 
	
		for(int idx = 0; idx < 4; idx++)
		{
			// is square enabled?
			if(is_set(cpu->io[IO_NR51],idx))
			{
				bufferin1 = cpu->square[idx].output / 100;
				SDL_MixAudioFormat((Uint8*)&bufferin0,(Uint8*)&bufferin1,AUDIO_F32SYS,sizeof(float),volume);
			}			
		}
		cpu->audio_buf[cpu->audio_buf_idx] = bufferin0;
		
		
		// right output
		// more correct but sounds weird with different volumes......
		//volume = (128 * (cpu->io[IO_NR50 >> 4] & 7)) / 7;
		//printf("vol: %d\n",volume);
		bufferin0 = 0;
	
		for(int idx = 0; idx < 4; idx++)
		{
			// is square enabled?
			if(is_set(cpu->io[IO_NR51],idx+4))
			{
				bufferin1 = cpu->square[idx].output / 100;
				SDL_MixAudioFormat((Uint8*)&bufferin0,(Uint8*)&bufferin1,AUDIO_F32SYS,sizeof(float),volume);
			}			
		}
		cpu->audio_buf[cpu->audio_buf_idx+1] = bufferin0;
		cpu->audio_buf_idx += 2;	
	}
	
	if(cpu->audio_buf_idx >= SAMPLE_SIZE)
	{
		cpu->audio_buf_idx = 0;
		
		//static const SDL_AudioDeviceID dev = 2; // should get from the cpu function but allow it for now when it doesent even work
		
		// legacy interface
		static const SDL_AudioDeviceID dev = 1;
		

#ifdef DEBUG
		// dont sync sound if we are overclocking
		// should ideally sample less often when speedup is active		
		if(!cpu->speed_up) 
		{
			// delay execution and let the que drain
			// on newer builds of sdl this locks up...
			while(SDL_GetQueuedAudioSize(dev) > (SAMPLE_SIZE * sizeof(float)))
			{ 
				SDL_Delay(1);
			}
		}
		
		if(!cpu->speed_up)
		{
			if(SDL_QueueAudio(dev,cpu->audio_buf,SAMPLE_SIZE * sizeof(float)) < 0)
			{
				printf("%s\n",SDL_GetError()); exit(1);
			}
		}
#else
		// delay execution and let the que drain
		while(SDL_GetQueuedAudioSize(dev) > (SAMPLE_SIZE * sizeof(float)))
		{
			SDL_Delay(1);
		}			

		
	
		
		if(SDL_QueueAudio(dev,cpu->audio_buf,SAMPLE_SIZE * sizeof(float)) < 0)
		{
			printf("%s\n",SDL_GetError()); exit(1);
		}

#endif		
		//fwrite(cpu->audio_buf,sizeof(float),SAMPLE_SIZE,cpu->fp);	
	}
#endif	
}
