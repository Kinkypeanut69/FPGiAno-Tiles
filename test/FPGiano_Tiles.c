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

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK task1_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK1_PRIORITY      10

/*variables - devices*/
alt_up_pixel_buffer_dma_dev* vgapixel;				//pixel buffer device
alt_up_video_dma_dev* vgachar;						//char buffer device

/* create a message to be displayed on the VGA and LCD displays */
char text_top_row[40] = "-FPGiano Tiles!-\0";
char text_bottom_row[40];


void task1(void* pdata) {
	int count = 0;
	int x1 = 80, y1=0, x2 = 240, y2 = 240;
	int x3 = 120, y3=0, x4 = 120, y4 = 240;
	int x5 = 80, y5=0, x6 = 240;
	int y6 = 60, y7 = 120, y8 = 180;
	int c1 = 0x0A0A;
	int c2 = 0xFFFF;
	srand(time(NULL));
	int blockSelect = rand() % 4 + 1;

	while (1) {

		sprintf(text_bottom_row, "%d", count++);

		alt_up_pixel_buffer_dma_draw_box(vgapixel, x1, y1, x2, y2, c1, 0);

		alt_up_pixel_buffer_dma_draw_line(vgapixel, x3, y3, x4, y4, c2, 0);
		alt_up_pixel_buffer_dma_draw_line(vgapixel, x3+40, y3, x4+40, y4, c2, 0);
		alt_up_pixel_buffer_dma_draw_line(vgapixel, x3+80, y3, x4+80, y4, c2, 0);

		while(1){
			switch(blockSelect){
			case 1:
//				alt_up_pixel_buffer_dma_draw_box(vgapixel, x5, y5+1, x5+40, y6, c2, 0);
				break;

			case 2:
//				alt_up_pixel_buffer_dma_draw_box(vgapixel, x1, y1, x2, y2, c1, 0);
				break;

			case 3:
//				alt_up_pixel_buffer_dma_draw_box(vgapixel, x1, y1, x2, y2, c1, 0);
				break;

			case 4:
//				alt_up_pixel_buffer_dma_draw_box(vgapixel, x1, y1, x2, y2, c1, 0);
				break;
			}

			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y5, x6, y5, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y5-1, x6, y5-1, c1, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+40, y5-1, x5+40, y5-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+80, y5-1, x5+80, y5-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+120, y5-1, x5+120, y5-1, c2, 0);
			y5++;

			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y6, x6, y6, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y6-1, x6, y6-1, c1, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+40, y6-1, x5+40, y6-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+80, y6-1, x5+80, y6-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+120, y6-1, x5+120, y6-1, c2, 0);
			y6++;

			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y7, x6, y7, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y7-1, x6, y7-1, c1, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+40, y7-1, x5+40, y7-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+80, y7-1, x5+80, y7-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+120, y7-1, x5+120, y7-1, c2, 0);
			y7++;

			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y8, x6, y8, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5, y8-1, x6, y8-1, c1, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+40, y8-1, x5+40, y8-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+80, y8-1, x5+80, y8-1, c2, 0);
			alt_up_pixel_buffer_dma_draw_line(vgapixel, x5+120, y8-1, x5+120, y8-1, c2, 0);
			y8++;

			if (y5 > 240){
				y5 = 0;
			}

			if (y6 > 240){
				y6 = 0;
			}
			if (y7 > 240){
				y7 = 0;
			}
			if (y8 > 240){
				y8 = 0;
			}
			OSTimeDlyHMSM(0,0,0,60);
		}
	}
}

/* The main function creates two task and starts multi-tasking */
int main(void) {
	OSInit();

	vgapixel = alt_up_pixel_buffer_dma_open_dev("/dev/VGA_Subsystem_VGA_Pixel_DMA");		//open pixel buffer
	if (vgapixel == NULL) {
		printf("Error: could not open VGA_Pixel_Buffer device\n");
		return -1;
	}
	else
		printf("Opened VGA_Pixel_Buffer device\n");	
	
	alt_up_pixel_buffer_dma_clear_screen(vgapixel, 0);							//clear screen
	
	vgachar = alt_up_video_dma_open_dev("/dev/VGA_Subsystem_Char_Buf_Subsystem_Char_Buf_DMA");				//open char buffer
	if (vgachar == NULL) {
		printf("Error: could not open VGA_Char_Buffer device\n");
		return -1;
	}
	else
		printf("Opened VGA_Char_Buffer device\n");	

	alt_up_video_dma_screen_clear(vgachar, 0);											//clear buffer

	alt_up_video_dma_draw_string(vgachar, text_top_row, 1, 2, 0);

	OSTaskCreateExt(task1, NULL, (void *) &task1_stk[TASK_STACKSIZE - 1],
			TASK1_PRIORITY, TASK1_PRIORITY, task1_stk, TASK_STACKSIZE, NULL, 0);

	OSStart();
	return 0;
}

