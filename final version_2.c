/* This file provides address values that exist in the system */
#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF
#define FPGA_CHAR_BASE        0xC9000000
#define FPGA_CHAR_END         0xC9001FFF

/* Cyclone V FPGA devices */
#define LEDR_BASE             0xFF200000
#define HEX3_HEX0_BASE        0xFF200020
#define HEX5_HEX4_BASE        0xFF200030
#define SW_BASE               0xFF200040
#define KEY_BASE              0xFF200050
#define JP1_BASE              0xFF200060
#define JP2_BASE              0xFF200070
#define PS2_BASE              0xFF200100
#define PS2_DUAL_BASE         0xFF200108
#define JTAG_UART_BASE        0xFF201000
#define JTAG_UART_2_BASE      0xFF201008
#define IrDA_BASE             0xFF201020
#define TIMER_BASE            0xFF202000
#define AV_CONFIG_BASE        0xFF203000
#define PIXEL_BUF_CTRL_BASE   0xFF203020
#define CHAR_BUF_CTRL_BASE    0xFF203030
#define AUDIO_BASE            0xFF203040
#define VIDEO_IN_BASE         0xFF203060
#define ADC_BASE              0xFF204000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C

/* ARM A9 MPCORE devices */
#define   PERIPH_BASE         0xFFFEC000    // base address of peripheral devices
#define   MPCORE_PRIV_TIMER   0xFFFEC600    // PERIPH_BASE + 0x0600

/* Interrupt controller (GIC) CPU interface(s) */
#define MPCORE_GIC_CPUIF      0xFFFEC100    // PERIPH_BASE + 0x100
#define ICCICR                0x00          // offset to CPU interface control reg
#define ICCPMR                0x04          // offset to interrupt priority mask reg
#define ICCIAR                0x0C          // offset to interrupt acknowledge reg
#define ICCEOIR               0x10          // offset to end of interrupt reg
	
/* Interrupt controller (GIC) distributor interface(s) */
#define MPCORE_GIC_DIST       0xFFFED000    // PERIPH_BASE + 0x1000
#define ICDDCR                0x00          // offset to distributor control reg
#define ICDISER               0x100         // offset to interrupt set-enable regs
#define ICDICER               0x180         // offset to interrupt clear-enable regs
#define ICDIPTR               0x800         // offset to interrupt processor targets regs
#define ICDICFR               0xC00         // offset to interrupt configuration regs

#define ABS(x) (((x) > 0) ? (x) : -(x))
	
/* function prototypes */
void plot_pixel(int, int, short int);
void video_text(int, int, char *);
void video_box(int, int, int, int, short);
void clear_box_and_line();
volatile int pixel_buffer_start; // global variable
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void clear_screen();
void wait_for_vsync();
void plot_pixel(int x, int y, short int line_color);
void draw_background();
void plot_box(int x,int y,short int box_color);
void display_score(int score);
char* generateChar(int num);
void draw_rhythm_master();
void draw_press_enter();
void display_miss(int score);

/* VGA colors */
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
	
/* Screen size. */
#define RESOLUTION_X 320
#define RESOLUTION_Y 240

/* Constants for animation */
#define BOX_LEN 2
#define NUM_BOXES 8

#define FALSE 0
#define TRUE 1

#include <stdlib.h>
#include <stdio.h>	
#include <stdbool.h>
	
/* global variables */
int screen_x; 
int screen_y; 
void swap(int *a,int *b);
bool press_enter = false;
bool press_D = false;
bool press_F = false;
bool press_J = false;
bool press_K = false;
int score = 0;
//int missClicking = 0;
int updateNum;
int addCountFix;
int falseClick = 0;
bool restart = false;
char empty[60] = "                                                       \0";

/*******************************************************************************
*	This program demonstrates use of the video in the computer system.
*	Draws a blue box on the video display, and places a text string inside the box
******************************************************************************/
int main(void) {
	video_text(29, 29, empty);
	video_text(29, 31, empty);
	
	//find the coordinates of the screen
	volatile int* video_resolution = (int *)(PIXEL_BUF_CTRL_BASE + 0x8); 
	screen_x = *video_resolution & 0xFFFF;
	screen_y = (*video_resolution >> 16) & 0xFFFF;
		
	video_box(0, 0, screen_x, screen_y, 0); // clear the display
	
	//initial set up
	//char text_top_row[40]	= "RHYTHM MASTER\0";
	//char text_bottom_row[40] = "PRESS ENTER TO START\0";
	
	//display the score when starts to play
	char text_top_left[40] = "SCORE:\0";
	
	//detects if user enters the game
	volatile int* PS2_ptr = (int *)PS2_BASE;
	*(PS2_ptr) = 0xff;
	press_enter = false;
	int	PS2_data, RVALID;
	char byte1 = 0, byte2 = 0, byte3 = 0;

	video_text(2, 3, empty);
	video_text(4, 2, empty);
	video_text(65,3, empty);
	
	volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
	pixel_buffer_start = *pixel_ctrl_ptr;
	clear_screen(); // pixel_buffer_start points to the pixel buffer

	/* set back pixel buffer to start of SDRAM memory */
	pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
	clear_screen();
	
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
	
	while (1) {
		PS2_data = *(PS2_ptr);	//read the Data register in the PS/2 port
		RVALID	= PS2_data & 0x8000; //extract the RVALID field
		
		//user press any keys
		if (RVALID) {
			byte1 = byte2; 
			byte2 = byte3;
			byte3 = PS2_data & 0xFF; 
		}

		//if press "Enter"
		if(byte1 == 0x5A && byte2 == 0xF0 && byte3 == 0x5A) {
			press_enter = true;
		}
		
		if(press_enter) {
			video_text(2, 3, text_top_left);
			video_text(65,3, "Missed:");
			break;
		} else {
			draw_rhythm_master();
			draw_press_enter();
			//video_text(33, 10, text_top_row);
			//video_text(29, 12, text_bottom_row);
			//video_box(29 * 4 - 2, 9 * 4, 49 * 4, 13 * 4 + 2, BLUE);
			
			video_text(23, 20, "Press Key 'D' 'F' 'J' 'K' to Play");
			video_text(26, 22, "D Represents Left-Most Region");
			video_text(25, 24, "K Represents Right-Most Region");
			video_box(23 * 4 - 2, 19 * 4, 56 * 4, 25 * 4 + 2, BLUE);
			
			video_text(19, 32, "Goal: As Pink Blocks Hit Bottom Regions");
			video_text(27, 34, "Press Correspoding Keys");
			video_text(19, 36, "Gain Points if Hit Quickly and Correctly");
			video_text(17, 38, "Otherwise No Points Gained and Lose the Game");
			video_box(17 * 4 - 2, 31 * 4, 61 * 4, 39 * 4 + 2, BLUE);
		}
	}
	
    clear_box_and_line();
	
	//volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
    // initialize location and direction of rectangles(not shown)

    /* set front pixel buffer to start of FPGA On-chip memory */
    //*(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    //wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    //pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
	//draw the game background
	draw_background();
	
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();
	
	//draw the game background
	draw_background();
	
	//initialize arrays for rectangles
	int x_box[10],y_box[10];
    int last_x[10],last_y[10],last_x_2[10],last_y_2[10];
	int count = 1; //for remember which
	//now the every 1/12s moves down 1 pixel
	int downCount = 1;
	//add box every 2s
	int addCount = 40;
	//number of currently displaying box
	int num = 0;
	
	while (1) {		
		if(score <= 10){
			updateNum = 1;
			addCountFix = 40;
		}else if(score >= 10 && score <=20){
			updateNum = 2;
			addCountFix = 20;
		}else if(score >= 20){
			updateNum = 3;
			addCountFix = 12;
		}
		
		downCount--;
		if(downCount == 0){
			//reset downCount
			downCount = 1;

			//check if need to add new box
			addCount--;
			if(addCount == 0){
				//add a new box
				x_box[num] = (rand()%4 * 79)+41;
				y_box[num] = 34;
				last_x[num] = x_box[num];
				last_x_2[num] = x_box[num];
				last_y[num] = y_box[num];
				last_y_2[num] = y_box[num];
				num++;
				//reset add count
				addCount = addCountFix;
			}

			/* Erase any boxes and lines that were drawn in the last iteration */
			if(count == 1){
				for(int m = 0;m < num;m++){
					plot_box(last_x[m],last_y[m],0x0000);
				}
			}else{
				for(int m = 0;m < num;m++){
					plot_box(last_x_2[m],last_y_2[m],0x0000);
				}
			}
			// code for drawing the boxes and lines (not shown)
			for(int p = 0;p < num;p++){
				if(x_box[p] == 41){
					plot_box(x_box[p],y_box[p],CYAN);
				}else if(x_box[p] == 120){
					plot_box(x_box[p],y_box[p],PINK);
				}else if(x_box[p] == 199){
					plot_box(x_box[p],y_box[p],YELLOW);
				}else{
					plot_box(x_box[p],y_box[p],GREEN);
				}
			}
			//remember the previous position
			if(count == 1){
				for(int n = 0;n < num;n++){
					last_x[n] = x_box[n];
					last_y[n] = y_box[n];
				}
			}else{
				for(int n = 0;n < num;n++){
					last_x_2[n] = x_box[n];
					last_y_2[n] = y_box[n];
				}
			}
			// code for updating the locations of boxes (not shown)
			for(int i = 0;i < num;i++){
				y_box[i] += updateNum;
				
				//if the any box touches the bottom
				if(y_box[i] >= 226){
					//redraw the bottom area so that it becomes 
					//original background again
					pixel_buffer_start = *pixel_ctrl_ptr;
					plot_box(last_x[i],last_y[i],0x0000);
					plot_box(last_x_2[i],last_y_2[i],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
			    	// then,draw on the back buffer
					pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
					plot_box(last_x[i],last_y[i],0x0000);
					plot_box(last_x_2[i],last_y_2[i],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
					//remove that box from the array and resettle the arrays
					for(int g = i ; g < 10 ; g++){
						x_box[g] = x_box[g+1];
					    y_box[g] = y_box[g+1];	
						last_x[g] = last_x[g+1];
					    last_y[g] = last_y[g+1];
						last_x_2[g] = last_x_2[g+1];
					    last_y_2[g] = last_y_2[g+1];
					}
					num--;
					falseClick++;
				}
			}
			count *= -1;
		}
		
		wait_for_vsync(); // swap front and back buffers on VGA vertical sync
		pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer	
		
		PS2_data = *(PS2_ptr);	//read the Data register in the PS/2 port
		RVALID	= PS2_data & 0x8000; //extract the RVALID field
		
		//user press any keys
		if (RVALID) {
			byte1 = byte2; 
			byte2 = byte3;
			byte3 = PS2_data & 0xFF; 
		}
		
	    if(byte1 == 0x23 && byte2 == 0xF0 && byte3 == 0x23) {
			press_D = true;
		}else if(byte1 == 0x2b && byte2 == 0xF0 && byte3 == 0x2b){
			press_F = true;
		}else if(byte1 == 0x3b && byte2 == 0xF0 && byte3 == 0x3b){
			press_J = true;
		}else if(byte1 == 0x42 && byte2 == 0xF0 && byte3 == 0x42){
			press_K = true;
		}
		
		if(press_D) {
			press_D = false;
			*(PS2_ptr) = 0xff;
			bool falseClicking = true;
			for(int d = 0;d < num;d++){
				if(x_box[d]==41 && (y_box[d] >= 205 && y_box[d] < 226)){
				     score++;
					 falseClicking = false;
					 //original background again
					pixel_buffer_start = *pixel_ctrl_ptr;
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
			    	// then,draw on the back buffer
					pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
					//remove that box from the array and resettle the arrays
					for(int g = d ; g < 10 ; g++){
						x_box[g] = x_box[g+1];
					    y_box[g] = y_box[g+1];	
						last_x[g] = last_x[g+1];
					    last_y[g] = last_y[g+1];
						last_x_2[g] = last_x_2[g+1];
					    last_y_2[g] = last_y_2[g+1];
					}
					num--;
				}
			}
			if(falseClicking){
				falseClick++;
			}
		}
		
		if(press_F) {
			press_F = false;
			*(PS2_ptr) = 0xff;
			bool falseClicking = true;
			for(int d = 0;d < num;d++){
				if(x_box[d]==120 && (y_box[d] >= 205 && y_box[d] < 226)){
				     score++;
					falseClicking = false;
					 //original background again
					pixel_buffer_start = *pixel_ctrl_ptr;
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
			    	// then,draw on the back buffer
					pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
					//remove that box from the array and resettle the arrays
					for(int g = d ; g < 10 ; g++){
						x_box[g] = x_box[g+1];
					    y_box[g] = y_box[g+1];	
						last_x[g] = last_x[g+1];
					    last_y[g] = last_y[g+1];
						last_x_2[g] = last_x_2[g+1];
					    last_y_2[g] = last_y_2[g+1];
					}
					num--;
				}
			}
			if(falseClicking){
				falseClick++;
			}
		}
		
		if(press_J) {
			press_J = false;
			*(PS2_ptr) = 0xff;
			bool falseClicking = true;
			for(int d = 0;d < num;d++){
				if(x_box[d]==199 && (y_box[d] >= 205 && y_box[d] < 226)){
				     score++;
					falseClicking = false;
					 //original background again
					pixel_buffer_start = *pixel_ctrl_ptr;
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
			    	// then,draw on the back buffer
					pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
					//remove that box from the array and resettle the arrays
					for(int g = d ; g < 10 ; g++){
						x_box[g] = x_box[g+1];
					    y_box[g] = y_box[g+1];	
						last_x[g] = last_x[g+1];
					    last_y[g] = last_y[g+1];
						last_x_2[g] = last_x_2[g+1];
					    last_y_2[g] = last_y_2[g+1];
					}
					num--;
				}
			}
			if(falseClicking){
				falseClick++;
			}
		}
		
		if(press_K) {
			press_K = false;
			*(PS2_ptr) = 0xff;
			bool falseClicking = true;
			for(int d = 0;d < num;d++){
				if(x_box[d]==278 && (y_box[d] >= 205 && y_box[d] < 226)){
				     score++;
					falseClicking = false;
					 //original background again
					pixel_buffer_start = *pixel_ctrl_ptr;
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
			    	// then,draw on the back buffer
					pixel_buffer_start = *(pixel_ctrl_ptr + 1); 
					plot_box(last_x[d],last_y[d],0x0000);
					plot_box(last_x_2[d],last_y_2[d],0x0000);
					draw_line(0,212,319,212,0xFC00);
					draw_line(0,213,319,213,0xFC00);
					draw_line(0,214,319,214,0xFC00);
					draw_line(0,215,319,215,0xFC00);
					//remove that box from the array and resettle the arrays
					for(int g = d ; g < 10 ; g++){
						x_box[g] = x_box[g+1];
					    y_box[g] = y_box[g+1];	
						last_x[g] = last_x[g+1];
					    last_y[g] = last_y[g+1];
						last_x_2[g] = last_x_2[g+1];
					    last_y_2[g] = last_y_2[g+1];
					}
					num--;
				}
			}
			if(falseClicking){
				falseClick++;
			}
		}
		if(falseClick >= 10){
			break;
		}
		//video_text(8,3,"5\0");
		display_score(score);
		display_miss(falseClick);
	}
	
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen();
	
	//video_text(35,29,"GAME OVEREEE\0");
	display_miss(falseClick);
	video_text(36, 29, "GAME OVER\0");
	video_text(30, 31, "Press ESC to Restart\0");
	
	while(1){
		PS2_data = *(PS2_ptr);	//read the Data register in the PS/2 port
		RVALID	= PS2_data & 0x8000; //extract the RVALID field
		
		//user press any keys
		if (RVALID) {
			byte1 = byte2; 
			byte2 = byte3;
			byte3 = PS2_data & 0xFF; 
		}
	    if(byte1 == 0x76 && byte2 == 0xF0 && byte3 == 0x76) {
			restart = true;
		}
		if(restart){
			restart = false;
			*(PS2_ptr) = 0xff;
			press_enter = false;
			press_D = false;
			press_F = false;
			press_J = false;
			press_K = false;
			score = 0;
			falseClick  = 0;
			
			pixel_buffer_start = *pixel_ctrl_ptr;
			clear_screen(); // pixel_buffer_start points to the pixel buffer

			/* set back pixel buffer to start of SDRAM memory */
			pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
			clear_screen();
			
			main();
		}
	}
}

/*******************************************************************************
*	Subroutine to send a string of text to the video monitor
******************************************************************************/
void video_text(int x, int y, char * text_ptr) {
	int	offset;
	volatile char* character_buffer = (char *)FPGA_CHAR_BASE; //video character buffer

/* assume that the text string fits on one line	*/
	offset = (y << 7) + x;
	while (*(text_ptr)) {
	*(character_buffer + offset) =
		*(text_ptr); //write to the character buffer
		++text_ptr;
		++offset;
	}
}

/*******************************************************************************
*	Draw a filled rectangle on the video monitor
*	Takes in points assuming 320x240 resolution
******************************************************************************/
void video_box(int x1, int y1, int x2, int y2, short pixel_color) {
	int pixel_buf_ptr = *(int *)PIXEL_BUF_CTRL_BASE;
	int pixel_ptr, row, col;

	/* assume that the box coordinates are valid */
	for (row = y1; row <= y2; row++) {
		for (col = x1; col <= x2; ++col) { 
			pixel_ptr = pixel_buf_ptr +(row << (10)) + (col << 1);
			*(short *)pixel_ptr = pixel_color; //set pixel color
		}
	}
}

/*******************************************************************************
*	plot pixels
******************************************************************************/
void plot_pixel(int x, int y, short int line_color)
{
	//volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
	//int pixel_buffer_start = *(pixel_ctrl_ptr);
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

/*******************************************************************************
*	swap two numbers
******************************************************************************/
void swap(int *a,int *b) {
	int temp = *a;
	*a=*b;
	*b=temp;
}

/*******************************************************************************
*	Draw a line on the video monitor
******************************************************************************/
void draw_line(int x0, int y0, int x1, int y1, short int line_color){
	bool is_steep = ABS(y1-y0) > ABS(x1-x0);
	
	if(is_steep) {
	swap(&x0,&y0);
	swap(&x1,&y1);
	}
	
	if(x0>x1) {
	swap(&x0,&x1);
	swap(&y0,&y1);
	}
	
	int deltax = x1-x0;
	int deltay = ABS(y1-y0);
	int error = -(deltax / 2);
	int y = y0;
	int y_step =0;
	if(y0<y1) {
		y_step =1;
	} else {
		y_step=-1;
	}
	
	for(int x=x0; x<=x1; x++) {
		if (is_steep) {
			plot_pixel(y,x, line_color);
		} else {
			plot_pixel(x,y, line_color);
		}
		
		error = error + deltay;
		if (error >= 0) {
			y=y+y_step;
			error = error - deltax;
		}
	}
}

/*******************************************************************************
*	clear previous texts and boxes
******************************************************************************/
void clear_box_and_line() {
	if (!press_enter) {
		video_text(0, 4, empty);
		//video_text(50,3, empty);
	} else {
		video_text(33, 10, empty);
			//video_text(29, 12, empty);
			//video_box(29 * 4 - 2, 9 * 4, 49 * 4, 13 * 4 + 2, 0x0);
			clear_screen();
			
			video_text(23, 20, empty);
			video_text(26, 22, empty);
			video_text(25, 24, empty);
			video_box(23 * 4 - 2, 19 * 4, 56 * 4, 25 * 4 + 2, 0x0);
			
			video_text(19, 32, empty);
			video_text(27, 34, empty);
			video_text(19, 36, empty);
			video_text(17, 38, empty);
			video_box(17 * 4 - 2, 31 * 4, 61 * 4, 39 * 4 + 2, 0x0);
	}
}


void clear_screen(){
	for(int a = 0 ; a < 320 ; a++){
		for(int b = 0 ; b < 240 ; b++){
			plot_pixel(a,b,0x0000);//black
		}
	}
}

void wait_for_vsync(){
    volatile int* pixel_ctrl_ptr = (int*)0xff203020;
	register int status;
	
	*pixel_ctrl_ptr = 1;// start synchronization process
	
	status = *(pixel_ctrl_ptr +3);
	while((status & 0x01) != 0){
		status = *(pixel_ctrl_ptr + 3); //polling to wait for s becomes 0
	}
}

void plot_box(int x,int y,short int box_color){

	for(int j = x-37 ; j <= x+37 ; j++){
		for(int k = y-10 ; k <= y+10 ; k++){			
		    	plot_pixel(j,k,box_color);
			
		}
	}
}
	
void draw_background(){
    draw_line(0,0,319,0,GREY);
	draw_line(0,1,319,1,GREY);
	draw_line(0,2,319,2,GREY);
	draw_line(0,3,319,3,GREY);
	
	draw_line(0,236,319,236,GREY);
	draw_line(0,237,319,237,GREY);
	draw_line(0,238,319,238,GREY);
	draw_line(0,239,319,239,GREY);
	
	draw_line(0,0,0,239,GREY);
	draw_line(1,0,1,239,GREY);
	draw_line(2,0,2,239,GREY);
	draw_line(3,0,3,239,GREY);
	
	draw_line(316,0,316,239,GREY);
	draw_line(317,0,317,239,GREY);
	draw_line(318,0,318,239,GREY);
	draw_line(319,0,319,239,GREY);
	
	draw_line(79,23,79,239,GREY);
	draw_line(80,23,80,239,GREY);
	draw_line(81,23,81,239,GREY);
	draw_line(82,23,82,239,GREY);
	
	draw_line(158,23,158,239,GREY);
	draw_line(159,23,159,239,GREY);
	draw_line(160,23,160,239,GREY);
	draw_line(161,23,161,239,GREY);
	
	draw_line(237,23,237,239,GREY);
	draw_line(238,23,238,239,GREY);
	draw_line(239,23,239,239,GREY);
	draw_line(240,23,240,239,GREY);
	
	draw_line(0,20,319,20,GREY);
	draw_line(0,21,319,21,GREY);
	draw_line(0,22,319,22,GREY);
	draw_line(0,23,319,23,GREY);
	
	draw_line(0,212,319,212,0xFC00);
	draw_line(0,213,319,213,0xFC00);
	draw_line(0,214,319,214,0xFC00);
	draw_line(0,215,319,215,0xFC00);
}

void display_score(int score){
	int ones = score%10;
 	int tens = (score-ones)/10;
 
    char *one = generateChar(ones);
 	char *ten = generateChar(tens);
 
 	video_text(8,3,ten);
 	video_text(9,3,one);
}

void display_miss(int score){
	int ones = score%10;
 	int tens = (score-ones)/10;
 
    char *one = generateChar(ones);
 	char *ten = generateChar(tens);
 
 	video_text(72,3,ten);
 	video_text(73,3,one);
}

char* generateChar(int num){
 
 	char string[10] = {};
    char *k = string;

 	if(num == 0){
		k="0";
 	}else if(num == 1){
  		k="1";
 	}else if(num == 2){
  		k="2";
 	}else if(num == 3){
  		k="3";
	}else if(num == 4){
  		k="4";
 	}else if(num == 5){
  		k="5";
 	}else if(num == 6){
  		k="6";
 	}else if(num == 7){
  		k="7";
 	}else if(num == 8){
  		k="8";
 	}else if(num == 9){
  		k="9";
 	}
 
 return k;
}

void draw_rhythm_master() {
	//letter R
	draw_line(30,20,30,50,ORANGE);
	draw_line(30,20,46,20,ORANGE);
	draw_line(46,20,46,35,ORANGE);
	draw_line(46,35,30,35,ORANGE);
	draw_line(30,35,46,50,ORANGE);
	
	//letter H
	draw_line(50,20,50,50,ORANGE);
	draw_line(50,35,66,35,ORANGE);
	draw_line(66,20,66,50,ORANGE);
	
	//letter Y
	draw_line(70,20,78,35,ORANGE);
	draw_line(86,20,78,35,ORANGE);
	draw_line(78,35,78,50,ORANGE);
	
	//letter T
	draw_line(90,20,106,20,ORANGE);
	draw_line(98,20,98,50,ORANGE);
	
	//letter H
	draw_line(110,20,110,50,ORANGE);
	draw_line(110,35,126,35,ORANGE);
	draw_line(126,20,126,50,ORANGE);
	
	//letter M
	draw_line(130,20,130,50,ORANGE);
	draw_line(130,20,138,50,ORANGE);
	draw_line(138,50,146,20,ORANGE);
	draw_line(146,20,146,50,ORANGE);
	
	//letter M
	draw_line(166,20,166,50,ORANGE);
	draw_line(166,20,174,50,ORANGE);
	draw_line(174,50,182,20,ORANGE);
	draw_line(182,20,182,50,ORANGE);
	
	//letter A
	draw_line(186,20,186,50,ORANGE);
	draw_line(186,20,202,20,ORANGE);
	draw_line(202,20,202,50,ORANGE);
	draw_line(186,35,202,35,ORANGE);
	
	//letter S
	draw_line(206,20,222,20,ORANGE);
	draw_line(206,20,206,35,ORANGE);
	draw_line(206,35,222,35,ORANGE);
	draw_line(222,35,222,50,ORANGE);
	draw_line(222,50,206,50,ORANGE);

	//letter T
	draw_line(226,20,242,20,ORANGE);
	draw_line(234,20,234,50,ORANGE);
	
	//letter E
	draw_line(246,20,246,50,ORANGE);
	draw_line(246,20,262,20,ORANGE);
	draw_line(246,35,262,35,ORANGE);
	draw_line(246,50,262,50,ORANGE);
	
	//letter R
	draw_line(266,20,266,50,ORANGE);
	draw_line(266,20,282,20,ORANGE);
	draw_line(282,20,282,35,ORANGE);
	draw_line(282,35,266,35,ORANGE);
	draw_line(266,35,282,50,ORANGE);
}

void draw_press_enter() {
	//letter P
	draw_line(56,180,56,194,WHITE);
	draw_line(56,180,64,180,WHITE);
	draw_line(64,180,64,187,WHITE);
	draw_line(64,187,56,187,WHITE);
	
	//letter R
	draw_line(67,180,67,194,WHITE);
	draw_line(67,180,75,180,WHITE);
	draw_line(75,180,75,187,WHITE);
	draw_line(75,187,67,187,WHITE);
	draw_line(67,187,75,194,WHITE);
	
	//letter E
	draw_line(78,180,78,194,WHITE);
	draw_line(78,180,87,180,WHITE);
	draw_line(78,187,87,187,WHITE);
	draw_line(78,194,87,194,WHITE);
	
	//letter S
	draw_line(90,180,99,180,WHITE);
	draw_line(90,180,90,187,WHITE);
	draw_line(90,187,99,187,WHITE);
	draw_line(99,187,99,194,WHITE);
	draw_line(99,194,90,194,WHITE);

	//letter S
	draw_line(102,180,111,180,WHITE);
	draw_line(102,180,102,187,WHITE);
	draw_line(102,187,111,187,WHITE);
	draw_line(111,187,111,194,WHITE);
	draw_line(111,194,102,194,WHITE);
	
	//letter E
	draw_line(122,180,122,194,WHITE);
	draw_line(122,180,130,180,WHITE);
	draw_line(122,187,130,187,WHITE);
	draw_line(122,194,130,194,WHITE);
	
	//letter N
	draw_line(133,180,133,194,WHITE);
	draw_line(133,180,141,194,WHITE);
	draw_line(141,180,141,194,WHITE);
	
	//letter T
	draw_line(144,180,152,180,WHITE);
	draw_line(148,180,148,194,WHITE);
	
	//letter E
	draw_line(155,180,155,194,WHITE);
	draw_line(155,180,163,180,WHITE);
	draw_line(155,187,163,187,WHITE);
	draw_line(155,194,163,194,WHITE);
	
	//letter R
	draw_line(165,180,165,194,WHITE);
	draw_line(165,180,173,180,WHITE);
	draw_line(173,180,173,187,WHITE);
	draw_line(173,187,165,187,WHITE);
	draw_line(165,187,173,194,WHITE);
	
	//letter T
	draw_line(184,180,192,180,WHITE);
	draw_line(188,180,188,194,WHITE);
	
	//letter O
	draw_line(195,180,195,194,WHITE);
	draw_line(195,180,203,180,WHITE);
	draw_line(203,180,203,194,WHITE);
	draw_line(195,194,203,194,WHITE);
	
	//letter S
	draw_line(214,180,222,180,WHITE);
	draw_line(214,180,214,187,WHITE);
	draw_line(214,187,222,187,WHITE);
	draw_line(222,187,222,194,WHITE);
	draw_line(214,194,222,194,WHITE);
	
	//letter T
	draw_line(225,180,233,180,WHITE);
	draw_line(229,180,229,194,WHITE);
	
	//letter A
	draw_line(235,180,235,194,WHITE);
	draw_line(235,180,243,180,WHITE);
	draw_line(243,180,243,194,WHITE);
	draw_line(235,187,243,187,WHITE);
		
	//letter R
	draw_line(246,180,246,194,WHITE);
	draw_line(246,180,254,180,WHITE);
	draw_line(254,180,254,187,WHITE);
	draw_line(254,187,246,187,WHITE);
	draw_line(246,187,254,194,WHITE);
	
	//letter T
	draw_line(257,180,265,180,WHITE);
	draw_line(261,180,261,194,WHITE);
}