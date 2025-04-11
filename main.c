#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFSIZE 50

enum {
    TILE_NORMAL = 0,
    TILE_PLAYER,
    TILE_WALL,
    TILE_END
};

enum {
    RETURN_VALID = 0,
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

typedef struct {
    char type;
    char visited;
} tile_t;

typedef struct {
    int side;
    tile_t** grid;
} map_t;

typedef struct{
    int x;
    int y;
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

void fillmap(map_t *map){
    for (int i = 0; i < map->side; i++) {
        for (int j = 0; j < map->side; j++) {
            tile_t *tile = &map->grid[j][i];
            tile->visited = 0;
            if (i == 0 || i == map->side - 1 || j == 0 || j == map->side - 1) {
                tile->type = TILE_WALL;
            }
            else{
                tile->type = TILE_NORMAL;
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

map_t *createmap(int side){
    map_t *map = malloc(sizeof(map_t));
    map->side = side + 2;
    map->grid = malloc(sizeof(tile_t*) * map->side);
    for (int i = 0; i < map->side; i++) {
        map->grid[i] = malloc(sizeof(tile_t) * map->side);
    }
    fillmap(map);
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
    }

    tile_t *next_tile = &map->grid[new_y][new_x];
    next_tile->visited = 1;
    switch(next_tile->type) {
        case TILE_NORMAL:
            map->grid[player->y][player->x].type = TILE_NORMAL;
            map->grid[new_y][new_x].type = TILE_PLAYER;
            player->x = new_x;
            player->y = new_y;
            return RETURN_VALID;
        case TILE_WALL:
            return RETURN_LOSE;
        case TILE_END:
            map->grid[player->y][player->x].type = TILE_NORMAL;
            map->grid[new_y][new_x].type = TILE_PLAYER;
            return RETURN_WIN;
        default:
            return RETURN_ERROR;
    }
}

void clrscr(){
    system("@cls||clear");
}

void exitgame(map_t *map, player_t *player, char *buffer){
    printmap(map);
    clearmap(map);
    free(player);
    free(buffer);
    exit(EXIT_SUCCESS);
}

int main(void){

    map_t *map = createmap(5);
    player_t *player = malloc(sizeof(player_t));
    placeplayer(map, player);
    placeend(map);
    printmap(map);
    sleep(3);
    clrscr();

    int result = RETURN_VALID;
    char *buffer = malloc(sizeof(char) * BUFSIZE);
    char direction = 0;

    scanf("%50[^\n]", buffer);
    for (int i = 0; buffer[i] != '\0'; i++){
        direction = buffer[i];
        result = moveplayer(map, direction, player);
        switch(result){
            case RETURN_VALID:
                break;
            case RETURN_LOSE:
                printf("You lost: You hit a wall\n");
                exitgame(map, player, buffer);
                break;
            case RETURN_WIN:
                printf("Congratulations: You won!\n");
                exitgame(map, player, buffer);
                break;
            case RETURN_ERROR:
                printf("Invalid input given: Use 'wasd'\n");
                exitgame(map, player, buffer);
        }
    }
    printf("You lost: You did not reach the goal\n");
    exitgame(map, player, buffer);
}
