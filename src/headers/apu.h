#pragma once
#include "cpu.h"

void tick_apu(Cpu *cpu, int cycles);
void tick_lengthc(Cpu *cpu);
void do_freqsweep(Cpu *cpu);
uint16_t calc_freqsweep(Cpu *cpu);