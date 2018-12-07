// for debugging purposes remove in final build
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/disass.h"

// disassembly routine should never modify passed args
void disass_8080(const uint8_t opcode, const Cpu *cpu)
{
	
	uint8_t op;
	// disass the opcode
	switch(opcode)
	{
		case 0x0: // NOP
			puts("nop");
			break;
		
		case 0x4: // inc b 
			puts("inc b");
			break;
		
		case 0x5: // dec b
			puts("dec b");
			break;
			
		case 0x6: // ld b, n
			printf("ld b, %x\n",cpu->mem[cpu->pc]);
			break;
		
		

		case 0xc: // inc c
			puts("inc c");
			break;
		
		case 0xd:
			puts("dec c");
			break;
			
		case 0xe: // ld c,n
			printf("ld c, %02x\n",cpu->mem[cpu->pc]);
			break;
			
	

		
		case 0xa: // ld a, (de)
			puts("ld a, (de)");
			break;
			
		case 0x11: // ld de, nn
			printf("ld de, %04x\n",load_word(cpu->pc,cpu->mem));
			break;	
		
		case 0x13: // inc de
			puts("inc de");
			break;
		
		case 0x15: // dec d
			puts("dec d");
			break;
		
		case 0x17: // rla
			puts("rla");
			break;
		
		case 0x18: // n 
			printf("jr %02x\n",(cpu->pc+1 + (int8_t)cpu->mem[cpu->pc]));
			break;
		
		case 0x1a: // ld a, (de)
			puts("ld a, (de)");
			break;
			
		case 0x1D: // dec e
			puts("dec e");
			break;
					
		case 0x1e: // ld e, n
			printf("ld b, %02x\n",cpu->mem[cpu->pc]);
			break;
			
		case 0x20: // jr nz, n
			printf("jr nz, %02x\n",(cpu->pc+1 + (int8_t)cpu->mem[cpu->pc]));
			break;
			
		case 0x21: // ld hl, nn
			printf("ld hl, %04x\n",load_word(cpu->pc,cpu->mem));
			break;
		
		case 0x22: // ldi (hl), a
			puts("ldi (hl) ,a");
			break;
		
		case 0x23: // inc hl
			puts("inc hl");
			break;
		
		case 0x24: // inc h
			puts("inc h");
			break;
		
		case 0x28: // jr z, n
			printf("jr z, %02x\n",(cpu->pc+1 + (int8_t)cpu->mem[cpu->pc]));
			break;
		
		case 0x2e: // ld l,n
			printf("ld l, %02x\n",cpu->mem[cpu->pc]);
			break;
		
		case 0x31: // ld sp, nn
			printf("ld sp, %04x\n",load_word(cpu->pc,cpu->mem));		
			break;
			
		case 0x32: // ldd (hl),a
			puts("ldd (hl),a");
			break;
			
		case 0x3d: // dec a
			puts("dec a");
			break;
			
		case 0x3e: // ld a, nn 
			printf("ld a, %04x\n",cpu->mem[cpu->pc]);
			break;
		

		
		case 0x4f: // ld c,a
			puts("ld c, a");
			break;
		
		case 0x57:  //ld d, a
			puts("ld d, a");
			break;
		
		case 0x67: // ld h,a
			puts("ld h, a");
			break;
		
		

		case 0x77: // ld (hl),a 
			puts("ld (hl),a");
			break;
		
		case 0x7b: // ld a,e
			puts("ld a, e");
			break;
		
		case 0x7c: // ld a,h
			puts("ld a, h");
			break;
		
		
		
		case 0x90: // sub a, b (a is implict)
			puts("sub b");
			break;
		
		case 0xaf:
			printf("xor a\n");
			break;	
		

		case 0xc1: // pop bc 
			puts("pop bc");
			break;
		
		case 0xc3: // jump
			printf("jp %04X\n",load_word(cpu->pc,cpu->mem));
			break;
		
		
		case 0xc5: // push bc
			puts("push bc");
			break;
		
		case 0xc9: // ret
			puts("ret");
			break;
		
		case 0xCB: // extended opcode 
			op = cpu->mem[cpu->pc];
			
			switch(op)
			{
				
				case 0x11: // rl c
					puts("rl c");
					break;
				
				case 0x7c:
					puts("bit 7, h");
					break;
				
				default:
					fprintf(stderr, "[disass] Unknown CB opcode: %x\n", op);
					cpu_state(cpu);
					exit(1);
			}
			break;
			
		case 0xCD: // call nn 
			printf("call %04x\n",load_word(cpu->pc,cpu->mem));
			break;
		
		case 0xE0: // LD(FFF0+N),a
			printf("ld (ff00+%02x),a\n",cpu->mem[cpu->pc]);
			break;
		
		case 0xE2:
			puts("ld (ff00+c),a");
			break;
		
		
		case 0xea: // ld (nnnn), a 
			printf("ld (%04x), a\n",load_word(cpu->pc,cpu->mem));
			break;
		
		case 0xf0: // ld a, (ff00+n)
			printf("ld a, (ff00+%02x)\n",cpu->mem[cpu->pc]);
			break;
		
		case 0xfe: // cp a, n (cmp)
			printf("cp a, %02x\n",cpu->mem[cpu->pc]);
			break;
		
		default:
			fprintf(stderr, "[Disass] Unknown opcode: %x\n", opcode);
			cpu_state(cpu);
			exit(1);	
	}
}


// print the current cpu state
void cpu_state(const Cpu *cpu)
{
	printf("PC %04x: AF:%04x BC:%04x DE:%04x HL:%04x SP:%04x\n",
		cpu->pc,cpu->af.reg, cpu->bc.reg, cpu->de.reg, cpu->hl.reg, cpu->sp.reg);	
}


void print_flags(const Cpu *cpu)
{
	
	short temp; // temp variable for bit minipualted flags
	// 1 for true
	// 0 for false
	printf("\nFlags:\n");	
	// carry flag
	temp = cpu->af.lb & 16; // 16 4 bit on
	if(temp == 16) // if bit is on
	{
		printf("Carry flag = 1\n");
	}
	else {
		printf("Carry flag = 0\n");
	}
	// half carry flag
	temp = cpu->af.lb & 32; // 5 bit is on 
	if(temp == 32) // if bit is on
	{
		printf("Half carry flag = 1\n");
	}
	else {
		printf("Half carry flag = 0\n");
	}
	// subtraction flag
	temp = cpu->af.lb & 64; // 6 bit is on
	if(temp == 64) // if bit is on
	{
		printf("Subtraction flag = 1\n");
	}
	else {
		printf("Subtraction flag = 0\n");
	}
	// zero flag
	temp = cpu->af.lb & 128; // 7 bit is on
	if(temp == 128) // if bit is on
	{
		printf("Zero flag = 1\n");
	}
	else {
		printf("Zero flag = 0\n");
	}
	printf("\n");
	
}