
// fetcher code needs a rework as it is beyond slow
// constantly copying around data it ideally should not
// (use a circular buffer)
// as well as make the tile fetch linear (done)
// then just shift the first few pixels out of the fifo
// also fix a error in the oracle games 
// where the hud is the wrong palette
// window also requires fixing for oracle games (From the badge build)

// everything in here is done in terms of T cycles
// https://github.com/sinamas/gambatte/tree/master/test/hwtests
// + gekkio's tests need to be passed

// then move onto cgb support

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "headers/cpu.h"
#include "headers/lib.h"
#include "headers/memory.h"




// need lcdon test passing 
// then the lcd timing tests 
// with scx sprite and window ones done last
// metroid 2 final boss




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
void read_sprites(Cpu *cpu);

// probably the largest room for optimization

bool is_lcd_enabled(Cpu *cpu)
{
	return ( is_set(cpu->io[IO_LCDC],7) );	
}


void do_hdma(Cpu *cpu)
{

	//puts("HDMA!");
	uint16_t source = cpu->dma_src;

	uint16_t dest = cpu->dma_dst | 0x8000;

	
	source += cpu->hdma_len_ticked*0x10;
	dest += cpu->hdma_len_ticked*0x10;
	
	/*if(!(source <= 0x7ff0 || ( source >= 0xa000 && source <= 0xdff0)))
	{
		printf("ILEGGAL HDMA SOURCE: %X!\n",source);
		exit(1);
	}
	*/
	// find out how many cycles we tick but for now just copy the whole damb thing 						
	for(int i = 0; i < 0x10; i++)
	{
		write_mem(cpu,dest+i,read_mem(source+i,cpu));
	}

	// 8 M cycles for each 0x10 block
	cycle_tick(cpu,8);
	
	// hdma is over 
	if(--cpu->hdma_len <= 0)
	{
		// indicate the tranfser is over
		cpu->io[IO_HDMA5] = 0xff;
		cpu->hdma_active = false;
	}

	// goto next block
	else
	{
		cpu->hdma_len_ticked++;
	}
}

// needs optimization in the rewrite
void update_graphics(Cpu *cpu, int cycles)
{
	//-----------------------
	// update the stat reg state
	// and trigger interrupts	
	
	// read out current stat reg and mask the mode
	uint8_t status = cpu->io[IO_STAT];
	status &= ~0x3;
	// mode is one if lcd is disabled
	if(!is_lcd_enabled(cpu)) // <-- should re enable on a delay?
	{
		cpu->scanline_counter = 0; // counter is reset?
		cpu->current_line = 0; // reset ly
		cpu->io[IO_STAT] = status; 
		cpu->mode = 0;
		cpu->signal = false; 
		return; // can exit if ppu is disabled nothing else to do
	}
	
	// save our current signal state
	bool signal_old = cpu->signal;


	cpu->scanline_counter += cycles; // advance the cycle counter


	switch(cpu->mode)
	{	
		case 0: // hblank
		{
			if(cpu->scanline_counter >= 456)
			{
				// reset the counter extra cycles should tick over
				cpu->scanline_counter = 0;

				cpu->current_line++;
				
				if(cpu->current_line == 144)
				{
					cpu->mode = 1; // switch to vblank
					cpu->new_vblank = true;
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
				
				else 
				{
					cpu->mode = 2;
				}		
			}
			break;
		}
		
		
		case 1: // vblank
		{
			if(cpu->scanline_counter >= 456)
			{
				cpu->scanline_counter = 0;
				cpu->current_line++;
				
				// vblank is over
				if(cpu->current_line > 153)
				{
					cpu->io[IO_STAT] &= ~3;
					cpu->io[IO_STAT] |= 2;
					cpu->current_line = 0;
					// enter oam search on the first line :)
					cpu->mode = 2; 				
				}	
			}
			break;
		}
		
		// mode 2 oam search
		case 2:
		{
			// mode 2 is over
			if(cpu->scanline_counter >= 80)
			{
				// switch to pixel transfer
				cpu->mode = 3;
				
				// read in the sprites we are about to draw
				read_sprites(cpu);

				cpu->x_scroll_tick = false;


				cpu->scx_cnt = (cpu->io[IO_SCX] & 0x7);
				if(cpu->scx_cnt)
				{
					cpu->x_scroll_tick = true;
				}				
			}
			break;
		}
		
		// pixel transfer
		case 3: 
		{
			draw_scanline(cpu,cycles);
			if(cpu->hblank) // if just entering hblank 
			{
				// switch to hblank
				cpu->mode = 0;
					
				// reset our fetcher :)
				cpu->x_cord = 0;
				cpu->tile_ready = false;
				cpu->pixel_count = 0;
				cpu->ppu_cyc = 0;
				cpu->hblank = false;
				cpu->tile_cord = 0;
				cpu->window_start = false;
				cpu->ppu_scyc = 0;						
					
				// on cgb do hdma
				if(cpu->is_cgb && cpu->hdma_active)
				{
					do_hdma(cpu);
				}
			}	
			break;
		}
		
		default:
		{
			printf("ILLEGAL MODE!");
			exit(1);
		}	
	}

	
	// check interrupts
	switch(cpu->mode)
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
	cpu->io[IO_STAT] = status | 128 | cpu->mode;		
}

// awful code
void shift_fifo(Cpu *cpu, int shift)
{
	memmove(cpu->ppu_fifo,&cpu->ppu_fifo[shift],cpu->pixel_count *sizeof(Pixel_Obj));
	cpu->pixel_count -= shift;
}



// shift a pixel out of the array and smash it to the screen 
// increment x afterwards

// returns wether it can keep pushing or not

bool push_pixel(Cpu *cpu) 
{

	// cant push anymore
	if(!(cpu->pixel_count > 8)) { return false; }

	 // ignore how much we are offset into the tile
	 // if we fetched a bg tile
	if(cpu->x_scroll_tick && cpu->ppu_fifo[0].scx_a)
	{
		shift_fifo(cpu,1);
		cpu->scx_cnt -= 1;
		if(!cpu->scx_cnt)
		{
			cpu->x_scroll_tick = false;
		}
		return (cpu->pixel_count > 8);
	}
	
	
	int col_num = cpu->ppu_fifo[0].colour_num; // save the pixel we will shift
	int scanline = cpu->current_line;
	
	if(!cpu->is_cgb)
	{
		int colour_address = cpu->ppu_fifo[0].source + 0xff47;	
		int col = get_colour(cpu,col_num,colour_address); 
		int red = 0;
		int green = 0;
		int blue = 0;
		
		// black is default
		switch(col)
		{
			case WHITE: red = 255; green = 255; blue = 255; break;
			case LIGHT_GRAY: red = 0xCC; green = 0xCC; blue = 0xCC;  break;
			case DARK_GRAY: red = 0x77; green = 0x77; blue = 0x77;  break;
		}
		
		cpu->screen[scanline][cpu->x_cord][0] = red;
		cpu->screen[scanline][cpu->x_cord][1] = green;
		cpu->screen[scanline][cpu->x_cord][2] = blue;
	}
	
	else // gameboy color
	{
		
		

		// for now we will assume tile just for arugments sake 
		int cgb_pal = cpu->ppu_fifo[0].cgb_pal;
		// each  rgb value takes two bytes in the pallete for cgb
		int offset = (cgb_pal*8) + (col_num * 2); 

		int col;
		if(cpu->ppu_fifo[0].source == TILE)
		{
			col = cpu->bg_pal[offset];
			col |= cpu->bg_pal[offset + 1] << 8;
		}
		
		
		else // is a sprite
		{
			col = cpu->sp_pal[offset];
			col |= cpu->sp_pal[offset + 1] << 8;			
		}
		
	
		// gameboy stores palletes in bgr format?
		int blue = col & 0x1f;
		int green = (col >> 5) & 0x1f;
		int red = (col >> 10) & 0x1f;
		
		// convert rgb15 to rgb888
		red = (red << 3) | (red >> 2);
		blue = (blue << 3) | (blue >> 2);
		green = (green << 3) | (green >> 2);
			
		cpu->screen[scanline][cpu->x_cord][0] = red;
		cpu->screen[scanline][cpu->x_cord][1] = green;
		cpu->screen[scanline][cpu->x_cord][2] = blue;
	}
	
	shift_fifo(cpu,1); // shift a pixel out by one
	if(++cpu->x_cord == 160)
	{
		// done drawing enter hblank
		cpu->hblank = true;
		return false;
	}
	return true;
}	
	





// todo proper scx and window timings
// as we current do not implement them at all 
// window should restart the fetcher when triggered 
// and take 6 cycles (this is now done)

// and we should implement scx properly with the fetcher...
// ^ this needs researching and implementing

// need to handle bugs with the window
void tick_fetcher(Cpu *cpu, int cycles) 
{

	// advance the fetcher if we dont have a tile dump waiting
	// fetcher operates at half of base clock (4mhz)
	if(!cpu->tile_ready) // fetch the tile
	{
		// should fetch the number then low and then high byte
		// but we will ignore this fact for now

		// 1 cycle is tile num 
		// 2nd is lb of data 
		// 3rd is high byte of data 

		cpu->ppu_cyc += cycles; // further along 

		if(cpu->ppu_cyc >= 3) // takes 3 cycles to fetch 8 pixels
		{
			tile_fetch(cpu);
			cpu->tile_ready = true;
			// any over flow will be used to push the next tile
			// into the fifo (the 4th clock)
			cpu->ppu_cyc = 0;
		}	
	}
	
	// if we have room to dump into the fifo
	// and we are ready to do so, do it now 
	// at 0 dump at start at 8 pixels dump in higher half
	if(cpu->tile_ready && cpu->pixel_count <= 8)
	{
		memcpy(&cpu->ppu_fifo[cpu->pixel_count], cpu->fetcher_tile,8*sizeof(Pixel_Obj));
		cpu->tile_ready = false;
		cpu->pixel_count += 8;
	}		
}	
	

void draw_scanline(Cpu *cpu, int cycles) 
{
	// get lcd control reg
	const int control = cpu->io[IO_LCDC];
	
	// fetcher operates at half of base clock
	tick_fetcher(cpu,cycles/2);
	
	// push out of fifo
	if(cpu->pixel_count > 8)
	{


		for(int i = 0; i < cycles; i++) // 1 pixel pushed per cycle
		{
			// ignore sprite timings for now
			if(is_set(control,1))
			{
				sprite_fetch(cpu);
			}
		
			// blit the pixel 
			// stop at hblank 
			// or if the fifo only has 8 pixels inside it
			if(!push_pixel(cpu)) 
			{ 
				return; 
			}
		}
	}
}

// fetch a single tile into the fifo

void tile_fetch(Cpu *cpu)
{
	// where to draw the visual area and window
	const uint8_t scroll_y = cpu->io[IO_SCY];
	const uint8_t scroll_x = cpu->io[IO_SCX];
	const uint8_t window_y = cpu->io[IO_WY];
	const uint8_t window_x = cpu->io[IO_WX] - 7; // 0,0 is at offest - 7 for window
	
	const int lcd_control = cpu->io[IO_LCDC];
	

	const int scanline = cpu->current_line;
	
	// is the window enabled check in lcd control
	// and is the current scanline the window pos?
	const bool using_window = is_set(lcd_control,5) && (window_y <= scanline); 

	
	// what kind of address are we using
	const bool unsig = is_set(lcd_control,4);
	
	// what tile data are we using
	const int tile_data = unsig ? 0x8000 : 0x8800; 
	
	
	// ypos is used to calc which of the 32 vertical tiles 
	// the current scanline is drawing	
	uint8_t y_pos = 0;
	

	int background_mem = 0;
	
	// which background mem?
	if(!using_window)
	{
		background_mem = is_set(lcd_control,3) ? 0x9c00 : 0x9800;
		y_pos = scroll_y + scanline;
	}
	
	else
	{
		// which window mem?
		background_mem = is_set(lcd_control,6) ? 0x9c00 : 0x9800;
		y_pos = scanline - window_y;
	}

	
	// which of the 8 vertical pixels of the scanline are we on
	const int tile_row = ((y_pos/8)*32);

	
	int cgb_pal = -1;
	bool priority = false;
	bool x_flip = false;
	bool y_flip = false;
	uint8_t data1 = -1; 
	uint8_t data2 = - 1; 
	//int tile_col_last = -1;
	int vram_bank = 0;
	//for(int pixel = 0; pixel < 8; pixel++)
	//{
		int x_pos = (cpu->tile_cord/8);
		
		if(!using_window) // <-- dont think this is correct
		{
			x_pos += (scroll_x/8);
		}

		
		x_pos &= 31;
	
		// which of the 32 horizontal tiles does x_pos fall in
		//int tile_col = (x_pos/8);
		
	

		// if we are still drawing the same tile 
		// dont bother reloading the tile data
		// it can switch to a different tile if we start drawing 
		// a tile in the middle due to scroll x
		//if(tile_col != tile_col_last)
		{
			// update the "last" col
			//tile_col_last = tile_col;
			// get the tile identity num it can be signed or unsigned
			// -0x8000 to account for the vram 
			int tile_address = (background_mem + tile_row+x_pos) - 0x8000;

			// deduce where this tile identifier is in memory
			int tile_location = tile_data;		

			int tile_num;
			// tile number is allways bank 0
			if(unsig)
			{
				tile_num = cpu->vram[0][tile_address];
				tile_location += (tile_num *16);
			}
			else
			{
				tile_num = (int8_t)cpu->vram[0][tile_address];
				tile_location += ((tile_num+128)*16);
			}
			
			// map to our array
			tile_location -= 0x8000;
			
			// should cache these values before the loop but ignore for now
			// x and y flip + priority needs to be implemented

			if(cpu->is_cgb) // we are drawing in cgb mode 
			{
				// bg attributes allways in bank 1
				uint8_t attr = cpu->vram[1][tile_address];
				cgb_pal = attr & 0x7; // get the pal number
				
				
				// draw over sprites
				priority = is_set(attr,7);
				
				x_flip = is_set(attr,5);
				y_flip = is_set(attr,6);

				// decide what bank data is coming out of
				// allready one so dont check the other condition
				vram_bank = is_set(attr,3) ? 1 : 0;
			}
			
			// find the correct vertical line we are on of the
			// tile to get the tile data		
			int line = y_pos % 8;
			
			// read the sprite backwards in y axis
			if(y_flip)
			{
				line = 7 - line;
			}		
		
			line *= 2; // each line takes up two bytes of mem
			data1 = cpu->vram[vram_bank][(tile_location+line)];
			data2 = cpu->vram[vram_bank][(tile_location+line+1)];
		}
	
		// pixel 0 in the tile is bit 7 of data1 and data2
		// pixel 1 is bit 6 etc
		for(int i = 0; i < 8; i++)
		{
			int color_bit = x_flip? i : 7-i;
	
		
	
			// combine data 2 and data 1 to get the color id for the pixel
			// in the tile
			// <--- verify 
			int colour_num = val_bit(data2,color_bit);
			colour_num <<= 1;
			colour_num |= val_bit(data1,color_bit);
			
			
			
			// save the color_num to the current pos int the tile fifo		
			if(!cpu->is_cgb)
			{
				cpu->fetcher_tile[i].colour_num = colour_num;
				cpu->fetcher_tile[i].source = TILE;		
			}
			
			else // cgb save the pallete value... 
			{
				cpu->fetcher_tile[i].colour_num = colour_num;
				cpu->fetcher_tile[i].cgb_pal = cgb_pal;
				// in cgb an priority bit is set it has priority over sprites
				// unless lcdc has the master overide enabled
				cpu->fetcher_tile[i].source = priority ? TILE_CGBD : TILE;		
			}
			cpu->fetcher_tile[i].scx_a = !using_window;
		}
	cpu->tile_cord += 8; // goto next tile fetch
}

int get_colour(Cpu *cpu ,uint8_t colour_num, uint16_t address)
{
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
	

	static const int colors[] = {WHITE,LIGHT_GRAY,DARK_GRAY,BLACK};

	return colors[colour];
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
			if(++x == 10) { break; } // only draw a max of 10 sprites per line
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

	
	uint8_t lcd_control = cpu->io[IO_LCDC]; // get lcd control reg

	// in cgb if lcdc bit 0 is deset sprites draw over anything
	bool draw_over_everything = (!is_set(lcd_control,0) && cpu->is_cgb);
	
	int y_size = is_set(lcd_control,2) ? 16 : 8;

	int scanline = cpu->current_line;
	
	bool did_draw;
	
	for(int i = 0; i < cpu->no_sprites; i++)
	{
		int vram_bank = 0;
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

		if(cpu->x_cord == 0 &&  x_pos + 7 > 255)
		{
			x_pos += 7;
			
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
		uint8_t y_pos = cpu->oam[sprite_index];
		uint8_t sprite_location = cpu->oam[(sprite_index+2)];
		uint8_t attributes = cpu->oam[(sprite_index+3)];
		
		bool y_flip = is_set(attributes,6);
		bool x_flip = is_set(attributes,5);
		
		
		// does this sprite  intercept with the scanline
		if( scanline -(y_size - 16) < y_pos  && scanline + 16 >= y_pos )
		{
			y_pos -= 16;
			uint8_t line = scanline - y_pos; 
			
			
			// read the sprite backwards in y axis
			if(y_flip)
			{
				line = y_size - (line + 1);
			}
			
			line *= 2; // same as for tiles
			uint16_t data_address = ((sprite_location * 16 )) + line; // in realitly this is offset into vram at 0x8000
			if(is_set(attributes,3) && cpu->is_cgb) // if in cgb and attr has bit 3 set 
			{
				vram_bank = 1; // sprite data is out of vram bank 1
			}
				

			uint8_t data1 = cpu->vram[vram_bank][data_address];
			uint8_t data2 = cpu->vram[vram_bank][(data_address+1)];
			
			// eaiser to read in from right to left as pixel 0
			// is bit 7 in the color data pixel 1 is bit 6 etc 
			for(int sprite_pixel = pixel_start; sprite_pixel >= 0; sprite_pixel--)
			{
				int colour_bit = sprite_pixel;
				// red backwards for x axis
				if(x_flip)
				{
					colour_bit = 7 - colour_bit;
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

				
				// transparent sprite so the tile wins
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
						// need to test for the overide in lcdc on cgb
						if(!draw_over_everything)
						{
							continue;
						}
					}	
				}
				
				
				int source = is_set(attributes,4)? SPRITE_ONE : SPRITE_ZERO;
			

				// if the current posisiton is the fifo is not a sprite
				// then this current pixel wins 
				
				
				// in cgb if tile has priority set in its attributes it will draw over it 
				// unless overidded by lcdc
				
				// if this is not set it can only draw if the tile does not have its priority 
				// bit set in cgb mode
				
				if(cpu->ppu_fifo[x_pix].source != TILE_CGBD || draw_over_everything)
				{
					cpu->ppu_fifo[x_pix].colour_num = colour_num;
					cpu->ppu_fifo[x_pix].source = source;
					//if(cpu->is_cgb) probably faster to just write the value anyways
					//{
						cpu->ppu_fifo[x_pix].cgb_pal = attributes & 0x7;
					//}
					cpu->ppu_fifo[x_pix].scx_a = false;
				}
			}
			did_draw = true;
		}
	}

	return did_draw;
}
