#define true 1
#define false 0

#include "stdlib.h"
#include "time.h"
#include "unistd.h"
#include "string.h"

#include "ncurses.h"

#define force_quit() endwin(); exit(0);

void Q(int a){
	if (a == -1){
		endwin();
		exit(0);
	}
}

int rows;
int cols;

struct control_scheme{
	int up;
	int down;
	int left;
	int right;

} controls[4] = {
	{'w', 's', 'a', 'd'},
	{'i', 'k', 'j', 'l'},
	{'A', 'B', 'D', 'C'}, // capture arrow keys but don't care about esc
	{'8', '5', '4', '6'}
};

struct int_dequeue_node{
	struct int_dequeue_node *prev, *next;
	int val;
};

struct int_dequeue{
	struct int_dequeue_node *first, *last;
	int size;
};

struct player_data{
	int is_dead;
	int x, y;
	int dx, dy;
	struct int_dequeue *movement_queue;
	struct int_dequeue *trail_queue_x;
	struct int_dequeue *trail_queue_y;
};

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


void draw_winner_screen(int winner){
	const char E_SUCKS[] = "Everyone sucks\0";
	const char WINNER[] = "Player %d won!\0";
	attron(COLOR_PAIR(1));
	mvprintw(rows/2, cols - 10, "                    ");
	attron(COLOR_PAIR(3));
	mvprintw(rows/2-1, cols - 10, "12345678901234567890");
	mvprintw(rows/2+1, cols - 10, "12345678901234567890");
	mvprintw(rows/2, cols-10, "  ");
	mvprintw(rows/2, cols+8, "  ");

	if (winner == -1){
		attron(COLOR_PAIR(1));
		mvprintw(rows/2, cols - strlen(E_SUCKS)/2, E_SUCKS);
	}else{
		attron(COLOR_PAIR(20+winner));
		mvprintw(rows/2, cols - (strlen(WINNER)-1)/2 , WINNER, winner+1); 
	}
}

void draw_arena(int no_players){
	attron(COLOR_PAIR(2));
	for (int i =0 ; i < cols; i++){
		mvprintw(0, i*2, "  ");
		mvprintw(rows-1, i*2, "  ");
	}
	for (int i=0 ; i< rows; i++){
		mvprintw(i, 0, "  ");
		mvprintw(i, (cols-1)*2, "  ");

	}
}

void draw_trace(int player, struct int_dequeue *dx, struct int_dequeue *dy){
	struct int_dequeue_node *x_node = dx->first->next;
	struct int_dequeue_node *y_node = dy->first->next;

	attron(COLOR_PAIR(10 + player));

	while(x_node->next != NULL){
		mvprintw(y_node->val, x_node->val*2, "  ");
		x_node = x_node->next;
		y_node = y_node->next;
	}
}

/**
 * Created an empty dequeue
 * \return returns NULL if dequeue could not be created
 */
struct int_dequeue* dequeue_create(){
	struct int_dequeue *dque = malloc(sizeof(struct int_dequeue));
	struct int_dequeue_node *last = malloc(sizeof(struct int_dequeue_node));
	struct int_dequeue_node *first = malloc(sizeof(struct int_dequeue_node));

	if (dque == NULL || last == NULL || first == NULL){
		return NULL;
	}

	dque->size = 0;
	dque->first = first;
	dque->last = last;
	first->next = last;
	last ->prev = first;
	first->prev = NULL;
	last ->next = NULL;
	first->val = -1;
	last->val = -2;

	return dque;
}

/**
 * Adds to dequeue and int
 * \return return -1 if error occured (could not allocate memory)
 */
int dequeue_push_last(struct int_dequeue *dque, int num){
	struct int_dequeue_node *node = malloc(sizeof(struct int_dequeue_node));
	if (node == NULL){
		return -1;
	}
	node->val = num;
	node->prev = dque->last->prev;
	node->next = dque->last;
	dque->last->prev->next = node;
	dque->last->prev = node;
	dque->size++;
	return true;
}

/**
 * \return returns first element or -1 if the element does not exist, otherise the value read 
 */
int dequeue_read_first(struct int_dequeue *dque){
	if (dque->size == 0){
		return -1;
	}
	return dque->first->next->val;
}

/**
 * \return returns -1 if there is nothing to pop otherwise true
 */
int dequeue_pop_first(struct int_dequeue *dque){
	if(dque->size == 0){
		return -1;
	}
	struct int_dequeue_node *to_pop = dque->first->next;
	dque->first->next = to_pop->next;
	to_pop->next->prev = dque->first;
	dque->size--;
	free(to_pop);
	return true;
}

void free_dequeue(struct int_dequeue *dque){
	struct int_dequeue_node *node = dque->first;
	while (node != NULL){
		struct int_dequeue_node *next = node->next;
		free(node);
		node = next;
	}
	free(dque);
}

int mov_dir_y[4] = {-1, 1, 0, 0 };
int mov_dir_x[4] = { 0, 0,-1, 1 };

/**
 * \param x of player
 * \param y of player
 * \param dequeue for x positions
 * \param dequeue for y positions
 * \return true if player died, false if he did not
 */
int check_if_dead(int x, int y, struct int_dequeue *dx, struct int_dequeue *dy){
	struct int_dequeue_node *x_node = dx->first->next; 
	struct int_dequeue_node *y_node = dy->first->next; 
	while (x_node->next != NULL){
		if (x == x_node->val && y == y_node->val){
			return true;
		}
		x_node = x_node->next;
		y_node = y_node->next;
	}
	return false;
}
/**
 * \return true if player died, false otherwise
 */
int check_if_out_of_bounds(int x, int y){
	if (x <= 0 || y <= 0) return true;
	if (x >= cols-1 || y >= rows-1) return true;
    return false;
}

int game_loop(int no_players){
	struct player_data *p_data = malloc(no_players*sizeof(struct player_data));
	if (p_data == NULL){
		force_quit();
	}
	for (int i=0; i < no_players; i++){
		p_data[i].is_dead = false;
		p_data[i].movement_queue = dequeue_create();
		p_data[i].trail_queue_x = dequeue_create();
		p_data[i].trail_queue_y = dequeue_create();
		if (p_data[i].movement_queue == NULL || 
				p_data[i].trail_queue_x == NULL ||
				p_data[i].trail_queue_y == NULL){
			force_quit();
		}
	}
	if (no_players == 1){
		p_data[0].x = 5; p_data[0].y = 0; p_data[0].dx = 1; p_data[0].dy = 0;
	}
	if (no_players == 2){
		p_data[0].x = 4; p_data[1].x = cols - 5;
		p_data[0].y = p_data[1].y = rows/2;
		p_data[0].dx = 1; p_data[1].dx = -1;
		p_data[0].dy = p_data[1].dy = 0;
	}
	if (no_players == 3){
		p_data[0].x = 4; p_data[1].x = cols-5; p_data[2].x = cols/2;
		p_data[0].y = p_data[1].y = rows - 5; p_data[2].y = 4;
		p_data[0].dx = 1; p_data[1].dx = -1;p_data[2].dx = 0;
		p_data[0].dy = p_data[1].dy = 0; p_data[2].dy = 1;
	}
	if (no_players == 4){
		p_data[0].x = p_data[2].x = 4; p_data[1].x = p_data[3].x = cols-5;
		p_data[0].y = p_data[1].y = 4; p_data[2].y = p_data[3].y = rows-5;
		p_data[0].dx = p_data[2].dx = 1; p_data[1].dx = p_data[3].dx = -1;
		p_data[0].dy = p_data[1].dy = p_data[2].dy = p_data[3].dy = 0;
	}

	int frame = 0;

	int remaining = no_players;
	int running = true;

	int trail_per_person = ((rows*cols)/12)/no_players;
	
	while (running){
		frame++;
		int input;
		while ((input = getch()) != -1){
			for (int i=0; i < no_players; i++){
				if (input == 'q') {
					force_quit();
				}
				if (input == controls[i].up){
					Q(dequeue_push_last(p_data[i].movement_queue, 0));
				}else if (input == controls[i].down){
					Q(dequeue_push_last(p_data[i].movement_queue, 1));
				}else if (input == controls[i].left){
					Q(dequeue_push_last(p_data[i].movement_queue, 2));
				}else if (input == controls[i].right){
					Q(dequeue_push_last(p_data[i].movement_queue, 3));
				}
			}
		}

		for (int i=0 ; i < no_players; i++){
			if (p_data[i].movement_queue->size > 0){
				int mov_dir = dequeue_read_first(p_data[i].movement_queue);
				dequeue_pop_first(p_data[i].movement_queue);
				p_data[i].dx = mov_dir_x[mov_dir];
				p_data[i].dy = mov_dir_y[mov_dir];
			}
			p_data[i].x += p_data[i].dx;
			p_data[i].y += p_data[i].dy;
		}
		
		for (int i=0; i < no_players; i++){
			if (!p_data[i].is_dead){
				int died = check_if_out_of_bounds(p_data[i].x , p_data[i].y);
				for (int j=0 ; j < no_players; j++){
					if (check_if_dead(p_data[i].x, p_data[i].y, p_data[j].trail_queue_x, p_data[j].trail_queue_y)){
						died = true;
					}
				}
				if (died){
					p_data[i].is_dead = true;
					remaining--;
				}
			}
		}

		for (int i=0 ; i < no_players; i++){
			if (!p_data[i].is_dead){
				Q(dequeue_push_last(p_data[i].trail_queue_x, p_data[i].x));
				Q(dequeue_push_last(p_data[i].trail_queue_y, p_data[i].y));

				if (p_data[i].trail_queue_x->size > trail_per_person){
					dequeue_pop_first(p_data[i].trail_queue_x);
				}
				if (p_data[i].trail_queue_y->size > trail_per_person){
					dequeue_pop_first(p_data[i].trail_queue_y);
				}
			}
		}


		clear();

		draw_arena(no_players);
		for (int i=0 ; i < no_players; i++){
			draw_trace(i, p_data[i].trail_queue_x, p_data[i].trail_queue_y);
		}

		if (remaining == 0){
			running = false;
			draw_winner_screen(-1);
		}
		if (remaining == 1){
			int winner = 0;
			for (int i=1; i < no_players; i++){
				if (!p_data[i].is_dead) winner = i;
			}
			running = false;
			draw_winner_screen(winner);
		}

		refresh();

		usleep(60000);
	}
	usleep(700000);


	for (int i=0 ; i < no_players; i++){
		free_dequeue(p_data[i].movement_queue);
		free_dequeue(p_data[i].trail_queue_x);
		free_dequeue(p_data[i].trail_queue_y);
	}
	free(p_data);
}

const char menu_options = 4;
const char menu_strings[4][10] = {
	"2 Player",
	"3 Player",
	"4 Player",
	"Quit"
};

void menu_loop(){
	int running = true;
	int option = 0;
	while (running){
		int input;
		while((input = getch()) != -1){
			switch(input){
				case 'q': force_quit(); break;
				case 's': option++; break;
				case 'w': option--; break;
				case 10: 
					if (option == 3){
						running = false;
					}else{
						game_loop(option + 2);
					}
					break;
			}
			if (option < 0) option = 0;
			if (option >= menu_options) option = menu_options - 1;
		}

		erase();
		attron(COLOR_PAIR(1));
		for (int i=0 ; i < menu_options; i++){
			mvprintw(2+i , 3, menu_strings[i]);
		}
		attron(COLOR_PAIR(2));
		mvprintw(2+option, 3, menu_strings[option]);

		draw_controls(10, 20, 20, 10, 'W', 'S', 'A', 'D');
		draw_controls(10, 35, 21, 11, 'I', 'K', 'J', 'L');
		draw_controls(10, 50, 22, 12, '^', 'v', '<', '>');
		draw_controls(10, 65, 23, 13, '8', '5', '4', '6');
		refresh();
	}
}

int main(){
	initscr();
	timeout(0);
	noecho();
	curs_set(0);

	int row, col;
	getmaxyx(stdscr, row,col);
	cols = col/2;
	rows = row;

	if (has_colors() == false){
		force_quit();
	}

	start_color();


	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_WHITE);
	init_pair(3, COLOR_WHITE, COLOR_WHITE);

	init_pair(10, COLOR_BLUE, COLOR_BLUE);
	init_pair(11, COLOR_RED, COLOR_RED);
	init_pair(12, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(13, COLOR_CYAN, COLOR_CYAN);

	init_pair(20, COLOR_BLUE, COLOR_BLACK);
	init_pair(21, COLOR_RED, COLOR_BLACK);
	init_pair(22, COLOR_YELLOW, COLOR_BLACK);
	init_pair(23, COLOR_CYAN, COLOR_BLACK);

	menu_loop();

	endwin();
	return 0;
}
