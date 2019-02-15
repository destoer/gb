#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

uint16_t load_word(uint16_t pc, uint8_t *mem) // <--- a word in this case is 16 bytes
{
	if(pc + 1 > 0xffff) { puts("invalid load_word"); return 0xff; } // best we can do for return only used to debug anyways
	uint16_t operand = mem[pc+1] & 0xff; // only get byte
	operand =  (operand>>8) | (operand<<8); // swap lo and ho
	operand += mem[pc] & 0xff; // only get byte
	return operand;
}

uint8_t set_bit(uint8_t num,uint8_t bit)
{
	return ((num) | (1 << bit));
}



uint8_t deset_bit(uint8_t num,uint8_t bit)
{
	return ((num) & ~(1 << bit));
}


bool is_set(uint8_t reg, uint8_t bit)
{
	if((reg >> bit) & 1)
	{
		return true;
	}
	
	else
	{
		return false;
	}
}



uint8_t val_bit(uint8_t data, int position)
{
	uint8_t mask = 1 << position ;
	return ( data & mask ) ? 1 : 0 ;
}
