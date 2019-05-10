#pragma once
#include "cpu.h"

void tick_apu(Cpu *cpu, int cycles);
void tick_lengthc(Cpu *cpu);
void do_freqsweep(Cpu *cpu);
uint16_t calc_freqsweep(Cpu *cpu);
uint16_t get_wave_freq(Cpu *cpu);

const uint8_t duty[] = 
{
    0,0,0,0,0,0,0,1,    // 12.5
    1,0,0,0,0,0,0,1,   // 25
    1,0,0,0,0,1,1,1,   // 50 
	0,1,1,1,1,1,1,0    // 75
};