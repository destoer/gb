// for debugging purposes remove in final build
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/disass.h"
#include "headers/memory.h"

#ifdef DEBUG
// disassembly routine should never modify passed args
int disass_8080(Cpu *cpu,const uint16_t addr)
{
	uint8_t opcode = read_mem(addr,cpu); // fetch opcode
	uint16_t address = addr + 1; // address for operand reads
	uint8_t op; // cb opcode
	uint8_t operand = read_mem(address,cpu); // operand
	uint16_t operandw = read_word(address,cpu); // word operand
	
	int len = lens[opcode]; 
	
	// disass the opcode
	printf("%04x: ", addr);
	switch(opcode)
	{
		case 0x0: // NOP
			puts("nop");
			break;
		
		case 0x1: // ld bc, nn
			printf("ld bc, %04x\n",operandw);
			break;
		
		case 0x2: // ld (bc), a
			puts("ld (bc), a");
			break;
		
		case 0x3: // inc bc
			puts("inc bc");
			break;
		
		case 0x4: // inc b 
			puts("inc b");
			break;
		
		case 0x5: // dec b
			puts("dec b");
			break;
			
		case 0x6: // ld b, n
			printf("ld b, %x\n",operand);
			break;
		
		case 0x7: // rlca
			puts("rlca");
			break;

		
		case 0x08: // ld (nnnn), sp
			printf("ld (%x), sp\n",operandw);
			break;
		
		case 0x9: // add hl, bc
			puts("add hl, bc");
			break;

		
		case 0xa: // ld a, (bc)
			puts("ld a, (bc)");
			break;
			
		case 0xb: // dec bc
			puts("dec bc");
			break;
			
		case 0xc: // inc c
			puts("inc c");
			break;
		
		case 0xd:
			puts("dec c");
			break;
			
		case 0xe: // ld c,n
			printf("ld c, %02x\n",operand);
			break;
			
		case 0xf: // rrca
			puts("rrca");
			break;

		case 0x10: // stop if followed by a 00
			if(read_mem(address,cpu) == 00)
			{
				puts("stop");
			}
			
			else
			{
				puts("corrupted stop");
				break;
			}
			break;

			
		case 0x11: // ld de, nn
			printf("ld de, %04x\n",operandw);
			break;	
		
		case 0x12: // ld (de),a
			puts("ld (de), a");
			break;
		
		case 0x13: // inc de
			puts("inc de");
			break;
		
		case 0x15: // dec d
			puts("dec d");
			break;
		
		case 0x14: // inc d
			puts("inc d");
			break;
		
		case 0x16: // ld d, n
			printf("ld d, %x",operand);
			break;
		
		case 0x17: // rla
			puts("rla");
			break;
		
		case 0x18: // n 
			printf("jr %02x\n",(address + (int8_t)operand));
			break;
		
		case 0x19: // add hl, de
			puts("add hl, de");
			break;
		
		case 0x1a: // ld a, (de)
			puts("ld a, (de)");
			break;
		
		case 0x1b: // dec de
			puts("dec de");
			break;
		
		case 0x1c: // inc e
			puts("inc e");
			break;
		
		case 0x1D: // dec e
			puts("dec e");
			break;
					
		case 0x1e: // ld e, n
			printf("ld e, %02x\n",operand);
			break;
		
		case 0x1f: // rra 
			puts("rra");
			break;
		
		case 0x20: // jr nz, n
			printf("jr nz, %02x\n",(address + (int8_t)operand));
			break;
			
		case 0x21: // ld hl, nn
			printf("ld hl, %04x\n",operandw);
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
		
		case 0x25: // dec h
			puts("dec h");
			break;

		case 0x26: // ld h, nn
			printf("ld h, %x\n",operand);
			break;
		
		case 0x27: // daa (decimal adjust a)
			puts("daa");
			break;
		
		case 0x28: // jr z, n
			printf("jr z, %02x\n",(address + (int8_t)operand));
			break;
		
		case 0x29: // add hl, hl 
			puts("add hl, hl");
			break;
		
		case 0x2a: // ldi a, (hl)
			puts("ldi a, (hl)");
			break;
		
		case 0x2b: // dec hl
			puts("dec hl");
			break;
		
		case 0x2c: // inc l
			puts("inc l");
			break;
		
		case 0x2d: // dec l
			puts("dec l");
			break;
		
		case 0x2e: // ld l,n
			printf("ld l, %02x\n",operand);
			break;
			
		case 0x2f: // cpl
			puts("cpl a");
			break;
		
		case 0x30: // jr nc, nn
			printf("jr nc, %x\n",(address + (int8_t)operand));
			break;
			
		case 0x31: // ld sp, nn
			printf("ld sp, %04x\n",operandw);		
			break;
			
		case 0x32: // ldd (hl),a
			puts("ldd (hl),a");
			break;
		
		case 0x33: // inc sp
			puts("inc sp");
			break;
		
		case 0x34: // inc (hl)
			puts("inc hl");
			break;
		
		case 0x35: // dec (hl)
			puts("dec (hl)");
			break;
		
		case 0x36: // ld (hl), n
			printf("ld (hl), %02x\n",operand);
			break;
		
		case 0x37: // scf
			puts("scf");
			break;
		
		case 0x38: // jr c, nnnn
			printf("jr c, %x\n",(address + (int8_t)operand));
			break;
		
		case 0x39: // add hl, sp
			puts("add hl, sp");
			break;
		
		case 0x3b: // dec sp
			puts("dec sp");
			break;
		
		case 0x3c: // inc a
			puts("inc a");
			break;
		
		case 0x3a: // ldd a, (hl)
			puts("ldd a, (hl)");
			break;
		
		case 0x3d: // dec a
			puts("dec a");
			break;
			
		case 0x3e: // ld a, nn 
			printf("ld a, %04x\n",operand);
			break;
		
		case 0x3f: // ccf
			puts("ccf");
			break;
		
		case 0x40: // ld b, b
			puts("ld b, b");
			break;
		
		
		case 0x41:
			puts("ld b, c");
			break;
		
		case 0x42: // ld b, d
			puts("ld b, d");
			break;
		
		
		case 0x43: // ld b, e
			puts("ld b, e");
			break;
			
		case 0x44: // ld b, h 	
			puts("ld b, h");
			break;
			
		case 0x45: // ld b, l 
			puts("ld b, l");
			break;
			
		case 0x46: // ld b, (hl)
			puts("ld b, (hl)");
			break;
		

		case 0x47: // ld b,a
			puts("ld b, a");
			break;
		
		case 0x48: // ld c,b
			puts("ld c, b");
			break;
		
		case 0x49: // ld c, c
			puts("ld c, c");
			break;
		
		case 0x4a: // ld c,d
			puts("ld c, d");
			break;
		
		case 0x4b: // ld c,e
			puts("ld c, e");
			break;
		
		case 0x4c: // ld c, h
			puts("ld c, h");
			break;
		
		case 0x4d: // ld c,l
			puts("ld c, l");
			break;
		
		case 0x4e: // ld c, (hl)
			puts("ld c, (hl)");
			break;
		
		case 0x4f: // ld c,a
			puts("ld c, a");
			break;
		
		case 0x50: // ld d, b
			puts("ld d, b");
			break;
		
		case 0x51: // ld d, c
			puts("ld d, c");
			break;
		
		case 0x52: // ld d, d
			puts("ld d, d");
			break;
		
		case 0x53: // ld d, e
			puts("ld d, e");
			break;
		
		case 0x54: // ld d,h
			puts("ld d, h");
			break;
		
		case 0x55: // ld d, l
			puts("ld d, l");
			break;
		
		case 0x56: // ld d,(hl)
			puts("ld d, (hl)");
			break;
		
		case 0x57:  //ld d, a
			puts("ld d, a");
			break;
		
		case 0x58: // ld e, b
			puts("ld e, b");
			break;
		
		case 0x59: // ld e, c 
			puts("ld e, c");
			break;
		
		case 0x5a: // ld e, d 
			puts("ld e, d ");
			break;
		
		case 0x5b: // ld e, e
			puts("ld e, e");
			break;
		
		case 0x5c: // ld e, h 
			puts("ld e, h");
			break;
		
		case 0x5d: // ld e ,l
			puts("ld e, l");
			break;
		
		case 0x5e: //ld e, (hl)
			puts("ld e, (hl)");
			break;
		
		
		case 0x5f: // ld e, a
			puts("ld e, a");
			break;
		
		case 0x60: // ld h, b
			puts("ld h, b");
			break;
		
		case 0x61: // ld h, c
			puts("ld h, c");
			break;
		
		case 0x62: // ld h, d
			puts("ld h, d");
			break;
		
		case 0x63: // ld h, e
			puts("ld h, e");
			break;
		
		case 0x64: // ld h ,h
			puts("ld h, h");
			break;
		
		case 0x65: // ld h, l
			puts("ld h, l");
			break;
		
		case 0x66: // ld h, (hl)
			puts("ld h, (hl)");
			break;
		
		case 0x67: // ld h,a
			puts("ld h, a");
			break;
		
		case 0x68: // ld l ,b
			puts("ld l, b");
			break;
		
		case 0x69: // ld l, c
			puts("ld l, c");
			break;

		case 0x6a: // ld e, d
			puts("ld l, d");
			break;	
			
		case 0x6b: // ld l, e
			puts("ld l, e");
			break;
		
		case 0x6c: // ld l, h
			puts("ld l, h");
			break;
		
		case 0x6d: // ld l, l
			puts("ld l, l");
			break;
		
		case 0x6e: // ld l, (hl)
			puts("ld l, (hl");
			break;
		
		case 0x6f: // ld l, a
			puts("ld l, a");
			break;
		
		case 0x70: // ld (hl), b
			puts("ld (hl), b");
			break;
		
		case 0x71: // ld (hl), c
			puts("ld (hl), c");
			break;
		
		case 0x72: // ld (hl), d
			puts("ld (hl), d");
			break;
		
		case 0x73: // ld (hl), e
			puts("ld (hl), e");
			break;
		
		case 0x74: // ld (hl), h 
			puts("ld ,");
			break;
		
		case 0x75: // ld (hl), l
			puts("ld ,");
			break;
		
		case 0x76: // halt
			puts("halt");
			break;
		
		case 0x77: // ld (hl),a 
			puts("ld (hl),a");
			break;
		
		case 0x78: // ld a, b
			puts("ld a, b");
			break;
		
		case 0x79: // ld a, c
			puts("ld a, c");
			break;
		
		case 0x7a: // ld a, d
			puts("ld a, d");
			break;
		
		case 0x7b: // ld a,e
			puts("ld a, e");
			break;
		
		case 0x7c: // ld a,h
			puts("ld a, h");
			break;
		
		
		case 0x7d: // ld a, l
			puts("ld a, l");
			break;
		
		case 0x7e: // ld a, (hl)
			puts("ld a, (hl)");
			break;
		
		case 0x7f: // ld a, d
			puts("ld a, a");
			break;
		
		case 0x80: // add b
			puts("add b");
			break;
		
		case 0x81: // add c
			puts("add c");
			break;
		
		case 0x82: // add d
			puts("add d");
			break;
		
		case 0x83: // add e
			puts("add e");
			break;
		
		case 0x84: // add h
			puts("add h");
			break;
		
		case 0x85: // add l
			puts("add l");
			break;
		
		
		case 0x86: // add (hl)
			puts("add (hl)");
			break;
		
		case 0x87: // add a
			puts("add a");
			break;
		
		case 0x88: // adc a,b 
			puts("adc a, b");
			break;
		
		case 0x89: // adc c
			puts("adc c");
			break;
		
		case 0x90: // sub a, b (a is implict)
			puts("sub b");
			break;
		
		case 0x91: // sub a, c
			puts("sub c");
			break;
		
		case 0x92: // sub a, d 
			puts("sub d");
			break;
		
		case 0x96: // sub (hl)
			puts("sub (hl)");
			break;
		
		case 0xa0: // and b
			puts("and b");
			break;
		
		case 0xa1: // and c
			puts("and c");
			break;
		
		case 0xa7: // and a
			puts("and a");
			break;
		
		case 0xa8: // xor b
			puts("xor b");
			break;
		
		case 0xa9: // xor c
			puts("xor c");
			break;
		
		case 0xad: // xor l
			puts("xor l");
			break;
		
		case 0xae:
			puts("xor (hl)");
			break;
		
		case 0xaf:
			puts("xor a");
			break;	
		

		case 0xb0: // or b
			puts("or b");
			break;
		
		case 0xb1: // or c 
			puts("or c");
			break;
		
		case 0xb2: // or d
			puts("or d");
			break;
		
		case 0xb3: // or a, e
			puts("or a, e");
			break;
			
		case 0xb4: // or a, h
			puts("or a, h");
			break;
			
		case 0xb5: // or a, l
			puts("or a, l");
			break;
		
		case 0xb6: // or (hl)
			puts("or (hl)");
			break;
		
		
		case 0xb7: // or a
			puts("or a");
			break;
		
		case 0xb8: // cp b 
			puts("cp b");
			break;
		
		case 0xb9: // cp c
			puts("cp c");
			break;
		
		case 0xba: // cp d
			puts("cp d");
			break;
		
		case 0xbb: // cp e
			puts("cp e");
			break;
		
		
		case 0xbc: // cp h
			puts("cp h");
			break;
			
		case 0xbd: // cp l
			puts("cp l");
			break;
		
		case 0xbe: // cp (hl)
			puts("cp (hl)");
			break;
		
		case 0xbf: // cp a
			puts("cp a");
			break;
		
		case 0xc0: // ret nz
			puts("ret nz");
			break;
		
		case 0xc1: // pop bc 
			puts("pop bc");
			break;
		
		case 0xc2: // jp nz nnnn
			printf("jp nz, %x\n",read_word(address,cpu));
			break;
		
		case 0xc3: // jump
			printf("jp %04X\n",read_word(address,cpu));
			break;
		
		
		case 0xc4: // call nz nnnn
			printf("call nz %x\n",read_word(address,cpu));
			break;
		
		case 0xc5: // push bc
			puts("push bc");
			break;
		
		case 0xc6: // add a, nn
			printf("add a, %x\n",read_mem(address,cpu));
			break;
		
		
		case 0xc7: // rst 00
			puts("rst 00");
			break;
		
		case 0xc8: // ret z
			puts("ret z");
			break;
		
		case 0xc9: // ret
			puts("ret");
			break;
		
		case 0xCA: // jp z, nnnn
			printf("jp z, %x\n",operandw);
			break;
		
		case 0xCB: // extended opcode 
			op = read_mem(address,cpu);
			len = 2; // all cb opcodes are two bytes each
			switch(op)
			{
				
				case 0x0: //  rlc b
					puts("rlc b");
					break;
				
				case 0x3: // rlc e
					puts("rlc e");
					break;
					
				case 0x4: // rlc h 
					puts("rlc h");
					break;
				
				case 0x9: // rrc
					puts("rrc");
					break;
				
				case 0xc: // rrh
					puts("rrh");
					break;
				
				case 0x11: // rl c
					puts("rl c");
					break;
				
				case 0x12: // rl d
					puts("rl d");
					break;
				
				case 0x16: //rl (hl)
					puts("rl (hl)");
					break;
				
				case 0x19: // rr c
					puts("rr c");
					break;
					
				case 0x1a: // rr d
					puts("rr d");
					break;
				
				case 0x1b: // rr e 
					puts("rr e");
					break;
				
				case 0x23: // sla e
					puts("sla e");
					break;
				
				case 0x27: // sla a
					puts("sla a");
					break;
				
				case 0x2f:
					puts("sra a");
					break;
				
				case 0x33: // swap e
					puts("swap e");
					break;
				
				case 0x37: // swap a
					puts("swap a");
					break;
				
				case 0x38: // srl b
					puts("srl b");
					break;
				
				case 0x3f: // srl a
					puts("srl a");
					break;
				
				case 0x40: // bit 0, b
					puts("bit 0, b");
					break;
				
				case 0x41: // bit 0, c
					puts("bit 0, c");
					break;
				
				case 0x42: // bit 0, d
					puts("bit 0, d");
					break;
				
				case 0x43: // bit 0, e
					puts("bit 0, e");
					break;
				
				case 0x44: // bit 0, h
					puts("bit 0, h");
					break;
				
				case 0x45: // bit 0, l
					puts("bit 0, l");
					break;
				
				case 0x46: // bit 0, (hl)
					puts("bit 0, (hl)");
					break;
				
				case 0x47: // bit 0, a
					puts("bit 0, a");
					break;
				
				case 0x48: // bit l, b
					puts("bit l, b");
					break;
				
				case 0x49: // bit 1, c
					puts("bit 1, c");
					break;
				
				case 0x4a: // bit 1, d
					puts("bit 1, d");
					break;
				
				case 0x4b: // bit 1, e
					puts("bit 1, e");
					break;
				
				case 0x50: // bit 2,b
					puts("bit 2, b");
					break;
				
				case 0x57: // bit 2, a
					puts("bit 2, a");
					break;
				
				case 0x58: // bit 3, b
					puts("bit 3, b");
					break;
				
				case 0x5f: // bit 3, a
					puts("bit 3, a");
					break;
				
				case 0x60: // bit 4, b
					puts("bit 4, b");
					break;
				
				case 0x61: // bit 4, c
					puts("bit 4, c");
					break;
				
				case 0x68: // bit 5, b
					puts("bit 5, b");
					break;
				
				case 0x69: // bit 5, c
					puts("bit 5, c");
					break;
				
				case 0x6f: // bit 5, a
					puts("bit 5, a");
					break;
				
				case 0x70: // bit 6, b
					puts("bit 6, b");
					break;
				
				case 0x78: // bit 7, b
					puts("bit 7, b");
					break;
				
				case 0x77: // bit 6, a
					puts("bit 6, a");
					break;
				
				case 0x7c:
					puts("bit 7, h");
					break;
				
				case 0x7d: // bit 7, l
					puts("bit 7, l");
					break;
				
				case 0x7e: // bit 7, (hl)
					puts("bit 7, (hl)");
					break;
				
				case 0x7f: // bit 7, a
					puts("bit 7, a");
					break;
				
				
				case 0x86: // res 0, (hl)
					puts("res 0, (hl)");
					break;
				
				case 0x87:
					puts("res 0, a");
					break;
		
				case 0x8d: // res 1, l
					puts("res 1, L");
					break;
		
				case 0x9e: // res 3, (hl)
					puts("res 3, (hl)");
					break;
				
				case 0xbe: // res 7, (hl)
					puts("res 7, (hl)");
					break;
				
				case 0xc6: // set 0, (hl)
					puts("set 0, (hl)");
					break;
				
				case 0xde: // set 3, (hl)
					puts("set 3, (hl)");
					break;
				
				case 0xf8: // set 7, b
					puts("set 7, b");
					break;
				
				case 0xfe: // set 7, (hl)
					puts("set 7, (hl)");
					break;
				
				default:
					fprintf(stderr, "[disass] Unknown CB opcode: %x\n", op);
					cpu_state(cpu);
					exit(1);
			}
			break;
		
		case 0xcc: // call z u16
			printf("call z, %x\n",operandw);
			break;
		
		case 0xCD: // call nn 
			printf("call %04x\n",operandw);
			break;
		
		case 0xce: // adc a, nn
			printf("adc a, %x\n",operand);
			break;
		
		case 0xcf: // rst 08
			puts("rst 08");
			break;
		
		case 0xd0: // ret nc
			puts("ret nc");
			break;
		
		case 0xd1: // pop de
			puts("pop de");
			break;
		
		case 0xd2: // jp nc u16
			printf("jp nc, %x\n",operandw);
			break;
		
		case 0xd4: // call nc
			printf("call nc %x\n",operandw);
			break;
		
		case 0xd5: // push de
			puts("push de");
			break;
		
		case 0xd6: // sub a, nn
			printf("sub a, %x\n",operand);
			break;
		
		case 0xd7: // rest 10
			puts("rst 10");
			break;
		
		case 0xd8: // ret c
			puts("ret c");
			break;
		
		case 0xd9: // reti
			puts("reti");
			break;
		
		
		case 0xda: // jp c u16
			printf("jp c, %x\n",operandw);
			break;
		
		case 0xdc: // call c, u16
			printf("call c, %x\n",operandw);
			break;
		
		case 0xde: // sbc a, nn
			printf("sbc a, %x\n",operand);
			break;
		
		case 0xE0: // LD(FFF0+N),a
			printf("ld (ff00+%02x),a\n",operand);
			break;
		
		case 0xe1: // pop hl
			puts("pop hl");
			break;
		
		case 0xE2:
			puts("ld (ff00+c),a");
			break;
		
		case 0xe5: // push hl
			puts("push hl");
			break;
		
		
		case 0xe6: // and a, n 
			printf("and a, %x\n",operand);
			break;
		
		case 0xe8: // add sp, i8
			printf("add sp, %x\n",(int8_t)operand);
			break;
		
		case 0xe9: // jp hl
			puts("jp hl");
			break;
		
		case 0xea: // ld (nnnn), a 
			printf("ld (%04x), a\n",operandw);
			break;
		
		case 0xee: // xor a, nn
			printf("cpu a, %x\n",operand);
			break;
		
		case 0xef: // rst 28
			puts("rst 28");
			break;
		
		case 0xf0: // ld a, (ff00+n)
			printf("ld a, (ff00+%02x)\n",operand);
			break;
		
		case 0xf1: // pop af
			puts("pop af");
			break;
		
		case 0xf2: // ld a, (ff00+c)
			puts("ld a, (ff00+c)");
			break;
		
		case 0xf3: // di 
			puts("di");
			break;
		
		case 0xf5: // push af
			puts("push af");
			break;
		
		case 0xf6: // or a, nn
			printf("or a, %x\n",operand);
			break;
		
		case 0xf8: // ld hlsp, n
			printf("ld sp, hl + %x\n",(int8_t)operand);
			
			break;
		
		case 0xf9: // ld sp, hl
			puts("ld sp, hl");
			break;
		
		case 0xfa: // ld a (nn) <- 16 bit address
			printf("ld a, (%x)\n",operandw);
			break;
		
		case 0xfb: // ei
			puts("ei");
			break;
		
		case 0xfe: // cp a, n (cmp)
			printf("cp a, %02x\n",operand);
			break;
		
		default:
			fprintf(stderr, "[Disass] Unknown opcode: %x\n", opcode);
			cpu_state(cpu);
			exit(1);	
	}
	return len;
}


// print the current cpu state
void cpu_state(const Cpu *cpu)
{
	printf("PC %04x: AF:%04x BC:%04x DE:%04x HL:%04x SP:%04x\n",
		cpu->pc,cpu->af.reg, cpu->bc.reg, cpu->de.reg, cpu->hl.reg, cpu->sp);	
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
#endif
