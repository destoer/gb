#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/memory.h"




// <-- get lyc_onoff test passing

// <-- consider caching ly as it is read alot and memory writes do not affect it....

// need to get the ppu timings passing (at the very least ones that dont use scx as it may be the source of issues)
// mooneye-gb_hwtests\acceptance\ppu\intr_2_0_timing.gb appears to behave VERY strangely...

// after sound the fetcher + pixel fifo needs implementing 
// if this doesent fix screen tears look elsewhere....

//add line 154 behaviour where it hits end of 153 but then waits an extra 114 cycles

// our new screen driver implementation causes screen tearing in Pokemon
// the current one on github where we dont just handle signals on mode changes
// also outdated from the linux one so merge with it

// metroid 2 final boss



// run mooneye ppu tests against it
// get lyc test passing <-- 236 for one round 3
// hits mode 2 too early should it block the re-enable?  for 1m-cycle



// gold locking up waiting for a vblank interrupt if left alone for too long ?
// get the vblank test passing 


// Metroid 2 (both versions) and gold ( with the signal code) have horrible screen tears
// and links awakening (during scrolls) + more 
int get_colour(Cpu *cpu ,uint8_t colour_num, uint16_t address);


#define WHITE 1
#define LIGHT_GRAY 2
#define DARK_GRAY 3
#define BLACK 4


void draw_scanline(Cpu *cpu, int cycles);
void tile_fetch(Cpu *cpu);
bool sprite_fetch(Cpu *cpu);
void set_lcd_status(Cpu *cpu);
int is_sprite_present(Cpu *cpu);

// probably the largest room for optimization

bool is_lcd_enabled(Cpu *cpu)
{
	return ( is_set(cpu->io[IO_LCDC],7) );	
}


/* old implentation in ppu_signal passing irq test but doesent seem right
   and causes screen tearing in many games */

void update_stat(Cpu *cpu, int cycles)
{
	//-----------------------
	// update the stat reg state
	// and trigger interrupts	
	
	// read out current stat reg and mask the mode
	uint8_t status = cpu->io[IO_STAT];
	int old_mode = status & 0x3;
	status &= ~0x3;
	// mode is one if lcd is disabled
	if(!is_lcd_enabled(cpu)) // <-- should re enable on a delay?
	{
		cpu->scanline_counter = 0; // counter is reset?
		cpu->current_line = 0; // reset ly
		cpu->io[IO_STAT] = status; 
		return; // can exit if ppu is disabled nothing else to do
	}
	





	
	uint8_t mode = 0;
	
	// vblank (mode 1)
	if(cpu->current_line >= 144)
	{
		mode = 1;
	}
	
	
	// else check based on our cycle counter
	// technically this should vary depending on
	// what is being drawn to the screen
	else // <-- test interrupt(if true set signal line) and update the mode
	{
		// oam search (mode 2)
		if(cpu->scanline_counter <= 20)
		{
			mode = 2;
		}
		
		// pixel transfer (mode 3)
		else if(!cpu->hblank) 
		{
			mode = 3;
			
			// we have just left oam search
			if(mode != old_mode)
			{
				read_sprites(cpu);	
			}

			/*
			; Expected behaviour:
			;   (SCX mod 8) = 0   => LY increments 51 cycles after STAT interrupt
			;   (SCX mod 8) = 1-4 => LY increments 50 cycles after STAT interrupt
			; (SCX mod 8) = 5-7 => LY increments 49 cycles after STAT interrupt
			*/
			
			draw_scanline(cpu,cycles);
			if(cpu->hblank)
			{
				mode = 0;
			}
		}
		
		// hblank mode 0
		else
		{ 
			mode = 0; 
		}
	}
	
	// save our current signal state
	bool signal_old = cpu->signal;

	
	switch(mode)
	{
		case 0: cpu->signal = is_set(status,3); break;
		case 1: cpu->signal = is_set(status,4); break;
		case 2: cpu->signal = is_set(status,5); break;
		default: cpu->signal = false; break;
	}
	
	


	
	// check coincidence  (lyc == ly)
	uint8_t lyc = cpu->io[IO_LYC];
	

	// line 153 can be treated as zero 
	// see https://forums.nesdev.com/viewtopic.php?f=20&t=13727
	//if( lyc == ly || (lyc == 0 && ly == 153) )
	if(lyc == cpu->current_line)
	{
		set_bit(status,2); // toggle coincidence bit
	}
	
	else
	{
		deset_bit(status,2); // deset coincidence bit
	}
	
	
	// if the lyc coeincidence is set and interrupt is enabled
	if(is_set(status,6) && is_set(status,2))
	{
		cpu->signal = true;
	}




	// if we have changed from 0 to 1 for signal(signal edge)
	// request a stat interrupt
	if(!signal_old && cpu->signal)
	{
		request_interrupt(cpu,1);	
	}
	
	// update our status reg
	cpu->io[IO_STAT] = status | 128 | mode;
}

void update_graphics(Cpu *cpu, int cycles)
{

	if(!is_lcd_enabled(cpu))
	{
		update_stat(cpu,cycles);	
		return;
	}
	
	//--------------------
	// tick the ppu
	cpu->scanline_counter += cycles; // advance the cycle counter
	
	// update the stat register
	update_stat(cpu,cycles);	
	
	// read out current stat reg
	uint8_t status = cpu->io[IO_STAT];
	
	// save our current signal state
	bool signal_old = cpu->signal;

	

	


	// we have finished drawing a line 
	if(cpu->scanline_counter >= 114) 
	{
		// reset the counter extra cycles should tick over
		cpu->scanline_counter = 0;

		
		
		
		cpu->current_line++;

		
		// reset our fetcher :)
		cpu->x_cord = 0;
		cpu->tile_ready = false;
		cpu->pixel_count = 0;
		cpu->ppu_cyc = 0;
		cpu->hblank = false;
		cpu->tile_cord = 0;
		cpu->window_start = false;
		cpu->ppu_scyc = 0;
		
		// vblank has been entered
		if(cpu->current_line == 144)
		{
			request_interrupt(cpu,0); // vblank interrupt
			
			// edge case oam stat interrupt is triggered here if enabled
			if(is_set(status,5))
			{
				if(signal_old == false)
				{
					request_interrupt(cpu,1);
				}
				cpu->signal = true;
			}
		}
		
		// if past 153 reset ly
		// need to emulate the difference 
		// on the first line properly here
		else if(cpu->current_line > 153)
		{
			cpu->io[IO_STAT] &= ~3;
			cpu->io[IO_STAT] |= 2;
			cpu->current_line = 0;

			if(is_set(status,5))
			{
				if(signal_old == false)
				{
					request_interrupt(cpu,1);
				}
				cpu->signal = true;
			}
		}
	}	
}

void shift_fifo(Cpu *cpu, int shift)
{
	/*for(int j = 0; j < shift; j++)
	{
		for(int i = 1; i < cpu->pixel_count; i++) 
		{
			//printf("%d\n",cpu->ppu_fifo[i]);
			cpu->ppu_fifo[i-1] = cpu->ppu_fifo[i]; // shift the array
		}
		cpu->pixel_count--;
	}*/
	
	
	memmove(cpu->ppu_fifo,&cpu->ppu_fifo[shift],cpu->pixel_count *sizeof(Pixel_Obj));
	cpu->pixel_count -= shift;
}



// shift a pixel out of the array and smash it to the screen 
// increment x afterwards
void push_pixel(Cpu *cpu) 
{

	int col_num = cpu->ppu_fifo[0].colour_num; // save the pixel we will shift
	
	
	int colour_address;
	
	if(cpu->ppu_fifo[0].source == TILE)
	{
		colour_address = 0xff47;
	}
	
	else if(cpu->ppu_fifo[0].source == SPRITE_ONE)
	{
		colour_address = 0xff49;
	}
	
	if(cpu->ppu_fifo[0].source == SPRITE_ZERO)
	{
		colour_address = 0xff48;
	}
	
	shift_fifo(cpu,1); // shift a pixel out by one

	
	
	// read the scanline 
	int scanline = cpu->current_line;
	
	
	int col = get_colour(cpu,col_num,colour_address); 
	int red = 0;
	int green = 0;
	int blue = 0;
	
	switch(col)
	{
		case WHITE: red = 255; green = 255; blue = 255; break;
		case LIGHT_GRAY: red = 0xCC; green = 0xCC; blue = 0xCC;  break;
		case DARK_GRAY: red = 0x77; green = 0x77; blue = 0x77;  break;
	}
	
	
	cpu->screen[scanline][cpu->x_cord][0] = red;
	cpu->screen[scanline][cpu->x_cord][1] = green;
	cpu->screen[scanline][cpu->x_cord][2] = blue;	
	
	cpu->x_cord++; // goto next pixel 
	if(cpu->x_cord >= 160)
	{
		// done drawing enter hblank
		cpu->hblank = true;
	}
}	
	




// first two tiles dont render properly

// now get sprites on teh fetcher 
// need it at the end of oam search 
// pulling the 10 sprites on the line or less 
// and then triggering on them when there is a push attempt
// in the pixel blit :)
// this has been partily been implemented but i high doubt its accuracy

// todo proper scx and window timings
// as we current do not implement them at all 
// window should restart the fetcher when triggered 
// and take 6 cycles (this is now done)

// and we should implement scx properly with the fetcher...
// ^ this needs researching and implementing


void tick_fetcher(Cpu *cpu) {


	// get lcd control reg
	int control = cpu->io[IO_LCDC];
	
	int scanline = cpu->current_line;
	int window_y  = cpu->io[IO_WY];
	int window_x = cpu->io[IO_WX];
	
	// if window has just started completly restart the tile fetcher
	// causes visual bugs currently...
	// maybye this should be done just after the xcord is incremented?
	/*
	if(!cpu->window_start && is_set(control,5))
	{
		// starts now
		if(scanline == window_y && (window_x+7) == cpu->x_cord)
		{
			// reset the fifo
			cpu->window_start = true;
			cpu->ppu_cyc = 0;
			cpu->pixel_count = 0;
			cpu->tile_ready = false; 
		}
	}
	*/
	// advance the fetcher if we dont have a tile dump waiting


	// for now we will ignore the window 
	// if there is 0 we will dump at the start 
	// if there is 8 we will dump in the 2nd half of the array
	
	// takes two cycles for a single operaiton from this compared to 
	// pushing the pixels but we will 
	// ignore that fact for now (this will likely prevent pipeline stalls)
	
	// order may not be right on this to
	// achieve the correct timing 
	if(is_set(control,0) && !cpu->tile_ready) // fetch the tile
	{
		// should fetch the number then low and then high byte
		// but we will ignore this fact for now

		if(cpu->ppu_cyc == 6)
		{
			// not sure if we should dump the fetcher into the fifo by here
			
			// we will however dump say that its ready to be dumped
			tile_fetch(cpu);
			cpu->tile_ready = true;	
			cpu->ppu_cyc = 0;
		}	
	
		// 1 cycle is tile num 
		// 2nd is lb of data 
		// 3rd is high byte of data 
		else if(cpu->ppu_cyc <= 5)
		{
			cpu->ppu_cyc += 1; // further along 
		}
	}
	

	
	// if we have room to dump into the fifo
	// and we are ready to do so, do it now 
	if(cpu->tile_ready && cpu->pixel_count <= 8)
	{
		
		// dump the current thing that is ready into the fifo 

		memcpy(&cpu->ppu_fifo[cpu->pixel_count], cpu->fetcher_tile,8*sizeof(Pixel_Obj));
		cpu->tile_ready = false;
		cpu->pixel_count += 8;
		

		/*uint8_t scx = cpu->io[IO_SCX];
		
		// handle scrolling? cleary wrong...
		// should probably save it when its written too 
		// and decrement out internal scx as we shift it
		
		// i think the shift should be done before sprite mixing is done?...
		
	
		
		for(int i = 0; i < scx; i++)
		{
			if(cpu->pixel_count == 8) { break; }
			shift_fifo(cpu,1);
			
		}
		*/
	}	

	// if the fifo has data in it
	// and is ready to push out  
	// shift it out and onto the screen
	if(cpu->pixel_count > 8)
	{
		// overlay the fifo with a sprite if it is at this posistion 
		// scroll thru our 10 sprites (may need a max sprite count)
		// if it triggers on this x 
		// fetch it by advancing the sprite fetcher
		// and then overlay it with the fifo when done
		
		// on the first cycle determine if we need to fetch it 
		// and if we do 
		// then begin incrementing cycles and reading it out
		
		// sprites are enabled?
		if(is_set(control,1))
		{
			// if we did need to draw a sprite
			if(sprite_fetch(cpu) && !cpu->sprite_drawn)
			{
				// now we should delay 6 cycles
				cpu->ppu_scyc = 0;
				cpu->sprite_drawn = true;
			}
		}
		
		
		if(cpu->sprite_drawn)
		{
			if(cpu->ppu_scyc <= 5)
			{
				cpu->ppu_scyc += 1;
				return;
			}
			
			else if(cpu->ppu_scyc == 6)
			{
				cpu->sprite_drawn = false; // we are done 
			}
		}
		
		// blit the pixel
		push_pixel(cpu);
	}

	
}	
	

void draw_scanline(Cpu *cpu, int cycles) 
{
	// https://forums.nesdev.com/viewtopic.php?f=23&t=16612
	// we use M cycles so * 4
	for(int i = cycles*4; i--; i != 0) // one pixel per clock?
	{
		tick_fetcher(cpu); // the ppu pipeline
		if(cpu->hblank) { return; } // we are done
	}
}

// window code can remain but we have to clear the state when the switch happens
// (find a good way to implement the switch but we wont worry about it just yet...)

// scroll handling must be changed..
// should fetch tile locations incrementally  from each tile num
// and then fetch the data

// currently we increment a tile_cord by 8 each time this works
// fine and should not have any issues but is still not how it 
// is is actually done

void tile_fetch(Cpu *cpu)
{

	uint16_t tile_data = 0;
	uint16_t background_mem = 0;
	bool unsig = true;
	
	// where to draw the visual area and window
	uint8_t scroll_y = cpu->io[IO_SCY];
	uint8_t scroll_x = cpu->io[IO_SCX];
	uint8_t window_y = cpu->io[IO_WY];
	uint8_t window_x = cpu->io[IO_WX] - 7; // 0,0 is at offest - 7 for window
	
	//uint8_t lcd_control = read_mem(0xff40,cpu); // get lcd control reg
	int lcd_control = cpu->io[IO_LCDC];
	
	bool using_window = false;
	
	int scanline = cpu->current_line;
	
	// is the window enabled check in lcd control
	if(is_set(lcd_control,5)) 
	{
		// is the current scanline the window pos?
		if(window_y <= scanline)
		{
			using_window = true;
		}
	}
	
	// which tile data are we using
	if(is_set(lcd_control,4))
	{
		tile_data = 0x8000;
	}
	else
	{
		// this mem region uses signed bytes
		// for tile identifiers 
		tile_data = 0x8800;
		unsig = false;
	}
	
	
	// which background mem?
	if(!using_window)
	{
		if(is_set(lcd_control,3))
		{
			background_mem = 0x9c00;
		}
		else
		{
			background_mem = 0x9800;
		}
	}
	
	else
	{
		// which window mem?
		if(is_set(lcd_control,6))
		{
			background_mem = 0x9c00;
		}
		else
		{
			background_mem = 0x9800;
		}
	}
	
	uint8_t y_pos = 0;
	
	// ypos is used to calc which of the 32 vertical tiles 
	// the current scanline is drawing
	if(!using_window)
	{
		y_pos = scroll_y + scanline;
	}
	
	else
	{	
		y_pos = scanline - window_y;
	}
	
	// which of the 8 vertical pixels of the scanline are we on
	uint16_t tile_row = (((uint8_t)(y_pos/8))*32);
	
	int i = 0;
	//printf("pixel: %d\n", cpu->x_cord);
	

	
	for(int pixel = cpu->tile_cord; i < 8; pixel++)
	{
		uint8_t x_pos = pixel;
		
		if(!using_window) // <-- dont think this is correct
		{
			x_pos += scroll_x;
		}
		// translate the current x pos to window space // <--- think this causes the weird wrapping behavior with links awakening
		// if needed
		if(using_window)
		{
			if(x_pos >= window_x)
			{
				x_pos = pixel - window_x;
			}
		}
	
	
		// which of the 32 horizontal tiles does x_pos fall in
		uint16_t tile_col = (x_pos/8);
		int16_t tile_num;
	
	
		// get the tile identity num it can be signed or unsigned
		uint16_t tile_address = background_mem + tile_row+tile_col;
		if(unsig)
		{
			tile_num = (uint8_t)read_vram(tile_address,cpu);
		}
		else
		{
			tile_num = (int8_t)read_vram(tile_address,cpu);
		}
	
		// deduce where this tile identifier is in memory
		uint16_t tile_location = tile_data;
		
		if(unsig)
		{
			tile_location += (tile_num *16);
		}
		else
		{
			tile_location += ((tile_num+128)*16);
		}
	
		// find the correct vertical line we are on of the
		// tile to get the tile data
	
		uint8_t line = y_pos % 8;
		line *= 2; // each line takes up two bytes of mem
		uint8_t data1 = read_vram(tile_location + line,cpu);
		uint8_t data2 = read_vram(tile_location + line+1,cpu);
	
	
		// pixel 0 in the tile is bit 7 of data1 and data2
		// pixel 1 is bit 6 etc
		int color_bit = x_pos % 8;
		color_bit -= 7;
		color_bit *= -1;
	
		// combine data 2 and data 1 to get the color id for the pixel
		// in the tile
		// <--- verify 
		int colour_num = val_bit(data2,color_bit);
		colour_num <<= 1;
		colour_num |= val_bit(data1,color_bit);
		
		
		// save the color_num to the current pos int the tile fifo
		cpu->fetcher_tile[i++].colour_num = colour_num;
	}
	cpu->tile_cord += 8; // goto next tile fetch
}

int get_colour(Cpu *cpu ,uint8_t colour_num, uint16_t address)
{
	int res = WHITE;
	uint8_t palette = read_mem(address,cpu);
	int hi = 0;
	int lo = 0;
	
	switch(colour_num)
	{
		case 0: hi = 1; lo = 0; break;
		case 1: hi = 3; lo = 2; break;
		case 2: hi = 5; lo = 4; break;
		case 3: hi = 7; lo = 6; break;
	}
	
	// use the palette to get the color
	int colour = 0;
	colour = val_bit(palette,hi) << 1;
	colour |= val_bit(palette,lo);
	
	// convert game color to emulator color
	switch(colour)
	{
		case 0: res = WHITE; break;
		case 1: res = LIGHT_GRAY; break;
		case 2: res = DARK_GRAY; break;
		case 3: res = BLACK; break;
	}
	return res;
}




int cmpfunc(const void *A, const void *B)
{
	const Obj* a = A;
	const Obj* b = B;

	// sort by the oam index
	if(a->x_pos == b->x_pos)
	{
		return -(a->index - b->index);
	}

	// sort by the x posistion
	else
	{
		return(a->x_pos - b->x_pos);
	}
}



// read the up to 10 sprites for the scanline
// called when when enter pixel transfer
void read_sprites(Cpu *cpu)
{
	uint8_t lcd_control = cpu->io[IO_LCDC]; // get lcd control reg

	int y_size = is_set(lcd_control,2) ? 16 : 8;
	
	uint8_t scanline = cpu->current_line;

	memset(&cpu->objects_priority,0,sizeof(Obj) * 10);
	int x = 0;

	for(int sprite = 0; sprite < 40; sprite++) // should fetch all these as soon as we leave oam search
	{
		uint8_t y_pos = read_oam(0xfe00+sprite*4,cpu);
		if( scanline -(y_size - 16) < y_pos  && scanline + 16 >= y_pos )
		{
			// intercepts with the line
			cpu->objects_priority[x].index = sprite*4; // save the index
			// and the x pos
			cpu->objects_priority[x].x_pos = read_oam(0xfe00+(sprite*4)+1,cpu)-8;
			x += 1;
			if(x == 10) { break; } // only draw a max of 10 sprites per line
		}
	}

	
	// sort the array
	// if x cords are same use oam as priority lower indexes draw last
	// else use the x cordinate again lower indexes draw last
	// this means they will draw on top of other sprites
	qsort(&cpu->objects_priority,x,sizeof(Obj),cmpfunc);	
	
	cpu->no_sprites = x; // save how many sprites we have	
}



// returns if they have been rendered
// because we will delay if they have been
bool sprite_fetch(Cpu *cpu) 
{
	
	uint8_t lcd_control = read_mem(0xff40,cpu); // get lcd control reg

	int y_size = is_set(lcd_control,2) ? 16 : 8;

	int scanline = cpu->current_line;

	bool did_draw = false;
	
	for(int i = 0; i < cpu->no_sprites; i++)
	{
		
		uint8_t x_pos = cpu->objects_priority[i].x_pos;
		
		
		// if wrap with the x posistion will cause it to wrap around
		// where its in range of the current sprite we still need to draw it
		// for thje pixels its in range 
		
		// say we have one at 255
		// it will draw 7 pixels starting from zero
		// so from 0-6
		// so if the xcord = 0 then the first 6 pixels must be mixed
		// but i have no clue how to actually do this under a fifo...
		
		
		// offset into the sprite we start drawing at 
		// 7 is the 0th pixel and is defualt for a 
		// sprite that we draw fully
		int pixel_start = 7; 

		if(cpu->x_cord == 0 &&  x_pos + 8 > 255)
		{
			x_pos += 8;
			
			// this will cause it to draw at the correct offset into the sprite
			pixel_start = x_pos;
		}
		
		
		
		// if it does not start at the current x cord 
		// and does not overflow then we dont care
		else if(x_pos != cpu->x_cord)
		{
			continue;
		}
		
		
		
		
		// sprite takes 4 bytes in the sprite attributes table
		uint8_t sprite_index = cpu->objects_priority[i].index;
		uint8_t y_pos = read_oam(0xfe00+sprite_index,cpu);
		
		
		uint8_t sprite_location = read_oam(0xfe00+sprite_index+2,cpu);
		uint8_t attributes = read_oam(0xfe00+sprite_index+3, cpu);
		
		bool y_flip = is_set(attributes,6);
		bool x_flip = is_set(attributes,5);
		
		
		// does this sprite  intercept with the scanline
		if( scanline -(y_size - 16) < y_pos  && scanline + 16 >= y_pos )
		{	
			did_draw = true;
			y_pos -= 16;
			uint8_t line = scanline - y_pos; 
			
			
			// read the sprite backwards in y axis
			if(y_flip)
			{
				line -= (y_size);
				line += 1;
				line *= -1;
			}
			
			line *= 2; // same as for tiles
			uint16_t data_address = (0x8000 + (sprite_location * 16 )) + line;
			uint8_t data1 = read_vram(data_address,cpu);
			uint8_t data2 = read_vram(data_address+1,cpu);
			
			// eaiser to read in from right to left as pixel 0
			// is bit 7 in the color data pixel 1 is bit 6 etc 
			for(int sprite_pixel = pixel_start; sprite_pixel >= 0; sprite_pixel--)
			{
				int colour_bit = sprite_pixel;
				// red backwards for x axis
				if(x_flip)
				{
					colour_bit -= 7;
					colour_bit *= -1;
				}
				
				// rest same as tiles
				int colour_num = val_bit(data2,colour_bit);
				colour_num <<= 1;
				colour_num |= val_bit(data1,colour_bit);

				
				
				
				// colour_num needs to be written out ot sprite_fetcher
				// however in our array that will be shifted to the screen
				// we need to say where it is from eg tile or sprite
				// so that we actually know where to read the color from 
				// so we need to implement an array with the color id and a bool 
				// so that it knows where the colour values have actually come from 
				
				// dont display pixels with color id zero
				// the color itself dosent matter we only care about the id
				
				uint8_t x_pix = 0 - sprite_pixel;
				x_pix += pixel_start;

				
				// transparent tile wins
				if(colour_num == 0)
				{
					continue;
				}
				
				
				// test if its hidden behind the background layer
				// white is transparent even if the flag is set
				if(is_set(attributes,7))
				{
					if(cpu->ppu_fifo[x_pix].colour_num != 0)
					{
						continue;
					}	
				}
				
				
				int source = is_set(attributes,4)? SPRITE_ONE : SPRITE_ZERO;
			

				// if the current posisiton is the fifo is not a sprite
				// then this current pixel wins 
				
				if(cpu->ppu_fifo[x_pix].source == TILE)
				{
					cpu->ppu_fifo[x_pix].colour_num = colour_num;
					cpu->ppu_fifo[x_pix].source = source;
				}
			}
		}
	}
}


