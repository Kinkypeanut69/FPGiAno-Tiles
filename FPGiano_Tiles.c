#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "includes.h"
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_avalon_parallel_port.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"    // "VGA_Subsystem_VGA_Pixel_DMA"#include "altera_up_avalon_video_dma_controller.h"		// "VGA_Subsystem_Char_Buf_Subsystem_Char_Buf_DMA"#include "string.h"#include <os/alt_sem.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "Altera_UP_SD_Card_Avalon_Interface.h"
#include "altera_avalon_pio_regs.h"
/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK game_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define game_PRIORITY      16

/*variables - devices*/
alt_up_pixel_buffer_dma_dev* vgapixel;				//pixel buffer device
alt_up_video_dma_dev* vgachar;						//char buffer device

/* create a message to be displayed on the VGA and LCD displays */
char text_top_row[40] = "- FPGiAno Tiles! -\0";
char text_bottom_row[40] = "-     Score:     -\0";
char mode1[40] = "> Arcade <\0";
char mode2[40] = "Disco\0";
char mode3[40] = "Hardcore\0";
char counter[40] = "\0";
int nr = 0;
void randomKeyPress();
int tileID;

void randomKeyPress() {
	int x;
	int c1 = 0xFFFF;
	int c2 = 0x9CD3;
	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}

	if (tileID == 0 || tileID == 3 || tileID == 8) {
		alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 180, x + 7, 240, c1, 0);
		alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 218, x + 10, 240, c1, 0);
	} else if (tileID == 2 || tileID == 7 || tileID == 11) {
		alt_up_pixel_buffer_dma_draw_box(vgapixel, x + 3, 180, x + 10, 240, c1, 0);
		alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 218, x + 10, 240, c1, 0);
	} else {
		alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 218, x + 10, 240, c1, 0);
		alt_up_pixel_buffer_dma_draw_box(vgapixel, x + 3, 180, x + 7, 240, c1, 0);
	}

	tileID = rand() % 11;

	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}
	if (tileID == 0 || tileID == 3 || tileID == 8) {
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 180, x + 7, 240, c2, 0);
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 218, x + 10, 240, c2, 0);
		} else if (tileID == 2 || tileID == 7 || tileID == 11) {
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x + 3, 180, x + 10, 240, c2, 0);
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 218, x + 10, 240, c2, 0);
		} else {
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x, 218, x + 10, 240, c2, 0);
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x + 3, 180, x + 7, 240, c2, 0);
		}
}

void loadImage(){
	volatile int i, j;
	short att1=0, att2=0, att3=0, att;
	short int handler;
	int pixel;

	for(i=239;i>=0;i=i-1){
		for(j=0;j<320;j=j+1){
			att1 = (unsigned char) alt_up_sd_card_read(handler);
			att2 = (unsigned char) alt_up_sd_card_read(handler);
			att3 = (unsigned char) alt_up_sd_card_read(handler);
			pixel = ((att3>>3)<<11) | ((att2>>2)<<5) | (att1>>3);
			VGA_box(j, i, j, i, pixel);
		}
	}
}

void VGA_box(int x1, int y1, int x2, int y2, int pixel_color){
	int offset, row, col;
	volatile short * pixel_buffer = (short *) 0x00000000; // VGA pixel buffer addr
	for(row = y1; row <= y2; row++){
		col = x1;
		while(col <= x2){
			offset = (row << 9) + col;
			*(pixel_buffer + offset) = pixel_color;
			++col;
		}
	}
}

void game(void* pdata) {
	int x1 = 80, y1 = 0, x2 = 240, y2 = 240;
	int x3 = 120, y3 = 0, x4 = 120, y4 = 240;
	int x5 = 80, y5 = 0, x6 = 120;
	int x7 = 160, x8 = 200, x9 = 160;
	int y6 = 60, y7 = 120, y8 = 180;
	int y52 = 60, y62 = 120;
	int y72 = 180, y82 = 240;
	int y9 = 240, y92 = 300;
	int i, j;
	int c1 = 0xFFFF;
	int c2 = 0x9CD3;
	int c3 = 0x1061;

	int blockSelect;

	while (1) {
		for (i = 0; i < 27; i++) {
			alt_up_pixel_buffer_dma_draw_box(vgapixel, i * 12, 180,
					(i * 12) + 10, 240, c1, 0);
		}
		for (j = 0; j < 26; j++) {
			if (j == 2 || j == 6 || j == 19 || j == 22) {
			} else {
				alt_up_pixel_buffer_dma_draw_box(vgapixel, j * 12 + 8, 180,
						(j * 12) + 14, 217, c3, 0);
			}
		}

		alt_up_pixel_buffer_dma_draw_box(vgapixel, x1, y1, x2, y2, c1, 0);

		alt_up_pixel_buffer_dma_draw_line(vgapixel, x3, y3, x4, y4, c2, 0);
		alt_up_pixel_buffer_dma_draw_line(vgapixel, x3 + 40, y3, x4 + 40, y4,
				c2, 0);
		alt_up_pixel_buffer_dma_draw_line(vgapixel, x3 + 80, y3, x4 + 80, y4,
				c2, 0);

		while (1) {

			if (y52 - y5 == 60) {
				y5++;
				alt_up_pixel_buffer_dma_draw_line(vgapixel, x5 + 1, y5 - 1,
						x5 + 39, y5 - 1, c1, 0);
			}
			y52++;
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x5 + 1, y5, x5 + 39, y52,
					c3, 0);

			if (y62 - y6 == 60) {
				y6++;
				alt_up_pixel_buffer_dma_draw_line(vgapixel, x6 + 1, y6 - 1,
						x6 + 39, y6 - 1, c1, 0);
			}
			y62++;
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x6 + 1, y6, x6 + 39, y62,
					c3, 0);

			if (y72 - y7 == 60) {
				y7++;
				alt_up_pixel_buffer_dma_draw_line(vgapixel, x7 + 1, y7 - 1,
						x7 + 39, y7 - 1, c1, 0);
			}
			y72++;
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x7 + 1, y7, x7 + 39, y72,
					c3, 0);

			if (y82 - y8 == 60) {
				y8++;
				alt_up_pixel_buffer_dma_draw_line(vgapixel, x8 + 1, y8 - 1,
						x8 + 39, y8 - 1, c1, 0);
			}
			y82++;
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x8 + 1, y8, x8 + 39, y82,
					c3, 0);

			if (y92 - y9 == 60) {
				y9++;
				alt_up_pixel_buffer_dma_draw_line(vgapixel, x9 + 1, y9 - 1,
						x9 + 39, y9 - 1, c1, 0);
			}
			y92++;
			alt_up_pixel_buffer_dma_draw_box(vgapixel, x9 + 1, y9, x9 + 39, y92,
					c3, 0);

			if (y5 > 240) {
				blockSelect = rand() % 4;
				x5 = (blockSelect * 40) + 80;
				y5 = 0;
				y52 = 0;
				randomKeyPress();
				nr++;
			}

			if (y6 > 240) {
				blockSelect = rand() % 4;
				x6 = (blockSelect * 40) + 80;
				y6 = 0;
				y62 = 0;
				randomKeyPress();
				nr++;
			}
			if (y7 > 240) {
				blockSelect = rand() % 4;
				x7 = (blockSelect * 40) + 80;
				y7 = 0;
				y72 = 0;
				randomKeyPress();
				nr++;
			}
			if (y8 > 240) {
				blockSelect = rand() % 4;
				x8 = (blockSelect * 40) + 80;
				y8 = 0;
				y82 = 0;
				randomKeyPress();
				nr++;
			}
			if (y9 > 240) {
				blockSelect = rand() % 4;
				x9 = (blockSelect * 40) + 80;
				y9 = 0;
				y92 = 0;
				randomKeyPress();
				nr++;
			}
			OSTimeDlyHMSM(0, 0, 0, 60);
			sprintf(counter, "%d", nr);
			alt_up_video_dma_draw_string(vgachar, counter, 67, 7, 0);
		}
	}
}

/* The main function creates two task and starts multi-tasking */
int main(void) {
	OSInit();
	srand(time(NULL));
	tileID = rand() % 11;

	vgapixel = alt_up_pixel_buffer_dma_open_dev(
			"/dev/VGA_Subsystem_VGA_Pixel_DMA");		//open pixel buffer
	if (vgapixel == NULL) {
		printf("Error: could not open VGA_Pixel_Buffer device\n");
		return -1;
	} else
		printf("Opened VGA_Pixel_Buffer device\n");

	alt_up_pixel_buffer_dma_clear_screen(vgapixel, 0);			//clear screen

	vgachar = alt_up_video_dma_open_dev(
			"/dev/VGA_Subsystem_Char_Buf_Subsystem_Char_Buf_DMA");//open char buffer
	if (vgachar == NULL) {
		printf("Error: could not open VGA_Char_Buffer device\n");
		return -1;
	} else
		printf("Opened VGA_Char_Buffer device\n");

	alt_up_video_dma_screen_clear(vgachar, 0);					//clear buffer

	alt_up_video_dma_draw_string(vgachar, text_top_row, 1, 2, 0);
	alt_up_video_dma_draw_string(vgachar, text_bottom_row, 61, 2, 0);
	alt_up_video_dma_draw_string(vgachar, mode1, 3, 8, 0);
	alt_up_video_dma_draw_string(vgachar, mode2, 3, 12, 0);
	alt_up_video_dma_draw_string(vgachar, mode3, 3, 16, 0);

	OSTaskCreateExt(game, NULL, (void *) &game_stk[TASK_STACKSIZE - 1],
	game_PRIORITY, game_PRIORITY, game_stk, TASK_STACKSIZE, NULL, 0);

	OSStart();
	return 0;
}

