#include <stdint.h>
#include <stdbool.h>

uint16_t load_word(int pc, char *mem) // <--- a word in this case is 16 bytes
{
	uint16_t operand = mem[pc+1] & 0xff; // only get byte
	operand =  (operand>>8) | (operand<<8); // swap lo and ho
	operand += mem[pc] & 0xff; // only get byte
	return operand;
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
