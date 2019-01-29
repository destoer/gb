#ifdef DEBUG
#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/disass.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define COMMANDS 6

/* todo add memory range printing */


const int lens[] =
{

    1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
    1,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
    2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
    2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,3,3,3,1,2,1,1,1,3,0,3,3,2,1,
	1,1,3,0,3,1,2,1,1,1,3,0,3,0,2,1,
    2,1,1,0,0,1,2,1,2,1,3,0,0,0,2,1,
    2,1,1,1,0,1,2,1,2,1,3,1,0,0,2,1
};	

// 1st word of commands
const char c1[COMMANDS][7] = 
{
	"break",
	"info",
	"disass",
	"write",
	"step",
	"run"
};

void step(Cpu* cpu);
void info(char *token, Cpu *cpu);
void breakpoint(char *token, Cpu* cpu);
void disass_addr(char *token, Cpu* cpu);
void write_addr(char *token, Cpu* cpu);
// input func for debugger
// will just  take commands and call other funcs to execute them
void enter_debugger(Cpu *cpu)
{
	void (*f[4])(char *, Cpu *) = {breakpoint,info,disass_addr,write_addr};
	
	char input[41] = {0};
	bool quit = false;
	char *token = NULL;
	while(!quit)
	{
		// read from input
		printf("? ");
		memset(input,0,40);
		token = NULL;
		fgets(input,39,stdin);
		fflush(stdin);
		input[strcspn(input,"\n")] = '\0';	// remove the \n from fgets
	
		// tokenize the input and search first word in command list
		// if needed get a second token and parse it to find the correct
		// command to call 
		token = strtok(input," ");
		
		if(token == NULL)
		{
			puts("[ERROR] invalid input");
			continue;
		}
		
		// search the list of command strings
		// and get index of the command 
		int command_num = -1;
		for(int i = 0; i < COMMANDS; i++)
		{
			//printf("%s : %s\n",token,c1[i]);
			if(0 == strcmp(token,c1[i]))
			{
				command_num = i;
				break;
			}
		}
		
		//printf("%d : %s\n",command_num,c1[command_num]);
		
		// call the selected function
		if(command_num < COMMANDS-1 && command_num >= 0)
		{
			if(command_num == 4)
			{ 
				quit = true; // step should  break immediatly
				cpu->step = true;
			}

			else
			{
				(*f[command_num])(token,cpu);
			}
		}
		
		
		// user typed run exit out of the debugger
		else if(command_num == COMMANDS-1)
		{
			quit = true;
		}
		
		// type something unknown just continue the loop
		else
		{
			printf("[ERROR] unknown command %s\n",token);
		}
	}
	//puts("resuming execution...");
}


void write_addr(char *token, Cpu* cpu)
{
	// read first token to get the address
	// we expect another for the value to write
	// if two just break the address
	// if 3 break the mem read or execute
	token = strtok(NULL," ");
	if(token == NULL)
	{
		puts("[ERROR] no address for write");
		return;
	}

	if(token[0] == '*') token += 1;

	int addr = strtol(token,NULL,16);


	token = strtok(NULL," ");
	if(token == NULL)
	{
		puts("[ERROR] no value for write");
		return;
	}


	int val = strtol(token,NULL,16);

	// write to ram etc
	if(addr >= 0x8000)
	{
		write_mem(cpu,addr,val);
	}

	if(addr >= 0x4000 && addr < 0x8000)
	{
		// write to rom we will bypass checks
		int16_t new_address = addr - 0x4000;
		cpu->rom_mem[new_address + (cpu->currentrom_bank*0x4000)] = val;
	}

	else
	{
		cpu->mem[addr] = val;
	}

}

void disass_addr(char *token, Cpu* cpu)
{
	// read first token to get the address
	// right now we are expecting one or two more tokens
	// if two just break the address
	// if 3 break the mem read or execute
	token = strtok(NULL," ");
	uint16_t pc_backup = cpu->pc;
	if(token == NULL)
	{
		puts("[ERROR] no address for disass");
		return;
	}

	if(token[0] == '*') token += 1;

	int address = strtol(token,NULL,16);
	cpu->pc = address;
	
	// get the next token if there isnt one just disas the address
	// else get the number and disass that many instructions from the address
	
	token = strtok(NULL," ");
	
	// just disass it
	if(token == NULL)
	{
		disass_8080(read_mem(cpu->pc++,cpu),cpu);
	}
	
	else
	{
		int num = atoi(token); // this is in base 10
	
		for(int i = 0; i < num; i++)
		{
			uint8_t opcode = read_mem(cpu->pc++,cpu);
			disass_8080(opcode,cpu);	
			cpu->pc += lens[opcode]-1;
		}
	}
	
	cpu->pc = pc_backup;
}

void breakpoint(char *token, Cpu* cpu)
{
	
	// right now we are expecting one or two more tokens
	// if two just break the address
	// if 3 break the mem read or execute
	token = strtok(NULL," ");

	if(token == NULL)
	{
		puts("[ERROR] Invalid breakpoint");
		return;
	}
	
	//printf("Token: %s\n",token);
	if(token[0] == '*') // address just use break the pc
	{
		token += 1; // skip *
		cpu->breakpoint = strtol(token,NULL,16);
		if(cpu->breakpoint > 0xffff)
		{
			puts("[ERROR] out of range breakpoint");
			return;
		}
		
		printf("breakpoint placed at 0x%x\n",cpu->breakpoint);
		return;
	}
	
	
	// else we may specifying a complex breakpoint
	// parse the arg and figure out what we are breaking
	// then get the next one to find the address
	else
	{
		bool wb = false; // write breakpoint
		bool rb = false; // read breakpoint
		bool xb = false; // execute breakpoint
		
		int len = strlen(token);
		
		if(len > 3)
		{
			len = 3; // can only pass max of 3
		}
		
		for(int i = 0; i < len; i++)
		{
			if(token[i] == 'r') rb = true;
			if(token[i] == 'w') wb = true;
			if(token[i] == 'x') xb = true;
		}
		
		
		// finally do the same as above and parse out the address
		token = strtok(NULL," ");
		if(token == NULL)
		{
			puts("[ERROR] Missing address for breakpoint");
			return;
		}
		
		if(token[0] == '*') token += 1; // skip *
		
		// convert token to breakpoint
		int breakpoint = strtol(token,NULL,16);
		
		if(breakpoint > 0xffff)
		{
			puts("[ERROR] out of range breakpoint");
			return;
		}
		
		printf("breakpoint placed at 0x%x\n",breakpoint);

		
		// set the breakpoints
		if(wb) cpu->memw_breakpoint = breakpoint;
		if(rb) cpu->memr_breakpoint = breakpoint;
		if(xb) cpu->breakpoint = breakpoint;
	
	}
}

void info(char *token, Cpu *cpu)
{
	//puts("called info");
	
	// parse arg see if its mem regs or control
	// and print info 
			
	token = strtok(NULL," ");
	if(token == NULL)
	{
		puts("[ERROR] Missing Operand for info");
		return;
	}
	
	// check if we wanna print whats at a mem address
	if(token[0] == '*')
	{
		token += 1;
		int address = strtol(token,NULL,16);
		
		if(address > 0xffff)
		{
			puts("[ERROR] out of range address");
			return;
		}
		
		printf("cpu->mem[%x] = (byte)%x, (word)%x\n",address,read_mem(address,cpu),read_word(address,cpu));
		return;
	}
	
	// print registers and flags
	else if(0 == strcmp(token,"regs"))
	{
		cpu_state(cpu);
		print_flags(cpu);
		return;
	}
	
	// print various control registers
	else if(0 == strcmp(token,"control"))
	{
		//printf("ly = %x\n",cpu->mem[0xff44]);
		printf("ly = %x\n",read_mem(0xff44,cpu));
		printf("div = %x\n",cpu->mem[DIV]);
		printf("tima = %x\n",cpu->mem[TIMA]);
		printf("lcdc = %x\n",cpu->mem[0xff40]);
		printf("if = %x\n",cpu->mem[0xff0f]);
		printf("ime = %x\n",cpu->interrupt_enable);
		printf("ie = %x\n",cpu->mem[0xffff]);
		printf("stat = %x\n",cpu->mem[0xff41]); // lcd stat
		printf("rom_bank = %x\n",cpu->currentrom_bank);
		printf("ram_bank = %x\n",cpu->currentram_bank);
		printf("ram_enable = %x\n",cpu->enable_ram);
		return;
	}
	
	// print the contents at the stack pointer
	else if(0 == strcmp(token,"stack"))
	{
		printf("stack: %x [%x]%x [%x]%x\n",read_word(cpu->sp+1,cpu),cpu->sp,read_mem(cpu->sp,cpu),cpu->sp+1,read_mem(cpu->sp+1,cpu));
		return;
	}
	
	else
	{
		printf("[ERROR] unrecognized param %s\n",token);
	}
}


// do other things in here later
void step(Cpu* cpu)
{
	cpu->step = true;
	//puts("Break set for next instruction");
}
#endif
