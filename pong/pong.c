#define TRUE 1
#define FALSE 0

#include "stdlib.h"
#include "time.h"
#include "unistd.h"

#include "ncurses.h"

#define force_quit() endwin(); exit(0)

int cols, rows;

int p1palettey;
int p2palettey;

int palette_size;

int ball_y, ball_x, ball_size;
int ball_vy, ball_vx;
int ball_move_y_every;

void draw_game(){
	attron(COLOR_PAIR(2));
	for (int i=0 ; i < palette_size; i++){
		mvprintw(p1palettey+i, 0, "  ");
		mvprintw(p2palettey+i, (cols-1)*2, "  ");
	}

	for (int i=0 ; i < ball_size; i++){
		for (int j=0; j < ball_size; j++){
			mvprintw(ball_y+i, (ball_x + j)*2, "  ");
		}
	}
}

const char DIGITS[10][5][6] = {
	{" ### ",
	 "#   #",
	 "#   #",
	 "#   #",
	 " ### "},
	{"  #  ",
	 " ##  ",
	 "  #  ",
	 "  #  ",
	 " ### "},
	{" ### ",
	 "#   #",
	 "  ## ",
	 " #   ",
	 "#####"},
	{" ### ",
	 "#   #",
	 "  ## ",
	 "#   #",
	 " ### "},
	{"#   #",
	 "#   #",
	 " ####",
	 "    #",
	 "    #"},
	{"#####",
	 "#    ",
	 "#### ",
	 "    #",
	 "#### "},
	{" ### ",
	 "#    ",
	 "#### ",
	 "#   #",
	 " ### "},
	{"#####",
	 "   # ",
	 "  #  ",
	 " #   ",
	 "#    "},
	{" ### ",
	 "#   #",
	 " ### ",
	 "#   #",
	 " ### "},
	{" ### ",
	 "#   #",
	 " ####",
	 "    #",
	 " ### "},
};
	

void draw_digit(int x, int y, int number){
	for (int i=0 ; i < 5; i++){
		for (int j = 0; j < 5; j++){
			if (DIGITS[number][i][j] == ' '){
				attron(COLOR_PAIR(1));
			}else{
				attron(COLOR_PAIR(2));
			}
			mvprintw(y+i, x+j, " ");
		}
	}
}

void draw_score(int p1, int p2){
	draw_digit(cols-10, 1, p1);
	draw_digit(cols+5, 1, p2);
}

void winning_screen(int p1, int p2){
	attron(COLOR_PAIR(1));
	clear();
	draw_score(p1,p2);

	attron(COLOR_PAIR(1));
	int wp = p1 == 9 ? 1 : 2;
	mvprintw(rows/2-2, cols-9, "Congratulations!");
	mvprintw(rows/2-1, cols-7, "Player %d won!", wp);
	refresh();
	usleep(10000000);


}

void reset_ball(){
	ball_x = (cols - ball_size)/2 + 1;
	ball_y = (rows - ball_size)/2 + 1;
	ball_vx = -1;
	ball_vy = 1;
	ball_move_y_every = 1;
}

void game_loop(int ai){
	int running = TRUE;

	reset_ball();
	
	int score_p1 = 0;
	int score_p2 = 0;

	int current_frame = 0;

	int frames_pause = 50;

	while(running){
		current_frame++;
		int input;
		while ((input = getch()) != -1){
			switch (input){
				case 'w':
					p1palettey--;
					break;
				case 's':
					p1palettey++;
					break;
				case 'i':
					if (ai == FALSE)
						p2palettey--;
					break;
				case 'k':
					if (ai == FALSE)
						p2palettey++;
					break;
				case 'q':
					force_quit();
					break;
			}
		}
		
		if (frames_pause){
			frames_pause --;
			if (score_p1 == 9 || score_p2 == 9){
				running = FALSE;
				winning_screen(score_p1, score_p2);
				continue;
			}
		}else{

			if (ai == TRUE && current_frame % 30){
				double diff = (p2palettey + palette_size/2.0 - ball_y - ball_size/2.0);
				if (diff < -0.5 || diff > 0.5){
					if (diff < 0){
						p2palettey++;
					}else if (diff > 0){
						p2palettey--;
					}
				}
			}

			if (ball_x <= 0) {
				score_p2 ++;
				reset_ball();
				frames_pause = 50;
			}
			if (ball_x + ball_size >= cols){
				score_p1 ++;
				reset_ball();
				frames_pause = 50;
			}

			if (ball_y <= 0){
				ball_vy = 1;
			}
			if (ball_y + ball_size >= rows){
				ball_vy = -1;
			}

			ball_x += ball_vx;
			if (current_frame % ball_move_y_every == 0)
				ball_y += ball_vy;

			if (ball_x == 1 && ball_y + ball_size > p1palettey && ball_y < p1palettey + palette_size){
				ball_vx = - ball_vx;
				double diff = (p1palettey + palette_size/2.0 - ball_y - ball_size/2.0);
				if (diff < 0){
					ball_vy = 1;
					diff = -diff;
				}else if (diff > 0){
					ball_vy = -1;
				}
					
				ball_move_y_every = palette_size/2 + 1 - diff;
				if (ball_move_y_every == 0){
					ball_move_y_every = 1;
				}
			}
			if (ball_x == cols-ball_size-1 && ball_y + ball_size > p2palettey && ball_y < p2palettey + palette_size){
				ball_vx = - ball_vx;

				double diff = (p2palettey + palette_size/2.0 - ball_y - ball_size/2.0);
				if (diff < 0){
					ball_vy = 1;
					diff = -diff;
				}else if (diff > 0){
					ball_vy = -1;
				}
					
				ball_move_y_every = palette_size/2 + 1 - diff;
				if (ball_move_y_every == 0){
					ball_move_y_every = 1;
				}
			}
		}

		attron(COLOR_PAIR(1));
		clear();

		draw_score(score_p1, score_p2);

		draw_game();

		if (frames_pause){
			draw_digit(cols-2, 0, (frames_pause + 5) / 10);
		}

		refresh();

		usleep(60000);
	}

}

const int  MENU_OPTIONS = 3;
const char MENU_STRINGS[][9]= {
	"1 Player",
	"2 Player",
	"Quit    "
};

void menu_loop(){
	int running = TRUE;
	int option = 0;
	while (running){
		int input;
		while((input = getch()) != -1){
			switch (input){
				case 'w':
					option --;
					break;
				case 's':
					option++;
					break;
				case 10:
					if (option == 0){
						game_loop(TRUE);
					}
					if (option == 1){
						game_loop(FALSE);
					}
					if (option == 2){
						running = FALSE;
					}
					break;
				case 'q':
					force_quit();
					break;
			}
			if (option < 0) option = 0;
			if (option >= MENU_OPTIONS) option = MENU_OPTIONS - 1;
		}

		attron(COLOR_PAIR(1));
		clear();
		for (int i=0 ; i < MENU_OPTIONS; i++){
			mvprintw(2+i, 2, "%s", MENU_STRINGS[i]);
		}
		attron(COLOR_PAIR(2));
		mvprintw(2+option, 2, "%s", MENU_STRINGS[option]);
		refresh();
	}
}

int main(){
	initscr();
	timeout(0);
	noecho();
	curs_set(0);

	int row, col;
	getmaxyx(stdscr, row, col);
	cols = col/2;
	rows = row;

	palette_size = rows/8;
	ball_size = palette_size/4;

	if (has_colors() == FALSE){
		endwin();
		exit(0);
	}

	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);
	init_pair(3, COLOR_BLACK, COLOR_BLACK);

	menu_loop();

	endwin();
	return 0;
}



