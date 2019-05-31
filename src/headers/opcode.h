#pragma once

#include "lib.h"
#include "cpu.h"
#include <stdint.h>
#include <stdbool.h>
void step_cpu(Cpu * cpu);
void write_log(Cpu *cpu,const char *fmt, ...);