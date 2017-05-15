#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "includes.h"
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_avalon_parallel_port.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"    // "VGA_Subsystem_VGA_Pixel_DMA"#include "altera_up_avalon_video_dma_controller.h"		// "VGA_Subsystem_Char_Buf_Subsystem_Char_Buf_DMA"#include "string.h"
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
#define DELAYVALUE 10

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK game_stk[TASK_STACKSIZE];
OS_STK startMenu_stk[TASK_STACKSIZE];
OS_STK pauseMenu_stk[TASK_STACKSIZE];
OS_STK countDownStart_stk[TASK_STACKSIZE];
OS_STK countDownPause_stk[TASK_STACKSIZE];
OS_STK startUpGame_stk[TASK_STACKSIZE];
OS_STK endGame_stk[TASK_STACKSIZE];

OS_EVENT *sem_pause;
OS_EVENT *sem_startUp;

/* Definition of Task Priorities */
#define game_PRIORITY      			17
#define startMenu_PRIORITY      	15
#define pauseMenu_PRIORITY      	14
#define countDownStart_PRIORITY     13
#define countDownPause_PRIORITY     13
#define startUpGame_PRIORITY      	16
#define endGame_PRIORITY      		14

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
char pauseMsgClr[40] = "                               \0";
char pauseMsg2[40] = "Press SPACEBAR\0";
char pauseMsg22[40] = "to pause\0";
char pauseMsg23[40] = "to continue\0";
char pauseMsg24[40] = "to restart\0";
char pauseMsg3[40] = "Press ESC\0";
char pauseMsg32[40] = "to quit\0";
char endGameMsg[40] = "GAME OVER!\0";
char keyMap[40] = "D         F         J         K\0";
char counter[40] = "\0";
char arrow[40] = ">\0";
char arrowClr[40] = " \0";
char count3[40] = "3\0";
char count2[40] = "2\0";
char count1[40] = "1\0";
char count0[40] = "0\0";

INT8U err;
int speedUpFactor = 0.02;
int scoreCounter = 1;
int gamemodeSelect;
int tileID;
int white = 0xFFFF;
int grey = 0x9CD3;
int black = 0x1061;
int block1_color, block2_color, block3_color, block4_color, block5_color;
int block1_reset, block2_reset, block3_reset, block4_reset, block5_reset;
int screenHeight = 240;
int screenWidth = 320;
KB_CODE_TYPE *decode_mode;
alt_u8 buf;
char ascii;
int screenTop = 0;
int blockSelect;
float dly = DELAYVALUE;
int state = 1;
int relevant_lane;
int button1, button2, button3, button4;
int block1_x_left, block1_x_right, block1_y_top, block1_y_bottom;
int block2_x_left, block2_x_right, block2_y_top, block2_y_bottom;
int block3_x_left, block3_x_right, block3_y_top, block3_y_bottom;
int block4_x_left, block4_x_right, block4_y_top, block4_y_bottom;
int block5_x_left, block5_x_right, block5_y_top, block5_y_bottom;

void randomKeyPress();
void game(void* pdata);
void countDownStart(void* pdata);
void countDownPause(void* pdata);
void blockMove(int* left, int* right, int* top, int* bottom, int color);
void blockMoveStartUp(int* left, int* right, int* top, int* bottom, int color);
void blockReset(int* left, int* right, int* top, int* bottom, int* blockSelect,
		int* color, int* reset);
void startUpGame(void* pdata);
void blockDetect();
void hitDetect(int* color, int* reset);
void buttonDetect();
void endGame(void* pdata);

void customDelay(float dly) {
	int i;
	for (i = 0; i < (dly * 250); i++) {
	}
}

void randomKeyPress() {
	int x;	//x-coordinate of decorative tiles
	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}

	//color previous key white
	if (tileID == 0 || tileID == 3 || tileID == 8) {//tiles with black key to the right
		DRAW_BOX(vgapixel, x, 0.75 * screenHeight, x + 7, screenHeight, white,
				0);
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, white, 0);	//218 is used to avoid drawing over black piano keys
	} else if (tileID == 2 || tileID == 7 || tileID == 11) {//tiles with black key to the left
		DRAW_BOX(vgapixel, x + 3, 0.75 * screenHeight, x + 10, screenHeight,
				white, 0);
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, white, 0);
	} else {	//tiles with black keys on both sides
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, white, 0);
		DRAW_BOX(vgapixel, x + 3, 0.75 * screenHeight, x + 7, screenHeight,
				white, 0);
	}

	tileID = rand() % 11;	//select new random key

	//prevent drawing on middle of screen
	if (tileID > 5) {
		x = 12 * (tileID + 15);
	} else {
		x = 12 * tileID;
	}
	//color current key grey
	if (tileID == 0 || tileID == 3 || tileID == 8) {//tiles with black key to the right
		DRAW_BOX(vgapixel, x, 0.75 * screenHeight, x + 7, screenHeight, grey,
				0);
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
	} else if (tileID == 2 || tileID == 7 || tileID == 11) {//tiles with black key to the left
		DRAW_BOX(vgapixel, x + 3, 0.75 * screenHeight, x + 10, screenHeight,
				grey, 0);
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
	} else {	//tiles with black keys on both sides
		DRAW_BOX(vgapixel, x, 218, x + 10, screenHeight, grey, 0);
		DRAW_BOX(vgapixel, x + 3, 0.75 * screenHeight, x + 7, screenHeight,
				grey, 0);
	}
}

void messageClear(int x, int y) {
	DRAW_STRING(vgachar, pauseMsgClr, x, y, 0);
}

void startMenu(void* pdata) {
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

	while (1) {
		decode_scancode(ps2, decode_mode, &buf, &ascii);
		if (*decode_mode == 1 || *decode_mode == 2) {
			if (ascii == '1') {
				DRAW_STRING(vgachar, arrowClr, 32, 41, 0);
				DRAW_STRING(vgachar, arrowClr, 32, 46, 0);
				gamemodeSelect = 1;
				DRAW_STRING(vgachar, arrow, 32, 36, 0);
			} else if (ascii == '2') {
				DRAW_STRING(vgachar, arrowClr, 32, 36, 0);
				DRAW_STRING(vgachar, arrowClr, 32, 46, 0);
				gamemodeSelect = 2;
				DRAW_STRING(vgachar, arrow, 32, 41, 0);
			} else if (ascii == '3') {
				DRAW_STRING(vgachar, arrowClr, 32, 36, 0);
				DRAW_STRING(vgachar, arrowClr, 32, 41, 0);
				gamemodeSelect = 3;
				DRAW_STRING(vgachar, arrow, 32, 46, 0);
			} else if (buf == 0x5A && gamemodeSelect) {
				OSTaskCreateExt(countDownStart, NULL,
						(void *) &countDownStart_stk[TASK_STACKSIZE - 1],
						countDownStart_PRIORITY, countDownStart_PRIORITY,
						countDownStart_stk, TASK_STACKSIZE, NULL, 0);
				OSTaskDel(OS_PRIO_SELF);
			}
		}
	}
}

void drawHUD() {
	DRAW_STRING(vgachar, text_top_row, 1, 2, 0);
	DRAW_STRING(vgachar, text_bottom_row, 61, 2, 0);
	DRAW_STRING(vgachar, mode1, 3, 8, 0);
	DRAW_STRING(vgachar, mode2, 3, 12, 0);
	DRAW_STRING(vgachar, mode3, 3, 16, 0);
	DRAW_STRING(vgachar, pauseMsg2, 3, 40, 0);
	messageClear(2, 42);
	DRAW_STRING(vgachar, pauseMsg22, 6, 42, 0);
	if (gamemodeSelect == 1) {
		DRAW_STRING(vgachar, arrow, 1, 8, 0);
	} else if (gamemodeSelect == 2) {
		DRAW_STRING(vgachar, arrow, 1, 12, 0);
	} else if (gamemodeSelect == 3) {
		DRAW_STRING(vgachar, arrow, 1, 16, 0);
	}
}

void countDownStart(void* pdata) {
	int decorativeWhiteTile, decorativeBlackTile;
	int screenTop = 0;

	CLRSCREEN;
	drawHUD();
	messageClear(32, 11);
	messageClear(28, 24);
	messageClear(28, 27);
	messageClear(34, 36);
	messageClear(34, 41);
	messageClear(34, 46);

	for (decorativeWhiteTile = 0; decorativeWhiteTile < 27;
			decorativeWhiteTile++) {//draws 27 white piano keys on background
		DRAW_BOX(vgapixel, decorativeWhiteTile * 12, screenHeight * 0.75,
				(decorativeWhiteTile * 12) + 10, screenHeight, white, 0);
	}
	for (decorativeBlackTile = 0; decorativeBlackTile < 26;
			decorativeBlackTile++) {//draws 26 black piano keys on background
		if (decorativeBlackTile == 2 || decorativeBlackTile == 6
				|| decorativeBlackTile == 19 || decorativeBlackTile == 22) {
		} else {
			DRAW_BOX(vgapixel, decorativeBlackTile * 12 + 8,
					screenHeight * 0.75, (decorativeBlackTile * 12) + 14, 217,
					black, 0);
		}
	}
	DRAW_BOX(vgapixel, screenWidth * 0.25, screenTop, screenWidth * 0.75,
			screenHeight, white, 0); //white game box
	DRAW_LINE(vgapixel, screenWidth * 0.375, screenTop, screenWidth * 0.375,
			screenHeight, grey, 0);
	DRAW_LINE(vgapixel, screenWidth * 0.5, screenTop, screenWidth * 0.5,
			screenHeight, grey, 0);
	DRAW_LINE(vgapixel, screenWidth * 0.625, screenTop, screenWidth * 0.625,
			screenHeight, grey, 0);

	DRAW_BOX(vgapixel, 90, 200, 110, 220, black, 0);
	DRAW_BOX(vgapixel, 130, 200, 150, 220, black, 0);
	DRAW_BOX(vgapixel, 171, 200, 191, 220, black, 0);
	DRAW_BOX(vgapixel, 211, 200, 231, 220, black, 0);
	DRAW_STRING(vgachar, keyMap, 25, 52, 0);

	DRAW_BOX(vgapixel, 268, 92, 286, 111, white, 0);
	DRAW_BOX(vgapixel, 269, 93, 285, 110, black, 0);
	DRAW_STRING(vgachar, count3, 69, 25, 0);
	OSTimeDlyHMSM(0, 0, 1, 100);
	DRAW_STRING(vgachar, count2, 69, 25, 0);
	OSTimeDlyHMSM(0, 0, 1, 100);
	DRAW_STRING(vgachar, count1, 69, 25, 0);
	OSTimeDlyHMSM(0, 0, 1, 100);
	DRAW_BOX(vgapixel, 90, 200, 110, 220, white, 0);
	DRAW_BOX(vgapixel, 130, 200, 150, 220, white, 0);
	DRAW_BOX(vgapixel, 171, 200, 191, 220, white, 0);
	DRAW_BOX(vgapixel, 211, 200, 231, 220, white, 0);
	messageClear(25, 52);
	messageClear(69, 25);
	DRAW_BOX(vgapixel, 268, 92, 286, 111, 0x0000, 0);

	OSTaskCreateExt(game, NULL, (void *) &game_stk[TASK_STACKSIZE - 1],
	game_PRIORITY, game_PRIORITY, game_stk, TASK_STACKSIZE, NULL, 0);

	OSTaskDel(OS_PRIO_SELF);

}

void countDownPause(void* pdata) {
	OSSemPend(sem_pause, 0, &err);
	DRAW_BOX(vgapixel, 268, 92, 286, 111, 0xFFFF, 0);
	DRAW_BOX(vgapixel, 269, 93, 285, 110, 0x1061, 0);
	DRAW_STRING(vgachar, count3, 69, 25, 0);
	OSTimeDlyHMSM(0, 0, 1, 100);
	DRAW_STRING(vgachar, count2, 69, 25, 0);
	OSTimeDlyHMSM(0, 0, 1, 100);
	DRAW_STRING(vgachar, count1, 69, 25, 0);
	OSTimeDlyHMSM(0, 0, 1, 100);
	messageClear(69, 25);
	DRAW_BOX(vgapixel, 268, 92, 286, 111, 0x0000, 0);

	OSSemPost(sem_pause);
	OSTaskDel(OS_PRIO_SELF);
}

void hitDetect(int* color, int* reset) {
	if (*reset == 1) {
		*color = black;
	} else if (*reset == 0) {
		*color = grey;
	}
}

void blockDetect() {
	switch (state) {
	case 1:
		hitDetect(&block5_color, &block5_reset);
		break;
	case 2:
		hitDetect(&block1_color, &block1_reset);
		break;
	case 3:
		hitDetect(&block2_color, &block2_reset);
		break;
	case 4:
		hitDetect(&block3_color, &block3_reset);
		break;
	case 5:
		hitDetect(&block4_color, &block4_reset);
		break;
	}
}

void pauseMenu(void* pdata) {
	alt_up_video_dma_screen_clear(vgachar, 0);
	DRAW_BOX(vgapixel, 8, 94, 70, 131, white, 0);
	DRAW_BOX(vgapixel, 9, 95, 69, 130, black, 0);
	DRAW_STRING(vgachar, pauseMsg, 5, 25, 0);
	DRAW_STRING(vgachar, pauseMsg3, 5, 29, 0);
	DRAW_STRING(vgachar, pauseMsg32, 6, 31, 0);
	drawHUD();
	messageClear(5, 42);
	DRAW_STRING(vgachar, pauseMsg23, 4, 42, 0);
	DRAW_STRING(vgachar, counter, 67, 7, 0);

	ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
	alt_up_ps2_init(ps2);

	while (1) {
		decode_scancode(ps2, decode_mode, &buf, &ascii);
		if (*decode_mode == 2) {
			if (buf == 0x29) {
				DRAW_BOX(vgapixel, 8, 94, 70, 131, 0x0000, 0);
				messageClear(5, 25);
				messageClear(4, 29);
				messageClear(5, 31);
				messageClear(2, 42);
				DRAW_STRING(vgachar, pauseMsg22, 6, 42, 0);
				messageClear(65, 25);
				OSTaskCreateExt(countDownPause, NULL,
						(void *) &countDownPause_stk[TASK_STACKSIZE - 1],
						countDownPause_PRIORITY, countDownPause_PRIORITY,
						countDownPause_stk, TASK_STACKSIZE, NULL, 0);
				OSTaskDel(OS_PRIO_SELF);
			} else if (buf == 0x76) {
				OSTaskDel(startUpGame_PRIORITY);
				OSTaskDel(game_PRIORITY);
				CLRSCREEN;
				state = 1;
				scoreCounter = 1;
				sprintf(counter, "%d", scoreCounter);
				gamemodeSelect = 0;
				dly = DELAYVALUE;
				OSTaskCreateExt(startMenu, NULL,
						(void *) &startMenu_stk[TASK_STACKSIZE - 1],
						startMenu_PRIORITY, startMenu_PRIORITY, startMenu_stk,
						TASK_STACKSIZE, NULL, 0);
				OSTaskDel(OS_PRIO_SELF);
			}
		}
	}
}

void endGame(void* pdata) {
	DRAW_BOX(vgapixel, 8, 104, 70, 123, white, 0);
	DRAW_BOX(vgapixel, 9, 105, 69, 122, black, 0);
	DRAW_STRING(vgachar, endGameMsg, 5, 28, 0);
	DRAW_STRING(vgachar, pauseMsg24, 5, 42, 0);

	ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
	alt_up_ps2_init(ps2);

	while (1) {
		decode_scancode(ps2, decode_mode, &buf, &ascii);
		if (*decode_mode == 2) {
			if (buf == 0x29) {
				OSTaskDel(startUpGame_PRIORITY);
				OSTaskDel(game_PRIORITY);
				CLRSCREEN;
				state = 1;
				scoreCounter = 1;
				sprintf(counter, "%d", scoreCounter);
				gamemodeSelect = 0;
				dly = DELAYVALUE;
				OSTaskCreateExt(startMenu, NULL,
						(void *) &startMenu_stk[TASK_STACKSIZE - 1],
						startMenu_PRIORITY, startMenu_PRIORITY, startMenu_stk,
						TASK_STACKSIZE, NULL, 0);
				OSTaskDel(OS_PRIO_SELF);
			}
		}
	}
}

void blockMove(int *left, int *right, int *top, int *bottom, int color) {
	if (*bottom - *top == TILE_HEIGHT) {
		*top = *top + 1;
		DRAW_LINE(vgapixel, *left + 1, *top - 1, *right - 1, *top - 1, white,
				0);
	}
	*bottom = *bottom + 1;
	DRAW_BOX(vgapixel, *left + 1, *top, *right - 1, *bottom, color, 0);
}

void blockMoveStartUp(int *left, int *right, int *top, int *bottom, int color) {
	if (*bottom - *top == TILE_HEIGHT) {
		*top = *top + 1;
		DRAW_LINE(vgapixel, *left + 1, *top - 1, *right - 1, *top - 1, white,
				0);
	}
	*bottom = *bottom + 1;
	DRAW_BOX(vgapixel, *left + 1, *top, *right - 1, *bottom, color, 0);
}

void blockReset(int* left, int* right, int* top, int* bottom, int* blockSelect,
		int* color, int* reset) {
	if (*top > screenHeight) {
		*blockSelect = rand() % 4;
		*left = (*blockSelect * TILE_WIDTH) + screenWidth * 0.25;
		*right = *left + TILE_WIDTH;
		*top = 0;
		*bottom = 0;
		*reset = 1;
		*color = black;
	}
}

void buttonDetect() {
	switch (state) {
	case 1:
		block1_reset = 0;
		break;
	case 2:
		block2_reset = 0;
		break;
	case 3:
		block3_reset = 0;
		break;
	case 4:
		block4_reset = 0;
		break;
	case 5:
		block5_reset = 0;
		break;
	}
	state++;
	if (state == 6) {
		state = 1;
	}
	randomKeyPress();
	sprintf(counter, "%d", scoreCounter++);
	DRAW_STRING(vgachar, counter, 67, 7, 0);
	dly = dly - (speedUpFactor * dly);
}

void startUpGame(void* pdata) {
	OSSemPend(sem_startUp, 0, &err);
	while (block1_y_bottom < screenHeight) {
		OSSemPend(sem_pause, 0, &err);
		OSSemPost(sem_pause);

		blockDetect();

		blockMoveStartUp(&block1_x_left, &block1_x_right, &block1_y_top,
				&block1_y_bottom, block1_color);
		if (block1_y_bottom > TILE_HEIGHT) {
			blockMoveStartUp(&block2_x_left, &block2_x_right, &block2_y_top,
					&block2_y_bottom, block2_color);
		}
		if (block2_y_bottom > TILE_HEIGHT) {
			blockMoveStartUp(&block3_x_left, &block3_x_right, &block3_y_top,
					&block3_y_bottom, block3_color);
		}
		if (block3_y_bottom > TILE_HEIGHT) {
			blockMoveStartUp(&block4_x_left, &block4_x_right, &block4_y_top,
					&block4_y_bottom, block4_color);
		}
		if (block4_y_bottom > TILE_HEIGHT) {
			blockMoveStartUp(&block5_x_left, &block5_x_right, &block5_y_top,
					&block5_y_bottom, block5_color);
		}

		decode_scancode(ps2, decode_mode, &buf, &ascii);
		if (*decode_mode == 2) {
			if (buf == 0x29) {
				OSTaskCreateExt(pauseMenu, NULL,
						(void *) &pauseMenu_stk[TASK_STACKSIZE - 1],
						pauseMenu_PRIORITY, pauseMenu_PRIORITY, pauseMenu_stk,
						TASK_STACKSIZE, NULL, 0);
			}
		} else if (*decode_mode == 1) {
			switch (state) {
			case 1:
				relevant_lane = (block1_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 2:
				relevant_lane = (block2_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 3:
				relevant_lane = (block3_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 4:
				relevant_lane = (block4_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 5:
				relevant_lane = (block5_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			}

			if (ascii == 'D' && relevant_lane == 0) {
				buttonDetect();
			} else if (ascii == 'F' && relevant_lane == 1) {
				buttonDetect();
			} else if (ascii == 'J' && relevant_lane == 2) {
				buttonDetect();
			} else if (ascii == 'K' && relevant_lane == 3) {
				buttonDetect();
			} else {
				OSTaskCreateExt(endGame, NULL,
						(void *) &endGame_stk[TASK_STACKSIZE - 1],
						endGame_PRIORITY, endGame_PRIORITY, endGame_stk,
						TASK_STACKSIZE, NULL, 0);
			}
		}
		customDelay(dly);
	}
	OSSemPost(sem_startUp);
	OSTaskDel(OS_PRIO_SELF);
}

void game(void* pdata) {
	srand(time(NULL));
	block1_color = black;
	block2_color = black;
	block3_color = black;
	block4_color = black;
	block5_color = black;
	block1_reset = 1;
	block2_reset = 1;
	block3_reset = 1;
	block4_reset = 1;
	block5_reset = 1;
	block1_x_left = (blockSelect * TILE_WIDTH) + screenWidth * 0.25, block1_x_right =
			block1_x_left + TILE_WIDTH, block1_y_top = screenTop, block1_y_bottom =
			block1_y_top;
	blockSelect = rand() % 4;
	block2_x_left = (blockSelect * TILE_WIDTH) + screenWidth * 0.25, block2_x_right =
			block2_x_left + TILE_WIDTH, block2_y_top = screenTop, block2_y_bottom =
			block2_y_top;
	blockSelect = rand() % 4;
	block3_x_left = (blockSelect * TILE_WIDTH) + screenWidth * 0.25, block3_x_right =
			block3_x_left + TILE_WIDTH, block3_y_top = screenTop, block3_y_bottom =
			block3_y_top;
	blockSelect = rand() % 4;
	block4_x_left = (blockSelect * TILE_WIDTH) + screenWidth * 0.25, block4_x_right =
			block4_x_left + TILE_WIDTH, block4_y_top = screenTop, block4_y_bottom =
			block4_y_top;
	blockSelect = rand() % 4;
	block5_x_left = (blockSelect * TILE_WIDTH) + screenWidth * 0.25, block5_x_right =
			block5_x_left + TILE_WIDTH, block5_y_top = screenTop, block5_y_bottom =
			block5_y_top;

	relevant_lane = (block1_x_left - (screenWidth * 0.25)) / TILE_WIDTH;

	ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
	alt_up_ps2_init(ps2);

	alt_up_video_dma_screen_clear(vgachar, 0);				//clear buffer

	drawHUD();
	OSTaskCreateExt(startUpGame, NULL,
			(void *) &startUpGame_stk[TASK_STACKSIZE - 1],
			startUpGame_PRIORITY, startUpGame_PRIORITY, startUpGame_stk,
			TASK_STACKSIZE, NULL, 0);

	while (1) {
		OSSemPend(sem_pause, 0, &err);
		OSSemPost(sem_pause);
		OSSemPend(sem_startUp, 0, &err);
		OSSemPost(sem_startUp);

		blockDetect();

		blockMove(&block1_x_left, &block1_x_right, &block1_y_top,
				&block1_y_bottom, block1_color);
		blockMove(&block2_x_left, &block2_x_right, &block2_y_top,
				&block2_y_bottom, block2_color);
		blockMove(&block3_x_left, &block3_x_right, &block3_y_top,
				&block3_y_bottom, block3_color);
		blockMove(&block4_x_left, &block4_x_right, &block4_y_top,
				&block4_y_bottom, block4_color);
		blockMove(&block5_x_left, &block5_x_right, &block5_y_top,
				&block5_y_bottom, block5_color);

		blockReset(&block1_x_left, &block1_x_right, &block1_y_top,
				&block1_y_bottom, &blockSelect, &block1_color, &block1_reset);
		blockReset(&block2_x_left, &block2_x_right, &block2_y_top,
				&block2_y_bottom, &blockSelect, &block2_color, &block2_reset);
		blockReset(&block3_x_left, &block3_x_right, &block3_y_top,
				&block3_y_bottom, &blockSelect, &block3_color, &block3_reset);
		blockReset(&block4_x_left, &block4_x_right, &block4_y_top,
				&block4_y_bottom, &blockSelect, &block4_color, &block4_reset);
		blockReset(&block5_x_left, &block5_x_right, &block5_y_top,
				&block5_y_bottom, &blockSelect, &block5_color, &block5_reset);

		decode_scancode(ps2, decode_mode, &buf, &ascii);
		if (*decode_mode == 2) {
			if (buf == 0x29) {
				OSTaskCreateExt(pauseMenu, NULL,
						(void *) &pauseMenu_stk[TASK_STACKSIZE - 1],
						pauseMenu_PRIORITY, pauseMenu_PRIORITY, pauseMenu_stk,
						TASK_STACKSIZE, NULL, 0);
			}
		} else if (*decode_mode == 1) {
			switch (state) {
			case 1:
				relevant_lane = (block1_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 2:
				relevant_lane = (block2_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 3:
				relevant_lane = (block3_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 4:
				relevant_lane = (block4_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			case 5:
				relevant_lane = (block5_x_left - (screenWidth * 0.25))
						/ TILE_WIDTH;
				break;
			}
			if (ascii == 'D' && relevant_lane == 0) {
				buttonDetect();
			} else if (ascii == 'F' && relevant_lane == 1) {
				buttonDetect();
			} else if (ascii == 'J' && relevant_lane == 2) {
				buttonDetect();
			} else if (ascii == 'K' && relevant_lane == 3) {
				buttonDetect();
			} else {
				OSTaskCreateExt(endGame, NULL,
						(void *) &endGame_stk[TASK_STACKSIZE - 1],
						endGame_PRIORITY, endGame_PRIORITY, endGame_stk,
						TASK_STACKSIZE, NULL, 0);
			}
		}
		customDelay(dly);
	}
}

/* The main function creates two task and starts multi-tasking */
int main(void) {
	OSInit();
	tileID = rand() % 11;

	sem_pause = OSSemCreate(1);
	sem_startUp = OSSemCreate(1);

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

	OSTaskCreateExt(startMenu, NULL,
			(void *) &startMenu_stk[TASK_STACKSIZE - 1],
			startMenu_PRIORITY, startMenu_PRIORITY, startMenu_stk,
			TASK_STACKSIZE, NULL, 0);

	OSStart();
	return 0;
}
