#define TRUE 1
#define FALSE 0

#define NEW_LN() (list_node*) calloc (1,sizeof(list_node))

int RLNS;
int RCOL; 
//#define draw(Y,X,str) mvprintw( Y , X *2 , str)

#define NO_LINES 2

#include "stdlib.h"
#include "time.h"
#include "unistd.h"

#include "ncurses.h"

typedef struct list_node list_node;

struct list_node{
	list_node *next;
	list_node *prev;
	int x;
	int y;
};

void draw (int y, int x, char *str){
	mvprintw(y,x*2, str);
}
list_node* add_copy_node(list_node *end){
	list_node *to_add = NEW_LN();
	if (to_add == NULL){
		exit(0);
	}
	to_add->x = end->x;
	to_add->y = end->y;
	to_add->prev = end;
	to_add->next = NULL;
	end->next = to_add;
	return to_add;
}

list_node *snake_beg;
list_node *snake_end;

list_node *berry_beg;
list_node *berry_end;

void remove_node(list_node *node){
	if (node->prev == NULL){
		node->next->prev = NULL;
		free(node);
		return;
	}
	if (node->next == NULL){
		node->prev->next = NULL;
		free(node);
	}

	node->next->prev = node->prev;
	node->prev->next = node->next;
	free(node);
}

void free_list(list_node *beg){
	if (beg == NULL){
		return;
	}
	free_list(beg->next);
	free(beg);
}

int mapoffsetx;
int mapoffsety;
int mapsizex;
int mapsizey;

void move_snake(int dx, int dy){
	for (list_node *temp = snake_end->prev; temp != NULL; temp = temp->prev){
		temp->next->x = temp->x;
		temp->next->y = temp->y;
	}
	snake_beg->x +=dx;
	snake_beg->y +=dy;
}

void draw_arena(){
	for (int i=0 ; i < RCOL; i++){
		draw(0, i, "--");
	}
	for (int i=0 ; i < RCOL ; i++){
		draw(RLNS-3, i , "--");
	}
	for (int i=0 ; i < RLNS-3; i++){
		draw(i , 0, "||");
	}
	for(int i=0 ; i < RLNS-3; i++){
		draw(i, RCOL-1, "||");
	}
	draw(0,0, "()");
	draw(0, RCOL-1, "()");
	draw(RLNS-3,0, "()");
	draw(RLNS-3,RCOL-1, "()");

	mvaddch(0,0, RCOL);
	mvaddch(0,1, COLS);
}

int check_death(){
	if (snake_beg->x < mapoffsetx ||
			snake_beg->x > mapoffsetx + mapsizex)
		return TRUE;
	if (snake_beg->y < mapoffsety ||
			snake_beg->y > mapoffsety + mapsizey){
		return TRUE;
	}

	for (list_node *temp = snake_beg->next; temp != NULL; temp = temp->next){
		if (temp->x == snake_beg->x &&
				temp->y == snake_beg->y){
			return TRUE;
		}
	}
	return FALSE;
}

void u_died(){
	clear();
	mvprintw(LINES/2, COLS/2 - 4, "Game over");
	refresh();
	sleep(2);
}

void add_berry(){
	berry_end = add_copy_node(berry_end);
	berry_end->x = rand()%mapsizex + mapoffsetx;
	berry_end->y = rand()%mapsizey + mapoffsety;
}

void game_loop(){
	mapoffsetx = 1;
	mapoffsety = 1;

	mapsizex = RCOL-2;
	mapsizey = RLNS-5;

	snake_beg = NEW_LN();
	snake_beg->x = 5;
	snake_beg->y = 5;
	snake_beg->next = NULL;
	snake_beg->prev = NULL;

	snake_end = snake_beg;

	snake_end = add_copy_node(snake_end);
	snake_end = add_copy_node(snake_end);


	berry_beg = NEW_LN();
	berry_beg->x = berry_beg->y = 10;
	berry_beg->next = NULL;
	berry_beg->prev = NULL;
	berry_end = berry_beg;

	add_berry();
	add_berry();
	add_berry();
	add_berry();
	add_berry();

	int running = TRUE;
	int dx = 1, dy=0;
	int time = -1;
	int score = 0;
	while (running){
		time++;
		int input;
		int got_input = FALSE;

		while (got_input == FALSE && (input = getch()) != -1 ){
			switch (input){
				case 'a':
					dx = -1;
					dy = 0;
					got_input = TRUE;
					break;
				case 'd':
					dx = 1;
					dy = 0;
					got_input = TRUE;
					break;
				case 'w':
					dx = 0;
					dy = -1;
					got_input = TRUE;
					break;
				case 's':
					dx = 0;
					dy = 1;
					got_input = TRUE;
					break;
			}
		}

		move_snake(dx,dy);

		for (list_node *temp = berry_beg->next; temp != NULL; temp = temp->next){
			if (temp->x == snake_beg->x &&
					temp->y == snake_beg->y){
				berry_end = add_copy_node(berry_end);
				berry_end->x = rand()%mapsizex + mapoffsetx;
				berry_end->y = rand()%mapsizey + mapoffsety;
				remove_node(temp);
				snake_end = add_copy_node(snake_end);
				score++;
				break;
			}
		}

		if (check_death()){
			running = FALSE;
			u_died();
		}
		
		clear();
		draw_arena();

		for (list_node *temp = snake_beg; temp != NULL; temp = temp->next){
			draw(temp->y, temp->x, "[]");
		}

		for (list_node *temp = berry_beg->next; temp != NULL; temp = temp->next){
			draw(temp->y, temp->x, "O`");
		}

		mvprintw(LINES-1, 2, "Time: %d", time);
		mvprintw(LINES-1, COLS-20, "Score: %d", score);



		refresh();

		usleep(60000);
	}

	free_list(snake_beg);
	free_list(berry_beg);
}

const char lines[NO_LINES][10] = { 
	"New game ", 
	"Quit     " 
};

void menu_loop(){
	int running = TRUE;
	int option = 0;
	while (running){
		int input;
		while((input = getch()) != -1){
			switch (input){
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
						running = FALSE;
					}
					break;
			}
			if (option < 0) option = 0;
			if (option >= NO_LINES) option = NO_LINES-1;
		}

		clear();

		for (int i=0 ; i < NO_LINES; i++){
			mvprintw(i+1, 2, lines[i]);
		}

		attron(COLOR_PAIR(2));
		mvprintw(option+1, 2, lines[option]);
		attron(COLOR_PAIR(1));

		refresh();
	}
}


int main(){
	srand(time(NULL));
	initscr();
	//noecho();
	timeout(0);

	int row, col;
	getmaxyx(stdscr, row, col);

	RCOL = col /2;
	RLNS = row;

	if (has_colors() == FALSE){
		endwin();
		exit(0);
	}
	start_color();

	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);

	menu_loop();

	endwin();
	return 0;
}
