#pragma once
#include <stdbool.h>

typedef struct
{
	int noRamBanks;
	int noRomBanks;
	int cartType;
	char filename[21]; // filename of rom
	bool mbc1;
	bool mbc2;
	bool mbc3;
	bool mbc5;
	bool has_rtc;
} RomInfo;

RomInfo parse_rom(uint8_t *rom);
uint8_t *load_rom(char *filename); // read rom out file
