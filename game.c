#include <stdbool.h> 
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


#define MAXPROJECTILES 20
 
volatile int pixel_buffer_start; // global variable

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
#define BLACK 0x0000

#define FPGA_CHAR_BASE        0xC9000000

#define FALSE 0
#define TRUE 1
	
const int screenWidth = 320, screenHeight = 240, maxProjectiles = 10, playersize = 20, playerspeed = 4, maxspeed = 5, maxsize = 5;

int numProjectiles = 5, projectileMinSize = 3, projectileSizeVar = 5, projectileMinSpeed = 1, projectileSpeedVar = 3, aimBuffer = 50;

typedef enum {
	TITLE, // loading, replay
	PLAY, // enter
	END, // hit by projectile
	HOW_TO  // press h
	} state;

state STATE; // global variable
state LASTSTATE;

typedef struct Object{
    int dx;
    int dy; 
    int x;
    int y; 
    int height; 
    int width; 
    int speed; 
    short int colour;
}Object; 

void clear_screen();
void clear_text();
void plot_pixel(int x, int y, short int line_color);
void wait();
void update_player(Object *player, unsigned char byte);
void update_screen(unsigned char byte);
void read_keyboard(unsigned char *pressedKey);
void plot_object(Object obj, short int colour);
Object create_projectile(Object player);
void update_projectiles(Object projectiles[], Object player);
bool does_collide(Object projectiles[], Object player);
void draw_title(int dx, int dy, short int line_color);
void draw_how_to(int dx, int dy, short int line_color);
void draw_end(int dx, int dy, short int line_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void draw_box(int x, int y, short int box_color);
void draw_text(int x, int y, char str[]);
void draw_char(int x, int y, char letter);
void draw_happy(int x, int y , short int square_color,short int eye_color);
void draw_annoyed(int x, int y , short int square_color,short int eye_color);
void draw_angry(int x, int y , short int square_color,short int eye_color);
void hex_timer();
//void draw_heart(int x,int y, short int line_color);
///void draw_dead(int x, int y , short int square_color,short int eye_color);


Object projectiles[MAXPROJECTILES];

// initialize score to 0
int score = 0;
int best_score = 0;

int main(void)
{
    srand (time(NULL));
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    /* Read location of the pixel buffer from the pixel buffer controller */
    pixel_buffer_start = *pixel_ctrl_ptr;

    //clear_screen from before
    clear_screen();
    //initialize player
    unsigned char pressedKey = 0;
    Object p1 = {0,0,screenWidth/2, screenHeight/2, playersize, playersize, playerspeed, GREEN};
	
	// initialize state to title page
	STATE = TITLE;
	LASTSTATE = TITLE;

	
	// boolean so that title doesnt redraw every time 
	int draw_once = FALSE;
	
	hex_timer();
	int i;
	
	int lives = 3;
	
    while(1){
		
		switch (STATE){
		
			case TITLE: 
				if(draw_once == 0){
					draw_title(11, 50, ORANGE);
					draw_title(14, 50, MAGENTA);
					draw_title(15, 50, YELLOW);
					draw_once = TRUE ;
				}
				LASTSTATE = STATE;
				read_keyboard(&pressedKey);
				update_screen(pressedKey);
				break;
			case PLAY: 
				//started new game, reinitialize everything
				if(LASTSTATE != PLAY){
					p1.dx = 0;
					p1.dy = 0;
					p1.x = screenWidth/2;
					p1.y = screenHeight/2;
					numProjectiles = 5;
					for(i = 0; i < numProjectiles; i++)
        				projectiles[i] = create_projectile(p1);
        			lives = 3;
        			aimBuffer = 50;
        			projectileMinSize = 3;
        			projectileMinSpeed = 1;

				}

				//plot objects
				if(lives == 3)
					draw_happy(p1.x, p1.y, CYAN,BLUE);
				else if(lives == 2)
					draw_annoyed(p1.x, p1.y, ORANGE,BLUE);
				else 
					draw_angry(p1.x, p1.y, RED,BLUE);
			

        		for(i = 0; i < numProjectiles; i++)
            		plot_object(projectiles[i], YELLOW);

            	LASTSTATE = STATE;

        		if(does_collide(projectiles, p1))
        			lives--;

        		if(lives == 0)
					STATE = END;

        		wait();

        		//clear objects
        		plot_object(p1, BLACK);
        		for(i = 0; i < numProjectiles; i++)
            		plot_object(projectiles[i], BLACK);

        		read_keyboard(&pressedKey);
        		update_player(&p1, pressedKey);
        		update_projectiles(projectiles, p1);
				
				//display current score
				score++;
				hex_timer();

				//increment the difficulty 
				
				if(score % 100 == 0){
					if(numProjectiles < maxProjectiles){
						projectiles[numProjectiles] = create_projectile(p1);
						numProjectiles++;
					}
					if(aimBuffer > 1)
						aimBuffer--;
			
				}
				if(score % 300 == 0){
					if(projectileMinSize < maxsize)
						projectileMinSize++;
					if(projectileMinSpeed < maxspeed)
						projectileMinSpeed++;
				}
				
				break;
			case HOW_TO:
				draw_once = FALSE;
				draw_how_to(0,0,GREEN);
				LASTSTATE = STATE;
				read_keyboard(&pressedKey);
				update_screen(pressedKey);
				break;
				
			case END: 
				draw_end(0,0,ORANGE);
				LASTSTATE = STATE;
				read_keyboard(&pressedKey);
				update_screen(pressedKey);
				break;

		}
    }
    return 0;
}

bool does_collide(Object projectiles[], Object player){

    int i;
    for(i = 0; i < numProjectiles; i++){
        if(player.x < projectiles[i].x + projectiles[i].width &&
            player.x + player.width > projectiles[i].x &&
            player.y < projectiles[i].y + projectiles[i].height &&
            player.y + player.height > projectiles[i].y){
        	plot_object(projectiles[i], BLACK);
        	projectiles[i] = create_projectile(player);
        	return true;
        }
           
    }
    return false;
}

void update_projectiles(Object projectiles[], Object player){
    int i;
    for(i = 0; i < numProjectiles; i++){
        
        //replace a projectile with a new one if it goes out of bounds
        if (projectiles[i].x < 0 ||  projectiles[i].x >= screenWidth || projectiles[i].y < 0 || projectiles[i].y >= screenHeight){
           projectiles[i] = create_projectile(player);
        }
        else{
            projectiles[i].x += projectiles[i].dx;
            projectiles[i].y += projectiles[i].dy;
        }

    }

}

Object create_projectile(Object player){
        Object projectile;
        int randint = rand();
        int whichSide = randint % 4;
        //randomize which side the projectiles come from
        if(whichSide == 0){
            projectile.x = 0;
            projectile.y = randint % screenHeight;
        }else if(whichSide == 1){
            projectile.y = 0;
            projectile.x = randint % screenWidth;
        }else if(whichSide == 2){
            projectile.y = screenHeight-1;
            projectile.x = randint % screenWidth;
        }else{
            projectile.x = screenWidth-1;
            projectile.y = randint % screenHeight;
        }
        int size = randint % projectileSizeVar + projectileMinSize;
        projectile.width = size;
        projectile.height = size;
        projectile.speed = randint % projectileSpeedVar + projectileMinSpeed;

        //make the projectiles generally aim at the player
        int targetx = player.x += randint % (2*aimBuffer) - aimBuffer;
        int targety = player.y += randint % (2*aimBuffer) - aimBuffer;


        if(targetx < aimBuffer)
            targetx = aimBuffer;
        else if(targetx > screenWidth - aimBuffer)
            targetx = screenWidth - aimBuffer;

        if(targety < aimBuffer)
            targety = aimBuffer;
        else if(targety > screenHeight - aimBuffer)
            targety = screenHeight - aimBuffer;


        double tempdx = targetx - projectile.x;
        double tempdy = targety - projectile.y;


        bool dxispos = true, dyispos = true;
        if(tempdx < 0)
        	dxispos = false;
        if(tempdy < 0)
        	dyispos = false;
        double absdx = abs(tempdx);
        double absdy = abs(tempdy);


        //"normalize" the dx and dy
        if(absdx > absdy){       
            if(absdx > projectile.speed)
                absdx = projectile.speed;
            absdy = 1;
        }
        else{  
            if(absdy > projectile.speed)
                absdy = projectile.speed;
            absdx = 1;
        }
        if(absdx == 1 && absdy == 1){
        	absdx = 2;
        	absdy = 2;
        }
        if(dxispos)
        	projectile.dx = absdx;
        else
        	projectile.dx = -absdx;

        if(dyispos)
        	projectile.dy = absdy;
        else
        	projectile.dy = -absdy;

        return projectile;
}

void plot_pixel(int x, int y, short int line_color)
{
	if(x < 0 || x >= screenWidth || y < 0 || y >= screenHeight)
		return;
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void plot_object(Object obj, short int colour)
{
    int leftbound = (obj.x) - (obj.width)/2;
    int rightbound = (obj.x) + (obj.width)/2;
    int lowerbound = (obj.y) - (obj.height)/2;
    int upperbound = (obj.y) + (obj.height)/2;

    if(leftbound < 0)
        leftbound = 0;
    else if(rightbound >= screenWidth)
        rightbound = screenWidth-1;

    if(lowerbound < 0)
        lowerbound = 0;
    else if(upperbound >= screenHeight)
        upperbound = screenHeight-1;


    for (int x = leftbound; x <= rightbound; ++x)
    {
        for (int y = lowerbound; y <= upperbound; ++y) 
            plot_pixel(x, y, colour);
    }
}

void clear_screen()
{
    int x, y;

    for (x = 0; x < 320; ++x)
    {
        for (y = 0; y < 240; ++y)
        {
            plot_pixel(x, y, 0x000);
        }
    }
}

void wait() {
    

    volatile int* pixel_ctrl_ptr = (int*)0xFF203020;
    volatile int* status_register_ptr = (int*)0xFF20302C;
	
	int swap = *(pixel_ctrl_ptr);
	*(pixel_ctrl_ptr+1) = swap ;

    *pixel_ctrl_ptr = 1;

    //Keep reading the status until Status S = 1
    while ((*status_register_ptr & 0x01) != 0) {
        status_register_ptr = status_register_ptr;
    }

    //Exit when Status S is 1
    return;
}

void read_keyboard(unsigned char *pressedKey) {
    volatile int * PS2_ptr = (int *) 0xFF200100;
    int data = *PS2_ptr;

    *pressedKey = data & 0xFF;

    while (data & 0x8000) {
        data = *PS2_ptr;
    }
}

void update_player(Object *player, unsigned char byte) {
    if (byte == 0x74){
        player->dx = 1;
        player->dy = 0;
    }
    else if (byte == 0x6B){
        player->dx = -1;
        player->dy = 0;
    }
    else if (byte == 0x75){
        player->dy = -1;
        player->dx = 0;
    }
    else if (byte == 0x72){
        player->dy = 1;
        player->dx = 0;
    }
    else
    {
       player->dx = 0;
       player->dy = 0;
    }
        

    if (player->x - player->width/2 + (player->speed)*(player->dx) < 0 || player->x + player->width/2 + (player->speed)*(player->dx) >= screenWidth){
        //player->dx = 0;
    }
    else
        player->x += (player->speed)*(player->dx);
    

    if (player->y - player->height/2 + (player->speed)*(player->dy) < 0 || player->y + player->height/2 + (player->speed)*(player->dy) >= screenHeight){
        //player->dy = 0;
    }
    else
        player->y += (player->speed)*(player->dy);
       
}

void update_screen(unsigned char byte){
	// [ENTER] from title page 
	if (STATE == TITLE && byte == 0x5A){
		draw_title(11, 50, BLACK);
		draw_title(14, 50, BLACK);
		draw_title(15, 50, BLACK);
		clear_text();
        STATE = PLAY;
    }
	// [ENTER] from end 
	if (STATE == END && byte == 0x5A){
		//clear_screen();
		draw_end(0,0,BLACK);

		clear_text();
        STATE = PLAY;
		// restart score
		score = 0;
    }
	// [H]
	else if(STATE == TITLE && byte == 0x33) {
		clear_screen();
		clear_text();
		STATE = HOW_TO;
	}
	else if( STATE == HOW_TO && byte == 0x76){
		clear_screen();
		clear_text();
		STATE = TITLE;
	}
}

void draw_title(int dx, int dy, short int line_color){
		
		// note: int dx and dy are translation factor
	
		// G
		for(int i = 0 ; i < 10; i ++){
		draw_line(170+i+dx-100,64+dy-51,170+i+dx-100,88+dy-51,line_color);
		}
		draw_box(175+dx-100,55+dy-51,line_color);
		draw_box(183+dx-100,50+dy-51,line_color);
		draw_box(191+dx-100,55+dy-51,line_color);
		draw_box(175+dx-100,86+dy-51,line_color);
		draw_box(183+dx-100,91+dy-51,line_color);
		draw_box(191+dx-100,86+dy-51,line_color);
		draw_box(195+dx-100,75+dy-51,line_color);
		draw_box(190+dx-100,75+dy-51,line_color);
		draw_box(199+dx-100,75+dy-51,line_color);
		for(int i = 0 ; i < 7; i++){
			draw_line(200+i+dx-100,80+dy-51,200+i+dx-100,99+dy-51,line_color);
		}
	
		//E
		for(int i = 0 ; i < 10; i ++){
			draw_line(213+i+dx-100,50+dy-51,213+i+dx-100,100+dy-51,line_color);
		}
		for(int i = 0 ; i < 9; i ++){
			draw_line(222+dx-100,50+i+dy-51,243+dx-100,50+i+dy-51,line_color);
			draw_line(222+dx-100,91+i+dy-51,243+dx-100,91+i+dy-51,line_color);
		}
		for(int i = 0 ; i < 7; i ++){
			draw_line(222+dx-100,75+i+dy-51,240+dx-100,75+i+dy-51,line_color);
		}
	
		// O
		for(int i = 0 ; i < 10; i ++){
			draw_line(90+i+dx+55,64+dy-51,90+i+dx+55,88+dy-51,line_color);
			draw_line(116+i+dx+55,64+dy-51,116+i+dx+55,88+dy-51,line_color);
		}
		draw_box(95+dx+55,55+dy-51,line_color);
		draw_box(103+dx+55,50+dy-51,line_color);
		draw_box(111+dx+55,55+dy-51,line_color);
		draw_box(95+dx+55,86+dy-51,line_color);
		draw_box(103+dx+55,91+dy-51,line_color);
		draw_box(111+dx+55,86+dy-51,line_color);
	
		// - 
		for(int i = 0 ; i < 10; i ++){
			draw_line(222+dx-35,75+i+dy-51,240+dx-25,75+i+dy-51,line_color);
		}
	
		// D
		for(int i = 0 ; i < 10; i ++){
			draw_line(50+i+dx,50+dy,50+i+dx,100+dy,line_color);
		}
		draw_box(59+dx,50+dy,line_color);
		draw_box(68+dx,55+dy,line_color);
		draw_box(75+dx,64+dy,line_color);
		draw_box(75+dx,73+dy,line_color);
		draw_box(75+dx,79+dy,line_color);
		draw_box(68+dx,88+dy,line_color);
		draw_box(59+dx,91+dy,line_color);
		
		// O
		for(int i = 0 ; i < 10; i ++){
			draw_line(90+i+dx,64+dy,90+i+dx,88+dy,line_color);
			draw_line(116+i+dx,64+dy,116+i+dx,88+dy,line_color);
		}
		draw_box(95+dx,55+dy,line_color);
		draw_box(103+dx,50+dy,line_color);
		draw_box(111+dx,55+dy,line_color);
		draw_box(95+dx,86+dy,line_color);
		draw_box(103+dx,91+dy,line_color);
		draw_box(111+dx,86+dy,line_color);
		
		// D
		for(int i = 0 ; i < 10; i ++){
			draw_line(130+i+dx,50+dy,130+i+dx,100+dy,line_color);
		}
		draw_box(139+dx,50+dy,line_color);
		draw_box(148+dx,55+dy,line_color);
		draw_box(155+dx,64+dy,line_color);
		draw_box(155+dx,73+dy,line_color);
		draw_box(155+dx,79+dy,line_color);
		draw_box(148+dx,88+dy,line_color);
		draw_box(139+dx,91+dy,line_color);
		
		// G
		for(int i = 0 ; i < 10; i ++){
			draw_line(170+i+dx,64+dy,170+i+dx,88+dy,line_color);
		}
		draw_box(175+dx,55+dy,line_color);
		draw_box(183+dx,50+dy,line_color);
		draw_box(191+dx,55+dy,line_color);
		draw_box(175+dx,86+dy,line_color);
		draw_box(183+dx,91+dy,line_color);
		draw_box(191+dx,86+dy,line_color);
		draw_box(195+dx,75+dy,line_color);
		draw_box(190+dx,75+dy,line_color);
		draw_box(199+dx,75+dy,line_color);
		for(int i = 0 ; i < 7; i++){
			draw_line(200+i+dx,80+dy,200+i+dx,99+dy,line_color);
		}

		// E
		for(int i = 0 ; i < 10; i ++){
			draw_line(213+i+dx,50+dy,213+i+dx,100+dy,line_color);
		}
		for(int i = 0 ; i < 9; i ++){
			draw_line(222+dx,50+i+dy,243+dx,50+i+dy,line_color);
			draw_line(222+dx,91+i+dy,243+dx,91+i+dy,line_color);
		}
		for(int i = 0 ; i < 7; i ++){
			draw_line(222+dx,75+i+dy,240+dx,75+i+dy,line_color);
		}
		
		draw_text(28,46, "press [H] for How To Play");
		
		draw_text(30,50,"press [ENTER] to Begin");
		
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color)
{
	char is_steep = FALSE;
	if(abs(y1 - y0) > abs(x1 - x0)) is_steep = TRUE;
	
	if(is_steep){
		int temp;
		temp = x0;
		x0 = y0;
		y0 = temp;
		
		temp = x1;
		x1 = y1;
		y1 = temp;
	}
	
	if(x0>x1){
		int temp;
		temp = x0;
		x0 = x1;
		x1 = temp;
		
		temp = y1;
		y1 = y0;
		y0 = temp;

	}
	
	int deltax = x1 - x0;
	int deltay = y1 - y0;
	if(y1<y0) deltay = y0 - y1;
	int error = -(deltax/2);
	int y = y0;
	
	int y_step;
	
	if(y0<y1) 
		y_step = 1; 
	else 
		y_step = -1;

	for(int x = x0; x<x1; ++x){
		if(is_steep)
			plot_pixel(y,x,line_color);
		else
			plot_pixel(x,y,line_color);
		
		error = error + deltay;
		if (error >= 0) {
			y = y + y_step; 
			error = error - deltax;
		}
	}
}

// 9x9 box
void draw_box(int x, int y, short int box_color) {
	for(int i = 0; i < 10 ; i++){
		draw_line(x+i,y,x+i,y+9,box_color);
	}
}

void draw_text(int x, int y, char str[]) {
	for (int i = 0; i < strlen(str); i++) {
		draw_char(x+i, y, str[i]);
	}
}

void draw_char(int x, int y, char letter) {
	*(char *)(FPGA_CHAR_BASE + (y << 7) + x) = letter;
}

void draw_end(int dx, int dy, short int line_color){
	// G
	for(int i = 0 ; i < 10; i ++){
		draw_line(170+i+dx-100,64+dy,170+i+dx-100,88+dy,line_color);
	}
	draw_box(175+dx-100,55+dy,line_color);
	draw_box(183+dx-100,50+dy,line_color);
	draw_box(191+dx-100,55+dy,line_color);
	draw_box(175+dx-100,86+dy,line_color);
	draw_box(183+dx-100,91+dy,line_color);
	draw_box(191+dx-100,86+dy,line_color);
	draw_box(195+dx-100,75+dy,line_color);
	draw_box(190+dx-100,75+dy,line_color);
	draw_box(199+dx-100,75+dy,line_color);
	for(int i = 0 ; i < 7; i++){
		draw_line(200+i+dx-100,80+dy,200+i+dx-100,99+dy,line_color);
	}
	// A
	for(int i = 0 ; i < 10; i ++){
		draw_line(170+i+dx-55,64+dy,170+i+dx-55,100+dy,line_color);
		draw_line(195+i+dx-55,64+dy,195+i+dx-55,100+dy,line_color);
		draw_line(170+dx-55,80+dy+i,195+dx-55,80+dy+i,line_color);
	}
	draw_box(175+dx-55,55+dy,line_color);
	draw_box(183+dx-55,50+dy,line_color);
	draw_box(191+dx-55,55+dy,line_color);
	// M
	for(int i = 0 ; i < 10; i ++){
		draw_line(170+i+dx-12,64+dy,170+i+dx-12,100+dy,line_color);
		draw_line(195+i+dx-17,64+dy,195+i+dx-17,100+dy,line_color);
		draw_line(195+i+dx+2,64+dy,195+i+dx+2,100+dy,line_color);
		draw_line(170+dx-55,80+dy+i,195+dx-55,80+dy+i,line_color);
	}
	draw_box(175+dx+6,55+dy,line_color);
	draw_box(183+dx+6,50+dy,line_color);
	draw_box(191+dx+6,55+dy,line_color);
	draw_box(175+dx-17,55+dy,line_color);
	draw_box(183+dx-17,50+dy,line_color);
	draw_box(191+dx-17,55+dy,line_color);
	
	// E

	for(int i = 0 ; i < 10; i ++){
		draw_line(213+i+dx,50+dy,213+i+dx,100+dy,line_color);
	}
	for(int i = 0 ; i < 9; i ++){
		draw_line(222+dx,50+i+dy,243+dx,50+i+dy,line_color);
		draw_line(222+dx,91+i+dy,243+dx,91+i+dy,line_color);
	}
	for(int i = 0 ; i < 7; i ++){
		draw_line(222+dx,75+i+dy,240+dx,75+i+dy,line_color);
	}
		
	// O 
	for(int i = 0 ; i < 10; i ++){
		draw_line(90+i+dx-20,64+dy+60,90+i+dx-20,88+dy+60,line_color);
		draw_line(116+i+dx-20,64+dy+60,116+i+dx-20,88+dy+60,line_color);
	}
	draw_box(95+dx-20,55+dy+60,line_color);
	draw_box(103+dx-20,50+dy+60,line_color);
	draw_box(111+dx-20,55+dy+60,line_color);
	draw_box(95+dx-20,86+dy+60,line_color);
	draw_box(103+dx-20,91+dy+60,line_color);
	draw_box(111+dx-20,86+dy+60,line_color);
	// V
	for(int i = 0 ; i < 10; i ++){
		draw_line(170+i+dx-55,64+dy+47,170+i+dx-55,100+dy+47,line_color);
		draw_line(195+i+dx-55,64+dy+47,195+i+dx-55,100+dy+47,line_color);
	}
	draw_box(95+dx+20,86+dy+60,line_color);
	draw_box(103+dx+20,91+dy+60,line_color);
	draw_box(111+dx+20,86+dy+60,line_color);
	// E
	for(int i = 0 ; i < 10; i ++){
		draw_line(213+i+dx-55,50+dy+60,213+i+dx-55,100+dy+60,line_color);
	}
	for(int i = 0 ; i < 9; i ++){
		draw_line(222+dx-55,50+i+dy+60,243+dx-55,50+i+dy+60,line_color);
		draw_line(222+dx-55,91+i+dy+60,243+dx-55,91+i+dy+60,line_color);
	}
	for(int i = 0 ; i < 7; i ++){
		draw_line(222+dx-55,75+i+dy+60,240+dx-55,75+i+dy+60,line_color);
	}
	// R
	for(int i = 0 ; i < 10; i ++){
		draw_line(213+i+dx-17,50+dy+60,213+i+dx-17,100+dy+60,line_color);
		draw_line(213+i+dx+10,50+dy+66,213+i+dx+10,100+dy+35,line_color);
		draw_line(213+i+dx,50+dy+85,213+i+dx+10,100+dy+60,line_color);
		draw_line(222+dx-16,50+i+dy+60,243+dx-16,50+i+dy+60,line_color);
	}
	
	int ones = score%10;
	int tens = ((score-ones)/10)%10;
	int huns = (score/100)%10;
	int thou = (score/1000)%10;
	
	// convert current score into a string 
	char sscore[] = {48+thou,48+huns,48+tens,48+ones,0};
	
	draw_text(30,44, "your score: ");
	draw_text(41,44, sscore);
	
	// check for best score and convert into a string 
	if(score > best_score) best_score = score;
	
	int bones = best_score%10;
	int btens = ((best_score-ones)/10)%10;
	int bhuns = (best_score/100)%10;
	int bthou = (best_score/1000)%10;
	
	char bscore[] = {48+bthou,48+bhuns,48+btens,48+bones,0};
	
	draw_text(30, 46, "best score: ");
	draw_text(41,46, bscore);
	
	draw_text(26, 50, "press [ENTER] to play again");
	
}

void draw_how_to(int dx, int dy, short int line_color){

	for(int i = 0 ; i < 5 ; i++){
		// H
		draw_line(30+i, 50, 30+i, 80, line_color);
		draw_line(45+i, 50, 45+i, 80, line_color);
		draw_line(34, 63+i, 45, 63+i, line_color);
		// O 
		draw_line(52+i, 55, 52+i, 75, line_color);
		draw_line(67+i, 55, 67+i, 75, line_color);
		draw_line(56, 50+i, 68, 50+i, line_color);
		draw_line(56, 75+i, 68, 75+i, line_color);
		// W
		draw_line(75+i,50,75+i,75,line_color);
		draw_line(84+i,50,84+i,75,line_color);
		draw_line(93+i,50,93+i,75,line_color);
		draw_line(77, 75+i, 85, 75+i, line_color);
		draw_line(88, 75+i, 96, 75+i, line_color);
		
		// T
		draw_line(120+i,50,120+i,80,line_color);
		draw_line(110,50+i,135,50+i,line_color);
		// O 
		draw_line(138+i, 55, 138+i, 75, line_color);
		draw_line(153+i, 55, 153+i, 75, line_color);
		draw_line(142, 50+i, 154, 50+i, line_color);
		draw_line(142, 75+i, 154, 75+i, line_color);
	
		draw_text(8, 25, "Use arrow keys to dodge projectiles as long as possible");
		
		draw_happy(50,120,CYAN,BLUE);
		draw_text(17,30,"You have all your three lives");
		
		draw_annoyed(50,150,ORANGE,BLUE);
		draw_text(17,37,"Be careful, you have two lives left");
		
		draw_angry(50,180,RED,BLUE);
		draw_text(17,44,"You are on your last life");
    	
		draw_text(8, 50, "The game is over when you get hit!");
		
		draw_text(8, 52, "press [ESC] to return");
		
	}
}


void clear_text (){
	int y,x;
	for(x=0;x<80;x++){
		for(y=0;y<60;y++){
			draw_text(x, y, " ");
		}
	}
}




void hex_timer(){
	volatile int * hex3_0_base = ( int * ) 0xff200020;
	
	
	int ones = score%10;
	int tens = ((score-ones)/10)%10;
	int huns = (score/100)%10;
	int thou = (score/1000)%10;
	
	
	int bit_patterns[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 
				  0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01100111};
	
	int ones_pattern = bit_patterns[ones];
	int tens_pattern = bit_patterns[tens];
	int huns_pattern = bit_patterns[huns];
	int thou_pattern = bit_patterns[thou];
	
	int display_pattern =  (thou_pattern << 24)+(huns_pattern << 16) + (tens_pattern << 8) + ones_pattern;
	
	* hex3_0_base = display_pattern;
	
}

void draw_happy(int x, int y , short int square_color,short int eye_color){
	// x and y are center of box
	// dimensions are 20x20 
	for (int i = 0; i < 10; i ++) {
		draw_line(x-10,y+i,x+10,y+i,square_color);
		draw_line(x-10,y-i,x+10,y-i,square_color);
	}
	
	// face 
	for (int i = 0; i <= 3; i ++) {
		draw_line(x+4+i,y+3,x+4+i,y-1,eye_color);
		draw_line(x-5-i,y+3,x-5-i,y-1,eye_color);
	}
	
	draw_line(x,y+6,x+3,y+3,eye_color);
	draw_line(x,y+6,x-3,y+4,eye_color);
}

void draw_annoyed(int x, int y, short int square_color,short int eye_color){
	// x and y are center of box
	// dimensions are 20x20 
	for (int i = 0; i < 10; i ++) {
		draw_line(x-10,y+i,x+10,y+i,square_color);
		draw_line(x-10,y-i,x+10,y-i,square_color);
	}
	
	// face 
	for (int i = 0; i <= 3; i ++) {
		draw_line(x+4+i,y+3,x+4+i,y,eye_color);
		draw_line(x-5-i,y+3,x-5-i,y,eye_color);
	}
	
	draw_line(x,y+4,x+2,y+4,eye_color);
	draw_line(x,y+4,x-2,y+4,eye_color);
}

void draw_angry(int x, int y , short int square_color,short int eye_color){
	// x and y are center of box
	// dimensions are 20x20 
	for (int i = 0; i < 10; i ++) {
		draw_line(x-10,y+i,x+10,y+i,square_color);
		draw_line(x-10,y-i,x+10,y-i,square_color);
	}
	
	// face 
	for (int i = 0; i <= 3; i ++) {
		draw_line(x+4+i,y+3,x+4+i,y,eye_color);
		draw_line(x-5-i,y+3,x-5-i,y,eye_color);
	}
	
	draw_line(x,y+4,x+2,y+5,eye_color);
	draw_line(x,y+4,x-2,y+5,eye_color);
	
	draw_line(x+4,y,x+7,y-3,eye_color);
	draw_line(x-4,y,x-7,y-2,eye_color);
}

/*
void draw_heart(int x, int y, short int line_color){
	for(int i = 0; i < 4 ; i++){
		draw_line(x+i, y+10, x+i, y-7, line_color);
		draw_line(x+i+4, y-9, x+i+4, y+6, line_color);
		draw_line(x+i+10, y-8, x+i+10, y-2, line_color);
		draw_line(x+i-4, y-9, x+i-4, y+6, line_color);
		draw_line(x+i-10, y-8, x+i-10, y-2, line_color);
		draw_line(x+i-7, y-11, x+i-7, y+1, line_color);
		draw_line(x+i+7, y-11, x+i+7, y+1, line_color);
	}
}*/

/*
void draw_dead(int x, int y , short int square_color,short int eye_color){
	// x and y are center of box
	// dimensions are 20x20 
	for (int i = 0; i < 10; i ++) {
		draw_line(x-10,y+i,x+10,y+i,square_color);
		draw_line(x-10,y-i,x+10,y-i,square_color);
	}
	
	// face 
	
	draw_line(x+3,y+2,x+8,y-1,eye_color);
	draw_line(x+2,y+2,x+7,y-1,eye_color);
	draw_line(x+3,y-1,x+8,y+2,eye_color);
	draw_line(x+2,y-1,x+7,y+2,eye_color);
	draw_line(x-3,y+2,x-8,y-1,eye_color);
	draw_line(x-2,y+2,x-7,y-1,eye_color);
	draw_line(x-3,y-1,x-8,y+2,eye_color);
	draw_line(x-2,y-1,x-7,y+2,eye_color);
	
	
	draw_line(x-2,y+4,x+2,y+4,eye_color);
	
}
*/



