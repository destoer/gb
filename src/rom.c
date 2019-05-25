#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "headers/rom.h"

uint8_t *load_rom(char *filename)
{
	// read in the rom
	FILE *fp = fopen(filename, "rb");
	
	if(fp == NULL)
	{
		fprintf(stderr, "Failed to open file: %s",filename);
		exit(1);
	}

	fseek(fp, 0, SEEK_END); // get file size and set it to len
	int len = ftell(fp); // get length of file
	printf("Rom length is %d (bytes)\n", len );
	rewind(fp); // go to start of file
	
	
	uint8_t *rom = malloc(len * sizeof(uint8_t));
	
	fread(rom, sizeof(uint8_t), len, fp); // load file into array buffer
	fclose(fp); // done with rom now

	return rom;
}


RomInfo parse_rom(uint8_t *rom)
{
	RomInfo romInfo;
	memset(&romInfo,0,sizeof(RomInfo));
	

	// should probably add handling to make this work for cgb
	// and to store it somewhere
	printf("Internal name: %s\n",&rom[0x134]); // print the name of the rom
	
	romInfo.cartType = rom[0x147]; // make a constant list later along with string array
	printf("Cart type: %x\n",romInfo.cartType);
	
	
	romInfo.mbc1 = false;
	romInfo.mbc2 = false;
	romInfo.mbc3 = false;
	romInfo.has_rtc = false;
	switch(romInfo.cartType)
	{
		case 0: puts("rom only"); break;
		case 1 ... 3: romInfo.mbc1 = true; puts("mbc1"); break;
		case 5 ... 6: romInfo.mbc2 = true; puts("mbc2"); break;
		case 10: romInfo.mbc3 = true; puts("mbc3"); romInfo.has_rtc = true;  break;
		case 0x1b: romInfo.mbc5 = true; puts("mbc5"); break; // is mbc5 but we will just run like this and ignore it for now
		default: romInfo.mbc3 = true; puts("mbc3"); break; // should be handled properly later
	}
	
	
	// get the number of rom banks
	int banks = rom[0x148];
	
	switch(banks)
	{
		case 0: romInfo.noRomBanks = 2; break;
		case 1: romInfo.noRomBanks = 4; break;
		case 2: romInfo.noRomBanks = 8; break;
		case 3: romInfo.noRomBanks = 16; break;
		case 4: romInfo.noRomBanks = 32; break;
		case 5: romInfo.noRomBanks = 64; break;
		case 6: romInfo.noRomBanks = 128; break;
		case 7: romInfo.noRomBanks = 256; break;
		case 0x52: romInfo.noRomBanks = 72; break;
		case 0x53: romInfo.noRomBanks = 80; break;
		case 0x54: romInfo.noRomBanks = 96; break;
	
		default:
			fprintf(stderr, "Unrecognized number of rom banks: %x\n",banks);
			exit(1);
	}
	


	printf("Number of rom banks: %d\n",romInfo.noRomBanks);
	
	
	banks = rom[0x149];
	
	switch(banks)
	{
		case 0: romInfo.noRamBanks = 0; break;
		case 1: romInfo.noRamBanks = 1; break;
		case 2: romInfo.noRamBanks = 1; break;
		case 3: romInfo.noRamBanks = 4; break;
		case 4: romInfo.noRamBanks = 16; break;
		
		default:
			fprintf(stderr, "Unrecognized number of ram banks");
			exit(1);
		
	}

	// hack to get mbc2 ram working <-- needs work
	if(romInfo.mbc2)
	{ 
		romInfo.noRamBanks = 1;
	}

	printf("Number of ram banks: %d\n",romInfo.noRamBanks);
	


	return romInfo;
	
}
