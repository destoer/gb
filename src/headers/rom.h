#pragma once
#include <stdbool.h>

typedef struct
{
	int noRamBanks;
	int noRomBanks;
	int cartType;
	bool mbc1;
	bool mbc2;
} RomInfo;

RomInfo parse_rom(uint8_t *rom);
uint8_t *load_rom(char *filename); // read rom out file