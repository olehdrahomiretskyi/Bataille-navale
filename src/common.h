#ifndef COMMON_H
#define COMMON_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define GRID_SIZE     10
#define CELL_SIZE     50
#define P_OFFSET_X    60
#define E_OFFSET_X    670
#define OFFSET_Y      150
#define WINDOW_W      1280
#define WINDOW_H      800
#define MAX_PARTICLES 90
#define NUM_SKINS     8

typedef enum {
    MODE_SPLASH=0, MODE_MAINMENU, MODE_SETTINGS,
    MODE_SHOP, MODE_PLACEMENT, MODE_GAME,
    MODE_GAMEOVER, MODE_RECORDS
} GameMode;

typedef enum {
    CELL_EMPTY, CELL_SHIP, CELL_MISS,
    CELL_HIT,   CELL_SUNK, CELL_NEAR_SUNK  /* safe zone around sunk ship */
} CellState;

typedef enum { DIFF_FACILE=0, DIFF_NORMAL=1, DIFF_EXPERT=2 } AIDifficulty;
typedef enum {
    SKIN_OCEAN=0, SKIN_NUIT=1, SKIN_ARCTIQUE=2,
    SKIN_ROUGE=3, SKIN_OR=4,
    SKIN_FORET=5, SKIN_PLASMA=6, SKIN_GLACE=7
} GridSkin;

typedef struct {
    CellState cells[GRID_SIZE][GRID_SIZE];
    int       shipsLeft;
} GameBoard;

typedef struct {
    SDL_Rect  rect;
    SDL_Color color;
    bool      isHovered;
    char      label[80];
} Button;

typedef struct {
    int  stack[100][2];
    int  size;
    int  remain[10];
    int  remainCount;
} CPUState;

typedef struct {
    float x, y, vx, vy, alpha;
    int   r, g, b;
} Particle;

typedef struct {
    int  wins, losses, bestMoves;
    int  coins;
    bool unlocked[NUM_SKINS];   /* true = owned */
} GameStats;

#endif
