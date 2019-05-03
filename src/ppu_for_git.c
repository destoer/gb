#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/memory.h"





// metroid 2 final boss



// run mooneye ppu tests against it
// get lyc test passing <-- 236 for one round 3
// hits mode 2 too early should it block the re-enable?  for 1m-cycle



// gold locking up waiting for a vblank interrupt if left alone for too long ?
// get the vblank test passing 

int get_colour(Cpu *cpu ,uint8_t colour_num, uint16_t address);


#define WHITE 1
#define LIGHT_GRAY 2
#define DARK_GRAY 3
#define BLACK 4


void draw_scanline(Cpu *cpu);
void render_tiles(Cpu *cpu);
void render_sprites(Cpu *cpu);
void set_lcd_status(Cpu *cpu);

// probably the largest room for optimization

bool is_lcd_enabled(Cpu *cpu)
{
	return ( is_set(cpu->io[IO_LCDC],7) );	
}

void update_stat(Cpu *cpu)
{
	//-----------------------
	// update the stat reg state
	// and trigger interrupts	
	
	// read out current stat reg and mask the mode
	uint8_t status = cpu->io[IO_STAT] & ~3;	
	
	// mode is one if lcd is disabled
	if(!is_lcd_enabled(cpu)) // <-- should re enable on a delay?
	{
		cpu->scanline_counter = 0; // counter is reset?
		cpu->io[IO_LY] = 0; // reset ly
		//status &= ~3; // stat mode 0 ^ done by default as it needs to be done anyways
		cpu->io[IO_STAT] = status; 
		return; // can exit if ppu is disabled nothing else to do
	}
	

	// save our current signal state
	bool signal_old = cpu->signal;

	uint8_t ly = cpu->io[IO_LY];

	
	
	
	// vblank (mode 1)
	if(ly >= 144)
	{
		status |= 1; // set mode 1
		cpu->signal = is_set(status,4);
	}
	
	
	// else check based on our cycle counter
	// technically this should vary depending on
	// what is being drawn to the screen
	else // <-- test interrupt(if true set signal line) and update the mode
	{
		// oam search (mode 2)
		if(cpu->scanline_counter <= 20)
		{
			status |= 2; // set mode 2
			cpu->signal = is_set(status,5);
		}
		
		// pixel transfer (mode 3)
		else if(cpu->scanline_counter <= 63) 
		{
			status |= 3; // set mode 3
		}
		
		// hblank mode 0
		else
		{ 
			cpu->signal = is_set(status,3);	
		}
	}
	


	
	// check coincidence  (lyc == ly)
	uint8_t lyc = cpu->io[IO_LYC];
	


	// line 153 can be treated as zero 
	// see https://forums.nesdev.com/viewtopic.php?f=20&t=13727
	if( lyc == ly || (lyc == 0 && ly == 153) )
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
	if(signal_old == false && cpu->signal)
	{
		request_interrupt(cpu,1);	
	}
	
	// update our status reg
	cpu->io[IO_STAT] = status | 128;
}

void update_graphics(Cpu *cpu, int cycles)
{
	// update the stat register
	update_stat(cpu);

	if(!is_lcd_enabled(cpu))
	{
		return;
	}
	
	//--------------------
	// tick the ppu

	// read out current stat reg
	uint8_t status = cpu->io[IO_STAT];
	
	// save our current signal state
	bool signal_old = cpu->signal;

	uint8_t ly = cpu->io[IO_LY];

	cpu->scanline_counter += cycles; // advance the cycle counter


	// we have finished drawing a line 
	if(cpu->scanline_counter >= 114) // <-- > or >=?
	{
		// if we are not in vblank draw the scanline
		if(ly < 144)
		{
			draw_scanline(cpu);
		}
		
		// inc the save ly and write it back 
		ly += 1;
		
		cpu->io[IO_LY] = ly;
		
		// reset the counter extra cycles should tick over
		cpu->scanline_counter = cpu->scanline_counter - 114;

		// vblank has been entered
		if(ly == 144)
		{
			request_interrupt(cpu,0);
			
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
		else if(ly > 153)
		{
			cpu->io[IO_STAT] &= ~3;
			cpu->io[IO_STAT] |= 2;
			cpu->io[IO_LY] = 0;	
		}

	}	
}

	
	


void draw_scanline(Cpu *cpu)
{
	
	// get lcd control reg
	uint8_t  control = read_mem(0xff40,cpu);

	if(is_set(control,0))
	{
		render_tiles(cpu);
	}
	
	if(is_set(control,1))
	{
		render_sprites(cpu);
	}
}


void render_tiles(Cpu *cpu)
{
	uint16_t tile_data = 0;
	uint16_t background_mem = 0;
	bool unsig = true;
	
	// where to draw the visual area and window
	uint8_t scroll_y = read_mem(0xff42,cpu);
	uint8_t scroll_x = read_mem(0xff43,cpu);
	uint8_t window_y = read_mem(0xff4a,cpu);
	uint8_t window_x = read_mem(0xff4b,cpu) - 7; // 0,0 is at offset - 7 x for the window
	
	uint8_t lcd_control = read_mem(0xff40,cpu); // get lcd control reg
	
	bool using_window = false;
	
	// is the window enabled check in lcd control
	if(is_set(lcd_control,5)) 
	{
		// is the current scanline the window pos?
		if(window_y <= read_mem(0xff44,cpu))
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
		y_pos = scroll_y + read_mem(0xff44,cpu);
	}
	
	else
	{
		
		y_pos = read_mem(0xff44,cpu) - window_y;
	}
	
	// which of the 8 vertical pixels of the scanline are we on
	uint16_t tile_row = (((uint8_t)(y_pos/8))*32);
	
	
	// time to start drawing the 160 horizontal pixels
	// for the scanline
	for(uint8_t pixel = 0; pixel < 160; pixel++)
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
			tile_num = (uint8_t)read_mem(tile_address,cpu);
		}
		else
		{
			tile_num = (int8_t)read_mem(tile_address,cpu);
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
		uint8_t data1 = read_mem(tile_location + line,cpu);
		uint8_t data2 = read_mem(tile_location + line+1,cpu);
	
	
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
	
		// now we have to colour id
		// to get the actual one from the palette
		// the hell is COLOUR <---
		int col = get_colour(cpu,colour_num,0xff47); 
		int red = 0;
		int green = 0;
		int blue = 0;
	
		// setup the rgb values
		// could probably optimse this a bunch
		// with a proper function
		switch(col)
		{
			case WHITE: red = 255; green = 255; blue = 255; break;
			case LIGHT_GRAY: red = 0xCC; green = 0xCC; blue = 0xCC; break;
			case DARK_GRAY: red = 0x77; green = 0x77; blue = 0x77; break;
		}
	
		uint8_t final_y = read_mem(0xff44,cpu);
	
	
	
		// saftey check (not required?)
		if((final_y>143)||(pixel>159))
        {
			continue;
		}
		// save the color to check priority
		cpu->screenp[final_y][pixel] = colour_num;
		
		cpu->screen[final_y][pixel][0] = red;
		cpu->screen[final_y][pixel][1] = green;
		cpu->screen[final_y][pixel][2] = blue;
	}
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


// add sprite precedence behaviour
void render_sprites(Cpu *cpu) 
{
	
	uint8_t lcd_control = read_mem(0xff40,cpu); // get lcd control reg

	int y_size = is_set(lcd_control,2) ? 16 : 8;
	
	for(int sprite = 39; sprite >= 0; sprite--) // more efficent to count loop down instead of up this ruins sprite ordering but we aint emulating it anyways
	{
		// sprite takes 4 bytes in the sprite attributes table
		uint8_t index = sprite*4; 
		uint8_t y_pos = (read_mem(0xfe00+index,cpu));
		uint8_t x_pos = read_mem(0xfe00+index+1, cpu)-8;
		uint8_t sprite_location = read_mem(0xfe00+index+2,cpu);
		uint8_t attributes = read_mem(0xfe00+index+3, cpu);
		
		bool y_flip = is_set(attributes,6);
		bool x_flip = is_set(attributes,5);
		
		uint8_t scanline = read_mem(0xff44,cpu);
		

		
		
		// does this sprite  intercept with the scanline
		if( scanline -(y_size - 16) < y_pos  && scanline + 16 >= y_pos )
		{	
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
			uint8_t data1 = read_mem(data_address,cpu);
			uint8_t data2 = read_mem(data_address+1,cpu);
			
			// eaiser to read in from right to left as pixel 0
			// is bit 7 in the color data pixel 1 is bit 6 etc 
			for(int sprite_pixel = 7; sprite_pixel >= 0; sprite_pixel--)
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
				
				
				// dont display pixels with color id zero
				// the color itself dosent matter we only care about the id
				
				
				if(colour_num == 0)
				{
					continue;
				}
				
				uint16_t colour_address = is_set(attributes,4)?0xff49 : 0xff48;
				int col  = get_colour(cpu,colour_num,colour_address);
				

				// black is default
				int red = 0;
				int green = 0;
				int blue = 0;
				
				switch(col)
				{
					case WHITE: red = 255; green = 255; blue = 255; break;
					case LIGHT_GRAY: red = 0xcc; green = 0xcc; blue = 0xcc; break;
					case DARK_GRAY: red =0x77; green = 0x77; blue = 0x77; break;
				}
				
				uint8_t x_pix = 0 - sprite_pixel;
				x_pix += 7;
				
				uint8_t pixel = x_pos + x_pix;
				
				if ((scanline>143)||(pixel>159))
				{
					continue;
				}
	
	
				// test if its hidden behind the background layer
				// white is transparent even if the flag is set
				if(is_set(attributes,7))
				{
					if(cpu->screenp[scanline][pixel] != 0)
					{
						continue;
					}	
				}
				
				cpu->screen[scanline][pixel][0] = red;
				cpu->screen[scanline][pixel][1] = green;
				cpu->screen[scanline][pixel][2] = blue;
				
				
				
			
			}
		}
	} 			
}
