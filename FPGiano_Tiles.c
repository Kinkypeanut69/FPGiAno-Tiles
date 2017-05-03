#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "includes.h"
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_avalon_parallel_port.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"    // "VGA_Subsystem_VGA_Pixel_DMA"
#include "altera_up_avalon_video_dma_controller.h"		// "VGA_Subsystem_Char_Buf_Subsystem_Char_Buf_DMA"
#include "string.h"
#include <os/alt_sem.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "Altera_UP_SD_Card_Avalon_Interface.h"
#include "altera_avalon_pio_regs.h"

#define	DRAW alt_up_pixel_buffer_dma_draw
#define CLRSCREEN alt_up_pixel_buffer_dma_clear_screen(vgapixel, 0)
#define DRAW_STRING alt_up_video_dma_draw_string
#define TILE_WIDTH 40
#define TILE_HEIGHT 60

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
int scoreCounter = 0;
void randomKeyPress();
int tileID;
int white = 0xFFFF;
int grey = 0x9CD3;
int black = 0x1061;
int screenHeight = 240;
int screenWidth = 320;

void randomKeyPress() {
	int x;	//x-coordinate of decorative tiles
	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}

	//color previous key white
	if (tileID == 0 || tileID == 3 || tileID == 8) {	//tiles with black key to the right
		DRAW_box(vgapixel, x, 0.75*screenHeight, x + 7, screenHeight, white, 0);
		DRAW_box(vgapixel, x, 218, x + 10, screenHeight, white, 0);	//218 is used to avoid drawing over black piano keys
	} else if (tileID == 2 || tileID == 7 || tileID == 11) {	//tiles with black key to the left
		DRAW_box(vgapixel, x + 3, 0.75*screenHeight, x + 10, screenHeight, white, 0);
		DRAW_box(vgapixel, x, 218, x + 10, screenHeight, white, 0);
	} else {	//tiles with black keys on both sides
		DRAW_box(vgapixel, x, 218, x + 10, screenHeight, white, 0);
		DRAW_box(vgapixel, x + 3, 0.75*screenHeight, x + 7, screenHeight, white, 0);
	}

	tileID = rand() % 11;	//select new random key

	//prevent drawing on middle of screen
	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}
	//color current key grey
	if (tileID == 0 || tileID == 3 || tileID == 8) {	//tiles with black key to the right
			DRAW_box(vgapixel, x, 0.75*screenHeight, x + 7, screenHeight, grey, 0);
			DRAW_box(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
		} else if (tileID == 2 || tileID == 7 || tileID == 11) {	//tiles with black key to the left
			DRAW_box(vgapixel, x + 3, 0.75*screenHeight, x + 10, screenHeight, grey, 0);
			DRAW_box(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
		} else {	//tiles with black keys on both sides
			DRAW_box(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
			DRAW_box(vgapixel, x + 3, 0.75*screenHeight, x + 7, screenHeight, grey, 0);
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
	int decorativeWhiteTile, decorativeBlackTile;
	int screenTop = 0;
	int block1_x_left = screenWidth*0.25, 					block1_x_right = block1_x_left + TILE_WIDTH, 	block1_y_top = screenTop, 					block1_y_bottom = screenTop + TILE_HEIGHT;
	int block2_x_left = screenWidth*0.25 + TILE_WIDTH, 		block2_x_right = block2_x_left + TILE_WIDTH, 	block2_y_top = screenTop + TILE_HEIGHT, 	block2_y_bottom = screenTop + TILE_HEIGHT*2;
	int block3_x_left = screenWidth*0.25 + TILE_WIDTH*2, 	block3_x_right = block3_x_left + TILE_WIDTH, 	block3_y_top = screenTop + TILE_HEIGHT*2, 	block3_y_bottom = screenTop + TILE_HEIGHT*3;
	int block4_x_left = screenWidth*0.25 + TILE_WIDTH*3, 	block4_x_right = block4_x_left + TILE_WIDTH, 	block4_y_top = screenTop + TILE_HEIGHT*3, 	block4_y_bottom = screenTop + TILE_HEIGHT*4;
	int block5_x_left = screenWidth*0.25 + TILE_WIDTH*4, 	block5_x_right = block5_x_left + TILE_WIDTH, 	block5_y_top = screenTop + TILE_HEIGHT*4, 	block5_y_bottom = screenTop + TILE_HEIGHT*5;
	int blockSelect;

	while (1) {
		for (decorativeWhiteTile = 0; decorativeWhiteTile < 27; decorativeWhiteTile++) {	//draws 27 white piano keys on background
			DRAW_box(vgapixel, decorativeWhiteTile * 12, screenHeight*0.75,
					(decorativeWhiteTile * 12) + 10, screenHeight, white, 0);
		}
		for (decorativeBlackTile = 0; decorativeBlackTile < 26; decorativeBlackTile++) {	//draws 26 black piano keys on background
			if (decorativeBlackTile == 2 || decorativeBlackTile == 6 || decorativeBlackTile == 19 || decorativeBlackTile == 22) {
			} else {
				DRAW_box(vgapixel, decorativeBlackTile * 12 + 8, screenHeight*0.75,
						(decorativeBlackTile * 12) + 14, 217, black, 0);
			}
		}

		DRAW_box(vgapixel, screenWidth*0.25, screenTop, screenWidth*0.75, screenHeight, white, 0); //white game box

		DRAW_line(vgapixel, screenWidth*0.375, screenTop, screenWidth*0.375, screenHeight, grey, 0);
		DRAW_line(vgapixel, screenWidth*0.5, screenTop, screenWidth*0.5, screenHeight, grey, 0);
		DRAW_line(vgapixel, screenWidth*0.625, screenTop, screenWidth*0.625, screenHeight, grey, 0);

		while (1) {

			if (block1_y_bottom - block1_y_top == TILE_HEIGHT) {
				block1_y_top++;
				DRAW_line(vgapixel, block1_x_left + 1, block1_y_top - 1, block1_x_right - 1, block1_y_top - 1, white, 0);
			}
			block1_y_bottom++;
			DRAW_box(vgapixel, block1_x_left + 1, block1_y_top, block1_x_right - 1, block1_y_bottom, black, 0);

			if (block2_y_bottom - block2_y_top == TILE_HEIGHT) {
				block2_y_top++;
				DRAW_line(vgapixel, block2_x_left + 1, block2_y_top - 1, block2_x_right - 1, block2_y_top - 1, white, 0);
			}
			block2_y_bottom++;
			DRAW_box(vgapixel, block2_x_left + 1, block2_y_top, block2_x_right - 1, block2_y_bottom, black, 0);

			if (block3_y_bottom - block3_y_top == TILE_HEIGHT) {
				block3_y_top++;
				DRAW_line(vgapixel, block3_x_left + 1, block3_y_top - 1, block3_x_right - 1, block3_y_top - 1, white, 0);
			}
			block3_y_bottom++;
			DRAW_box(vgapixel, block3_x_left + 1, block3_y_top, block3_x_right - 1, block3_y_bottom, black, 0);

			if (block4_y_bottom - block4_y_top == TILE_HEIGHT) {
				block4_y_top++;
				DRAW_line(vgapixel, block4_x_left + 1, block4_y_top - 1, block4_x_right - 1, block4_y_top - 1, white, 0);
			}
			block4_y_bottom++;
			DRAW_box(vgapixel, block4_x_left + 1, block4_y_top, block4_x_right - 1, block4_y_bottom, black, 0);

			if (block5_y_bottom - block5_y_top == TILE_HEIGHT) {
				block5_y_top++;
				DRAW_line(vgapixel, block5_x_left + 1, block5_y_top - 1, block5_x_right - 1, block5_y_top - 1, white, 0);
			}
			block5_y_bottom++;
			DRAW_box(vgapixel, block5_x_left + 1, block5_y_top, block5_x_right - 1, block5_y_bottom, black, 0);



			if (block1_y_top > screenHeight) {
				blockSelect = rand() % 4;
				block1_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
				block1_y_top = 0;
				block1_y_bottom = 0;
				randomKeyPress();
				scoreCounter++;
			}

			if (block2_y_top > screenHeight) {
				blockSelect = rand() % 4;
				block2_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
				block2_y_top = 0;
				block2_y_bottom = 0;
				randomKeyPress();
				scoreCounter++;
			}

			if (block3_y_top > screenHeight) {
				blockSelect = rand() % 4;
				block3_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
				block3_y_top = 0;
				block3_y_bottom = 0;
				randomKeyPress();
				scoreCounter++;
			}

			if (block4_y_top > screenHeight) {
				blockSelect = rand() % 4;
				block4_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
				block4_y_top = 0;
				block4_y_bottom = 0;
				randomKeyPress();
				scoreCounter++;
			}

			if (block5_y_top > screenHeight) {
				blockSelect = rand() % 4;
				block5_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
				block5_y_top = 0;
				block5_y_bottom = 0;
				randomKeyPress();
				scoreCounter++;
			}

			OSTimeDlyHMSM(0, 0, 0, 60);
			sprintf(counter, "%d", scoreCounter);
			DRAW_STRING(vgachar, counter, 67, 7, 0);
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

	CLRSCREEN;			//clear screen

	vgachar = alt_up_video_dma_open_dev(
			"/dev/VGA_Subsystem_Char_Buf_Subsystem_Char_Buf_DMA");//open char buffer
	if (vgachar == NULL) {
		printf("Error: could not open VGA_Char_Buffer device\n");
		return -1;
	} else
		printf("Opened VGA_Char_Buffer device\n");

	alt_up_video_dma_screen_clear(vgachar, 0);					//clear buffer

	DRAW_STRING(vgachar, text_top_row, 1, 2, 0);
	DRAW_STRING(vgachar, text_bottom_row, 61, 2, 0);
	DRAW_STRING(vgachar, mode1, 3, 8, 0);
	DRAW_STRING(vgachar, mode2, 3, 12, 0);
	DRAW_STRING(vgachar, mode3, 3, 16, 0);

	OSTaskCreateExt(game, NULL, (void *) &game_stk[TASK_STACKSIZE - 1],
	game_PRIORITY, game_PRIORITY, game_stk, TASK_STACKSIZE, NULL, 0);

	OSStart();
	return 0;
}

