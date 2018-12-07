#pragma once

typedef struct
{
	int noRamBanks;
	int noRomBanks;
	int cartType;
} RomInfo;

RomInfo parse_rom(uint8_t *rom);
uint8_t *load_rom(char *filename); // read rom out file