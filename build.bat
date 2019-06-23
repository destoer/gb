gcc  -g -O3   -Wextra  -Wall -DDEBUG -DSOUND -DCGB src/apu.c src/gb.c src/memory.c src/rom.c src/debug.c src/opcode.c src/opcode_cb.c src/instr.c src/banking.c src/lib.c src/main.c src/cpu.c src/disass.c  src/ppu.c src/joypad.c -lmingw32 -lSDL2main -lSDL2 -o emu

