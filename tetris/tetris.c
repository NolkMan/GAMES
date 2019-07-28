#define TRUE 1
#define FALSE 0

#include "stdlib.h"
#include "time.h"
#include "unistd.h"

#include "ncurses.h"

#define force_quit() endwin(); exit(0)

int cols, rows;


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

void draw_border(int y, int x){
	attron(COLOR_PAIR(2));
	mvprintw(y,x*2,"            ");
	for (int i=0 ; i < 4; i++){
		mvprintw(y+1+i, x*2, "  ");
		mvprintw(y+1+i, x*2+10, "  ");
	}
	mvprintw(y+5,x*2,"            ");
}

void draw_controls(int y, int x, int primary_color, int secondary_color, char up, char down, char left, char right){
	attron(COLOR_PAIR(secondary_color));
	mvprintw(y,  x+3,"     ");
	mvprintw(y+1,x+3,"     ");
	mvprintw(y+2,x,"           ");
	mvprintw(y+3,x,"           ");
	mvprintw(y+4,x,"           ");
	attron(COLOR_PAIR(primary_color));
	mvprintw(y+1, x+5, "%c", up);
	mvprintw(y+3, x+5, "%c", down);
	mvprintw(y+3, x+2, "%c", left);
	mvprintw(y+3, x+8, "%c", right);
}

void draw_digit(int y, int x, int number){
	for (int i=0 ; i < 5; i++){
		for (int j = 0; j < 5; j++){
			if (DIGITS[number][i][j] == ' '){
				attron(COLOR_PAIR(10));
			}else{
				attron(COLOR_PAIR(11));
			}
			mvprintw(y+i, x+j, " ");
		}
	}
}

char pieces [7][4][4][5] = {
	{ // Line
		{
			" #  ",
			" #  ",
			" #  ",
			" #  "
		},
		{
			"    ",
			"####",
			"    ",
			"    "
		},
		{
			" #  ",
			" #  ",
			" #  ",
			" #  "
		},
		{
			"    ",
			"####",
			"    ",
			"    "
		},
	},
	{ // L
		{
			" #  ",
			" #  ",
			" ## ",
			"    "
		},
		{
			"  # ",
			"### ",
			"    ",
			"    "
		},
		{
			"##  ",
			" #  ",
			" #  ",
			"    "
		},
		{
			"    ",
			"### ",
			"#   ",
			"    "
		},
	},
	{ // _|
		{
			" #  ",
			" #  ",
			"##  ",
			"    "
		},
		{
			"#   ",
			"### ",
			"    ",
			"    "
		},
		{
			" ## ",
			" #  ",
			" #  ",
			"    "
		},
		{
			"    ",
			"### ",
			"  # ",
			"    "
		},
	},
	{ // Z
		{
			"##  ",
			" ## ",
			"    ",
			"    "
		},
		{
			" #  ",
			"##  ",
			"#   ",
			"    "
		},
		{
			"##  ",
			" ## ",
			"    ",
			"    "
		},
		{
			" #  ",
			"##  ",
			"#   ",
			"    "
		},
	},
	{ // S
		{
			" ## ",
			"##  ",
			"    ",
			"    "
		},
		{
			"#   ",
			"##  ",
			" #  ",
			"    "
		},
		{
			" ## ",
			"##  ",
			"    ",
			"    "
		},
		{
			"#   ",
			"##  ",
			" #  ",
			"    "
		},
	},
	{ // BLOCK
		{
			" ## ",
			" ## ",
			"    ",
			"    "
		},
		{
			" ## ",
			" ## ",
			"    ",
			"    "
		},
		{
			" ## ",
			" ## ",
			"    ",
			"    "
		},
		{
			" ## ",
			" ## ",
			"    ",
			"    "
		},
	},
	{ // T
		{
			" #  ",
			"### ",
			"    ",
			"    "
		},
		{
			" #  ",
			"##  ",
			" #  ",
			"    "
		},
		{
			"    ",
			"### ",
			" #  ",
			"    "
		},
		{
			" #  ",
			" ## ",
			" #  ",
			"    "
		},
	},
};

void draw_piece(int y, int x, int piece, int rot){
	for (int i=0; i<4; i++){
		for (int j=0; j<4; j++){
			if (pieces[piece][rot][i][j] == '#'){
				attron(COLOR_PAIR(piece+1));
				mvprintw(y+i, (x+j)*2, "  ");
			}
		}
	}
}

void draw_board(int board[26][14], int y, int x){
	for (int i=4 ; i < 26; i++){
		for (int j=0; j < 14; j++){
			if (board[i][j] == 0)
				attron(COLOR_PAIR(8));
			else
				attron(COLOR_PAIR(board[i][j]));
			mvprintw(y+i, (x+j)*2, "  ");
		}
	}
}

void randomize_pool(int* pool){
	for (int i=0 ; i < 7; i++){
		pool[i] = i;
	}
	for (int i = 6; i >= 1; i--){
		int num = rand() % i;

		int temp = pool[num];
		pool[num] = pool[i];
		pool[i] = temp;
	}
}

int can_move_left(int board[26][14], int piece, int rot, int y, int x){
	int cx = x-1;
	for (int i=0 ; i < 4; i++){
		for (int j=0 ; j < 4; j++){
			if (pieces[piece][rot][i][j] == '#'){
			   if (board[y+i][cx+j] != 0){
				   return FALSE;
			   }
			}
		}
	}
	return TRUE;
}

int can_move_right(int board[26][14], int piece, int rot, int y, int x){
	int cx = x+1;
	for (int i=0 ; i < 4; i++){
		for (int j=0 ; j < 4; j++){
			if (pieces[piece][rot][i][j] == '#'){
			   if (board[y+i][cx+j] != 0){
				   return FALSE;
			   }
			}
		}
	}
	return TRUE;
}

int can_rotate(int board[26][14], int piece, int rot, int y, int x){
	int crot = rot + 1;
	crot %= 4;
	for (int i=0 ; i < 4; i++){
		for (int j=0 ; j < 4; j++){
			if (pieces[piece][crot][i][j] == '#'){
			   if (board[y+i][x+j] != 0){
				   return FALSE;
			   }
			}
		}
	}
	return TRUE;
}

int can_fall(int board[4][14], int piece, int rot, int y, int x){
	int cy = y+1;
	for (int i=0 ; i < 4; i++){
		for (int j=0 ; j < 4; j++){
			if (pieces[piece][rot][i][j] == '#'){
			   if (board[cy+i][x+j] != 0){
				   return FALSE;
			   }
			}
		}
	}
	return TRUE;
}

void add_to_board(int board[26][14], int piece, int rot, int y, int x){
	for (int i=0 ; i < 4; i++){
		for (int j=0; j < 4;j++){
			if (pieces[piece][rot][i][j] == '#'){
				board[y+i][x+j] = piece+1;
			}
		}
	}
}

void setup_board(int board[26][14]){
	for (int i=0 ; i < 14; i++){
		for (int j=0; j < 26; j++){
			board[j][i] = 0;
		}
	}
	for (int i=0 ; i < 2; i++){
		for (int j=0; j < 26; j++){
			board[j][i] = 2;
			board[j][12+i] = 2;
		}
	}
	for (int i=0; i < 14; i++){
		for (int j=24; j < 26; j++){
			board[j][i] = 2;
		}
	}
}

void collapse_line(int board[26][14], int line){
	for (int i=2; i < 12; i++){
		board[line+1][i] = board[line][i];
		board[line][i] = 0;
	}
}

void clear_line(int board[26][14], int line){
	for (int i=2 ; i < 12; i++){
		board[line][i] = 0;
	}
}

int line_to_clear(int board[26][14], int line){
	for (int i = 2; i < 12; i++){
		if (board[line][i] == 0){
			return FALSE;
		}
	}
	return TRUE;
}

int check_if_dead(int board[26][14]){
	for (int i=2; i < 12; i++){
		if (board[3][i] != 0){
			return TRUE;
		}
	}
}

int check_for_lines(int board[26][14]){
	int cleared = 0;
	for (int i=4; i < 24; i++){
		if (line_to_clear(board, i)){
			clear_line(board, i);
			cleared++;
			for (int j=i-1; j>= 4; j--){
				collapse_line(board, j);
			}
		}
	}
	return cleared;
}

void death_screen(){
	clear();
	
	mvprintw(cols, rows/2, "YOU DEID!");

	refresh();

	usleep(5000000);
}

void shift_pool(int *pool){
	for (int i =0 ; i < 7; i++){
		pool[i] = pool[i+7];
	}
}

void game_loop(){
	int score = 0;
	int board[26][14];
	int frame = 0;
	setup_board(board);

	int pool[14];
	int pool_index = 0;
	randomize_pool(pool);
	randomize_pool(pool+7);

	int running = TRUE;

	int x = 8, y = 0;
	int rot = 0;

	while (running){
		frame++;
		int piece = pool[pool_index];
		int input;
		while ((input = getch()) != -1){
			switch(input){
				case 'w': if (can_rotate(board, piece, rot, y, x)) {rot++; rot%=4;}  break;
				case 'a': if (can_move_left(board, piece, rot, y ,x)) x--; break;
				case 'd': if (can_move_right(board, piece, rot, y, x)) x++; break;
				case 's': while (can_fall(board,piece,rot,y,x)) y++; break;
				case 'q': force_quit(); break;
			}
		}

		if (frame%4 == 0){	
			if (can_fall(board, piece, rot, y ,x)){
				y++;
			}else{
				add_to_board(board, piece, rot,y,x);
				pool_index++;
				rot = 0;
				y = 0;
				x = 6;
				if (pool_index >= 7){
					
					pool_index = 0;
					shift_pool(pool);
					randomize_pool(pool+7);
				}
				int s = check_for_lines(board);
				score += s*s;
				piece = pool[pool_index];
			}
		}

		// TODO add global score
		// TODO add next piece

		if (check_if_dead(board)){
			death_screen();
			running = FALSE;
		}
		

		clear();

		draw_board(board, 3, 3);
		draw_piece(3+y,3+x,piece, rot);
		draw_border(3+4, 3+13);
		draw_piece(3+5, 3+14, pool[pool_index+1], 0);

		int sco = score;
		for (int i=0 ; i <4; i++){
			int dig = sco%10;
			sco /= 10;
			draw_digit(5, cols*2 - i*6 - 5, dig);
		}

		refresh();


		usleep(50000);
	}
}

void p2_game_loop(){
	int player_died[2] = {0,0};
	int score[2] = {0,0};

	int board[2][26][14];

	setup_board(board[1]);
	setup_board(board[0]);

	int pool[2][14];
	int pool_index[2];
	pool_index[0] = 0;
	pool_index[1] = 0;

	randomize_pool(pool[1]);
	randomize_pool(pool[1]+7);
	randomize_pool(pool[0]);
	randomize_pool(pool[0]+7);

	int boardx[2];
	boardx[0] = (cols/2-13)/2;
	boardx[1] = cols/2 + boardx[0];	

	int running = TRUE;
	int frame = 0;

	int x[2], y[2];
	int rot[2];

	x[1] = 8; y[1] = 0;
	rot[1] = 0;

	x[0] = 8; y[0] = 0;
	rot[0] = 0;

	int can_go_back = FALSE;

	while(running){
		frame++;
		int piece[2];
		piece[0] = pool[0][pool_index[0]];
		piece[1] = pool[1][pool_index[1]];

		int input;
		while ((input = getch()) != -1){
			switch(input){
				case 'w': if (can_rotate(board[0], piece[0], rot[0], y[0],x[0])) { rot[0]++; rot[0]%=4;} break;
				case 'a': if (can_move_left(board[0], piece[0], rot[0], y[0],x[0])) x[0]--; break;
				case 'd': if (can_move_right(board[0], piece[0], rot[0], y[0], x[0])) x[0]++; break;
				case 's': while (can_fall(board[0], piece[0], rot[0], y[0], x[0])) y[0]++; break;
				case 'i': if (can_rotate(board[1], piece[1], rot[1], y[1],x[1])) { rot[1]++; rot[1]%=4;} break;
				case 'j': if (can_move_left(board[1], piece[1], rot[1], y[1],x[1])) x[1]--; break;
				case 'l': if (can_move_right(board[1], piece[1], rot[1], y[1], x[1])) x[1]++; break;
				case 'k': while (can_fall(board[1], piece[1], rot[1], y[1], x[1])) y[1]++; break;
				case 'b': if (can_go_back) running = false; break;
			}
		}

		for (int i=0; i < 2 ; i++){
			if (frame%4 == 0 && !player_died[i]){
				if(can_fall(board[i], piece[i], rot[i], y[i], x[i])){
					y[i]++;
				}else{
					add_to_board(board[i], piece[i], rot[i], y[i], x[i]);
					pool_index[i]++;
					rot[i] = 0;
					y[i] = 0;
					x[i] = 6;
					if (pool_index[i] >= 7){
						pool_index[i] = 0;
						shift_pool(pool[i]);
						randomize_pool(pool[i]+7);
					}
					int sc = check_for_lines(board[i]);
					score[i] += sc * sc;
					piece[i] = pool[i][pool_index[i]];
				}
			}

			if (check_if_dead(board[i])){
				player_died[i] = TRUE;
			}
		}


		


		clear();

		for (int i=0 ; i < 2; i++){
			draw_board(board[i], 0, boardx[i]);
			draw_piece(0+y[i],boardx[i]+x[i],piece[i], rot[i]);
			draw_border(4,boardx[i]+13);
			draw_piece(5,boardx[i]+14,pool[i][pool_index[i]+1],0);

			int sc = score[i];
			for (int j=0 ; j < 4; j++){
				int dig = sc%10;
				sc/=10;
				draw_digit(rows - 7, boardx[i]*2 + 1 + 21 - j*7, dig);
			}
		}

		if (player_died[0] && player_died[1]){
			mvprintw(rows/2, cols-7, "Player %d wins!", score[0] > score[1] ? 1 : 2);
			mvprintw(rows/2 + 2, cols-14, "Press b to go back to menu", score[0] > score[1] ? 1 : 2);
			can_go_back = TRUE;
		}

		refresh();

		usleep(50000);

	}
}

int MENU_OPTIONS = 3;
char MENU_STRINGS[3][9] = {
	"1 Player",
	"2 Player",
	"Quit"
};


int menu_loop(){
	int running = TRUE;
	int option = 0;

	while (running){
		int input;
		while((input = getch()) != -1){
			switch(input){
				case 'q':
					force_quit();
					break;
				case 'w':
					option--;
					break;
				case 's':
					option++;
					break;
				case 10:
					if (option == 0){
						game_loop();
					}
					if (option == 1){
						p2_game_loop();
					}
					if (option == 2){
						running = false;
					}
					break;
			}
		}

		erase();
		attron(COLOR_PAIR(10));
		for (int i=0; i < MENU_OPTIONS; i++){
			mvprintw(2+i, 2, "%s", MENU_STRINGS[i]);
		}
		attron(COLOR_PAIR(11));
		mvprintw(2+option, 2, "%s", MENU_STRINGS[option]);

		draw_controls(10, 15, 10, 3, 'W', 'S', 'A', 'D');
		draw_controls(10, 30, 10, 4, 'I', 'K', 'J', 'L');

		refresh();
		usleep(60000);

	}
}

int main(){
	initscr();
	timeout(0);
	noecho();
	curs_set(0);

	srand(time(NULL));

	int row, col;
	getmaxyx(stdscr, row, col);
	cols = col/2;
	rows = row;

	if (has_colors() == FALSE){
		force_quit();
	}
	start_color();

	init_pair(1, COLOR_BLACK, COLOR_CYAN);
	init_pair(2, COLOR_WHITE, COLOR_WHITE);
	init_pair(3, COLOR_BLUE, COLOR_BLUE);
	init_pair(4, COLOR_RED, COLOR_RED);
	init_pair(5, COLOR_GREEN, COLOR_GREEN);
	init_pair(6, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(7, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(8, COLOR_BLACK, COLOR_BLACK);

	init_pair(10, COLOR_WHITE, COLOR_BLACK);
	init_pair(11, COLOR_BLACK, COLOR_WHITE);

	menu_loop();

	endwin();
	return 0;
}

