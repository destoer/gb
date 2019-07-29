#pragma once
#include "cpu.h"

void handle_input(Cpu *cpu);
void init_sdl(Cpu *cpu);
void load_save(Cpu *cpu);
void handle_connection(int argc, char *argv[], Cpu *cpu);
