#pragma once

#include "lib.h"
#include "cpu.h"
#include <stdint.h>
#include <stdbool.h>
void step_cpu(Cpu * cpu);
void handle_instr_effects(Cpu *cpu);