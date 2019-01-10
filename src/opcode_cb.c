#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/disass.h"
#include <stdio.h>

void decode_cb(uint8_t cbop, Cpu *cpu)
{
	
			
			// figure out instruction bits and decode fields
			// unless u want 100s of switch statements 
			// ^ not going with above may resort to later
			switch(cbop)
			{
				
				case 0x0: // rlc b
					cpu->bc.hb = rlc(cpu,cpu->bc.hb);
					break;
					
				case 0x1: // rlc c
					cpu->bc.lb = rlc(cpu,cpu->bc.lb);
					break;
				
				case 0x2: // rlc d
					cpu->de.hb = rlc(cpu,cpu->de.hb);
					break;
					
				case 0x3: // rlc e
					cpu->de.lb = rlc(cpu,cpu->de.lb);
					break;
					
				case 0x4: // rlc h
					cpu->hl.hb = rlc(cpu,cpu->hl.hb);
					break;
					
				case 0x5: // rlc l
					cpu->hl.lb = rlc(cpu,cpu->hl.lb);
					break;
					
				case 0x6: // rlc (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = rlc(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
					
				case 0x7: // rlc a
					cpu->af.hb = rlc(cpu,cpu->af.hb);
					break;
				
				case 0x8: // rrc b
					cpu->bc.hb = rrc(cpu,cpu->bc.hb);
					break;
					
				case 0x9: // rrc c
					cpu->bc.lb = rrc(cpu,cpu->bc.lb);
					break;
					
				case 0xa: // rrc d
					cpu->de.hb = rrc(cpu,cpu->de.hb);
					break;
					
				case 0xb: // rrc e
					cpu->de.lb = rrc(cpu,cpu->de.lb);
					break;
				
				case 0xc: // rrc h
					cpu->hl.hb = rrc(cpu,cpu->hl.hb);
					break;
					
				case 0xd: // rrc l
					cpu->hl.lb = rrc(cpu,cpu->hl.lb);
					break;
				
				case 0xe: // rrc (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = rrc(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xf: // rrc a
					cpu->af.hb = rrc(cpu,cpu->af.hb);
					break;
					
				case 0x10: // rl b
					cpu->bc.hb = rl(cpu,cpu->bc.hb);
					break;
				
				case 0x11: // rl c (rotate left)
					cpu->bc.lb = rl(cpu,cpu->bc.lb);
					break;
				
				case 0x12: // rl d
					cpu->de.hb = rl(cpu,cpu->de.hb);
					break;
					
				case 0x13: // rl e
					cpu->de.lb = rl(cpu,cpu->de.lb);
					break;
					
				case 0x14: // rl h 
					cpu->hl.hb = rl(cpu,cpu->hl.hb);
					break;
					
				case 0x15: // rl l
					cpu->hl.lb = rl(cpu,cpu->hl.lb);
					break;
				
				case 0x16: // rl (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = rl(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x17: // rl a
					cpu->af.hb = rl(cpu,cpu->af.hb);
					break;
				
				case 0x18: // rr b
					cpu->bc.hb = rr(cpu,cpu->bc.hb);
					break;
				
				case 0x19: // rr c
					cpu->bc.lb = rr(cpu,cpu->bc.lb);
					break;
					
				case 0x1a: // rr d
					cpu->de.hb = rr(cpu,cpu->de.hb);
					break;
				
				case 0x1b: // rr e 
					cpu->de.lb = rr(cpu,cpu->de.lb);
					break;
				
				case 0x1c: // rr h
					cpu->hl.hb = rr(cpu,cpu->hl.hb);
					break;
				
				case 0x1d: // rr l
					cpu->hl.lb = rr(cpu,cpu->hl.lb);
					break;
				
				case 0x1e: // rr (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = rr(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x1f: // rr a
					cpu->af.hb = rr(cpu,cpu->af.hb);
					break;
				
				
				case 0x20: // sla b
					cpu->bc.hb = sla(cpu,cpu->bc.hb);
					break;
					
				case 0x21: // sla c
					cpu->bc.lb = sla(cpu,cpu->bc.lb);
					break;
				
				case 0x22: // sla d
					cpu->de.hb = sla(cpu,cpu->de.hb);
					break;
				
				case 0x23: // sla e
					cpu->de.lb = sla(cpu,cpu->de.lb);
					break;
					
				case 0x24: // sla h
					cpu->hl.hb = sla(cpu,cpu->hl.hb);
					break;
					
				case 0x25: // sla l
					cpu->hl.lb = sla(cpu,cpu->hl.lb);
					break;
				
				case 0x26: // sla (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = sla(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x27: // sla a
					cpu->af.hb = sla(cpu,cpu->af.hb);
					break;
				
				case 0x28: // sra b
					cpu->bc.hb = sra(cpu,cpu->bc.hb);
					break;
					
				case 0x29: // sra c
					cpu->bc.lb = sra(cpu,cpu->bc.lb);
					break;
				
				case 0x2a: // sra d
					cpu->de.hb = sra(cpu,cpu->de.hb);
					break;
					
				case 0x2b: // sra e
					cpu->de.lb = sra(cpu,cpu->de.lb);
					break;
					
				case 0x2c: // sra h
					cpu->hl.hb = sra(cpu,cpu->hl.hb);
					break;
					
				case 0x2d: // sra l
					cpu->hl.lb = sra(cpu,cpu->hl.lb);
					break;
				
				case 0x2e: // sra (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = sra(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
					
				case 0x2f: // sra a
					cpu->af.hb = sra(cpu,cpu->af.hb);
					break;
				
				case 0x30: // swap b
					cpu->bc.hb = swap(cpu,cpu->bc.hb);
					break;
					
				case 0x31: // swap c
					cpu->bc.lb = swap(cpu,cpu->bc.lb);
					break;
					
				case 0x32: // swap d
					cpu->de.hb = swap(cpu,cpu->de.hb);
					break;
				
				case 0x33: // swap e
					cpu->de.lb = swap(cpu,cpu->de.lb);
					break;
				
				case 0x34: // swap h
					cpu->hl.hb = swap(cpu,cpu->hl.hb);
					break;
					
				case 0x35: // swap l
					cpu->hl.lb = swap(cpu,cpu->hl.lb);
					break;
					
				case 0x36: // swap (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = swap(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x37: // swap a 
					cpu->af.hb = swap(cpu,cpu->af.hb);
					break;
				
				case 0x38: // srl b
					cpu->bc.hb = srl(cpu,cpu->bc.hb);
					break;
				
				case 0x39: // srl c
					cpu->bc.lb = srl(cpu,cpu->bc.lb);
					break;
				
				case 0x3a: // srl d
					cpu->de.hb = srl(cpu,cpu->de.hb);
					break;
					
				case 0x3b: // srl e
					cpu->de.lb = srl(cpu,cpu->de.lb);
					break;
					
				case 0x3c: // srl h
					cpu->hl.hb = srl(cpu,cpu->hl.hb);
					break;
					
				case 0x3d: // srl l
					cpu->hl.lb = srl(cpu,cpu->hl.lb);
					break;
				
				case 0x3e: // srl (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					cbop = srl(cpu,cbop);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x3f: // srl a
					cpu->af.hb = srl(cpu,cpu->af.hb);
					break;
				
				case 0x40: // bit 0, b
					bit(cpu,cpu->bc.hb,0);
					break;
				
				case 0x41: // bit 0, c <--- just before the glitched screen
					bit(cpu,cpu->bc.lb,0);
					break;
				
				case 0x42: // bit 0, d
					bit(cpu,cpu->de.hb,0);
					break;
				
				case 0x43: // bit 0, e
					bit(cpu,cpu->de.lb,0);
					break;
				
				case 0x44: // bit 0, h
					bit(cpu,cpu->hl.hb,0);
					break;
					
				case 0x45: // bit 0, l
					bit(cpu,cpu->hl.lb,0);
					break;
					
				
				case 0x46: // bit 0, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					bit(cpu,cbop,0);
					break;
				
				case 0x47: // bit 0, a
					bit(cpu,cpu->af.hb,0);
					break;
				
				case 0x48: // bit l, b  <--- just before the glitch print
					bit(cpu,cpu->bc.hb,1);
					break;
				
				case 0x49: // bit 1, c
					bit(cpu,cpu->bc.lb,1);
					break;
				
				case 0x4a: // bit 1, d
					bit(cpu,cpu->de.hb,1);
					break;
				
				case 0x4b: // bit 1, e
					bit(cpu,cpu->de.lb,1);
					break;
				
				case 0x4c: // bit 1,  h
					bit(cpu,cpu->hl.hb,1);
					break;
					
				case 0x4d: // bit 1, l
					bit(cpu,cpu->hl.lb,1);
					break;
				
				case 0x4e: // bit 1, (hl)
					bit(cpu,read_mem(cpu->hl.reg,cpu),1);
					break;
				
				case 0x4f: // bit 1, a
					bit(cpu,cpu->af.hb,1);
					break;
				
				case 0x50: // bit 2, b
					bit(cpu,cpu->bc.hb,2);
					break;
					
				case 0x51: // bit  2, c 
					bit(cpu,cpu->bc.lb,2);
					break;	
					
				case 0x52: // bit 2, d 
					bit(cpu,cpu->de.hb,2);
					break;
				
				case 0x53: // bit 2 , e  
					bit(cpu,cpu->de.lb,2);
					break;
				
				case 0x54: // bit 2, h  
					bit(cpu,cpu->hl.hb,2);
					break;
				
				case 0x55: // bit 2, l
					bit(cpu,cpu->hl.lb,2);
					break;
				
				case 0x56: // bit 2, (hl) 
					bit(cpu,read_mem(cpu->hl.reg,cpu),2);
					break;
				
				case 0x57: // bit 2, a
					bit(cpu,cpu->af.hb,2);
					break;
				
				
				case 0x58: // bit 3, b
					bit(cpu,cpu->bc.hb,3);
					break;
				
				case 0x59: // bit 3, c  
					bit(cpu,cpu->bc.lb,3);
					break;
				
				case 0x5a: // bit 3, d 
					bit(cpu,cpu->de.hb,3);
					break;
				
				case 0x5b: // bit 3, e 
					bit(cpu,cpu->de.lb,3);
					break;
					
				case 0x5c: // bit 3, h 
					bit(cpu,cpu->hl.hb,3);
					break;	
				
				case 0x5d: // bit 3, l
					bit(cpu,cpu->hl.lb,3);
					break;
				
				case 0x5e: // bit 3, (hl)
					bit(cpu,read_mem(cpu->hl.reg,cpu),3);
					break;
				
				
				case 0x5f: // bit 3, a
					bit(cpu,cpu->af.hb,3);
					break;
				
				case 0x60: // bit 4, b
					bit(cpu,cpu->bc.hb,4);
					break;
			

			
				case 0x61: // bit 4, c
					bit(cpu,cpu->bc.lb,4);
					break;
				
				case 0x62: // bit  4, d 
					bit(cpu,cpu->de.hb,4);
					break;
				
				case 0x63: // bit 4, e 
					bit(cpu,cpu->de.lb ,4);
					break;
				
				case 0x64: // bit 4, h 
					bit(cpu,cpu->hl.hb,4);
					break;
				
				case 0x65: // bit 4, l
					bit(cpu,cpu->hl.lb,4);
					break;
				
				case 0x66: // bit 4, (hl) 
					bit(cpu,read_mem(cpu->hl.reg,cpu),4);
					break;
				
				case 0x67: // bit 4, a  
					bit(cpu,cpu->af.hb,4);
					break;
				
				case 0x68: // bit 5, b
					bit(cpu,cpu->bc.hb,5);
					break;
				
				case 0x69: // bit 5, c 
					bit(cpu,cpu->bc.lb,5);
					break;
				
				case 0x6a: //bit 5, d 
					bit(cpu,cpu->de.hb,5);
					break;
					
				case 0x6b: // bit 5, e  
					bit(cpu,cpu->de.lb,5);
					break;
				
				case 0x6c: // bit 5, h 
					bit(cpu,cpu->hl.hb,5);
					break;
				
				
				case 0x6d: // bit 5, l
					bit(cpu,cpu->hl.lb,5);
					break;
				
				case 0x6e: // bit 5, (hl) 
					bit(cpu,read_mem(cpu->hl.reg,cpu),5);
					break;
				
				case 0x6f: // bit 5, a
					bit(cpu,cpu->af.hb,5);
					break;
				
				case 0x70: // bit 6, b 
					bit(cpu,cpu->bc.hb,6);
					break;
				
				case 0x71: // bit 6, c 
					bit(cpu,cpu->bc.lb,6);
					break;
				
				case 0x72: // bit 6, d  
					bit(cpu,cpu->de.hb,6);
					break;
				
				case 0x73: // bit  6, e
					bit(cpu,cpu->de.lb,6);
					break;

				case 0x74: // bit 6, h  
					bit(cpu,cpu->hl.hb,6);
					break;	
					
				case 0x75: // bit 6, l 
					bit(cpu,cpu->hl.lb,6);
					break;
				
				case 0x76: // bit 6, (hl) 
					bit(cpu,read_mem(cpu->hl.reg,cpu),6);
					break;
				
				case 0x77: // bit 6, a
					bit(cpu,cpu->af.hb,6);
					break;
					
				case 0x78: // bit  7, b
					bit(cpu,cpu->bc.hb,7);
					break;
				
				case 0x79: // bit 7, c  
					bit(cpu,cpu->bc.lb,7);
					break;
				
				case 0x7a: // bit 7, d  
					bit(cpu,cpu->de.hb,7);
					break;
				
				case 0x7b: // bit bit 7, e  
					bit(cpu,cpu->de.lb,7);
					break;
				
				case 0x7c: // bit 7, h
					bit(cpu, cpu->hl.hb, 7);
					break;
				
				case 0x7d: // bit 7, l
					bit(cpu,cpu->hl.lb,7);
					break;
			
			
				case 0x7e: // bit 7, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					bit(cpu,cbop,7);
					break;
					
				case 0x7f: // bit 7, a
					bit(cpu,cpu->af.hb, 7);
					break;
			
			
				case 0x80: // res 0, b
					deset_bit(cpu->bc.hb,0);
					break;
			
				case 0x81: // res  0, c 
					deset_bit(cpu->bc.lb,0);
					break;
					
				case 0x82: // res 0, d  
					deset_bit(cpu->de.hb,0);
					break;	
				
				case 0x83: // res 0, e  
					deset_bit(cpu->de.lb,0);
					break;
				
				case 0x84: // res 0, h 
					deset_bit(cpu->hl.hb,0);
					break;

				case 0x85: // res 0, l  
					deset_bit(cpu->hl.lb,0);
					break;	
					
				case 0x86: // res 0, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,0);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x87: //res 0,a
					deset_bit(cpu->af.hb,0);
					break;
				
				case 0x88: // res 1 ,b
					deset_bit(cpu->bc.hb,1);
					break;
				
				case 0x89: // res 1, c
					deset_bit(cpu->bc.lb,1);
					break;
					
				case 0x8a: // res 1, d  
					deset_bit(cpu->de.hb,1);
					break;
					
				case 0x8b: // res 1, e
					deset_bit(cpu->de.lb,1);
					break;
				
				
				case 0x8c: // res  1, h  
					deset_bit(cpu->hl.hb,1);
					break;
				
				case 0x8d: // res 1, l
					deset_bit(cpu->hl.lb,1);
					break;
				
				
				case 0x8e: // res 1, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,1);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x8f: // res 1, a 
					deset_bit(cpu->af.hb,1);
					break;
				
				case 0x90: // res 2, b
					deset_bit(cpu->bc.hb,2);
					break;
				
				case 0x91: // res 2, c  
					deset_bit(cpu->bc.lb,2);
					break;
				
				case 0x92: // res 2, d  
					deset_bit(cpu->de.hb,2);
					break;
				
				case 0x93: // res 2, e 
					deset_bit(cpu->de.lb,2);
					break;
				
				case 0x94: // res 2, h  
					deset_bit(cpu->hl.hb,2);
					break;
				
				case 0x95: // res 2, l  
					deset_bit(cpu->hl.lb,2);
					break;
				
				case 0x96: // res 2, (hl) 
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,2);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0x97: // res 2, a  
					deset_bit(cpu->af.hb,2);
					break;
				 
				case 0x98: // res 3, b
					deset_bit(cpu->bc.hb,3);
					break;
				
				case 0x99: // res 3, c 
					deset_bit(cpu->bc.lb,3);
					break;
				
				
				case 0x9a: // res 3, d 
					deset_bit(cpu->de.hb,3);
					break;
				
				case 0x9b: // res 3, e  
					deset_bit(cpu->de.lb,3);
					break;
				
				case 0x9c: // res 3, h 
					deset_bit(cpu->hl.hb,3);
					break;
				
				case 0x9d: // res 3, l
					deset_bit(cpu->hl.lb,3);
					break;
				

				
				case 0x9e: // res 3, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,3);
					write_mem(cpu,cpu->hl.reg,cbop);
					break; 				
				
				case 0x9f: // res 3, a
					deset_bit(cpu->af.hb,3);
					break;
				
				case 0xa0: // res 4, b 
					deset_bit(cpu->bc.hb,4);
					break;
				
				case 0xa1: // res 4, c  
					deset_bit(cpu->bc.lb,4);
					break;
				
				case 0xa2: // res  4, d 
					deset_bit(cpu->de.hb,4);
					break;
				
				case 0xa3: // res 4, e 
					deset_bit(cpu->de.lb,4);
					break;
				
				case 0xa4: // res 4, h  
					deset_bit(cpu->hl.hb,4);
					break;
				
				case 0xa5: // res 4, l  
					deset_bit(cpu->hl.lb,4);
					break;
					
				case 0xa6: // res 4, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,4);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xa7: // res 4, a 
					deset_bit(cpu->af.hb,4);
					break;
				
				case 0xa8: // res 5, b  
					deset_bit(cpu->bc.hb,5);
					break;
				
				case 0xa9: // res 5, c  
					deset_bit(cpu->bc.lb,5);
					break;
				
				case 0xaa: // res 5, d  
					deset_bit(cpu->de.hb,5);
					break;
				
				case 0xab: // res 5, e
					deset_bit(cpu->de.lb,5);
					break;
				
				case 0xac: // res 5, h 
					deset_bit(cpu->hl.hb,5);
					break;
				
				case 0xad: // res 5, l
					deset_bit(cpu->hl.lb,5);
					break;
				
				case 0xae: // res 5, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,5);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xaf: // res 5, a 
					deset_bit(cpu->af.hb,5);
					break;
				
				case 0xb0: // res 6, b  
					deset_bit(cpu->bc.hb,6);
					break;
				
				case 0xb1: // res 6, c  
					deset_bit(cpu->bc.lb,6);
					break;
				
				case 0xb2: // res 6, d 
					deset_bit(cpu->de.hb,6);
					break;
				
				case 0xb3: // res 6, e  
					deset_bit(cpu->de.lb,6);
					break;
				
				case 0xb4: // res 6, h
					deset_bit(cpu->hl.hb,6);
					break;
				
				case 0xb5: // res 6, l 
					deset_bit(cpu->hl.lb,6);
					break;
				
				case 0xb6: // res 6, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,6);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xb7: // res 6,a 
					deset_bit(cpu->af.hb,6);
					break;
				
				case 0xb8: // res 7, b  
					deset_bit(cpu->bc.hb,7);
					break;
				
				case 0xb9: // res 7, c 
					deset_bit(cpu->bc.lb,7);
					break;
					
				case 0xba: // res 7, d 
					deset_bit(cpu->de.hb,7);
					break;
					
				case 0xbb: // res 7, e
					deset_bit(cpu->de.lb,7);
					break;
				
				case 0xbc: // res 7, h  
					deset_bit(cpu->hl.hb,7);
					break;
				
				case 0xbd: // res 7, l  
					deset_bit(cpu->hl.lb,7);
					break;
				
				case 0xbe: // res 7, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					deset_bit(cbop,7);
					write_mem(cpu,cpu->hl.reg,cbop);
					break; 
				
				case 0xbf: // res 7, a  
					deset_bit(cpu->af.hb,7);
					break;
				
				case 0xc0: // set 0, b
					set_bit(cpu->bc.hb,0);
					break;
				
				case 0xc1: // set 0,c 
					set_bit(cpu->bc.lb,0);
					break;
				
				case 0xc2: // set 0,d 
					set_bit(cpu->de.hb,0);
					break;
					
				case 0xc3: // set 0,e
					set_bit(cpu->de.lb,0);
					break;
				
				case 0xc4: // set 0, h 
					set_bit(cpu->hl.hb,0);
					break;
				
				case 0xc5: // set 0, l
					set_bit(cpu->hl.lb,0);
					break;
				
				case 0xc6: // set 0, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,0);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xc7: // set 0, a 
					set_bit(cpu->af.hb,0);
					break;
				
				case 0xc8: // set 1, b 
					set_bit(cpu->bc.hb,1);
					break;
				
				case 0xc9: // set 1, c 
					set_bit(cpu->bc.lb,1);
					break;
				
				case 0xca: // set 1, d 
					set_bit(cpu->de.hb,1);
					break;
				
				case 0xcb: // set  1, e 
					set_bit(cpu->de.lb ,1);
					break;
				
				case 0xcc: // set 1, h 
					set_bit(cpu->hl.hb,1);
					break;
				
				case 0xcd: // set l, l
					set_bit(cpu->hl.lb,1);
					break;
				
				case 0xce: // set 1, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,1);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xcf: // set 1, a 
					set_bit(cpu->af.hb,1);
					break;
				
				case 0xd0: // set 2 ,b
					set_bit(cpu->bc.hb,2);
					break;
				
				case 0xd1: // set 2, c 
					set_bit(cpu->bc.lb,2);
					break;
				
				case 0xd2: // set 2, d 
					set_bit(cpu->de.hb,2);
					break;
				
				case 0xd3: // set 2 , e 
					set_bit(cpu->de.lb,2);
					break;
				
				case 0xd4: // set 2, h 
					set_bit(cpu->hl.hb,2);
					break;
				
				case 0xd5: // set 2, l
					set_bit(cpu->hl.lb,2);
					break;
					
				case 0xd6: // set 2, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,2);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xd7: // set 2 a 
					set_bit(cpu->af.hb,2);
					break;
					
				case 0xd8: // set 3, b
					set_bit(cpu->bc.hb,3);
					break;
				
				case 0xd9: // set 3, c 
					set_bit(cpu->bc.lb,3);
					break;
				
				case 0xda: // set 3, d 
					set_bit(cpu->de.hb,3);
					break;
				
				case 0xdb: // set 3, e
					set_bit(cpu->de.lb,3);
					break;
				
				case 0xdc: // set 3, h
					set_bit(cpu->hl.hb,3);
					break;
				
				case 0xdd: // set 3, l
					set_bit(cpu->hl.lb,3);
					break;
				
				case 0xde: // set 3, (hl)	
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,3);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xdf: // set 3, a 
					set_bit(cpu->af.hb,3);
					break;
				
				case 0xe0: // set 4, b 
					set_bit(cpu->bc.hb,4);
					break;
				
				case 0xe1: // set 4, c
					set_bit(cpu->bc.lb,4);
					break;
					
				case 0xe2: // set 4, d 
					set_bit(cpu->de.hb,4);
					break;
				
				case 0xe3: // set 4, e 
					set_bit(cpu->de.lb,4);
					break;
				
				case 0xe4: // set 4 , h 
					set_bit(cpu->hl.hb,4);
					break;
				
				case 0xe5: // set 4 , l 
					set_bit(cpu->hl.lb,4);
					break;
				
				case 0xe6: // set 4, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,4);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xe7: // set 4, a 
					set_bit(cpu->af.hb,4);
					break;
				
				case 0xe8: // set 5, b 
					set_bit(cpu->bc.hb,5);
					break;
				
				case 0xe9: // set 5, c 
					set_bit(cpu->bc.lb,5);
					break;
				
				case 0xea: //set 5, d
					set_bit(cpu->de.hb,5);
					break;
				
				case 0xeb: // set 5, e 
					set_bit(cpu->de.lb,5);
					break;
				
				case 0xec: // set 5, h 
					set_bit(cpu->hl.hb,5);
					break;
				
				case 0xed: // set 5 , l
					set_bit(cpu->hl.lb,5);
					break;
				
				case 0xee: // set 5, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,5);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xef: // set 5, a
					set_bit(cpu->af.hb,5);
					break;
				
				case 0xf0: // set 6, b 
					set_bit(cpu->bc.hb,6);
					break;
				
				case 0xf1: // set 6, c 
					set_bit(cpu->bc.lb,6);
					break;
				
				case 0xf2: // set 6, d 
					set_bit(cpu->de.hb,6);
					break;
				
				case 0xf3: // set 6, e 
					set_bit(cpu->de.lb,6);
					break;
				
				case 0xf4: // set 6, h 
					set_bit(cpu->hl.hb,6);
					break;
				
				case 0xf5: // set 6, l
					set_bit(cpu->hl.lb,6);
					break;
				
				case 0xf6: // set 6, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,6);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xf7: // set 6, a 
					set_bit(cpu->af.hb,6);
					break;
				
				case 0xf8: // set 7, b  
					set_bit(cpu->bc.hb,7);
					break;
					
				case 0xf9: // set 7, c 	
					set_bit(cpu->bc.lb,7);
					break;
					
				case 0xfa: // set 7, d 
					set_bit(cpu->de.hb,7);
					break;	
				
				case 0xfb: // set 7 , e 
					set_bit(cpu->de.lb,7);
					break;
				
				case 0xfc: // set 7, h
					set_bit(cpu->hl.hb,7);
					break;
				
				case 0xfd: // set 7, l
					set_bit(cpu->hl.lb,7);
					break;
				
				case 0xfe: // set 7, (hl)
					cbop = read_mem(cpu->hl.reg,cpu);
					set_bit(cbop,7);
					write_mem(cpu,cpu->hl.reg,cbop);
					break;
				
				case 0xff: // set 7,a 
					set_bit(cpu->af.hb,7);
					break;
				
				
				default:
					fprintf(stderr, "[cpu] Unknown CB opcode: %x\n", cbop);
					cpu_state(cpu);
					print_flags(cpu);
					for(;;) {}
			}
}