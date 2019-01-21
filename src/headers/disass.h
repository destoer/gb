#pragma once
#ifdef DEBUG
void cpu_state(const Cpu *cpu); // print the cpu state
void disass_8080(const uint8_t opcode, const Cpu *cpu);
void print_flags(const Cpu *cpu);
#endif
