a gameboy and gameboy color emulator written in C
using sdl2 for sound and graphics output

![alt text](https://raw.githubusercontent.com/destoer/gb/master/pics/red.png)
![alt text](https://raw.githubusercontent.com/destoer/gb/master/pics/zelda.PNG)
![alt text](https://raw.githubusercontent.com/destoer/gb/master/pics/crystal.PNG)
![alt text](https://raw.githubusercontent.com/destoer/gb/master/pics/image.png)

# Building:
  
  git clone https://github.com/destoer/gb.git
  
  cd gb
  
  mkdir obj
  
  mkdir "obj/src"
  
  make

# Usage:

emu.exe <path to rom>

Controls A,S,SPACE,ARROW KEYS,ENTER

## Debugger: (compile with -DDEBUG)
0 to save state

9 to load state

p to enter debugger while game is running

l to fast word

commands:
info, break, disass, step, run
