#include "../tcpSocket/tcpSocket.h"
#include <windows.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define Board_size 15
#define sleep_ms 0
#define serverip "10.78.243.224"

#include <easyx.h>
#include <conio.h>

void print_board(int b[][Board_size]);
void player_input(int b[][Board_size], int flag, ExMessage msg, int g_x, int g_y, SOCKET fd, char* buf);
void network_input(SOCKET fd, char* buf, int b[][Board_size], int flag);
int  check_player(int b[][Board_size], int flag);//return 0:continue  1:player win
int  check_computer(int b[][Board_size], int flag);//return 0:continue  1:computer win
int check_full(int b[][Board_size]);
void mysend(SOCKET fd,char* buf,int i,int j) {	//0-14
	memset(buf, 0, 520);
	buf[0] = 'a' + i;
	buf[1] = 'a' + j;
	buf[2] = 0;
	send(fd, buf, strlen(buf), 0);
}

int main() {
	init_Socket();
	//connecting
	char ip[16] = "10.78.243.224"; //255.255.255.255
	printf("Connect with author LIBEROR? (y/n) : \n");
	char op = getchar();
	while(op == ' ' || op == '\n') op = getchar();
	if (op != 'y' && op != 'Y') {
		printf("enter ip address to connect with others ,in format like 255.255.255.255 : \n");
		while (getchar()!='\n');
		while ((op = getchar()) == ' ');
		for (int i = 0; i < 15; i++) {
			ip[i] = op;
			op = getchar();
			if (op == '\n') {
				ip[i + 1] = 0;
				break;
			}
		}
		ip[15] = 0;
	}
	
	printf("Connecting...\n");

	SOCKET fd = createClientSocket(ip);
	//connected
	printf("Successfully Connected\n");
	char buf[520] = { 0 };
	
	char quit = 0;
	initgraph(15 * 40, 15 * 40);
	while (!quit) {
		ExMessage msg = {0};
		int g_x = 0;
		int g_y = 0;
		setbkcolor(BROWN);
		setlinecolor(BLACK);
		setbkmode(TRANSPARENT);
		setfillcolor(0x999999);
		settextcolor(WHITE);
		settextstyle(30, 0, L"宋体");
		int height1 = textheight(L"先手");
		int width1 = textwidth(L"先手");

		BeginBatchDraw();
		cleardevice();
		for (int i = 0; i < 15; i++) {
			line(20 + 40 * i, 20, 20 + 40 * i, 580);
			line(20, 20 + 40 * i, 580, 20 + 40 * i);
		}
		fillroundrect(150, 250, 250, 310, 20, 20);
		outtextxy(150 + 50 - width1 / 2, 250 + 30 - height1 / 2, L"先手");

		fillroundrect(350, 250, 450, 310, 20, 20);
		outtextxy(350 + 50 - width1 / 2, 250 + 30 - height1 / 2, L"后手");

		EndBatchDraw();
		settextcolor(RED);
		settextstyle(60, 0, L"宋体");
		int height2 = textheight(L"五子连珠");
		int width2 = textwidth(L"五子连珠");
		int text_x = 300 - width2 / 2;
		int text_y = 300 - height2 / 2;
		//printf("1: 先手  2:后手\n");
		int input2 = 0;
		//scanf("%d", &input2);
		while (input2 == 0) {
			if (peekmessage(&msg, EX_MOUSE)) {
				if (msg.message == WM_LBUTTONDOWN) {
					if (msg.x >= 150 && msg.x <= 250 && msg.y >= 250 && msg.y <= 310) {
						input2 = 1;
						break;
					}
					else if (msg.x >= 350 && msg.x <= 450 && msg.y >= 250 && msg.y <= 310) {
						input2 = 2;
						break;
					}
				}
			}
		}
		int board[Board_size][Board_size];
		memset(board, 0, sizeof(int) * Board_size * Board_size);
		//0:empty 1:black 2:white
		print_board(board);
		int init_i = 7;
		int init_j = 7;
		if (input2 == 1) {
			buf[0] = '1';
			if (send(fd, buf, strlen(buf), 0) == SOCKET_ERROR) {
				return 1;
			}
			setfillcolor(BLACK);
			while (1) {
				if (peekmessage(&msg, EX_MOUSE)) {
					if (msg.message == WM_LBUTTONDOWN) {
						init_i = msg.y / 40;
						init_j = msg.x / 40;
						break;
					}
					else {
						int t_gx = g_x;
						int t_gy = g_y;
						g_x = msg.x / 40;
						g_y = msg.y / 40;
						if (g_x != t_gx || g_y != t_gy)
						{
							BeginBatchDraw();
							print_board(board);
							solidcircle(20 + 40 * (msg.x / 40), 20 + 40 * (msg.y / 40), 3);
							EndBatchDraw();
						}
					}
				}
			}
			board[init_i][init_j] = 1;

			print_board(board);
			mysend(fd,buf,init_i,init_j);
		}
		else {
			buf[0] = '2';
			if (send(fd, buf, strlen(buf), 0) == SOCKET_ERROR) {
				return 1;
			}
		}
		//init_random_step(init_i, init_j, board, input2, 0);
		network_input(fd,buf,board,input2);
		print_board(board);

		player_input(board, input2, msg, g_x, g_y,fd,buf);
		print_board(board);
		//determine(board, input2, 0);
		network_input(fd, buf, board, input2);
		print_board(board);

		int gameover = 0;
		int cnt = 0;
		while (!gameover) {
			player_input(board, input2, msg, g_x, g_y,fd,buf);
			print_board(board);
			if (check_player(board, input2)) {
				outtextxy(text_x, text_y, L"五子连珠");
				gameover = 1;
				break;
			}
			if (check_full(board)) {
				outtextxy(text_x, text_y, L"双方平局 胜负难分");
				gameover = 1;
				break;
			}
			network_input(fd, buf, board, input2);
			Sleep(sleep_ms);
			print_board(board);
			if (check_computer(board, input2)) {
				outtextxy(text_x, text_y, L"五子连珠");
				gameover = 1;
				break;
			}
			if (check_full(board)) {
				outtextxy(text_x, text_y, L"双方平局 胜负难分");
				gameover = 1;
				break;
			}
			cnt++;
		}
		while (1) {
			if (peekmessage(&msg, EX_MOUSE)) {
				if (msg.message == WM_LBUTTONDOWN) {
					memset(buf,0,520);
					buf[0] = 'r';
					send(fd,buf,strlen(buf),0);
					break;
				}
			}
		}
	}
	closegraph();


	closesocket(fd);
	close_Socket();
	return 0;
}
void print_board(int b[][Board_size]) {
	BeginBatchDraw();
	cleardevice();
	for (int i = 0; i < 15; i++) {
		line(20 + 40 * i, 20, 20 + 40 * i, 580);
		line(20, 20 + 40 * i, 580, 20 + 40 * i);
	}
	for (int i = 0; i < 15; i++) {
		for (int j = 0; j < 15; j++) {
			if (b[i][j] != 0) {
				switch (b[i][j]) {
				case 1:
					setfillcolor(BLACK);
					solidcircle(20 + j * 40, 20 + i * 40, 20 - 3);
					break;
				case 2:
					setfillcolor(WHITE);
					solidcircle(20 + j * 40, 20 + i * 40, 20 - 3);
					break;
				}
			}
		}
	}
	EndBatchDraw();
}


void player_input(int b[][Board_size], int flag, ExMessage msg, int g_x, int g_y,SOCKET fd,char* buf) {
	while (peekmessage(&msg));
	int i, j;
	do {
		if (peekmessage(&msg, EX_MOUSE)) {
			if (msg.message == WM_LBUTTONDOWN) {
				i = msg.y / 40;
				j = msg.x / 40;
				if (b[i][j] == 0) {
					b[i][j] = (flag - 1) ? 2 : 1;
					mysend(fd,buf,i,j);
					break;
				}
			}
			else {
				int t_gx = g_x;
				int t_gy = g_y;
				g_x = msg.x / 40;
				g_y = msg.y / 40;
				if ((g_x != t_gx || g_y != t_gy) && b[g_y][g_x] == 0)
				{
					BeginBatchDraw();
					print_board(b);
					setfillcolor(BLACK);
					solidcircle(20 + 40 * (msg.x / 40), 20 + 40 * (msg.y / 40), 3);
					EndBatchDraw();
				}
				else if (g_x != t_gx || g_y != t_gy) {
					print_board(b);
				}
			}
		}
	} while (1);

}

void network_input(SOCKET fd, char* buf, int b[][Board_size], int flag) {
	memset(buf,0,520);
	int len = recv(fd,buf,520,0);
	if (len > 0) {
		int i = buf[0] - 'a';
		int j = buf[1] - 'a';
		b[i][j] = (flag - 1) ? 1 : 2;
	}
}

int check_player(int b[][Board_size], int flag) {
	int i, j;
	int isOver = 0;
	int player = (flag == 1) ? 1 : 2;
	for (i = 0; i < Board_size; i++) {
		for (j = 0; j < Board_size; j++)
		{
			if (b[i][j] == player) {
				//core
				//up 
				if (i - 4 >= 0) {
					if (b[i - 1][j] == player && b[i - 2][j] == player && b[i - 3][j] == player && b[i - 4][j] == player) {
						isOver = 1;
						break;
					}
				}
				//down
				if (i + 4 <= Board_size - 1) {
					if (b[i + 1][j] == player && b[i + 2][j] == player && b[i + 3][j] == player && b[i + 4][j] == player) {
						isOver = 1;
						break;
					}
				}
				//left
				if (j - 4 >= 0) {
					if (b[i][j - 1] == player && b[i][j - 2] == player && b[i][j - 3] == player && b[i][j - 4] == player) {
						isOver = 1;
						break;
					}
				}
				//right
				if (j + 4 <= Board_size - 1) {
					if (b[i][j + 1] == player && b[i][j + 2] == player && b[i][j + 3] == player && b[i][j + 4] == player) {
						isOver = 1;
						break;
					}
				}
				//左上 
				if (i - 4 >= 0 && j - 4 >= 0) {
					if (b[i - 1][j - 1] == player && b[i - 2][j - 2] == player && b[i - 3][j - 3] == player && b[i - 4][j - 4] == player) {
						isOver = 1;
						break;
					}
				}
				//左下 
				if (i + 4 <= Board_size - 1 && j - 4 >= 0) {
					if (b[i + 1][j - 1] == player && b[i + 2][j - 2] == player && b[i + 3][j - 3] == player && b[i + 4][j - 4] == player) {
						isOver = 1;
						break;
					}
				}
				//右上 
				if (i - 4 >= 0 && j + 4 <= Board_size - 1) {
					if (b[i - 1][j + 1] == player && b[i - 2][j + 2] == player && b[i - 3][j + 3] == player && b[i - 4][j + 4] == player) {
						isOver = 1;
						break;
					}
				}
				//右下 
				if (i + 4 <= Board_size - 1 && j + 4 <= Board_size - 1) {
					if (b[i + 1][j + 1] == player && b[i + 2][j + 2] == player && b[i + 3][j + 3] == player && b[i + 4][j + 4] == player) {
						isOver = 1;
						break;
					}
				}
			}
		}
		if (isOver == 1) {
			return 1;
		}
	}
	return 0;
}

int  check_computer(int b[][Board_size], int flag) {
	int i, j;
	int isOver = 0;
	int computer = (flag == 2) ? 1 : 2;
	for (i = 0; i < Board_size; i++) {
		for (j = 0; j < Board_size; j++)
		{
			if (b[i][j] == computer) {
				//core
				//up 
				if (i - 4 >= 0) {
					if (b[i - 1][j] == computer && b[i - 2][j] == computer && b[i - 3][j] == computer && b[i - 4][j] == computer) {
						isOver = 1;
						break;
					}
				}
				//down
				if (i + 4 <= Board_size - 1) {
					if (b[i + 1][j] == computer && b[i + 2][j] == computer && b[i + 3][j] == computer && b[i + 4][j] == computer) {
						isOver = 1;
						break;
					}
				}
				//left
				if (j - 4 >= 0) {
					if (b[i][j - 1] == computer && b[i][j - 2] == computer && b[i][j - 3] == computer && b[i][j - 4] == computer) {
						isOver = 1;
						break;
					}
				}
				//right
				if (j + 4 <= Board_size - 1) {
					if (b[i][j + 1] == computer && b[i][j + 2] == computer && b[i][j + 3] == computer && b[i][j + 4] == computer) {
						isOver = 1;
						break;
					}
				}
				//左上 
				if (i - 4 >= 0 && j - 4 >= 0) {
					if (b[i - 1][j - 1] == computer && b[i - 2][j - 2] == computer && b[i - 3][j - 3] == computer && b[i - 4][j - 4] == computer) {
						isOver = 1;
						break;
					}
				}
				//左下 
				if (i + 4 <= Board_size - 1 && j - 4 >= 0) {
					if (b[i + 1][j - 1] == computer && b[i + 2][j - 2] == computer && b[i + 3][j - 3] == computer && b[i + 4][j - 4] == computer) {
						isOver = 1;
						break;
					}
				}
				//右上 
				if (i - 4 >= 0 && j + 4 <= Board_size - 1) {
					if (b[i - 1][j + 1] == computer && b[i - 2][j + 2] == computer && b[i - 3][j + 3] == computer && b[i - 4][j + 4] == computer) {
						isOver = 1;
						break;
					}
				}
				//右下 
				if (i + 4 <= Board_size - 1 && j + 4 <= Board_size - 1) {
					if (b[i + 1][j + 1] == computer && b[i + 2][j + 2] == computer && b[i + 3][j + 3] == computer && b[i + 4][j + 4] == computer) {
						isOver = 1;
						break;
					}
				}
			}
		}
		if (isOver == 1) {
			return 1;
		}
	}
	return 0;
}

int check_full(int b[][Board_size]) {
	int i, j;
	for (i = 0; i < Board_size; i++) {
		for (j = 0; j < Board_size; j++) {
			if (b[i][j] == 0) return 0;
		}
	}
	return 1;
}