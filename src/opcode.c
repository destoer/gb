// VBLANK interrupt is what should be taking the game out of the loop
// but it is not running after left is pressed...


// fix adc and sbc

#include "headers/lib.h"
#include "headers/cpu.h"
#include "headers/opcode_cb.h"
#include "headers/disass.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


//static int breakpoint = 0x2299;
//static int breakpoint = 0x29fe;
//static int breakpoint = 0x100;
//static int breakpoint = 0x1c4f; // by the second time its reached bc has the wrong value from the ret
//static int breakpoint = 0x2b40;
//static int breakpoint = 0x2afe;
//static int breakpoint = 0x1638; // break in bgb at 0346 after first hit
//static int breakpoint = 0x164f;

// potentially need the rominfo too but not needed yet
int step_cpu(Cpu * cpu)
{
	


	// breakpoint (fail inside call at 1c3c) <--- specifically at 2b03 with a read from ff89
	//printf("%x: %x\n",cpu->pc,cpu->breakpoint);
	if(cpu->pc == cpu->breakpoint || cpu->step) 
	{
		
		
		// enter into the debug console
		// enter commands exit when run command typed
		cpu->step = false; // disable stepping
		printf("execution breakpoint hit at %x\n",cpu->pc);
		cpu->breakpoint = -1; // deset the breakpoint
		uint8_t opcode = read_mem(cpu->pc++,cpu);
		disass_8080(opcode,cpu);
		enter_debugger(cpu);
		cpu->pc -= 1; // correct pc 
		
	} 
	

	
	// one byte immediate
	// instrucitons may need to be signed? eg ld a, (ff00 + n)
	
	// read an opcode and inc the program counter
	
	
	
	uint8_t opcode = read_mem(cpu->pc++,cpu); // <--- appears to attempt and extra read before even exiting the loop but cant pinpoint where...
	int8_t operand;
	uint8_t cbop;
	//print cpu state and disassemble the opcode
	//cpu_state(cpu);
	//disass_8080(opcode, cpu);
	
	if(cpu->halt_bug)
	{
		cpu->pc -= 1; // correct the opcode
		exit(1);
		cpu->halt_bug = false;
	}
	
	

	
	
	
	
	
	// decode and execute 
	//(opcode loads may need to go through the read mem funcion at some point)
	switch(opcode)
	{
		case 0x0: // nop
			break;
		
		case 0x1: // ld bc, nn
			cpu->bc.reg = read_word(cpu->pc,cpu);
			cpu->pc += 2;
			break;
		
		case 0x2: // ld (bc), a
			write_mem(cpu,cpu->bc.reg,cpu->af.hb);
			break;
		
		case 0x3: // inc bc
			cpu->bc.reg += 1;
			break;
		
		case 0x4: // inc b
			inc(cpu, cpu->bc.hb);
			cpu->bc.hb+=1;
			break;
			
		case 0x5: // dec b
			dec(cpu, cpu->bc.hb--);
			//print_flags(cpu);
			break;
			
		case 0x6: // ld b, n
			cpu->bc.hb = read_mem(cpu->pc++,cpu);
			break;
			
		case 0x7: // rlca (rotate a left bit 7 to carry)
			cpu->af.hb = rlc(cpu,cpu->af.hb);
			deset_bit(cpu->af.lb,Z);
			break;
		
		
		case 0x8: // ld (nnnn), sp
			write_word(cpu,read_word(cpu->pc,cpu),cpu->sp);
			//printf("write: [%x]%x [%x]%x\n",read_word(cpu->pc,cpu),read_mem(read_word(cpu->pc,cpu),cpu),read_word(cpu->pc,cpu)+1,read_mem(read_word(cpu->pc,cpu)+1,cpu));
			cpu->pc += 2; // for two immediate ops
			break;
		
		case 0x9: // add hl,bc
			cpu->hl.reg = addw(cpu,cpu->hl.reg,cpu->bc.reg);
			break;
			

		
		case 0xa: // ld a, (bc)
			cpu->af.hb = read_mem(cpu->bc.reg,cpu);
			break;
		
		
		case 0xb: // dec bc 
			cpu->bc.reg -= 1;
			break;
		
		
		
		case 0xc: // inc c
			inc(cpu,cpu->bc.lb);
			cpu->bc.lb+=1;
			break;
		
		case 0xd: // dec c
			dec(cpu, cpu->bc.lb--);
			break;
		
		
		case 0xe: // ld c, n
			cpu->bc.lb = read_mem(cpu->pc++,cpu);
			break;

			
		case 0xf: // rrca
			cpu->af.hb = rrc(cpu,cpu->af.hb);
			deset_bit(cpu->af.lb,Z);
			break;
			
		case 0x10: // stop <-- FIX THIS LATER
			puts("fix stop");
			//exit(1);
			break;
			
		case 0x11: // ld de, nn
			cpu->de.reg = read_word(cpu->pc,cpu);
			cpu->pc += 2;
			break;
		
		case 0x12: // ld (de), a
			write_mem(cpu, cpu->de.reg, cpu->af.hb);
			break;
		
		case 0x13: // inc de
			cpu->de.reg += 1;
			break;
		
		case 0x14: // inc d
			inc(cpu,cpu->de.hb++);
			break;
		
		case 0x15: // dec d
			dec(cpu, cpu->de.hb--);
			break;
		
		case 0x16: // ld d, nn 
			cpu->de.hb = read_mem(cpu->pc++,cpu);
			break;
		
		
		
		
		case 0x17: // rla (rotate left through carry flag) 
			cpu->af.hb = rl(cpu,cpu->af.hb,1);
			deset_bit(cpu->af.lb,Z);
			break;
		
		case 0x18: // jr n
			operand = (int8_t)read_mem(cpu->pc++, cpu);
			cpu->pc += operand;
			break;
		
		case 0x19: // add hl, de
			cpu->hl.reg = addw(cpu,cpu->hl.reg,cpu->de.reg);
			break;
		
		case 0x1a: // ld a,(de) <--- fix memory reading / writing
			cpu->af.hb = read_mem(cpu->de.reg,cpu);
			break;
		

		case 0x1b: // dec de
			cpu->de.reg -= 1;
			break;
			
		case 0x1c: // inc e
			inc(cpu,cpu->de.lb++);
			break;
		
		case 0x1d: // dec e
			dec(cpu, cpu->de.lb--);
			break;
			
		case 0x1e: // ld e, n
			cpu->de.lb = read_mem(cpu->pc++, cpu);
			break;
		
		case 0x1f: // rra
			cpu->af.hb = rr(cpu,cpu->af.hb);
			deset_bit(cpu->af.lb,Z);
			break;
		
		case 0x20: // jr nz, n
			operand = (uint8_t)read_mem(cpu->pc++, cpu);
			if(!is_set(cpu->af.lb, Z))
			{
				cpu->pc += operand;
				return mtcycles[opcode];
			}
			break;
			
		case 0x21: // ld hl, nn
			cpu->hl.reg = read_word(cpu->pc,cpu);
			cpu->pc += 2;
			break;
		
		case 0x22: // ldi (hl), a
			write_mem(cpu,cpu->hl.reg++,cpu->af.hb);
			break;
		
		case 0x23: // inc hl
			cpu->hl.reg += 1; // increment hl
			break;
		
		case 0x24: // inc h
			inc(cpu,cpu->hl.hb++);
			break;
		
		case 0x25: // dec h
			dec(cpu,cpu->hl.hb--);
			break;
		
		case 0x26: // ld h, nn
			cpu->hl.hb = read_mem(cpu->pc++,cpu);
			break;
		
		case 0x27: // daa (lots of edge cases) <-- fix 2morrowh
			//https://forums.nesdev.com/viewtopic.php?f=20&t=15944
		
		
	
			if (!is_set(cpu->af.lb,N)) 
			{  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
				if (is_set(cpu->af.lb,C)|| cpu->af.hb > 0x99) 
				{ 
					cpu->af.hb += 0x60; set_bit(cpu->af.lb,C); 
				}
				if (is_set(cpu->af.lb,H) || (cpu->af.hb & 0x0f) > 0x09) 
				{ 
					cpu->af.hb += 0x6; 
				}
			} 
			
			else 
			{  // after a subtraction, only adjust if (half-)carry occurred
				if (is_set(cpu->af.lb,C)) 
				{
					cpu->af.hb -= 0x60; 
				}
				
				if (is_set(cpu->af.lb,H)) 
				{ 
					cpu->af.hb -= 0x6; 
				}
			}
			
			
			deset_bit(cpu->af.lb,H);
			set_zero(cpu, cpu->af.hb);
			break;
			
		case 0x28: // jr z, n
			operand = read_mem(cpu->pc++, cpu);
			if(is_set(cpu->af.lb, Z))
			{
				cpu->pc += operand;
				return mtcycles[opcode];
			}
			break;
		
		case 0x29: // add hl, hl
			cpu->hl.reg = addw(cpu,cpu->hl.reg,cpu->hl.reg);
			break;
		
		// flags affected by this?
		case 0x2a: // ldi a, (hl)
			cpu->af.hb = read_mem(cpu->hl.reg++,cpu);
			break;
		
		case 0x2b: // dec hl
			cpu->hl.reg -= 1;
			break;
		
		case 0x2c: // inc l
			inc(cpu,cpu->hl.lb++);
			break;
		
		case 0x2d: // dec l
			dec(cpu,cpu->hl.lb--);
			break;
		
		case 0x2e: // ld l, n
			cpu->hl.lb = read_mem(cpu->pc++, cpu);
			break;
			
		case 0x2f: // cpl (flip bits in a)
			// set H and N
			set_bit(cpu->af.lb,N);
			set_bit(cpu->af.lb,H);
			cpu->af.hb = ~cpu->af.hb;
			break;
		
		case 0x30: // jr nc, nn
			operand = read_mem(cpu->pc++, cpu);
			if(!is_set(cpu->af.lb,C))
			{
				cpu->pc += operand;
				return mtcycles[opcode];
			}
			break;
		
		case 0x31: // ld sp, nn
			cpu->sp = read_word(cpu->pc,cpu);
			cpu->pc += 2;
			break;
		
		case 0x32: // ldd (hl), a // <--- check memory was written properly
			write_mem(cpu,cpu->hl.reg--,cpu->af.hb);
			break;
		
		case 0x33: // inc sp
			cpu->sp += 1;
			break;
		
		case 0x34: // inc (hl)
			cbop = read_mem(cpu->hl.reg,cpu); // use to store (hl)
			
			inc(cpu,cbop++); // inc 
			
			write_mem(cpu,cpu->hl.reg,cbop); // and write back
			break;
		
		case 0x35: // dec (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			dec(cpu,cbop--); // dec it
			
			write_mem(cpu,cpu->hl.reg,cbop); // and write straight back
			break;
			
		
		case 0x36: // ld (hl), n 
			write_mem(cpu,cpu->hl.reg,read_mem(cpu->pc++,cpu));
			break;
		
		case 0x37: // scf
			// set the carry flag deset h and N
			set_bit(cpu->af.lb,C);
			deset_bit(cpu->af.lb,N);
			deset_bit(cpu->af.lb,H);
			break;
		
		case 0x38: // jr c, nnnn
			operand = read_mem(cpu->pc++, cpu);
			if(is_set(cpu->af.lb,C))
			{
				cpu->pc += operand;
				return mtcycles[opcode];
			}
			break;
			
		case 0x39: // add hl, sp 
			cpu->hl.reg = addw(cpu,cpu->hl.reg,cpu->sp);
			break;	
			
		case 0x3a: // ldd a, (hl)
			cpu->af.hb = read_mem(cpu->hl.reg,cpu);
			cpu->hl.reg -= 1; // full reg doesent care about flags
			break;
		

		
		case 0x3b: // dec sp
			cpu->sp -= 1;
			break;
		
		case 0x3c: // inc a
			inc(cpu,cpu->af.hb++);
			break;
		
		case 0x3d: // dec a
			dec(cpu,cpu->af.hb--);
			break;
			
		
		case 0x3e: // ld a, n
			cpu->af.hb = read_mem(cpu->pc++, cpu);
			break;
		
		case 0x3f: // ccf
			if(is_set(cpu->af.lb,C))
			{ // complement the carry flag (probably a neat way to do this)
				deset_bit(cpu->af.lb,C);
			}
			
			else
			{
				set_bit(cpu->af.lb,C);
			}
			
			deset_bit(cpu->af.lb,N);
			deset_bit(cpu->af.lb,H);
			break;
		
		case 0x40: // ld b, b
			// do nothing lol
			break;
		
		case 0x41: // ld b, c
			cpu->bc.hb = cpu->bc.lb;
			break;
		
		case 0x42: // ld b, d
			cpu->bc.hb = cpu->de.hb;
			break;
		
		case 0x43: // ld b, e
			cpu->bc.hb = cpu->de.lb;
			break;
		
		case 0x44: // ld b, h
			cpu->bc.hb = cpu->hl.hb;
			break;
		
		case 0x45: // ld b, l
			cpu->bc.hb = cpu->hl.lb;
			break;
		
		case 0x46: // ld b, (hl)
			cpu->bc.hb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x47: // ld b,a
			cpu->bc.hb = cpu->af.hb;
			break;
		
		case 0x48: // ld c, b 
			cpu->bc.lb = cpu->bc.hb;
			break;
		
		case 0x49: // ld c, c 
			cpu->bc.lb = cpu->bc.lb;
			break;
		
		case 0x4a: // ld c, d 
			cpu->bc.lb = cpu->de.hb;
			break;
		
		case 0x4b: // ld c, e 
			cpu->bc.lb = cpu->de.lb;
			break;
		
		case 0x4c: // ld c, h
			cpu->bc.lb = cpu->hl.hb;
			break;
		
		case 0x4d: // ld c ,l 
			cpu->bc.lb = cpu->hl.lb;
			break;
		
		case 0x4e: // ld c, (hl)
			cpu->bc.lb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x4f: // ld c,a
			cpu->bc.lb = cpu->af.hb;
			break;
		
		
		case 0x50: // ld d, b 
			cpu->de.hb = cpu->bc.hb;
			break;
		
		case 0x51: // ld d, c 
			cpu->de.hb = cpu->bc.lb;
			break;
		
		
		case 0x52: // ld d, d
			// nop lol 
			break;
		
		
		case 0x53: // ld d, e
			cpu->de.hb = cpu->de.lb;
			break;
		
		case 0x54: // ld d, h
			cpu->de.hb = cpu->hl.hb;
			break;
		
		case 0x55: // ld d , l 
			cpu->de.hb = cpu->hl.lb;
			break;
		
		case 0x56: // ld d, (hl)
			cpu->de.hb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x57: // ld d, a
			cpu->de.hb = cpu->af.hb;
			break;
		
		
		case 0x58: // ld e, b 
			cpu->de.lb = cpu->bc.hb;
			break;
		
		case 0x59: // ld e, c
			cpu->de.lb = cpu->bc.lb;
			break;
		
		case 0x5a: // ld e, d
			cpu->de.lb = cpu->de.hb; 
			break;
		
		case 0x5b: // ld ,
			// nop 
			break;
		
		case 0x5c: // ld e,h
			cpu->de.lb = cpu->hl.hb;
			break;
			
		case 0x5d: // ld e, l
			cpu->de.lb = cpu->hl.lb;
			break;
			
		case 0x5e: // ld e, (hl)
			cpu->de.lb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x5f: // ld e, a
			cpu->de.lb = cpu->af.hb;
			break;
		
		
		case 0x60: // ld h, b
			cpu->hl.hb = cpu->bc.hb;
			break;
		
		case 0x61: // ld h, c
			cpu->hl.hb = cpu->bc.lb;
			break;
		
		case 0x62: // ld h, d
			cpu->hl.hb = cpu->de.hb;
			break;
		
		
		case 0x63: // ld h, e
			cpu->hl.hb = cpu->de.lb;
			break;
			
		case 0x64: // ld h, h
			// nop;
			break;
			
		case 0x65: // ld h, l 	
			cpu->hl.hb = cpu->hl.lb;
			break;
			
		case 0x66: // ld h, (hl)
			cpu->hl.hb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x67: // ld h, a 
			cpu->hl.hb = cpu->af.hb;
			break;
		
		case 0x68: // ld l, b
			cpu->hl.lb = cpu->bc.hb;
			break;
		
		case 0x69: // ld l,c
			cpu->hl.lb = cpu->bc.lb;
			break;
		
		
		case 0x6a: // ld l, d
			cpu->hl.lb = cpu->de.hb;
			break;
		
		case 0x6b: // ld l, e
			cpu->hl.lb = cpu->de.lb;
			break;
		
		case 0x6c: // ld l, h 
			cpu->hl.lb = cpu->hl.hb;
			break;
			
		case 0x6d: // ld l, l
			// nop
			break;
		
		case 0x6e: // ld l, (hl)
			cpu->hl.lb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x6f: // ld l, a
			cpu->hl.lb = cpu->af.hb;
			break;
		
		case 0x70: // ld (hl),b
			write_mem(cpu,cpu->hl.reg,cpu->bc.hb);
			break;
		
		case 0x71: // ld (hl), c
			write_mem(cpu,cpu->hl.reg, cpu->bc.lb);
			break;
		
		case 0x72: // ld (hl), d
			write_mem(cpu,cpu->hl.reg,cpu->de.hb);
			break;
		
		case 0x73: // ld (hl), e
			write_mem(cpu,cpu->hl.reg,cpu->de.lb);
			break;
		
		case 0x74: // ld (hl), h
			write_mem(cpu,cpu->hl.reg,cpu->hl.hb);
			break;
		
		case 0x75: // ld (hl), l
			write_mem(cpu,cpu->hl.reg,cpu->hl.lb);
			break;
		
		case 0x76: // halt <-- todo
			// caller will handle
			//puts("Implement halt");
			cpu->halt = true;
			break;
		
		case 0x77: // ld (hl), a 
			write_mem(cpu,cpu->hl.reg,cpu->af.hb);
			break;
		
		case 0x78: // ld a, b
			cpu->af.hb = cpu->bc.hb;
			break;
		
		case 0x79: //ld a, c
			cpu->af.hb = cpu->bc.lb;
			break;
		
		case 0x7a: // ld a, d
			cpu->af.hb = cpu->de.hb;
			break;
		
		case 0x7b: // ld a, e
			cpu->af.hb = cpu->de.lb;
			break;
		
		
		case 0x7c: // ld a, h
			cpu->af.hb = cpu->hl.hb;
			break;
		
		
		case 0x7d: // ld a, l
			cpu->af.hb = cpu->hl.lb;
			break;
		
		case 0x7e: // ld a, (hl)
			cpu->af.hb = read_mem(cpu->hl.reg,cpu);
			break;
		
		case 0x7f: // ld a, a
			// nop 
			break;
		
		case 0x80: // add b
			cpu->af.hb = add(cpu,cpu->af.hb,cpu->bc.hb);
			break;
		
		case 0x81: // add c
			cpu->af.hb = add(cpu,cpu->af.hb,cpu->bc.lb);
			break;
		
		case 0x82: // add d
			cpu->af.hb = add(cpu,cpu->af.hb,cpu->de.hb);
			break;
		
		case 0x83: // add e
			cpu->af.hb =add(cpu,cpu->af.hb,cpu->de.lb);
			break;
		
		case 0x84: // add h
			cpu->af.hb = add(cpu,cpu->af.hb,cpu->hl.hb);
			break;
		
		case 0x85: // add l
			cpu->af.hb = add(cpu,cpu->af.hb,cpu->hl.lb);
			break;
		
		case 0x86: // add a, (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			cpu->af.hb = add (cpu,cpu->af.hb,cbop);
			break;
		
		case 0x87: // add a
			cpu->af.hb = add(cpu,cpu->af.hb, cpu->af.hb);
			break;
		
		
		case 0x88: // adc a, b
			cpu->af.hb  = adc(cpu,cpu->af.hb,cpu->bc.hb);
			break;
		
		case 0x89: // adc c (add carry + n)
			//add(cpu,cpu->af.hb,cpu->bc.lb + val_bit(cpu->af.lb,C));
			cpu->af.hb = adc(cpu,cpu->af.hb,cpu->bc.lb);
			break;
		
		case 0x8a: // adc d
			cpu->af.hb = adc(cpu,cpu->af.hb,cpu->de.hb);
			break;
			
		case 0x8b: // adc e
			cpu->af.hb = adc(cpu,cpu->af.hb,cpu->de.lb);
			break;
			
		case 0x8c: // adc h
			cpu->af.hb = adc(cpu,cpu->af.hb,cpu->hl.hb);
			break;
			
		case 0x8d: // adc l
			cpu->af.hb = adc(cpu,cpu->af.hb,cpu->hl.lb);
			break;
		
		case 0x8e: // adc (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			cpu->af.hb = adc(cpu,cpu->af.hb,cbop);
			break;
		
		case 0x8f: // adc a
			cpu->af.hb = adc(cpu,cpu->af.hb,cpu->af.hb);
			break;
		
		case 0x90: // sub b
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->bc.hb);
			//cpu_state(cpu); print_flags(cpu); exit(1);
			break;
		
		case 0x91: // sub c
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->bc.lb);
			break;
		
		case 0x92: // sub d
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->de.hb);
			break;
			
		case 0x93: // sub e
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->de.lb);
			break;
			
		case 0x94: // sub h
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->hl.hb);
			break;
			
		case 0x95: // sub l
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->hl.lb);
			break;
		
		case 0x96: // sub (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			cpu->af.hb = sub(cpu,cpu->af.hb,cbop);
			break;
		
		case 0x97: // sub a 
			cpu->af.hb = sub(cpu,cpu->af.hb,cpu->af.hb);
			break;
		
		case 0x98: // sbc, a, b
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->bc.hb);
			break;
			
		case 0x99: // sbc a, c
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->bc.lb);
			break;
			
		case 0x9a: // sbc a ,d
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->de.hb);
			break;
			
		case 0x9b: // sbc a, e
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->de.lb);
			break;
			
		case 0x9c: // sbc a, h 
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->hl.hb);
			break;
			
		case 0x9d: // sbc a, l
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->hl.lb);
			break;
		
		case 0x9e: // sbc a, (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			cpu->af.hb = sbc(cpu,cpu->af.hb,cbop);
			break;
		
		case 0x9f: // sbc a, a
			cpu->af.hb = sbc(cpu,cpu->af.hb,cpu->af.hb);
			break;
		
		case 0xa0: // and b
			and(cpu,cpu->bc.hb);
			break;
		
		case 0xa1: // and c
			and(cpu,cpu->bc.lb);
			break;
		
		case 0xa2: // and d
			and(cpu,cpu->de.hb);
			break;
		
		case 0xa3: // and e
			and(cpu,cpu->de.lb);
			break;
		
		case 0xa4: // and h
			and(cpu,cpu->hl.hb);
			break;
			
		case 0xa5: // and l
			and(cpu,cpu->hl.lb);
			break;
		
		case 0xa6: // and (hl)
			and(cpu,read_mem(cpu->hl.reg,cpu));
			break;
		
		case 0xa7: // and a
			and(cpu, cpu->af.hb);
			break;
		

		case 0xa8: // xor b
			xor(cpu,cpu->bc.hb);
			break;
		
		case 0xa9: // xor c 
			xor(cpu,cpu->bc.lb);
			break;
		
		case 0xaa: // xor d
			xor(cpu,cpu->de.hb);
			break;
			
		case 0xab: // xor e
			xor(cpu,cpu->de.lb);
			break;
			
		case 0xac: // xor h
			xor(cpu,cpu->hl.hb);
			break;
		
		case 0xad: // xor l
			xor(cpu,cpu->hl.lb);
			break;
		
		case 0xae: // xor (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			xor(cpu,cbop);
			break;
		
		// shortcut case end up with just zero flag being set <-- should be optimise later
		case 0xaf: // xor a, a
			xor(cpu,cpu->af.hb);
			//cpu->af.reg = 128; 
			break;
		
		case 0xb0: // or b
			or(cpu,cpu->bc.hb);
			break;
		
		case 0xb1: // or c (a is implicit)
			or(cpu,cpu->bc.lb);
			break;
		
		case 0xb2: // or d
			or(cpu,cpu->de.hb);
			break;
		
		case 0xb3: // or e
			or(cpu,cpu->de.lb);
			break;
			
		case 0xb4: // or h
			or(cpu,cpu->hl.hb);
			break;
			
		case 0xb5: // or l
			or(cpu,cpu->hl.lb);
			break;
		
		case 0xb6: // or (hl)
			or(cpu,read_mem(cpu->hl.reg,cpu));
			break;
		
		case 0xb7: // or a
			or(cpu,cpu->af.hb);
			break;
		
		case 0xb8: // cp b (sub but ignore result only keep flags)
			sub(cpu,cpu->af.hb,cpu->bc.hb);
			break;

		case 0xb9: // cp c
			sub(cpu,cpu->af.hb,cpu->bc.lb);
			break;
		
		case 0xba: // cp d
			sub(cpu,cpu->af.hb,cpu->de.hb);
			break;
		
		case 0xbb: // cp e
			sub(cpu,cpu->af.hb,cpu->de.lb);
			break;
		
		case 0xbc: // cp h
			sub(cpu,cpu->af.hb,cpu->hl.hb);
			break;
			
		case 0xbd: // cp l
			sub(cpu,cpu->af.hb,cpu->hl.lb);
			break;
		
		case 0xbe: // cp (hl)
			cbop = read_mem(cpu->hl.reg,cpu);
			sub(cpu,cpu->af.hb,cbop);
			break;
		
		case 0xbf: // cp a <-- probably can be optimised to a constant
			sub(cpu,cpu->af.hb,cpu->af.hb);
			break;
		
		case 0xc0: // ret nz
			if(!is_set(cpu->af.lb,Z))
			{
				cpu->pc = read_stackw(cpu);
				return mtcycles[opcode];
			}
			break;
	
		case 0xc1: // pop bc
			cpu->bc.reg = read_stackw(cpu);
			break;
		
		case 0xc2: // jp nz, nnnn
		
			if(!is_set(cpu->af.lb,Z))
			{
				cpu->pc = read_word(cpu->pc,cpu);
				return mtcycles[opcode];
			}
			
			else 
			{
				cpu->pc += 2;
			}
			break;
			
		case 0xc3: // jump
			cpu->pc = read_word(cpu->pc,cpu);
			break;
		
		
		case 0xc4: // call nz
			if(!is_set(cpu->af.lb,Z))
			{
				write_stackw(cpu,cpu->pc+2);
				cpu->pc = read_word(cpu->pc, cpu);
				return mtcycles[opcode];
			}
			
			else 
			{
				cpu->pc += 2;
			}
			break;
		
		case 0xc5: // push bc 
			write_stackw(cpu,cpu->bc.reg);
			break;
		
		
		case 0xc6: // add a, nn
			cpu->af.hb = add(cpu,cpu->af.hb,read_mem(cpu->pc++,cpu));
			break;
		
		case 0xc7: // rst 00
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0;
			break;
		
		case 0xc8: // ret z
			if(is_set(cpu->af.lb,Z))
			{
				cpu->pc = read_stackw(cpu);
				return mtcycles[opcode];
			}
			break;
		
		case 0xc9: // ret 
			cpu->pc = read_stackw(cpu);
			break;
		
		case 0xca: // jp z, nnnn
			if(is_set(cpu->af.lb, Z))
			{
				cpu->pc = read_word(cpu->pc,cpu);
				return mtcycles[opcode];
			}
			
			else
			{
				cpu->pc += 2; // skip jump opcodes
			
			}
			break;
			
		// return early from here and return the cycles
		// from the cb prefix table (could error with interrupt enable before it as edge case...)
		case 0xcb: // multi len opcode (cb prefix)
		
			
			cbop = read_mem(cpu->pc++, cpu); // fetch the opcode
			decode_cb(cbop,cpu); // exec it 

			return cbmcycles[cbop]; // update machine cylces for cb prefix
			break; 
		
		
		case 0xcc:
			if(is_set(cpu->af.lb,Z))
			{
				write_stackw(cpu,cpu->pc+2);
				cpu->pc = read_word(cpu->pc,cpu);
				return  mtcycles[opcode];
			}
			
			else
			{
				cpu->pc += 2;
			}
			break;
		
		case 0xCD: // call nn <-- verify
			write_stackw(cpu,cpu->pc+2);
			cpu->pc = read_word(cpu->pc, cpu);
			break;

		case 0xce: // adc a, nn
			cpu->af.hb = adc(cpu,cpu->af.hb,read_mem(cpu->pc++,cpu));
			break;
		
		case 0xcf: // rst 08
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x8;
			break;
		
		case 0xd0: // ret nc
			if(!is_set(cpu->af.lb,C))
			{
				cpu->pc = read_stackw(cpu);
				return mtcycles[opcode];
			}
			break;
		
		case 0xd1: // pop de
			cpu->de.reg = read_stackw(cpu);
			break;
		
		case 0xd2: // jp nc u16
			if(!is_set(cpu->af.lb,C))
			{
				cpu->pc = read_word(cpu->pc,cpu);
				return mtcycles[opcode];
			}
			
			else
			{
				cpu->pc += 2;
			}
			break;
		
		case 0xd4: // call nc nnnn
			if(!is_set(cpu->af.lb,C))
			{
				write_stackw(cpu,cpu->pc+2);
				cpu->pc = read_word(cpu->pc, cpu);
				return  mtcycles[opcode];
			}
			
			else 
			{
				cpu->pc += 2;
			}
			break;			
		
		case 0xD5: // push de
			write_stackw(cpu,cpu->de.reg);
			break;
		
		case 0xd6: // sub a, nn
			cpu->af.hb = sub(cpu,cpu->af.hb,read_mem(cpu->pc++,cpu));
			break;
		
		case 0xd7: // rst 10
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x10;
			break;
		
		case 0xd8: // ret c
			if(is_set(cpu->af.lb,C))
			{
				cpu->pc = read_stackw(cpu);
				return mtcycles[opcode];
			}
			break;
			
		case 0xd9: // reti
			cpu->pc = read_stackw(cpu);
			cpu->interrupt_enable = true; // re-enable interrupts
			break;
		
		case 0xda: // jp c, u16
			if(is_set(cpu->af.lb,C))
			{
				cpu->pc = read_word(cpu->pc,cpu);
				return mtcycles[opcode];
			}
			
			else
			{
				cpu->pc += 2;
			}
			break;
		
		case 0xdc: // call c, u16
			if(is_set(cpu->af.lb,C))
			{
				write_stackw(cpu,cpu->pc+2);
				cpu->pc = read_word(cpu->pc,cpu);
				return mtcycles[opcode];
			}
			
			else
			{
				cpu->pc += 2;
			}
			break;
		
		case 0xde: // sbc a, n
			cpu->af.hb = sbc(cpu,cpu->af.hb,read_mem(cpu->pc++,cpu));
			break;
		
		
		case 0xdf: // rst 18
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x18;
			break;
		
		case 0xE0: // ld (ff00+n),a
			write_mem(cpu,0xff00+read_mem(cpu->pc++,cpu),cpu->af.hb);
			break;

		case 0xe1: // pop hl
			cpu->hl.reg = read_stackw(cpu);
			break;
			
		case 0xE2: // LD ($FF00+C),A
			write_mem(cpu,0xff00 + cpu->bc.lb, cpu->af.hb);
			break;

		case 0xe5: // push hl
			write_stackw(cpu, cpu->hl.reg);
			break;
		

		case 0xe6: // and a, n
			and(cpu, read_mem(cpu->pc++,cpu));
			break;
		
		case 0xe7: // rst 20
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x20;
			break;
		
		case 0xe8: // add sp, i8 <--- verify
			cpu->sp = addi(cpu,cpu->sp, ((int8_t)read_mem(cpu->pc++,cpu)) );
			break;
		
		case 0xe9: // jp hl
			cpu->pc = cpu->hl.reg;
			break;
		
		case 0xea: // ld (nnnn), a
			write_mem(cpu,read_word(cpu->pc,cpu),cpu->af.hb);
			cpu->pc += 2;
			break;
		
		case 0xee: // xor a, nn
			xor(cpu,read_mem(cpu->pc++,cpu));
			break;
		
		case 0xef: // rst 28
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x28;
			break;
		
		case 0xF0: // ld a, (ff00+n)
			cpu->af.hb = read_mem(0xff00+read_mem(cpu->pc++, cpu),cpu);
			break;
		
		case 0xf1: // pop af
			cpu->af.reg = read_stackw(cpu);
			cpu->af.reg &= 0xfff0; // mask bottom 4 bits at they go unused 
			break;
		
		case 0xf2: // ld a, (ff00+c)
			cpu->af.hb = read_mem(0xff00 + cpu->bc.lb ,cpu);
			break;
		
		case 0xf3: // disable interrupt
			// needs to be executed after the next instr
			// main routine will handle
			cpu->di = true;
			break;
		
		case 0xf5: // push af
			write_stackw(cpu,cpu->af.reg);
			break;
		
		case 0xf6: // or a, nn
			or(cpu, read_mem(cpu->pc++,cpu));
			break;
		
		case 0xf7: // rst 30
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x30;
			break;
		
		case 0xf8: // ld hl, sp + i8 <--- verify 
			cpu->hl.reg = addi(cpu,cpu->sp,(int8_t)read_mem(cpu->pc++,cpu));
			break;
		
		case 0xf9: // ld sp, hl
			cpu->sp = cpu->hl.reg;
			break;
		
		case 0xfa: // ld a (nn) <-- 16 bit address
			//cpu->af.hb = read_mem(cpu->mem,read_word(cpu->mem,cpu->pc));
			cpu->af.hb = read_mem(read_word(cpu->pc,cpu),cpu);
			cpu->pc += 2;
			break;
		
		case 0xfb: 
			// caller will check opcode and handle it
			cpu->ei = true;
			break;
		
		case 0xFE: // cp a, n (do a sub and discard result)
			sub(cpu,cpu->af.hb,read_mem(cpu->pc++,cpu));
		//	cpu_state(cpu);
		//	print_flags(cpu); exit(1);
			break;
			
		
		case 0xff: // rst 38
			write_stackw(cpu,cpu->pc);
			cpu->pc = 0x38;
			break;
		
		default:
			fprintf(stderr, "[cpu] Unknown opcode: %x\n", opcode);
			cpu_state(cpu);
			print_flags(cpu);
			for(;;) { }
			//exit(1);
	}
	
    return mcycles[opcode]; // update the machine cycles		
}