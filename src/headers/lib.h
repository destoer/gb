#pragma once

// cpu clock speed
#define CLOCKSPEED 419430

// timers
#define TIMA 0xFF05
#define TMA 0xFF06
#define TMC 0xFF07 
#define DIV 0XFF04

// gb is 160x144 pixels
#define X 160
#define Y 144

// read a word from memory (bypasses the read_memory function)
// only use when memory reads dont have side affects 
uint16_t load_word(int pc, char *mem);


// set a bit 
#define set_bit(dest,bit) ((dest) |= (1 << bit))

// deset (that is set to zero a bit)
#define deset_bit(dest,bit) ((dest) &= ~(1 << bit))

bool is_set(uint8_t reg, uint8_t bit);
uint8_t val_bit(uint8_t data, int position);