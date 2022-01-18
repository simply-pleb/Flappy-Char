#include <stdio.h>
#include <time.h>
#include <string.h>

#include <math.h>
#include <stdlib.h>

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

#define SCREEN_WIDTH_DISPLAY 32
#define SCREEN_HEIGHT_DISPLAY 32
#define SCREEN_WIDTH 32
#define SCREEN_HEIGHT 32
#define DISPLAY_LAYERS 3
// THE GAME IS 32x32, but the width is stretched by the factor of 2.

const int JUMP_DRAG = 20;
const int GRAVITATIONAL_PULL = 2;
const int SLEEP_TIME = 100;
const int OBSTACLE_OPENNING = 4;

void sleepFor(unsigned int mSecond)
{
#ifdef WIN32
    Sleep(mSecond);
#elif _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = mSecond / 1000;
    ts.tv_nsec = (mSecond % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (mSecond >= 1000)
    sleep(mSecond / 1000);
    usleep((mSecond % 1000) * 1000);
#endif
}

struct Position{
    float xCoor;
    float yCoor;
};

struct Player{
    struct Position pos;
    float upwardsDrag;
    float downwardDrag;
};

struct Obstacle{
    struct Position pos;

    int width;
    int topPartHeight;
    int bottomPartHeight;
};

static char display[SCREEN_HEIGHT_DISPLAY][SCREEN_WIDTH_DISPLAY][DISPLAY_LAYERS];
static int score = 1;

void drawDisplay()
{
    printf("\x1b[H");
    // printf("\x1b[2J");

    for(int i=1; i<=SCREEN_HEIGHT_DISPLAY; i++){
        for(int j=1; j<=SCREEN_WIDTH_DISPLAY; j++){
            char c;
            if (display[i][j][2] != '.'){
                c = display[i][j][2];
            }
            else if (display[i][j][1]  != '.'){
                c = display[i][j][1];
            }
            else{
                c = display[i][j][0];
            }

            printf("%c", c);
        }
        printf("\n");
    }
    sleepFor(SLEEP_TIME);

    memset(display, '.', sizeof(display));
}

void addPlayerToDisplay(struct Player player){
    int xPos = player.pos.xCoor;
    int yPos = player.pos.yCoor;

    if(xPos<0 || yPos<0){
        return;
    }

    if(xPos>SCREEN_WIDTH || yPos>SCREEN_HEIGHT){
        return;
    }

    display[yPos][xPos][1] = '@';
}

void addObstacleToDisplay(struct Obstacle obstacles[]){
    for(int i=0; i<3; i++){
        int x = obstacles[i].pos.xCoor;
        if(x >= 0 && x < SCREEN_WIDTH){
            int y = obstacles[i].pos.yCoor;
            for(int j=0; j<y-OBSTACLE_OPENNING/2; j++){
                display[j][x][0] = '#';
            }
            for(int j=y-OBSTACLE_OPENNING/2; j<y+OBSTACLE_OPENNING/2; j++){
                display[j][x][0] = '.';
            }
            for(int j=y+OBSTACLE_OPENNING/2; j<SCREEN_HEIGHT_DISPLAY; j++){
                display[j][x][0] = '#';
            }
        }
    }
    // xPos += SCREEN_HEIGHT_DISPLAY;
    // yPos += SCREEN_WIDTH_DISPLAY;
}

void addScoreToDisplay(){
    display[2][2][2] = 'S';
    display[2][3][2] = 'C';
    display[2][4][2] = 'O';
    display[2][5][2] = 'R';
    display[2][6][2] = 'E';
    display[2][7][2] = ' ';
    display[2][8][2] = '=';
    display[2][9][2] = ' ';
    
    int x = score;
    unsigned int idx = 10+x/10;
    while(x){
        display[2][idx][2] = x%10+'0';
        x/=10;
        idx--;
    }
}

void applyPhysicsOnPlayer(struct Player *player){
    player->pos.yCoor -= 0.1f * player->upwardsDrag;
    player->pos.yCoor += 0.05f * player->downwardDrag;
    
    player->upwardsDrag /= 1.5;
    player->downwardDrag *= 1.2;
}


void makeJump(struct Player *player)
{
    player->upwardsDrag = JUMP_DRAG;
    player->downwardDrag = GRAVITATIONAL_PULL;
}

char flagMake_jump;

void* getPlayerInput(void *arg)
{
    char c;
    flagMake_jump = 0;
    while(1){
        c = getchar();
        if(c == '\n' && !flagMake_jump){
            flagMake_jump = 1;
        }
    }
}


struct Player getPlayer(){
    struct Player player;

    player.pos.xCoor = SCREEN_WIDTH/2;
    player.pos.yCoor = SCREEN_HEIGHT/2;
    player.downwardDrag = GRAVITATIONAL_PULL;

    return player;
};

struct Obstacle getObstacle(int displacement){
    struct Obstacle obstacle;
    
    time_t t;
    srand((unsigned) time(&t));

    obstacle.pos.xCoor = SCREEN_WIDTH/2 + displacement + rand()%(SCREEN_WIDTH/2);

    obstacle.pos.yCoor = rand()%(SCREEN_HEIGHT/2-OBSTACLE_OPENNING);
    obstacle.pos.yCoor += rand()%(SCREEN_HEIGHT/2-OBSTACLE_OPENNING);

    return obstacle;
}

void updateObstacles(struct Obstacle* obstacles){
    for(int i=0; i<3; i++){
        // printf("%f ", obstacles[i].pos.xCoor);
        if(obstacles[i].pos.xCoor < 0){
            obstacles[i] = getObstacle(obstacles[(i+2)%3].pos.xCoor);
        }
    }
    for(int i=0; i<3; i++){
        obstacles[i].pos.xCoor -= 1;
    }
}

char checkCollision(struct Player player, struct Obstacle obstacles[])
{
    for(int i=0; i<3; i++){
        if(player.pos.xCoor == obstacles[i].pos.xCoor){
            if(player.pos.yCoor <= obstacles[i].pos.yCoor-OBSTACLE_OPENNING/2){
                return 1;
            }
            else if(player.pos.yCoor >= obstacles[i].pos.yCoor+OBSTACLE_OPENNING/2){
                return 1;
            }
            else{
                score++;
            }
        }
    }

    if(player.pos.yCoor <= 0.0 || player.pos.yCoor >= SCREEN_HEIGHT){
        return 1;
    }

    return 0;
}

int main()
{
    score = 0;
    struct Player player = getPlayer();
    struct Obstacle obstacles[3];

    obstacles[0] = getObstacle(player.pos.xCoor);
    obstacles[1] = getObstacle(obstacles[0].pos.xCoor);
    obstacles[2] = getObstacle(obstacles[1].pos.xCoor);

    time_t t;
    srand((unsigned) time(&t));

    printf("\x1b[2J");
    printf("Hello! This is a very simple Flappy Bird clone.\nPress \"enter\" to jump.\n");

    pthread_t threadInput;
    pthread_create(&threadInput, NULL, getPlayerInput, NULL);

    player.pos.yCoor = SCREEN_HEIGHT/2;
    player.upwardsDrag = 0;

    while(1){
        if(flagMake_jump){
            makeJump(&player);
            flagMake_jump = 0;
        }
        applyPhysicsOnPlayer(&player);

        updateObstacles(obstacles);
        
        addPlayerToDisplay(player);
        addObstacleToDisplay(obstacles);
        addScoreToDisplay();
        
        drawDisplay();
        // printf("%f %f\n", player.pos.xCoor, player.pos.yCoor);

        if(checkCollision(player, obstacles)){
            printf("YOU LOST!\n");
            break;
        }
    }
}

// TODO:
// - Fix segmentation faults