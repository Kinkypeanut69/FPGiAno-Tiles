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
#include <altera_up_avalon_ps2.h>
#include <altera_up_ps2_keyboard.h>

#define DRAW_BOX alt_up_pixel_buffer_dma_draw_box
#define DRAW_LINE alt_up_pixel_buffer_dma_draw_line
#define CLRSCREEN alt_up_pixel_buffer_dma_clear_screen(vgapixel, 0)
#define DRAW_STRING alt_up_video_dma_draw_string
#define TILE_WIDTH 40
#define TILE_HEIGHT 60

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK game_stk[TASK_STACKSIZE];
OS_STK startMenu_stk[TASK_STACKSIZE];
OS_STK pauseMenu_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define game_PRIORITY      		16
#define startMenu_PRIORITY      15
#define pauseMenu_PRIORITY      14

/*variables - devices*/
alt_up_pixel_buffer_dma_dev* vgapixel;				//pixel buffer device
alt_up_video_dma_dev* vgachar;						//char buffer device
alt_up_ps2_dev *ps2;

/* create a message to be displayed on the VGA and LCD displays */
char text_top_row[40] = "- FPGiAno Tiles! -\0";
char text_bottom_row[40] = "-     Score:     -\0";
char mode1[40] = "1  Arcade\0";
char mode2[40] = "2  Disco\0";
char mode3[40] = "3  Hardcore\0";
char msg1[40] = "FPGiAno Tiles\0";
char msg2[40] = "- Select a gamemode  -\0";
char msg3[40] = "- Hit ENTER to start -\0";
char pauseMsg[40] = "- Paused -\0";
char pauseMsgClr[40] = "                       \0";
char pauseMsg2[40]   = "Press SPACEBAR\0";
char pauseMsg22[40] = "to pause\0";
char pauseMsg23[40] = "to continue\0";
char pauseMsg3[40] = "Press ESC\0";
char pauseMsg32[40] = "to quit\0";
char counter[40] = "\0";
char arrow[40] = ">\0";
char arrowClr[40] = " \0";

int scoreCounter = 0;
int gamemodeSelect;
int tileID;
int white = 0xFFFF;
int grey = 0x9CD3;
int black = 0x1061;
int screenHeight = 240;
int screenWidth = 320;
KB_CODE_TYPE *decode_mode;
alt_u8 buf;
char ascii;

void randomKeyPress();
void game(void* pdata);

void randomKeyPress() {
	int x;	//x-coordinate of decorative tiles
	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}

	//color previous key white
	if (tileID == 0 || tileID == 3 || tileID == 8) {	//tiles with black key to the right
		DRAW_BOX(vgapixel, x, 0.75*screenHeight, x + 7, screenHeight, white, 0);
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, white, 0);	//218 is used to avoid drawing over black piano keys
	} else if (tileID == 2 || tileID == 7 || tileID == 11) {	//tiles with black key to the left
		DRAW_BOX(vgapixel, x + 3, 0.75*screenHeight, x + 10, screenHeight, white, 0);
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, white, 0);
	} else {	//tiles with black keys on both sides
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, white, 0);
		DRAW_BOX(vgapixel, x + 3, 0.75*screenHeight, x + 7, screenHeight, white, 0);
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
			DRAW_BOX(vgapixel, x, 0.75*screenHeight, x + 7, screenHeight, grey, 0);
			DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
		} else if (tileID == 2 || tileID == 7 || tileID == 11) {	//tiles with black key to the left
			DRAW_BOX(vgapixel, x + 3, 0.75*screenHeight, x + 10, screenHeight, grey, 0);
			DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
		} else {	//tiles with black keys on both sides
			DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
			DRAW_BOX(vgapixel, x + 3, 0.75*screenHeight, x + 7, screenHeight, grey, 0);
		}
}

void startMenu(void* pdata){
	alt_up_video_dma_screen_clear(vgachar, 0);
	DRAW_BOX(vgapixel, 79, 34, 231, 121, white, 0);
	DRAW_BOX(vgapixel, 80, 35, 230, 120, black, 0);
	DRAW_BOX(vgapixel, 143, 58, 166, 81, white, 0);
	DRAW_BOX(vgapixel, 144, 59, 165, 80, black, 0);
	DRAW_BOX(vgapixel, 155, 59, 165, 69, white, 0);
	DRAW_BOX(vgapixel, 144, 70, 154, 80, white, 0);
	DRAW_STRING(vgachar, msg1, 32, 11, 0);
	DRAW_STRING(vgachar, msg2, 28, 24, 0);
	DRAW_STRING(vgachar, msg3, 28, 27, 0);
	DRAW_STRING(vgachar, mode1, 34, 36, 0);
	DRAW_STRING(vgachar, mode2, 34, 41, 0);
	DRAW_STRING(vgachar, mode3, 34, 46, 0);

	ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
	alt_up_ps2_init(ps2);

	while(1){
		decode_scancode(ps2,decode_mode,&buf,&ascii);
		if (*decode_mode == 1 || *decode_mode == 2){
			if(ascii == '1'){
				DRAW_STRING(vgachar, arrowClr, 32, 41, 0);
				DRAW_STRING(vgachar, arrowClr, 32, 46, 0);
				gamemodeSelect = 1;
				DRAW_STRING(vgachar, arrow, 32, 36, 0);
			}else if(ascii == '2'){
				DRAW_STRING(vgachar, arrowClr, 32, 36, 0);
				DRAW_STRING(vgachar, arrowClr, 32, 46, 0);
				gamemodeSelect = 2;
				DRAW_STRING(vgachar, arrow, 32, 41, 0);
			}else if(ascii == '3'){
				DRAW_STRING(vgachar, arrowClr, 32, 36, 0);
				DRAW_STRING(vgachar, arrowClr, 32, 41, 0);
				gamemodeSelect = 3;
				DRAW_STRING(vgachar, arrow, 32, 46, 0);
			}else if(buf == 0x5A && gamemodeSelect){
				OSTaskCreateExt(game, NULL, (void *) &game_stk[TASK_STACKSIZE - 1],
				game_PRIORITY, game_PRIORITY, game_stk, TASK_STACKSIZE, NULL, 0);
				OSTaskDel(OS_PRIO_SELF);
			}
		}
	}
}

void drawHUD(){
	DRAW_STRING(vgachar, text_top_row, 1, 2, 0);
	DRAW_STRING(vgachar, text_bottom_row, 61, 2, 0);
	DRAW_STRING(vgachar, mode1, 3, 8, 0);
	DRAW_STRING(vgachar, mode2, 3, 12, 0);
	DRAW_STRING(vgachar, mode3, 3, 16, 0);
	DRAW_STRING(vgachar, pauseMsg2, 3, 40, 0);
	DRAW_STRING(vgachar, pauseMsgClr, 2, 42, 0);
	DRAW_STRING(vgachar, pauseMsg22, 6, 42, 0);
	if (gamemodeSelect == 1){
		DRAW_STRING(vgachar, arrow, 1, 8, 0);
	} else if (gamemodeSelect == 2){
		DRAW_STRING(vgachar, arrow, 1, 12, 0);
	} else if (gamemodeSelect == 3){
		DRAW_STRING(vgachar, arrow, 1, 16, 0);
	}
}

void pauseMenu(void* pdata){
	alt_up_video_dma_screen_clear(vgachar, 0);
	DRAW_BOX(vgapixel, 8, 94, 70, 131, 0xFFFF, 0);
	DRAW_BOX(vgapixel, 9, 95, 69, 130, 0x1061, 0);
	DRAW_STRING(vgachar, pauseMsg, 5, 25, 0);
	DRAW_STRING(vgachar, pauseMsg3, 5, 29, 0);
	DRAW_STRING(vgachar, pauseMsg32, 6, 31, 0);
	drawHUD();
	DRAW_STRING(vgachar, pauseMsgClr, 5, 42, 0);
	DRAW_STRING(vgachar, pauseMsg23, 4, 42, 0);
	DRAW_STRING(vgachar, counter, 67, 7, 0);

	ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
	alt_up_ps2_init(ps2);

	while(1){
		decode_scancode(ps2,decode_mode,&buf,&ascii);
		if (*decode_mode == 2){
			if(buf == 0x29){
				DRAW_BOX(vgapixel, 8, 94, 70, 131, 0x0000, 0);
				DRAW_STRING(vgachar, pauseMsgClr, 5, 25, 0);
				DRAW_STRING(vgachar, pauseMsgClr, 4, 29, 0);
				DRAW_STRING(vgachar, pauseMsgClr, 5, 31, 0);
				DRAW_STRING(vgachar, pauseMsgClr, 2, 42, 0);
				DRAW_STRING(vgachar, pauseMsg22, 6, 42, 0);
				DRAW_STRING(vgachar, pauseMsgClr, 65, 25, 0);
				OSTaskDel(OS_PRIO_SELF);
			}else if(buf == 0x76){
				OSTaskDel(game_PRIORITY);
				CLRSCREEN;
				OSTaskCreateExt(startMenu, NULL, (void *) &startMenu_stk[TASK_STACKSIZE - 1],
				startMenu_PRIORITY, startMenu_PRIORITY, startMenu_stk, TASK_STACKSIZE, NULL, 0);
				scoreCounter = 0;
				gamemodeSelect = 0;
				OSTaskDel(OS_PRIO_SELF);
			}
		}
	}
}

void game(void* pdata) {
	int decorativeWhiteTile, decorativeBlackTile;
	int screenTop = 0;
	int block1_x_left = screenWidth*0.25, 					block1_x_right = block1_x_left + TILE_WIDTH, 	block1_y_top = screenTop, 					block1_y_bottom = block1_y_top + TILE_HEIGHT;
	int block2_x_left = screenWidth*0.25 + TILE_WIDTH, 		block2_x_right = block2_x_left + TILE_WIDTH, 	block2_y_top = (screenTop + TILE_HEIGHT), 		block2_y_bottom = block2_y_top + TILE_HEIGHT;
	int block3_x_left = screenWidth*0.25 + TILE_WIDTH*2, 	block3_x_right = block3_x_left + TILE_WIDTH, 	block3_y_top = (screenTop + TILE_HEIGHT*2), 	block3_y_bottom = block3_y_top + TILE_HEIGHT;
	int block4_x_left = screenWidth*0.25 + TILE_WIDTH*3, 	block4_x_right = block4_x_left + TILE_WIDTH, 	block4_y_top = (screenTop + TILE_HEIGHT*3), 	block4_y_bottom = block4_y_top + TILE_HEIGHT;
	int block5_x_left = screenWidth*0.25 + TILE_WIDTH*4, 	block5_x_right = block5_x_left + TILE_WIDTH, 	block5_y_top = (screenTop + TILE_HEIGHT*4), 	block5_y_bottom = block5_y_top + TILE_HEIGHT;
	int blockSelect;

	ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
	alt_up_ps2_init(ps2);

	alt_up_video_dma_screen_clear(vgachar, 0);					//clear buffer
	CLRSCREEN;

	drawHUD();

	for (decorativeWhiteTile = 0; decorativeWhiteTile < 27; decorativeWhiteTile++) {	//draws 27 white piano keys on background
		DRAW_BOX(vgapixel, decorativeWhiteTile * 12, screenHeight*0.75,
				(decorativeWhiteTile * 12) + 10, screenHeight, white, 0);
	}
	for (decorativeBlackTile = 0; decorativeBlackTile < 26; decorativeBlackTile++) {	//draws 26 black piano keys on background
		if (decorativeBlackTile == 2 || decorativeBlackTile == 6 || decorativeBlackTile == 19 || decorativeBlackTile == 22) {
		} else {
			DRAW_BOX(vgapixel, decorativeBlackTile * 12 + 8, screenHeight*0.75,
					(decorativeBlackTile * 12) + 14, 217, black, 0);
		}
	}

	DRAW_BOX(vgapixel, screenWidth*0.25, screenTop, screenWidth*0.75, screenHeight, white, 0); //white game box

	DRAW_LINE(vgapixel, screenWidth*0.375, screenTop, screenWidth*0.375, screenHeight, grey, 0);
	DRAW_LINE(vgapixel, screenWidth*0.5, screenTop, screenWidth*0.5, screenHeight, grey, 0);
	DRAW_LINE(vgapixel, screenWidth*0.625, screenTop, screenWidth*0.625, screenHeight, grey, 0);

	while (1) {
		if (block1_y_bottom - block1_y_top == TILE_HEIGHT) {
			block1_y_top++;
			DRAW_LINE(vgapixel, block1_x_left + 1, block1_y_top - 1, block1_x_right - 1, block1_y_top - 1, white, 0);
		}
		block1_y_bottom++;
		DRAW_BOX(vgapixel, block1_x_left + 1, block1_y_top, block1_x_right - 1, block1_y_bottom, black, 0);

		if (block2_y_bottom - block2_y_top == TILE_HEIGHT) {
			block2_y_top++;
			DRAW_LINE(vgapixel, block2_x_left + 1, block2_y_top - 1, block2_x_right - 1, block2_y_top - 1, white, 0);
		}
		block2_y_bottom++;
		DRAW_BOX(vgapixel, block2_x_left + 1, block2_y_top, block2_x_right - 1, block2_y_bottom, black, 0);

		if (block3_y_bottom - block3_y_top == TILE_HEIGHT) {
			block3_y_top++;
			DRAW_LINE(vgapixel, block3_x_left + 1, block3_y_top - 1, block3_x_right - 1, block3_y_top - 1, white, 0);
		}
		block3_y_bottom++;
		DRAW_BOX(vgapixel, block3_x_left + 1, block3_y_top, block3_x_right - 1, block3_y_bottom, black, 0);

		if (block4_y_bottom - block4_y_top == TILE_HEIGHT) {
			block4_y_top++;
			DRAW_LINE(vgapixel, block4_x_left + 1, block4_y_top - 1, block4_x_right - 1, block4_y_top - 1, white, 0);
		}
		block4_y_bottom++;
		DRAW_BOX(vgapixel, block4_x_left + 1, block4_y_top, block4_x_right - 1, block4_y_bottom, black, 0);

		if (block5_y_bottom - block5_y_top == TILE_HEIGHT) {
			block5_y_top++;
			DRAW_LINE(vgapixel, block5_x_left + 1, block5_y_top - 1, block5_x_right - 1, block5_y_top - 1, white, 0);
		}
		block5_y_bottom++;
		DRAW_BOX(vgapixel, block5_x_left + 1, block5_y_top, block5_x_right - 1, block5_y_bottom, black, 0);


		if (block1_y_top > screenHeight) {
			blockSelect = rand() % 4;
			block1_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
			block1_x_right = block1_x_left + TILE_WIDTH;
			block1_y_top = 0;
			block1_y_bottom = 0;
			randomKeyPress();
			scoreCounter++;
		}

		if (block2_y_top > screenHeight) {
			blockSelect = rand() % 4;
			block2_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
			block2_x_right = block2_x_left + TILE_WIDTH;
			block2_y_top = 0;
			block2_y_bottom = 0;
			randomKeyPress();
			scoreCounter++;
		}

		if (block3_y_top > screenHeight) {
			blockSelect = rand() % 4;
			block3_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
			block3_x_right = block3_x_left + TILE_WIDTH;
			block3_y_top = 0;
			block3_y_bottom = 0;
			randomKeyPress();
			scoreCounter++;
		}

		if (block4_y_top > screenHeight) {
			blockSelect = rand() % 4;
			block4_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
			block4_x_right = block4_x_left + TILE_WIDTH;
			block4_y_top = 0;
			block4_y_bottom = 0;
			randomKeyPress();
			scoreCounter++;
		}

		if (block5_y_top > screenHeight) {
			blockSelect = rand() % 4;
			block5_x_left = (blockSelect * TILE_WIDTH) + screenWidth*0.25;
			block5_x_right = block5_x_left + TILE_WIDTH;
			block5_y_top = 0;
			block5_y_bottom = 0;
			randomKeyPress();
			scoreCounter++;
		}

		decode_scancode(ps2,decode_mode,&buf,&ascii);
		if (*decode_mode == 2){
			if(buf == 0x29){
				OSTaskCreateExt(pauseMenu, NULL, (void *) &pauseMenu_stk[TASK_STACKSIZE - 1],
				pauseMenu_PRIORITY, pauseMenu_PRIORITY, pauseMenu_stk, TASK_STACKSIZE, NULL, 0);
			}
		}

		OSTimeDlyHMSM(0, 0, 0, 60);
		sprintf(counter, "%d", scoreCounter);
		DRAW_STRING(vgachar, counter, 67, 7, 0);
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

	OSTaskCreateExt(startMenu, NULL, (void *) &startMenu_stk[TASK_STACKSIZE - 1],
	startMenu_PRIORITY, startMenu_PRIORITY, startMenu_stk, TASK_STACKSIZE, NULL, 0);

	OSStart();
	return 0;
}

