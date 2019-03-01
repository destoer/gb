clang -g  -O3  -Wextra -Wshadow  -Wall -Wpointer-arith -Wformat=2 -Wcast-qual src/rom.c src/memory.c src/debug.c src/opcode.c src/opcode_cb.c src/instr.c src/banking.c src/lib.c src/main.c src/cpu.c src/disass.c  src/ppu.c src/joypad.c -lSDL2main -lSDL2 -o emu
#-fsanitize=address
#-Wshadow
#-Wconversion
