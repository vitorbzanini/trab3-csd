#include <hf-risc.h>
#include "vga_drv.h"


#define brick_height 8
#define brick_widht 12

#define brick_columns 5
#define brick_per_columns 23

#define paddleHeight 5
#define paddleWidht 50  //size of paddle
#define paddleY 211		

int lives = 3;
int totalbricks = brick_columns * brick_per_columns;
int pontos = 0,init_game = 0, flag = 0;

struct ball_s {
	unsigned int ballx, bally;
	unsigned int last_ballx, last_bally;
	int dx, dy;
};


struct paddle_p{
    unsigned int paddlex;
    int dx; 
};


struct brick_b{
	unsigned int brickx, bricky;
    uint16_t color;
};


void init_display()
{
	display_background(BLACK);
	display_frectangle(0, 8, VGA_WIDTH, 1, WHITE);
	update_livesdisplay();
	print_pontos();
}

void init_ball(struct ball_s *ball, int x, int y, int dx, int dy)
{

	if (GPIOB->IN & MASK_P8){
		ball->ballx = x;
		ball->bally = y;
		ball->last_ballx = ball->ballx;
		ball->last_bally = ball->bally;
		ball->dx = dx;
		ball->dy = dy;
		init_game = 1;
	}

}

void init_input()
{
	/* configure GPIOB pins 8 .. 12 as inputs */
	GPIOB->DDR &= ~(MASK_P8 | MASK_P9 | MASK_P10 | MASK_P11 | MASK_P12);
}



void test_limits(char *limits, struct ball_s *ball)
{
	unsigned int ballx, bally;
	
	ballx = ball->ballx;
	bally = ball->bally;
	
	display_pixel(ball->last_ballx, ball->last_bally, BLACK);

	limits[0] = display_getpixel(ballx + ball->dx, bally+ ball->dy);

}

char test_collision(char *limits, struct ball_s *ball, struct brick_b *bricks, struct paddle_p *paddle)
{
	char hit = 0;
	int i;
	
	if ((ball->ballx < VGA_WIDTH-1) && (ball->ballx > 0) && (ball->bally < VGA_HEIGHT-1) && (ball->bally > 0)) {
		if(ball->bally+1 >= (VGA_HEIGHT-5)){
			if (ball->ballx <= paddle->paddlex + paddleWidht/2 && ball->ballx >= paddle->paddlex){
				ball->dx = -1;
				ball->dy = -1;
			}
			else{
				if (paddle->paddlex + paddleWidht/2 < ball->ballx && ball->ballx <= paddle->paddlex + paddleWidht){
				ball->dx = 1;
				ball->dy = -1;
				}
			}
		}
		else {
			if ((ball->ballx + ball->dx >= VGA_WIDTH-1) || (ball->ballx + ball->dx < 1)) ball->dx = -ball->dx;
			else {
				if (ball->bally + ball->dy <= 8) {
					ball->dy = -ball->dy;
				}
				else { 
					if(limits[0] != 7 && limits[0] != 0) test_blockHit(limits, ball, bricks, 0);
				} 
			} 
		}
	} 

	return hit;

	display_pixel(ball->last_ballx, ball->last_bally, WHITE);

}

void test_blockHit(char *limits, struct ball_s *ball, struct brick_b *bricks, char hit){
	int ballx = ball->ballx + ball->dx;
	int bally = ball->bally + ball->dy;

	//if((brick_height + 1)*brick_columns + 9 >= ball->bally){
		int paddle_x =0, paddle_y = 10;
		for(int i = 1; i <= brick_columns; i++){
			if((bally >= paddle_y-1) && (bally <= (paddle_y+8))){
				if(((bally == paddle_y) || (bally == paddle_y+1)) || ((bally == paddle_y+brick_height-1) || (bally == paddle_y+brick_height-2))) ball->dy = -ball->dy;
				//printf("paddle_y = %d\n",paddle_y);
				pontos +=  6 - ((int) paddle_y / 9);
				print_pontos();
				break;
			}
			paddle_y += brick_height + 1;
		}
	
		for(int j = 1; j <= VGA_WIDTH - brick_widht - 1; j += brick_widht + 1){
			if((ballx >= j) && (ballx <= (j+brick_widht-1)))  {
				paddle_x = j;
				if(((ballx == paddle_x) || (ball == paddle_x+1)) || ((ballx == paddle_x+brick_widht-1) || (ballx == paddle_x+brick_widht-2))) ball->dx = -ball->dx;
				display_frectangle(paddle_x, paddle_y, brick_widht, brick_height, BLACK);
				totalbricks -= 1;
				//printf("paddle_x: %d\n", paddle_x);
				return;		
			}
		}
	//} 	
}

void print_pontos() { 
	static char str[11] = "Pontos:";
	display_print(str,1,0,1,0);
	itoa(pontos, str+7 ,10);
	display_print(str,1,0,1,7);
}


void update_ball(struct ball_s *ball)
{
			ball->ballx = ball->ballx + ball->dx;
			ball->bally = ball->bally + ball->dy;
			display_pixel(ball->last_ballx, ball->last_bally, BLACK);
			display_pixel(ball->ballx, ball->bally, WHITE);
			ball->last_ballx = ball->ballx;
			ball->last_bally = ball->bally;
}


void update_paddle(struct paddle_p *paddle)
{
	display_frectangle(paddle->paddlex, paddleY, paddleWidht, paddleHeight, WHITE);

	if(paddle->dx == 0)
		return;

	paddle->paddlex = paddle->paddlex + paddle->dx;

	if(paddle->dx > 0){
		display_frectangle(paddle->paddlex - paddle->dx, paddleY, 1, paddleHeight, BLACK);
	}
	else{
		display_frectangle(paddle->paddlex + paddleWidht, paddleY, 1, paddleHeight, BLACK);
	} 
}

void init_paddle(struct paddle_p * paddle){
	if ((GPIOB->IN & MASK_P8) && flag != 1){
		display_frectangle((VGA_WIDTH/2)-15, (VGA_HEIGHT)-5, paddleWidht, 5, WHITE);
		paddle->paddlex = (VGA_WIDTH/2)-15;
		flag = 1;
	}
}

void get_input(struct paddle_p * paddle)
{
	int values_read[2];
	read_mouse(values_read);
	static int changes = 0;
	if(abs(values_read[1]) > 10000) changes = 3; 
	else if(abs(values_read[1]) > 1000)  changes = 2; 
	else if(abs(values_read[1]) > 100)   changes = 1;
	else changes = 0;


	if((values_read[0] == 0) && (flag == 1) && ((paddle->paddlex - changes) > 2)){
		display_frectangle(paddle->paddlex+paddleWidht-changes, (VGA_HEIGHT)-5, changes, 5, BLACK);
		display_frectangle(paddle->paddlex-changes, (VGA_HEIGHT)-5, changes, 5, WHITE);
		paddle->paddlex = paddle->paddlex - changes;
	}
	if((values_read[0] == 1) && (flag == 1) && ((paddle->paddlex + changes) < VGA_WIDTH-paddleWidht-1)){
		display_frectangle(paddle->paddlex, (VGA_HEIGHT)-5, changes, 5, BLACK);
		display_frectangle(paddle->paddlex+paddleWidht, (VGA_HEIGHT)-5, changes, 5, WHITE);
		paddle->paddlex = paddle->paddlex + changes;
	}

}

char test_Death(struct ball_s *ball, struct paddle_p * paddle){
	if(ball->bally + ball->dy > VGA_HEIGHT-1){
		lives--;
		update_livesdisplay();
		init_game = 0;
		display_pixel(ball->ballx, ball->bally, BLACK);
		return 1;
	}

	return 0;
}

void update_livesdisplay(void){
	display_frectangle(250, 1, 14, 6, BLACK);

	int x = 250;
	for(int i = 0; i < lives; i++){
		display_frectangle(x, 1, 2, 6, WHITE);
		x += 4;
	}
}



int main(void)
{
	struct ball_s ball;
	struct ball_s *pball = &ball;
	pball->dx = 0;
	pball->dy = 0;
	char limits[9],cont=0;

	struct paddle_p Inpaddle;
	struct paddle_p *paddle = &Inpaddle;

	const int initialTotalBricks = totalbricks;
	struct brick_b bricks[initialTotalBricks];

	while (1){
		init_display();
		init_input();

		int startBrickY = 10;
		int brickIndex = 0;
		for(int i = 1; i <= brick_columns; i++){
			for(int j = 1; j <= VGA_WIDTH - brick_widht - 1; j += brick_widht + 1){
				bricks[brickIndex].brickx = j;
				bricks[brickIndex].bricky = startBrickY;
				bricks[brickIndex].color = i;
				display_frectangle(j, startBrickY, brick_widht, brick_height, i);
				brickIndex++;
			}
			startBrickY += brick_height + 1;
		}


		/*while (lives > 0 && totalbricks > 0) {
			if(cont>1 && (init_game==1)){
				test_Death(pball, &init_game);
				test_limits(limits, pball);
				test_collision(limits, pball, bricks, paddle);
				cont=0;
				update_ball(pball);
			}
			else cont++;
			init_ball(pball, 0, VGA_HEIGHT/2, 1, 1, &init_game);
			get_input(paddle);
			delay_ms(5);
		}*/

		while (lives > 0 && totalbricks > 0) {
			init_ball(pball, 40, VGA_HEIGHT/2, 1, 1); 
			init_paddle(paddle);
			if(init_game) {
				if(test_Death(pball, paddle)) continue;
				get_input(paddle);
				test_limits(limits, pball);
				test_collision(limits, pball, bricks, paddle);
				update_ball(pball);
				delay_ms(5);
			}
		}			

		display_frectangle(paddle->paddlex, (VGA_HEIGHT)-5, paddleWidht, 5, BLACK);
		if(lives == 0) flag = 0;

		while (1){
			if (GPIOB->IN & MASK_P9){
				lives = 3;
				pontos = 0;
				totalbricks = brick_columns * brick_per_columns;
				break;
			}
			if (GPIOB->IN & MASK_P12){
				return 1;
			}
		}
		
	}

	return 0;
}
