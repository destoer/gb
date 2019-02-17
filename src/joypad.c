#include "headers/lib.h"
#include "headers/cpu.h"
#include <stdint.h>
#include <stdbool.h>


void key_released(int key,Cpu *cpu)
{
	set_bit(cpu->joypad_state, key);
}


void key_pressed(int key, Cpu *cpu)
{
	bool previously_unset = false;
	
	// if setting from 1 to 0 we may have to req 
	// and interrupt
	if(!is_set(cpu->joypad_state,key))
	{
		previously_unset = true;
	}
	
	// remember if a key is pressed its bit is 0 not 1
	deset_bit(cpu->joypad_state, key);
	
	// button pressed
	bool button = true;
	
	
	// is this a standard button or a directional one?
	if(key > 3)
	{
		button = true; // <--- kinda redundant
	}
	else
	{
		button = false; // directional one pressed
	}
	
	uint8_t key_req = cpu->io[0x00];
	bool req_int = false;
	
	// only request an interrupt if the button just pressed
	// is the style of button the game is intereted in
	if(button && !is_set(key_req,5))
	{
		req_int = true;
	}
	
	// same but for direcitonals
	
	else if(!button && !is_set(key_req,4))
	{
		req_int = true;
	}
	
	// req an int
	if(req_int && !previously_unset)
	{
		request_interrupt(cpu,4);
	}
}
