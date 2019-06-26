#pragma once
#include "cpu.h"

void tick_apu(Cpu *cpu,int cycles, int end);
void tick_lengthc(Cpu *cpu);
void do_freqsweep(Cpu *cpu);
uint16_t calc_freqsweep(Cpu *cpu);
uint16_t get_wave_freq(Cpu *cpu);
void advance_sequencer(Cpu *cpu);


static const uint8_t duty[4][8] = 
{
    {0,0,0,0,0,0,0,1},    // 12.5
    {1,0,0,0,0,0,0,1},   // 25
    {1,0,0,0,0,1,1,1},   // 50 
    {0,1,1,1,1,1,1,0}    // 75
};

//http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Noise_Channel
static const int divisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
