/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 *
 *
 *
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xio.h"
#include "xil_exception.h"
#include "vga_periph_mem.h"
#include "digdug_sprites_directors_cut.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#define SIZE 10
#define UP 0b01000000
#define DOWN 0b00000100
#define LEFT 0b00100000
#define RIGHT 0b00001000
#define CENTER 0b00010000
#define SW0 0b00000001
#define SW1 0b00000010
#define BOMB '*'
#define NUM1 '1'
#define NUM2 '2'
#define NUM3 '3'
#define NUM4 '4'
#define BLANK '0'
#define FLAG '#'
#define NUMOFMINES 9
//BEG---unpened field
#define BEG '@'

int endOfGame;
int inc1;
int inc2;
int i, x, y, ii, oi, R, G, B, RGB, kolona, red, RGBgray;
int numOfFlags;
int flagTrue;
int randomCounter = 50;
int numOfMines;
int firstTimeCenter;
//map that is hidden from the user-it contains the solution
char solvedMap[9][9];
//map that has all of player's moves
char blankMap[9][9];
//map used for opening the blank fields that surround blank field selected
char indicationMap[9][9];

//end of game
void printOutEndOfGame(char blankTable[SIZE][SIZE], char solvedMap[SIZE][SIZE]) {
	int i, j, ii, jj;
	for (i = 0; i < SIZE; i++) {
		for (j = 0; j < SIZE; j++) {
			ii = (i * 16) + 80;
			jj = (j * 16) + 80;
			if (blankTable[i][j] == FLAG) {
				if (solvedMap[i][j] != BOMB) {
					drawMap(16, 16, ii, jj, 16, 16);
				}
			} else if (blankTable[i][j] != FLAG && solvedMap[i][j] == BOMB) {
				drawMap(0, 16, ii, jj, 16, 16);
			}
		}
	}
}

//when the blank field is pressed, open all blank fields around it

void clean(int x, int y, char resultTable[SIZE][SIZE],
		char indicationMap[SIZE][SIZE]) {
	int i, j;

	indicationMap[x][y] = 'x';

	if (resultTable[x][y] == BLANK) {
		for (i = x - 1; i <= x + 1; i++) {
			for (j = y - 1; j <= y + 1; j++) {
				if (i >= 0 && j >= 0 && i < 9 && j < 9 && !(x == i && y == j)) {
					if (indicationMap[i][j] == BLANK) {
						clean(i, j, resultTable, indicationMap);
					}
				}
			}
		}
	}
}

//extracting pixel data from a picture for printing out on the display

void drawMap(int in_x, int in_y, int out_x, int out_y, int width, int height) {
	int ox, oy, oi, iy, ix, ii;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			ox = out_x + x;
			oy = out_y + y;
			oi = oy * 320 + ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * gimp_image.width + ix;
			R = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel] >> 5;
			G = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel + 1] >> 5;
			B = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF + oi * 4,
					RGB);
		}
	}

}

//reverse right -> left
void drawMapReverseRL(int in_x, int in_y, int out_x, int out_y, int width,
		int height) {
	int ox, oy, oi, iy, ix, ii;
	for (y = 0; y < height; y++) {
		for (x = width - 1; x >= 0; x--) {
			ox = out_x + 13 - x;
			oy = out_y + y;
			oi = oy * 320 + ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * gimp_image.width + ix;
			R = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel] >> 5;
			G = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel + 1] >> 5;
			B = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF + oi * 4,
					RGB);
		}
	}

}

//reverse down -> up
void drawMapReverseDU(int in_x, int in_y, int out_x, int out_y, int width,
		int height) {
	int ox, oy, oi, iy, ix, ii;
	for (y = height - 1; y >= 0; y--) {
		for (x = 0; x < width; x++) {
			ox = out_x + x;
			oy = out_y + 13 - y;
			oi = oy * 320 + ox;
			ix = in_x + x;
			iy = in_y + y;
			ii = iy * gimp_image.width + ix;
			R = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel] >> 5;
			G = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel + 1] >> 5;
			B = gimp_image.pixel_data[ii * gimp_image.bytes_per_pixel + 2] >> 5;
			R <<= 6;
			G <<= 3;
			RGB = R | G | B;

			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF + oi * 4,
					RGB);
		}
	}

}

int startX = 153, startY = 103, endX = 166, endY = 116;
int oldStartX, oldStartY, oldEndX, oldEndY;
int x, y, ic, ib, i, j, t, tmp;
int flagGoRight1 = 1, flagGoLeft1 = 0, rightH1 = 227, leftH1 = 0;
int flagGoRight2 = 1, flagGoLeft2 = 0, rightH2 = 62, leftH2 = 0;
int flagGoDown1 = 1, flagGoUp1 = 0, downH1 = 43, upH1 = 0;
int flagGoDown2 = 1, flagGoUp2 = 0, downH2 = 148, upH2 = 0;
int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
int previousState = 0;
int endCheck = 0;

void down() {
	if (endY < 221) {
		oldStartY = startY;
		oldEndY = endY;
		startY += 1;
		endY += 1;
		previousState = 1;

		if (tmp < 4) {
			drawMap(32, 16, startX, startY, 14, 14);
			drawMap(150, 16, startX, oldStartY, 14, 1);
			tmp++;
		} else if (tmp < 8) {
			drawMap(48, 16, startX, startY, 14, 14);
			drawMap(150, 16, startX, oldStartY, 14, 1);
			tmp++;

		} else {
			tmp = 0;
			drawMap(150, 16, startX, oldStartY, 14, 1);
		}

	}

}

void right() {
	if (endX < 316) {
		oldStartX = startX;
		oldEndX = endX;
		startX += 1;
		endX += 1;
		previousState = 2;

		if (tmp < 4) {
			drawMap(30, 0, startX, startY, 14, 14);
			drawMap(150, 16, oldStartX, startY, 1, 14);
			tmp++;
		} else if (tmp < 8) {
			drawMap(46, 0, startX, startY, 14, 14);
			drawMap(150, 16, oldStartX, startY, 1, 14);
			tmp++;

		} else {
			tmp = 0;
			drawMap(150, 16, oldStartX, startY, 1, 14);
		}

	}

}

void left() {
	if (startX > 3) {
		oldStartX = startX;
		oldEndX = endX;
		startX -= 1;
		endX -= 1;
		previousState = 3;

		if (tmp < 4) {
			drawMapReverseRL(30, 0, startX, startY, 14, 14);
			drawMap(150, 16, oldEndX, startY, 1, 14);
			tmp++;
		} else if (tmp < 8) {
			drawMapReverseRL(46, 0, startX, startY, 14, 14);
			drawMap(150, 16, oldEndX, startY, 1, 14);
			tmp++;

		} else {
			tmp = 0;
			drawMap(150, 16, oldEndX, startY, 1, 14);

		}

	}

}

void up() {
	if (startY > 28) {
		oldStartY = startY;
		oldEndY = endY;
		startY -= 1;
		endY -= 1;
		previousState = 4;

		if (tmp < 4) {
			drawMapReverseDU(32, 16, startX, startY, 14, 14);
			drawMap(150, 16, startX, oldEndY, 14, 1);
			tmp++;
		} else if (tmp < 8) {
			drawMapReverseDU(48, 16, startX, startY, 14, 14);
			drawMap(150, 16, startX, oldEndY, 14, 1);
			tmp++;

		} else {
			tmp = 0;
			drawMap(150, 16, startX, oldEndY, 14, 1);
		}

	}

}

void moveRightZivanic() {
	if (flagGoRight1 == 1) {
		rightH1++;
		if (rightH1 > 272) {
			leftH1 = rightH1;
			rightH1 = 228;
			flagGoRight1 = 0;
			flagGoLeft1 = 1;
		} else {
			if (tmp1 < 10) {
				drawMap(16, 38, rightH1 + 1, 59, 13, 13);
				drawMap(150, 16, rightH1, 59, 1, 13);
				tmp1++;
			} else if (tmp1 < 20) {
				drawMap(0, 38, rightH1 + 1, 58, 13, 13);
				drawMap(150, 16, rightH1, 59, 1, 13);
				drawMap(150, 16, rightH1, 71, 13, 1);
				tmp1++;
			} else {
				tmp1 = 0;
				drawMap(150, 16, rightH1, 59, 1, 13);
			}

		}
	} else if (flagGoLeft1 == 1) {
		leftH1--;
		if (leftH1 < 228) {
			rightH1 = leftH1;
			leftH1 = 272;
			flagGoLeft1 = 0;
			flagGoRight1 = 1;
		} else {
			if (tmp1 < 10) {
				drawMapReverseRL(16, 38, leftH1 - 1, 59, 13, 13);
				drawMapReverseRL(150, 16, leftH1, 59, 1, 13);
				tmp1++;
			} else if (tmp1 < 20) {
				drawMapReverseRL(0, 38, leftH1 - 1, 58, 13, 13);
				drawMapReverseRL(150, 16, leftH1, 59, 1, 13);
				drawMapReverseRL(150, 16, leftH1, 71, 13, 1);
				tmp1++;
			} else {
				tmp1 = 0;
				drawMapReverseRL(150, 16, leftH1, 59, 1, 13);
			}

		}

	}
}

void moveBottomRightZivanic() {

	if (flagGoDown2 == 1) {
		downH2++;
		if (downH2 > 193) {
			upH2 = downH2;
			downH2 = 148;
			flagGoDown2 = 0;
			flagGoUp2 = 1;
		} else {
			if (tmp5 < 10) {
				drawMap(16, 39, 228, downH2 + 1, 13, 12);
				drawMap(150, 16, 228, downH2, 14, 1);
				drawMap(150, 16, 241, downH2, 1, 14);
				tmp5++;
			} else if (tmp5 < 20) {
				drawMap(0, 39, 229, downH2 + 1, 13, 12);
				drawMap(150, 16, 228, downH2, 13, 1);
				drawMap(150, 16, 228, downH2, 1, 13);
				tmp5++;
			} else {
				tmp5 = 0;
				drawMap(150, 16, 228, downH2, 14, 1);
			}

		}
	} else if (flagGoUp2 == 1) {
		upH2--;
		if (upH2 < 149) {
			downH2 = upH2;
			upH2 = 193;
			flagGoDown2 = 1;
			flagGoUp2 = 0;
		} else {
			if (tmp6 < 10) {
				drawMap(150, 16, 228, upH2 - 1, 1, 14);
				drawMapReverseRL(16, 39, 228, upH2, 13, 12);
				drawMap(150, 16, 228, upH2 + 12, 14, 1);
				tmp6++;
			} else if (tmp6 < 20) {
				drawMap(150, 16, 241, upH2 - 1, 1, 14);
				drawMapReverseRL(0, 39, 227, upH2, 13, 12);
				drawMap(150, 16, 228, upH2 + 12, 14, 1);
				tmp6++;
			} else {
				tmp6 = 0;
				drawMapReverseRL(150, 16, 228, upH2 + 12, 13, 1);
			}

		}

	}
}

void moveLeftZivanic() {

	if (flagGoDown1 == 1) {
		downH1++;
		if (downH1 > 88) {
			upH1 = downH1;
			downH1 = 43;
			flagGoDown1 = 0;
			flagGoUp1 = 1;
		} else {
			if (tmp3 < 10) {
				drawMap(16, 39, 48, downH1 + 1, 13, 12);
				drawMap(150, 16, 48, downH1, 14, 1);
				drawMap(150, 16, 61, downH1, 1, 14);
				tmp3++;
			} else if (tmp3 < 20) {
				drawMap(0, 39, 49, downH1 + 1, 13, 12);
				drawMap(150, 16, 48, downH1, 13, 1);
				drawMap(150, 16, 48, downH1, 1, 13);
				tmp3++;
			} else {
				tmp3 = 0;
				drawMap(150, 16, 48, downH1, 14, 1);
			}

		}
	} else if (flagGoUp1 == 1) {
		upH1--;
		if (upH1 < 44) {
			downH1 = upH1;
			upH1 = 88;
			flagGoDown1 = 1;
			flagGoUp1 = 0;
		} else {
			if (tmp4 < 10) {
				drawMap(150, 16, 48, upH1 - 1, 1, 14);
				drawMapReverseRL(16, 39, 48, upH1, 13, 12);
				drawMap(150, 16, 48, upH1 + 12, 14, 1);
				tmp4++;
			} else if (tmp4 < 20) {
				drawMap(150, 16, 61, upH1 - 1, 1, 14);
				drawMapReverseRL(0, 39, 47, upH1, 13, 12);
				drawMap(150, 16, 48, upH1 + 12, 14, 1);
				tmp4++;
			} else {
				tmp4 = 0;
				drawMapReverseRL(150, 16, 48, upH1 + 12, 13, 1);
			}

		}

	}
}

void moveBottomLeftZivanic() {
	if (flagGoRight2 == 1) {
		rightH2++;
		if (rightH2 > 108) {
			leftH2 = rightH2;
			rightH2 = 63;
			flagGoRight2 = 0;
			flagGoLeft2 = 1;
		} else {
			if (tmp2 < 10) {
				drawMap(16, 38, rightH2 + 1, 164, 13, 13);
				drawMap(150, 16, rightH2, 164, 1, 13);
				tmp2++;
			} else if (tmp2 < 20) {
				drawMap(0, 38, rightH2 + 1, 163, 13, 13);
				drawMap(150, 16, rightH2, 164, 1, 13);
				drawMap(150, 16, rightH2, 176, 13, 1);
				tmp2++;
			} else {
				tmp2 = 0;
				drawMap(150, 16, rightH2, 164, 1, 13);
			}

		}
	} else if (flagGoLeft2 == 1) {
		leftH2--;
		if (leftH2 < 63) {
			rightH2 = leftH2;
			leftH2 = 122;
			flagGoLeft2 = 0;
			flagGoRight2 = 1;
		} else {
			if (tmp2 < 10) {
				drawMapReverseRL(16, 38, leftH2 - 1, 164, 13, 13);
				drawMapReverseRL(150, 16, leftH2, 164, 1, 13);
				tmp2++;
			} else if (tmp2 < 20) {
				drawMapReverseRL(0, 38, leftH2 - 1, 163, 13, 13);
				drawMapReverseRL(150, 16, leftH2, 164, 1, 13);
				drawMapReverseRL(150, 16, leftH2, 176, 13, 1);
				tmp2++;
			} else {
				tmp2 = 0;
				drawMapReverseRL(150, 16, leftH2, 164, 1, 13);
			}

		}

	}
}
void endOfGameCheck() {

	//Check left
	if (flagGoDown1 == 1) {
		if (downH1 == startY - 14 || downH1 == startY - 13
				|| downH1 == startY - 15) {
			if (startX == 47 || startX == 48) {
				endOfGame = 1;
			}
		}
	} else if (flagGoUp1 == 1) {
		if (upH1 == startY + 14 || upH1 == startY + 13 || upH1 == startY + 15) {
			if (startX == 47 || startX == 48) {
				endOfGame = 1;
			}
		}
	}

	//Check bottom left

	if (flagGoRight2 == 1) {
		if (rightH2 == startX - 14 || rightH2 == startX - 13
				|| rightH2 == startX - 15) {
			if (startY == 163 || startY == 164) {
				endOfGame = 1;
			}
		}
	} else if (flagGoLeft2 == 1) {
		if (leftH2 == startX + 14 || leftH2 == startX + 13
				|| leftH2 == startX + 15) {
			if (startY == 163 || startY == 164) {
				endOfGame = 1;
			}
		}
	}

	//Check right

	if (flagGoRight1 == 1) {
		if (rightH1 == startX - 14 || rightH1 == startX - 13
				|| rightH1 == startX - 15) {
			if (startY == 58 || startY == 59) {
				endOfGame = 1;
			}
		}
	} else if (flagGoLeft1 == 1) {
		if (leftH1 == startX + 14 || leftH1 == startX + 13
				|| leftH1 == startX + 15) {
			if (startY == 58 || startY == 59) {
				endOfGame = 1;
			}
		}
	}

	//Check bottom right

	if (flagGoDown2 == 1) {
		if (downH2 == startY - 14 || downH2 == startY - 13
				|| downH2 == startY - 15) {
			if (startX == 227 || startX == 228) {
				endOfGame = 1;
			}
		}
	} else if (flagGoUp2 == 1) {
		if (upH2 == startY + 14 || upH2 == startY + 13 || upH2 == startY + 15) {
			if (startX == 227 || startX == 228) {
				endOfGame = 1;
			}
		}
	}
}
void move() {

	while (endOfGame != 1) {

		endOfGameCheck();

		if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & DOWN) == 0) {
			if ((startX - 3) % 15 != 0) {
				if (previousState == 3) {
					while ((startX - 3) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						left();
					}
				}
				if (previousState == 2) {
					while ((startX - 3) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						right();
					}
				}

			} else {
				down();
			}
		}

		else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & RIGHT) == 0) {
			if ((startY - 28) % 15 != 0) {
				if (previousState == 4) {
					while ((startY - 28) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						up();
					}
				}
				if (previousState == 1) {
					while ((startY - 28) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						down();
					}
				}
			} else {
				right();
			}
		} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & LEFT) == 0) {

			if ((startY - 28) % 15 != 0) {
				if (previousState == 4) {
					while ((startY - 28) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						up();
					}
				}
				if (previousState == 1) {
					while ((startY - 28) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						down();
					}
				}
			} else {
				left();
			}

		} else if ((Xil_In32(XPAR_MY_PERIPHERAL_0_BASEADDR) & UP) == 0) {
			if ((startX - 3) % 15 != 0) {
				if (previousState == 3) {
					while ((startX - 3) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						left();
					}
				}
				if (previousState == 2) {
					while ((startX - 3) % 15 != 0) {
						for (t = 0; t < 150000; t++) {
						}
						right();
					}
				}

			} else {
				up();
			}
		}

		moveRightZivanic();
		moveBottomLeftZivanic();
		moveLeftZivanic();
		moveBottomRightZivanic();

		for (t = 0; t < 170000; t++) {
		}

	}
}

void initializeDiggerMoving() {

	///////////// START POSITION ///////////////
	int tmp = 0;

	//steps classic - left
	for (i = 250; i >= 153; i--) {
		if (tmp < 4) {
			drawMapReverseRL(0, 0, i, 13, 14, 14);
			drawMapReverseRL(150, 16, i + 2, 13, 2, 14);
			for (j = 0; j < 150000; j++) {
			}
			tmp++;
		} else if (tmp < 8) {
			drawMapReverseRL(14, 0, i, 13, 14, 14);
			drawMapReverseRL(150, 16, i + 2, 13, 2, 14);
			for (j = 0; j < 150000; j++) {
			}
			tmp++;

		} else {
			tmp = 0;
		}

	}

	drawMap(150, 16, 153, 13, 16, 14);

	//steps digger - down
	for (i = 0; i < 75; i++) {
		if (tmp < 4) {
			drawMapReverseRL(32, 16, 153, i + 14, 14, 14);
			drawMapReverseRL(150, 16, 153, i + 13, 14, 1);
			for (j = 0; j < 150000; j++) {
			}
			tmp++;
		} else if (tmp < 8) {
			drawMapReverseRL(48, 16, 153, i + 14, 14, 14);
			drawMapReverseRL(150, 16, 153, i + 13, 14, 1);
			for (j = 0; j < 150000; j++) {
			}
			tmp++;
		} else {
			tmp = 0;
			drawMapReverseRL(150, 16, 153, i + 13, 14, 1);
		}
	}

	drawMap(150, 16, 153, 87, 14, 14);

	//steps classic - down
	for (i = 0; i < 13; i++) {
		if (tmp < 4) {
			drawMapReverseRL(0, 16, 153, i + 90 + 1, 14, 14);
			drawMapReverseRL(150, 16, 153, i + 89, 14, 1);
			for (j = 0; j < 150000; j++) {
			}
			tmp++;
		} else if (tmp < 8) {
			drawMapReverseRL(16, 16, 153, i + 90 + 1, 14, 14);
			drawMapReverseRL(150, 16, 153, i + 89, 14, 1);
			for (j = 0; j < 150000; j++) {
			}
			tmp++;
		} else {
			tmp = 0;
			drawMapReverseRL(150, 16, 153, i + 89, 14, 1);
		}
	}

	//steps classic right
	drawMap(150, 16, 153, 102, 14, 1);
	drawMap(0, 0, 153, 103, 14, 14);

	/////////////////////////////////////////////
}

void initializeMapDrawing() {
	/************************   MAP   ************************/

	int i;
	//blue
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 20; i++) {
			drawMap(150, 0, 16 * i, 16 * j, 0, 0);
		}
	}

	//yellow up

	for (i = 0; i < 20; i++) {
		drawMap(118, 0, 16 * i, 27, 16, 5);
	}

	//yellow

	for (j = 0; j < 3; j++) {
		for (i = 0; i < 20; i++) {
			drawMap(118, 0, 16 * i, 32 + 16 * j, 16, 16);
		}
	}

	//orange
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 20; i++) {
			drawMap(134, 0, 16 * i, 80 + 16 * j, 16, 16);
		}
	}

	//red light
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 20; i++) {
			drawMap(118, 16, 16 * i, 128 + 16 * j, 16, 16);
		}
	}

	//red dark
	for (j = 0; j < 3; j++) {
		for (i = 0; i < 20; i++) {
			drawMap(134, 16, 16 * i, 176 + 16 * j, 16, 16);
		}
	}

	drawMap(0, 103, 0, 225, 16, 13);
	drawMap(0, 103, 16, 225, 16, 13);
	drawMap(0, 103, 32, 225, 16, 13);

	// Flower
	drawMap(107, 86, 306, 14, 13, 13);
	//Left hole
	// x -> 3 + 3 * (14 + 1) = 48
	// y -> /27/ + 1 + (14 + 1) = 43

	drawMap(150, 16, 48, 43, 14, 15);
	drawMap(150, 16, 48, 58, 14, 15);
	drawMap(150, 16, 48, 73, 14, 15);
	drawMap(150, 16, 48, 88, 14, 14);

	//Center hole
	// x -> 3 + 9 * (14 + 1) = 138
	// y -> /27/ + 1 + 6 * (14 + 1) = 103

	drawMap(150, 16, 138, 103, 15, 14);
	drawMap(150, 16, 153, 103, 15, 14);
	drawMap(150, 16, 168, 103, 14, 14);

	//Right hole
	// x -> 3 + 15 * (14 + 1) = 228
	// y -> /27/ + 1 + 2 * (14 + 1) =  58

	drawMap(150, 16, 228, 58, 15, 14);
	drawMap(150, 16, 243, 58, 15, 14);
	drawMap(150, 16, 258, 58, 15, 14);
	drawMap(150, 16, 273, 58, 14, 14);

	//Bottom left hole
	//x -> 3 + 4 * (14 + 1) = 63
	//y -> /27/ + 1 + 9 * (14 + 1) = 163

	drawMap(150, 16, 63, 163, 15, 14);
	drawMap(150, 16, 78, 163, 15, 14);
	drawMap(150, 16, 93, 163, 15, 14);
	drawMap(150, 16, 108, 163, 14, 14);

	//Bottom right hole
	//x -> 3 + 15 * (14 + 1) = 228
	//y -> /27/ + 1 + 8 * (14 + 1) = 148

	drawMap(150, 16, 228, 148, 14, 15);
	drawMap(150, 16, 228, 163, 14, 15);
	drawMap(150, 16, 228, 178, 14, 15);
	drawMap(150, 16, 228, 193, 14, 14);

}

void initializeZivanis() {

	/////////// ZIVANICI ////////////////

	drawMap(16, 38, 228, 59, 14, 13);
	drawMap(16, 38, 63, 164, 14, 13);
	drawMap(16, 38, 48, 43, 13, 13);
	drawMap(16, 38, 228, 148, 13, 13);
}

void deleteCoveculjak() {
	if (endCheck == 0) {
		drawMap(54, 103, 0, 225, 16, 13);
		drawMap(0, 103, 16, 225, 16, 13);
		drawMap(0, 103, 32, 225, 16, 13);
	} else if (endCheck == 1) {
		drawMap(54, 103, 0, 225, 16, 13);
		drawMap(54, 103, 16, 225, 16, 13);
		drawMap(0, 103, 32, 225, 16, 13);
	} else if (endCheck == 2) {
		drawMap(54, 103, 0, 225, 16, 13);
		drawMap(54, 103, 16, 225, 16, 13);
		drawMap(54, 103, 32, 225, 16, 13);
	}
}


int main() {

	int j, p, r;
	inc1 = 0;
	inc2 = 0;
	numOfFlags = NUMOFMINES;
	flagTrue = 0;
	numOfMines = NUMOFMINES;
	firstTimeCenter = 0;

	init_platform();
	unsigned char string_s[] = "LPRS 2\n";

	//helping map for cleaning the table when blank button is pressed
	for (p = 0; p < SIZE; p++) {
		for (r = 0; r < SIZE; r++) {
			indicationMap[p][r] = BLANK;
		}
	}

	//map which contains all the moves of the player
	for (i = 0; i < 9; i++) {
		for (j = 0; j < 9; j++) {
			blankMap[i][j] = BEG;
		}
	}

	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x00, 0x0); // direct mode   0
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x04, 0x3); // display_mode  1
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x08, 0x0); // show frame      2
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x0C, 0x1); // font size       3
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x10, 0xFFFFFF); // foreground 4
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x14, 0x0000FF); // background color 5
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x18, 0xFF0000); // frame color      6
	VGA_PERIPH_MEM_mWriteMemory(
			XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + 0x20, 1);

	//black background
	for (x = 0; x < 320; x++) {
		for (y = 0; y < 240; y++) {
			i = y * 320 + x;
			VGA_PERIPH_MEM_mWriteMemory(
					XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF + i * 4,
					0x000000);

		}
	}

	initializeMapDrawing();

	initializeZivanis();

	initializeDiggerMoving();

	while (1) {
		if (endCheck == 3) {
			for (x = 0; x < 320; x++) {
				for (y = 0; y < 240; y++) {
					i = y * 320 + x;
					VGA_PERIPH_MEM_mWriteMemory(
							XPAR_VGA_PERIPH_MEM_0_S_AXI_MEM0_BASEADDR + GRAPHICS_MEM_OFF + i * 4,
							0x000000);

				}
			}

			//DRAW END

			drawMap(150, 0, 80, 90, 10, 10);
			drawMap(150, 0, 80, 100, 10, 10);
			drawMap(150, 0, 80, 110, 10, 10);
			drawMap(150, 0, 80, 120, 10, 10);
			drawMap(150, 0, 80, 130, 10, 10);
			drawMap(150, 0, 80, 140, 10, 10);

			drawMap(150, 0, 90, 90, 10, 10);
			drawMap(150, 0, 100, 90, 10, 10);
			drawMap(150, 0, 110, 90, 10, 10);

			drawMap(150, 0, 90, 115, 10, 10);
			drawMap(150, 0, 100, 115, 10, 10);
			drawMap(150, 0, 110, 115, 10, 10);

			drawMap(150, 0, 90, 140, 10, 10);
			drawMap(150, 0, 100, 140, 10, 10);
			drawMap(150, 0, 110, 140, 10, 10);

			drawMap(150, 0, 130, 90, 10, 10);
			drawMap(150, 0, 130, 100, 10, 10);
			drawMap(150, 0, 130, 110, 10, 10);
			drawMap(150, 0, 130, 120, 10, 10);
			drawMap(150, 0, 130, 130, 10, 10);
			drawMap(150, 0, 130, 140, 10, 10);

			drawMap(150, 0, 170, 90, 10, 10);
			drawMap(150, 0, 170, 100, 10, 10);
			drawMap(150, 0, 170, 110, 10, 10);
			drawMap(150, 0, 170, 120, 10, 10);
			drawMap(150, 0, 170, 130, 10, 10);
			drawMap(150, 0, 170, 140, 10, 10);

			drawMap(150, 0, 135, 105, 10, 10);
			drawMap(150, 0, 140, 110, 10, 10);
			drawMap(150, 0, 145, 115, 10, 10);
			drawMap(150, 0, 150, 120, 10, 10);
			drawMap(150, 0, 155, 125, 10, 10);
			drawMap(150, 0, 160, 130, 10, 10);
			drawMap(150, 0, 165, 135, 10, 10);

			drawMap(150, 0, 190, 90, 10, 10);
			drawMap(150, 0, 190, 100, 10, 10);
			drawMap(150, 0, 190, 110, 10, 10);
			drawMap(150, 0, 190, 120, 10, 10);
			drawMap(150, 0, 190, 130, 10, 10);
			drawMap(150, 0, 190, 140, 10, 10);

			drawMap(150, 0, 225, 95, 10, 10);
			drawMap(150, 0, 230, 100, 10, 10);
			drawMap(150, 0, 230, 110, 10, 10);
			drawMap(150, 0, 230, 120, 10, 10);
			drawMap(150, 0, 230, 130, 10, 10);
			drawMap(150, 0, 225, 135, 10, 10);

			drawMap(150, 0, 200, 90, 10, 10);
			drawMap(150, 0, 210, 90, 10, 10);
			drawMap(150, 0, 220, 90, 10, 10);

			drawMap(150, 0, 200, 140, 10, 10);
			drawMap(150, 0, 210, 140, 10, 10);
			drawMap(150, 0, 220, 140, 10, 10);

			//////////////////////

			break;
		} else {
			move();

			int s;
			for (s = 0; s < 1500000; s++) {
			};

			startX = 153;
			startY = 103;
			endX = 166;
			endY = 116;
			oldStartX = 0;
			oldStartY = 0;
			oldEndX = 0;
			oldEndY = 0;
			endOfGame = 0;

			initializeMapDrawing();
			deleteCoveculjak();
			drawMap(0, 0, 153, 103, 14, 14);

			endCheck++;
		}



	}

	cleanup_platform();

	return 0;
}
