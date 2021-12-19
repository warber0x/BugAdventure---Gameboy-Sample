/***
 * Author: Radouane SAMIR
 * Tested on: Gameboy classic & color
 * Date: 12/12/2021
 * GDBK v2020
 ***/


#include <gb/gb.h>
#include <gbdk/font.h>
#include <rand.h>

#include "bgtiles.c"
#include "bgmap.c"
#include "bugSprite.c"
#include "food.c"
#include "numbers.c"
#include "numbers_letters.c"
#include "stagecomplete_map.c"
#include "stagecomplete_data.c"
#include "bug_adventure_data.c"
#include "bug_adventure_map.c"

#define MAX_DIGIT_COORDINATE 3

#define UP              (UINT8)0x20
#define LEFT            (UINT8)0x21
#define RIGHT           (UINT8)0x22
#define DOWN            (UINT8)0x23

#define PIXEL_SPEED     8
#define GAME_SPEED      4
#define TAIL_SPEED      10

#define WIDTH           160
#define HEIGHT          144
#define W_BG_BLOCK      8
#define H_BG_BLOCK      8
#define RAND_MAX        32767

#define TRUE            1
#define FALSE           0
#define FOODMAPLEN      2

typedef struct Object 
{
    UINT16 x;
    UINT16 y;
    UINT8 width;
    UINT8 height;
    UINT8 food_to_eat;
} Object;

typedef struct GameCharacter 
{
    UBYTE spriteids[2];
    UINT16 x;
    UINT16 y;
    UINT8 width;
    UINT8 height;
    UINT8 snake_face_pos;
    UINT8 food_eaten;
} GameCharacter;

typedef struct TailObject 
{
    UINT16 x;
    UINT16 y;
    UINT8 width;
    UINT8 height;
    UINT8 mv_counter1;
    UINT8 mv_counter2;
    UINT8 direction;
} TailObject;

void performDelay(UINT8 numloops)
{
    UINT8 i;
    for (i = 0; i < numloops; i++)
    {
        wait_vbl_done();
    }
}

/** Transform a number to char and put it in array **/

UINT8 nbrToArray(UINT8 nbr, UINT8* array)
{
    UINT8 temp = 0;
    UINT8 counter = 0, counter_return = 0;
    UINT8 n,x,i = 0;

    n = nbr;
    while(n!=0)  
    {  
        n=n/10;  
        counter++;  
    }

    counter_return = counter;
    temp = nbr;
    while (counter > 0)
    {
        x = temp % 10;
        array[counter-1] = x;
        temp /= 10; 
        counter--;
    }

    return counter_return;
}

/** This is used to change number of food eaten during the game **/

void intToTilePosition(Object *food)
{
    set_sprite_tile(4, tile_position[food->food_to_eat]);
    set_sprite_tile(5, tile_position[EMPTY_TILE]); //delete second number
}

/** basic tile based collision detection when the character touches the food**/

UINT8 collisionCheck(UINT8 x1, UINT8 y1, UINT8 w1, UINT8 h1, UINT8 x2, UINT8 y2, UINT8 w2, UINT8 h2) 
{
	if ((x1 < (x2+w2)) && ((x1+w1) > x2) && (y1 < (h2+y2)) && ((y1+h1) > y2))
		return 1;

    return 0;
}

/** Background collision detection **/

UBYTE canItMove(UINT8 x1, UINT8 y1)
{
    unsigned int xpos, ypos;
    unsigned int position = 0;

    xpos = (x1-8)/8;  // xpos--;
    ypos = (y1-16)/8; // ypos--;

    position = 20 * ypos;
    position = position + xpos;

    if (backgroundmap[position] == 0x2E || backgroundmap[position] == 0x2D) // ||
    return 0;

    return 1;
}

/** Code pasted from gaming Monsters but not used here **/
void animateSprite(UINT8 spriteIdx, INT8 x, INT8 y)
{
    while (x != 0)
    {
        scroll_sprite(spriteIdx, x < 0 ? -1 : 1, 0);
        x += (x < 0 ? 1: -1);
        performDelay(2);
    }
    while (y != 0)
    {
        scroll_sprite(spriteIdx, 0, y < 0 ? -1 : 1);
        y += (y < 0 ? 1: -1);
        performDelay(2);
    }
}

/** Function created to make the tail move but it consumes too much cpu **/

void toggleTail(TailObject *tail, UINT8 tilenbr)
{
    set_sprite_tile(2, tilenbr);
    if (tail->direction == RIGHT)
    move_sprite(2,  tail->x,  tail->y);   

    if (tail->direction == LEFT)
    move_sprite(2,  tail->x+16,  tail->y);   

    if (tail->direction == UP)
    move_sprite(2,  tail->x+8,  tail->y+8);   

    if (tail->direction == DOWN)
    move_sprite(2,  tail->x+8,  tail->y-8);   

}

/** function not used here but used if you want to make the tail move **/

void moveTail(TailObject *tail)
{
    if (tail->direction == RIGHT)
    toggleTail(tail, 0x29);
    else if (tail->direction == LEFT)
    toggleTail(tail, 0x27);
    else if (tail->direction == UP)
    toggleTail(tail, 0x25);
    else if (tail->direction == DOWN)
    toggleTail(tail, 0x2B);

    /*if (tail->mv_counter1 < TAIL_SPEED) 
    {
        if (tail->direction == RIGHT)
        toggleTail(tail, 0x29);
        else if (tail->direction == LEFT)
        toggleTail(tail, 0x27);
        else if (tail->direction == UP)
        toggleTail(tail, 0x25);
        else if (tail->direction == DOWN)
        toggleTail(tail, 0x2B);
        tail->mv_counter1++;
    }

    if  (tail->mv_counter2  < TAIL_SPEED && tail->mv_counter1  == TAIL_SPEED)
    {
        if (tail->direction == RIGHT)
        toggleTail(tail, 0x2A);
        else if (tail->direction == LEFT)
        toggleTail(tail, 0x28);
        else if (tail->direction == UP)
        toggleTail(tail, 0x26);
        else if (tail->direction == DOWN)
        toggleTail(tail, 0x2C);
        tail->mv_counter2++;
    } 
    else if (tail->mv_counter1  == TAIL_SPEED && tail->mv_counter2 == TAIL_SPEED)
    {
        tail->mv_counter1  = 0;
        tail->mv_counter2 = 0;
    } */

    
}

/** Sound registers - dont change the order **/

void initSound()
{
     // these registers must be in this specific order!
    NR52_REG = 0x80; // is 1000 0000 in binary and turns on sound
    NR50_REG = 0x11; // sets the volume for both left and right channel just set to max 0x77
    NR51_REG = 0x77; // is 1111 1111 in binary,
}

/** make some noise **/

void playSound()
{
    NR10_REG = 0x00;
    NR11_REG = 0x81;
    NR12_REG = 0x43;
    NR13_REG = 0x73;
    NR14_REG = 0x86;	    
}

/** the most important function in the universe - lol ;) **/

void byTheRed1()
{
    // TheRed1
    set_sprite_tile(7,  tile_position_letters[CHAR_T]);
    set_sprite_tile(8,  tile_position_letters[CHAR_H]);
    set_sprite_tile(9,  tile_position_letters[CHAR_E]);
    set_sprite_tile(10, tile_position_letters[CHAR_O]);
    set_sprite_tile(11, tile_position_letters[CHAR_N]);
    set_sprite_tile(12, tile_position_letters[CHAR_E]);

    move_sprite(7,  112, 152);
    move_sprite(8,  120, 152);
    move_sprite(9,  128, 152);
    move_sprite(10, 136, 152);
    move_sprite(11, 144, 152);
    move_sprite(12, 152, 152);
}

/** initiate the Numbers and letters and put them in the VRAM **/

void initNumbersAndLetters()
{
    /** Numbers **/
    set_sprite_data(0x2D, 38, numbers_and_letters);
    set_sprite_tile(4, tile_position[NUMBER_ONE]);
    set_sprite_tile(5, tile_position[NUMBER_ZERO]);
    set_sprite_tile(6, tile_position[DASH]);

    move_sprite(6, 25, 152);
    move_sprite(4, 33, 152);
    move_sprite(5, 40, 152);
}

void initSnake(GameCharacter *character)
{
    /** Snake sprite **/
    character->x = 32;
    character->y = 40;

    character->width  = 8;
    character->height = 8;
    character->snake_face_pos = RIGHT;

    set_bkg_data(38, 9, backgroundTiles);
    set_bkg_tiles(0,0,20,18, backgroundmap);

    set_sprite_data(0x20, 4, snakeSprite);
    set_sprite_tile(0, RIGHT);  //default position is RIGHT

    move_sprite(0,  character->x,  character->y);
}

void initSnakeTail(TailObject *snakeTails)
{
    /** Tail **/
    snakeTails->x = 32-8; 
    snakeTails->y = 40;
 
    snakeTails->width  = 8;
    snakeTails->height = 8;

    set_sprite_data(0x25, 8, snakeTail);
    set_sprite_tile(2, 0x29);
    snakeTails->direction = RIGHT;
    move_sprite(2, snakeTails->x  , snakeTails->y);
}

void initStartScreen()
{
    initNumbersAndLetters();

    /** Start screen **/
    set_bkg_data(0, 197, bug_adventure_data);
    set_bkg_tiles(0, 0, 20, 18, bug_adventure_map);

    SHOW_BKG;
    waitpad(J_START);
}

void initFood(Object *food)
{
    /** Food **/
    food->width  = 8;
    food->height = 8;
    food->x = 64;
    food->y = 64;
    food->food_to_eat = 10; //Food counter

    set_sprite_data(0x24, 1, foodToEat);
    set_sprite_tile(1, 0x24);
    move_sprite(1,  (UINT8)food->x, (UINT8)food->y);

    /** food to eat in the bottom **/
    set_sprite_tile(3, 0x24);   
    move_sprite(3, 16, 152) ;
}

void init(GameCharacter *snake, Object *food, TailObject *snakeTails)
{
    initStartScreen();
    initSound();
    byTheRed1();
    initSnake(snake);
    initSnakeTail(snakeTails);
    initFood(food);
    
    
    /** Gameboy Interrupts **/
    SHOW_SPRITES;
    DISPLAY_ON;
}

void main(void)
{
    Object food ;
    GameCharacter snake;
    TailObject snakeTails;

    init(&snake, &food, &snakeTails);

    while (1)
    {
        switch(joypad())
        {
            case J_LEFT:
                if (snake.snake_face_pos != RIGHT && snake.x > W_BG_BLOCK * 2)
                {
                    if (canItMove(snake.x-8, snake.y) == 1){
                        snake.snake_face_pos = LEFT;
                        snakeTails.direction = LEFT;
                        set_sprite_tile(0, LEFT);

                        snake.x -= PIXEL_SPEED;
                        snakeTails.x -= PIXEL_SPEED;

                        move_sprite(0,  snake.x,  snake.y); 
                        //animateSprite(0, -8, 0);
                        moveTail(&snakeTails);
                    }
                }
                break;

            case J_RIGHT:
                if (snake.snake_face_pos != LEFT && snake.x < WIDTH - (W_BG_BLOCK))
                {
                    if (canItMove(snake.x+8, snake.y) == 1){
                        snake.snake_face_pos = RIGHT;
                        snakeTails.direction = RIGHT;
                        set_sprite_tile(0, RIGHT);

                        snake.x += PIXEL_SPEED;
                        snakeTails.x += PIXEL_SPEED;

                        move_sprite(0,  snake.x,  snake.y); 
                        //animateSprite(0, 8, 0);
                        moveTail(&snakeTails);
                    }
                }
                break;

            case J_UP:
                if (snake.snake_face_pos != DOWN  && snake.y > H_BG_BLOCK * 3)
                {
                    if (canItMove(snake.x, snake.y-8) == 1){
                        snake.snake_face_pos = UP;
                        snakeTails.direction = UP;
                        set_sprite_tile(0, UP);

                        snake.y -= PIXEL_SPEED;
                        snakeTails.y -= PIXEL_SPEED;

                        move_sprite(0,  snake.x,  snake.y);
                        //animateSprite(0, 0, -8);
                        moveTail(&snakeTails);
                    }
                }
                break;

            case J_DOWN:
                if (snake.snake_face_pos != UP && snake.y < HEIGHT - H_BG_BLOCK)
                {
                    if (canItMove(snake.x, snake.y+8) == 1){
                        snake.snake_face_pos = DOWN;
                        snakeTails.direction = DOWN;
                        set_sprite_tile(0, DOWN);

                        snake.y += PIXEL_SPEED;
                        snakeTails.y += PIXEL_SPEED;

                        move_sprite(0,  snake.x,  snake.y);
                        //animateSprite(0, 0, 8);
                        moveTail(&snakeTails);
                    }
                }
                break;
        }

        /*if  ((snake.snake_face_pos == UP && snake.y < H_BG_BLOCK+2) ||  (snake.snake_face_pos == DOWN && snake.y > (HEIGHT - H_BG_BLOCK*4)) ||
            (snake.snake_face_pos == LEFT && snake.x < W_BG_BLOCK*2) || (snake.snake_face_pos == RIGHT && snake.x >= WIDTH - W_BG_BLOCK))*/
        if (food.food_to_eat == 0)
        {
            HIDE_SPRITES;
            HIDE_BKG;
            
            /** Filling the food **/
            food.food_to_eat = 10;

            /** Setting everything to default **/
            set_bkg_data(0, 240, stagecomplete_data);
            set_bkg_tiles(0, 0, 20, 18, stagecomplete_map);

            SHOW_BKG;

            waitpad(J_START);

            //====================================//
            set_bkg_data(38, 9, backgroundTiles);
            set_bkg_tiles(0,0,20,18, backgroundmap);

            snake.snake_face_pos = RIGHT;
            set_sprite_tile(0, RIGHT);

            snake.x = 32;
            snake.y = 40;

            move_sprite(0,  snake.x,  snake.y);
            
            snakeTails.x = 32-8; //25 in lieu of 24 to make the tile next to snake
            snakeTails.y = 40;

            set_sprite_data(0x25, 8, snakeTail);
            set_sprite_tile(2, 0x29);
            snakeTails.direction = RIGHT;
            move_sprite(2, snakeTails.x  , snakeTails.y);

            food.food_to_eat = 10;
            snake.food_eaten = 0;

            initNumbersAndLetters();
            
            SHOW_SPRITES;
            continue;
        }
        /*else // To move automatically
        {
            if  (snake.snake_face_pos == UP && snake.y > H_BG_BLOCK * 3)  && (fakemap[snake.x-1][(snake.y-1)/8] != 0x08)) 
            {
                if (canItMove(snake.x, snake.y-8) == 1){
                    snake.y -= PIXEL_SPEED;
                    snakeTails.y -= PIXEL_SPEED;

                    move_sprite(0,  snake.x,  snake.y);
                    moveTail(&snakeTails);
                }
                
            }
            //else if (snake.snake_face_pos == DOWN && snake.y < HEIGHT - H_BG_BLOCK) && (fakemap[snake.x-1][(snake.y-1)/8] != 0x08)) 
            {
                if (canItMove(snake.x, snake.y+8) == 1) {
                    snake.y += PIXEL_SPEED;
                    snakeTails.y += PIXEL_SPEED;

                    move_sprite(0,  snake.x,  snake.y);
                    moveTail(&snakeTails);
                }
                
            }
            //else if (snake.snake_face_pos == LEFT && snake.x > W_BG_BLOCK * 2 )  && (fakemap[(snake.x/8)-1][snake.y] != 0x08)) 
            {
                 if (canItMove(snake.x-8, snake.y) == 1){
                    snake.x -= PIXEL_SPEED;
                    snakeTails.x -= PIXEL_SPEED;

                    move_sprite(0,  snake.x,  snake.y);
                    moveTail(&snakeTails);
                } 
            }
            //else if ((snake.snake_face_pos == RIGHT && snake.x < WIDTH - (W_BG_BLOCK)))
            {
                 if (canItMove(snake.x+8, snake.y) == 1){
                    snake.x += PIXEL_SPEED;
                    snakeTails.x += PIXEL_SPEED;

                    move_sprite(0,  snake.x,  snake.y); 
                    moveTail(&snakeTails);
                } 
            }
        }*/
        
        /** random calculation to change the position of food object in the screen **/
        if (collisionCheck(snake.x, snake.y, snake.width, snake.height, food.x, food.y, food.width, food.height))
        {
            food.x = ((UINT8)rand() % ((UINT8)(152  - (W_BG_BLOCK*4))  + 1)) + (UINT8)W_BG_BLOCK*4;
            food.y = ((UINT8)rand() % ((UINT8)(104 - (H_BG_BLOCK*4))  + 1)) + (UINT8)H_BG_BLOCK*4;
            
            while (!canItMove(food.x, food.y))
            {
                food.x = ((UINT8)rand() % ((UINT8)(152  - (W_BG_BLOCK*4))  + 1)) + (UINT8)W_BG_BLOCK*4;
                food.y = ((UINT8)rand() % ((UINT8)(104 - (H_BG_BLOCK*4))  + 1)) + (UINT8)H_BG_BLOCK*4;
            }
            
            move_sprite(1,  food.x, food.y);
            snake.food_eaten++;
            food.food_to_eat--;
            playSound();
            intToTilePosition(&food);
        }
        
        performDelay(GAME_SPEED);
    }
}