#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define BUFSIZE 50
#define PU_RATE 0.02

enum {
    TILE_NORMAL = 0,
    TILE_PLAYER,
    TILE_WALL,
    TILE_END
};

enum {
    RETURN_VALID = 0,
    RETURN_RESHOW,
    RETURN_WIN,
    RETURN_LOSE,
    RETURN_ERROR
};

enum {
    MOVE_UP = 'w',
    MOVE_LEFT = 'a',
    MOVE_DOWN = 's',
    MOVE_RIGHT = 'd'
};

enum {
    PU_RESHOW = 0,
    PU_EXTRAPOINTS,
    PU_CTR
};

typedef struct {
    char type;
    char visited;
    char in_path;
    char is_powerup;
    char powerup;
} tile_t;

typedef struct {
    int side;
    tile_t** grid;
} map_t;

typedef struct {
    int x;
    int y;
    int points;
} player_t;

void printmap(map_t *map) {
    for (int i = 0; i < map->side; i++) {
        for (int j = 0; j < map->side; j++) {
            tile_t *tile = &map->grid[j][i];
            switch((tile->type)) {
                case TILE_NORMAL:
                    if (tile->visited){
                        printf("o");
                    }
                    else if (tile->is_powerup) {
                        switch(tile->powerup) {
                            case PU_RESHOW:
                                printf("S");
                                break;
                            case PU_EXTRAPOINTS:
                                printf("P");
                                break;
                        }
                    }
                    else{
                        printf(".");
                    }
                    break;
                case TILE_WALL:
                    if (tile->visited){
                        printf("X");
                    }
                    else{
                        printf("#");
                    }
                    break;
                case TILE_PLAYER:
                    printf("@");
                    break;
                case TILE_END:
                    printf("*");
                    break;
            }
            printf(" ");
        }
        printf("\n");
    }
}

void find_path(map_t *map){
    int x = 1;
    int y = 1;

    tile_t *current = &map->grid[y][x];

    int move_picker[] = {
        MOVE_RIGHT,
        MOVE_RIGHT,
        MOVE_RIGHT,
        MOVE_RIGHT,
        MOVE_DOWN,
        MOVE_DOWN,
        MOVE_DOWN,
        MOVE_DOWN,
        MOVE_LEFT,
        MOVE_UP
    };

    int move = -1;
    int valid;

    while (x < (map->side - 2) || (y < (map->side - 2))) {

        int valid = 0;
        while (!valid){
            move = move_picker[rand() % 10];

            switch (move) {
                case (MOVE_RIGHT):
                    if (y < (map->side - 2)){
                        y++;
                        valid = 1;
                    }
                    break;
                case (MOVE_DOWN):
                    if (x < (map->side - 2)){
                        x++;
                        valid = 1;
                    }
                    break;
                case (MOVE_LEFT):
                    if (y > 2){
                        y--;
                        valid = 1;
                    }
                    break;
                case (MOVE_UP):
                    if (x > 2){
                        x--;
                        valid = 1;
                    }
                    break;
            }

            current = &map->grid[y][x];
            current->in_path = 1;
        }
    }
}

void fillmap(map_t *map, int density){

    find_path(map);

    for (int i = 0; i < map->side; i++) {
        for (int j = 0; j < map->side; j++) {
            tile_t *tile = &map->grid[j][i];
            tile->visited = 0;
            tile->is_powerup = 0;
            tile->powerup = 0;
            if ((!tile->in_path)
                && (i == 0
                    || i == map->side - 1
                    || j == 0
                    || j == map->side - 1
                    || rand() % 100 <= density)) {
                tile->type = TILE_WALL;
            }
            else {
                tile->type = TILE_NORMAL;
                if (rand() % (map->side * map->side) < (int)(map->side * map->side) * PU_RATE){
                    tile->is_powerup = 1;
                    tile->powerup = rand() % PU_CTR;
                }
            }
        }
    }
}

void placeplayer(map_t *map, player_t *player){
    tile_t *spawn = &map->grid[1][1];
    spawn->type = TILE_PLAYER;
    spawn->visited = 1;
    player->x = 1;
    player->y = 1;
    return;
}

void placeend(map_t *map){
    map->grid[map->side - 2][map->side - 2].type = TILE_END;
}

map_t *createmap(int side, int density){
    map_t *map = malloc(sizeof(map_t));
    map->side = side + 2;
    map->grid = malloc(sizeof(tile_t*) * map->side);
    for (int i = 0; i < map->side; i++) {
        map->grid[i] = calloc(map->side, sizeof(tile_t));
    }
    fillmap(map, density);
    return map;
}

int clearmap(map_t *map){
    for (int i = 0; i < map->side; i++) {
        free(map->grid[i]);
    }
    free(map->grid);
    free(map);
    return 0;
}

int moveplayer(map_t *map, char direction, player_t *player){
    int new_x = player->x;
    int new_y = player->y;

    switch(direction){
        case MOVE_UP:
            new_x--;
            break;
        case MOVE_DOWN:
            new_x++;
            break;
        case MOVE_LEFT:
            new_y--;
            break;
        case MOVE_RIGHT:
            new_y++;
            break;
        default:
            return RETURN_ERROR;
    }

    tile_t *next_tile = &map->grid[new_y][new_x];
    switch(next_tile->type) {
        case TILE_NORMAL:
            map->grid[player->y][player->x].type = TILE_NORMAL;
            map->grid[new_y][new_x].type = TILE_PLAYER;
            player->x = new_x;
            player->y = new_y;

            if (!next_tile->visited) {
                player->points += 1;
                next_tile->visited = 1;

                if (next_tile->is_powerup) {
                    switch (next_tile->powerup) {
                        case PU_RESHOW:
                            return RETURN_RESHOW;
                        case PU_EXTRAPOINTS:
                            player->points += 10;
                    }
                }
            }

            return RETURN_VALID;

        case TILE_WALL:
            next_tile->visited = 1;
            return RETURN_LOSE;

        case TILE_END:
            map->grid[player->y][player->x].type = TILE_NORMAL;
            map->grid[new_y][new_x].type = TILE_PLAYER;
            player->x = new_x;
            player->y = new_y;
            return RETURN_WIN;

        default:
            return RETURN_ERROR;
    }
}

void clrscr(){
    system("@cls||clear");
}

void exitgame(map_t *map, player_t *player, char *buffer, int world, int stage){
    printmap(map);
    printf("You got to World %d, Stage %d\n", world, stage);
    printf("Points: %d\n", player->points);
    clearmap(map);
    free(player);
    free(buffer);
    exit(EXIT_SUCCESS);
}

void give_input(char *buffer) {

    // Clear the user input buffer
    tcflush(0, TCIOFLUSH);

    int invalid_input = 1;
    char direction = 0;

    while(invalid_input){

        invalid_input = 0;
        printf("Your moves: ");

        memset(buffer, 0, BUFSIZE);
        scanf("%50[^\n]", buffer);
        while(getchar() != '\n'); // Clear the newline character

        for (int i = 0; buffer[i] != '\0'; i++) {
            direction = buffer[i];

            // Check if input is valid before processing
            if (direction != MOVE_UP && direction != MOVE_DOWN &&
                direction != MOVE_LEFT && direction != MOVE_RIGHT) {
                printf("Invalid input '%c': Use 'wasd' only\n", direction);
                invalid_input = 1;
                break;
            }
            if (invalid_input) {
                tcflush(0, TCIOFLUSH);
            }
        }
    }
}

void enter_shop(){

}

int main(void){

    srand(time(NULL));

    player_t *player = malloc(sizeof(player_t));
    player->points = 0;

    int result = RETURN_VALID;
    char *buffer = malloc(sizeof(char) * BUFSIZE);

    int mapsize = 5;
    int density = 10;
    int stage_nmb = 1;
    int world_nmb = 1;
    map_t *map;

    while(1){

        map = createmap(mapsize, density);
        placeplayer(map, player);
        placeend(map);
        printf("World %d, Stage %d\n", world_nmb, stage_nmb);
        printmap(map);
        sleep(3);
        clrscr();

        give_input(buffer);

        int next_round = 0;
        char direction;
        for (int i = 0; buffer[i] != '\0'; i++){
            direction = buffer[i];
            result = moveplayer(map, direction, player);
            switch(result){

                case RETURN_VALID:
                    break;

                case RETURN_RESHOW:
                    printmap(map);
                    sleep(3);
                    clrscr();
                    give_input(buffer);
                    i = -1; // Reset buffer loop
                    break;

                case RETURN_LOSE:
                    printf("You lost: You hit a wall\n");
                    exitgame(map, player, buffer, world_nmb, stage_nmb);
                    break;

                case RETURN_WIN:
                    printf("Congratulations: You won World %d, Stage %d\n", world_nmb, stage_nmb);
                    printf("Points: %d\n", player->points);
                    printmap(map);

                    printf("Press Enter to proceed to next level\n");
                    getchar();
                    tcflush(0, TCIOFLUSH);

                    clrscr();
                    clearmap(map);
                    next_round = 1;
                    i = strlen(buffer); // exit for-loop
                    break;

                case RETURN_ERROR:
                    printf("Wrong input to move player\n");
                    exitgame(map, player, buffer, world_nmb, stage_nmb);
            }
        }
        if (!next_round){
            printf("You lost: You did not reach the goal\n");
            exitgame(map, player, buffer, world_nmb, stage_nmb);
        }
        next_round = 0;
        if (stage_nmb < 5){
            density += 5;
            stage_nmb++;
        }
        else{
            stage_nmb = 1;
            density = 10;
            mapsize++;
            world_nmb++;
        }
    }
}
