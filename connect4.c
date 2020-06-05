
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include "img.h"
#include "UART.h"
#include <stdio.h>



#define ROWs 6
#define COLs 7
#define leftMargin 6

int SW1;
int SW2;
int i;
int j;
int Grid[ROWs][COLs] ;
int CoinsPerCol[COLs] ;
int winner;
unsigned int x = 9;  //initial x-position of coin
unsigned int y = 9;   //initial y-position of coin
unsigned int turn;
unsigned int lastTurn;
unsigned int currentPlayer;
unsigned int currentPOS = 0;

void DrawGrid() {
	for ( i = 0; i < COLs+1 ; i++)
	{
		Nokia5110_PrintBMP(leftMargin + i * 10, SCREENH - 1, vvLine, 0);
	}
	for (i = 0; i < ROWs ; i++)
	{
		Nokia5110_PrintBMP(leftMargin , SCREENH - (1 + i * 6) , hLine, 0);
	}
}

void DrawCoin(int col, int row, int player) {
	if (player == 0)
		Nokia5110_PrintBMP(x + (col * 10), y + (row * 6), pl1coin, 0);
	else
		Nokia5110_PrintBMP(x + (col * 10), y + (row * 6), pl2coin, 0);
}

int connectFOUR() {
	int status = -1;
	for (i = 0; i < ROWs; i++) {
		for (j = 0; j < COLs; j++) {
			if (i + 3 < ROWs) {
				if (Grid[i][j] == Grid[i + 1][j] && Grid[i + 1][j] == Grid[i + 2][j] && Grid[i + 2][j] == Grid[i + 3][j] && Grid[i][j] != -1) {
					status = Grid[i][j];
					break;
				}
			}
			if (j + 3 < COLs) {
				if (Grid[i][j] == Grid[i][j + 1] && Grid[i][j + 1] == Grid[i][j + 2] && Grid[i][j + 2] == Grid[i][j + 3] && Grid[i][j]!= -1) {
					status = Grid[i][j];
					break;
				}
			}
			if (i + 3 < ROWs && j + 3 < COLs) {
				if (Grid[i][j] == Grid[i + 1][j + 1] && Grid[i + 1][j + 1] == Grid[i + 2][j + 2] && Grid[i + 2][j + 2] == Grid[i + 3][j + 3] && Grid[i][j] != -1) {
					status = Grid[i][j];
					break;
				}
			}
			if (i + 3 < ROWs && j - 3 >= 0) {
				if (Grid[i][j] == Grid[i + 1][j - 1] && Grid[i + 1][j - 1] == Grid[i + 2][j - 2] && Grid[i + 2][j - 2] == Grid[i + 3][j - 3] && Grid[i][j] != -1) {
					status = Grid[i][j];
					break;
				}
			}
		}
		if (status != -1)
			break;
	}
	return status;
}



void dropCoin() {
	if (CoinsPerCol[currentPOS] < ROWs) {
		Grid[ROWs-CoinsPerCol[currentPOS]-1][currentPOS] = currentPlayer ;
		currentPlayer = (currentPlayer + 1) % 2;
		CoinsPerCol[currentPOS]++;
		turn++;
	}
}

void displayGame() {

	
	Nokia5110_ClearBuffer();
	DrawGrid();
	if (turn > lastTurn) {
		DrawCoin(0, 0, currentPlayer);
		lastTurn = turn;
		currentPOS = 0;
	}
	else
		DrawCoin(currentPOS, 0, currentPlayer);

	for (i = 0; i < ROWs; i++) {
		for (j = 0; j < COLs; j++) {
			if(Grid[ROWs - 1 - i][j] == 0 || Grid[ROWs - 1 - i][j] == 1)
			
				DrawCoin(j, ROWs - i, Grid[ROWs - 1 - i][j]);
		}
	}
	Nokia5110_DisplayBuffer();

}


void Delay100ms(unsigned long count) {
	unsigned long volatile time;
	while (count>0) {
		time = 727240;  // 0.1sec at 80 MHz
		while (time) {
			time--;
		}
		count--;
	}
}

void PortF_Init(void) {
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
	delay = SYSCTL_RCGC2_R;           // delay   
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0  
	GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0       
	GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
	GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL  
	GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output   
	GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
	GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0       
	GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0        
}

void Game_Init() {
	turn = 0;
	lastTurn = 0;
	currentPlayer = 0;
	
	//displayGame();

	Nokia5110_ClearBuffer();
	Nokia5110_PrintBMP(10, SCREENH - 10, logo1, 0);
	Nokia5110_DisplayBuffer();
	Delay100ms(4);
	Nokia5110_ClearBuffer();
	Nokia5110_Clear();

	for (i = 0; i < ROWs; i++) {
		for (j = 0; j < COLs; j++) {
			Grid[ROWs - 1 - i][j] = -1;
		}
	}


	//DrawGrid();
	//DrawCoin(x, y, 1);
	while (1) {
		Nokia5110_ClearBuffer();
		SW1 = GPIO_PORTF_DATA_R&0x10;
		SW2 = GPIO_PORTF_DATA_R&0x01;
		winner=connectFOUR();
		displayGame();
		
		if (winner!= -1) {
			Nokia5110_SetCursor(1, 0);
			if (connectFOUR() == 0) {

				Nokia5110_OutString("P1 wins");
			}
			else {
				Nokia5110_OutString("P2 wins");
			}
			break;
		}
		else if (turn == 41) { // winner == 0
			Nokia5110_OutString("Tie");
			break;
		}
		

		while (SW1 && SW2) {
			SW1 = GPIO_PORTF_DATA_R & 0x10;
			SW2 = GPIO_PORTF_DATA_R & 0x01;
		}
		if (!SW1) {
			while (!SW1) SW1 = GPIO_PORTF_DATA_R & 0x10;
			currentPOS = (currentPOS + 1) % COLs;
			
		}
		if (!SW2) {
			while (!SW2)  SW2 = GPIO_PORTF_DATA_R & 0x01;
			dropCoin();
		}
		
	

	}

}


int main(void) {
	UART1_Init();
	TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
	Random_Init(1);
	Nokia5110_Init();
	Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();      // draw buffer
	PortF_Init();

	Game_Init();
}
		

