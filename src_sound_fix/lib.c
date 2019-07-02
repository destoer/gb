#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


uint16_t load_word(uint16_t pc, uint8_t *mem) // <--- a word in this case is 16 bytes
{
	if(pc + 1 > 0xffff) { puts("invalid load_word"); return 0xff; } // best we can do for return only used to debug anyways
	uint16_t operand = mem[pc+1] & 0xff; // only get byte
	operand =  (operand>>8) | (operand<<8); // swap lo and ho
	operand += mem[pc] & 0xff; // only get byte
	return operand;
}



bool is_set(int reg, int bit)
{
	return ((reg >> bit) & 1);
}



uint8_t val_bit(uint8_t data, int position)
{
	uint8_t mask = 1 << position ;
	return ( data & mask ) ? 1 : 0 ;
}


void binary(int number)
{
	int counter = 1;
	int mask = 128;
	while(counter <= 8)
	{
		putchar(number & mask? '1' : '0');
		number = number << 1;
		if(counter % 8 == 0)
		{
			putchar(' ');
		}
		counter++;
	}
}