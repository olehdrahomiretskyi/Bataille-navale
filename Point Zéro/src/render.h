#ifndef RENDER_H
#define RENDER_H
#include "common.h"

void SetSkin        (GridSkin s);
void DrawText       (SDL_Renderer* ren, TTF_Font* font, const char* text,
                     int x, int y, SDL_Color col);
void DrawButton     (SDL_Renderer* ren, TTF_Font* font, Button* b);
void UpdateParticles(SDL_Renderer* ren);
void DrawSplash     (SDL_Renderer* ren, TTF_Font* font, Uint32 ticks);
void DrawMainMenu   (SDL_Renderer* ren, TTF_Font* font,
                     Button* play, Button* shop, Button* settings,
                     Button* records, Button* quit, int coins);
void DrawSettings   (SDL_Renderer* ren, TTF_Font* font,
                     Button* diff, Button* skin, Button* back);
void DrawShop       (SDL_Renderer* ren, TTF_Font* font,
                     GameStats* stats, GridSkin activeSkin,
                     Button* btnBuy[NUM_SKINS], Button* btnSelect[NUM_SKINS],
                     Button* back);
void DrawBoard      (SDL_Renderer* ren, TTF_Font* font, GameBoard* b,
                     int ox, int oy, bool hide, bool dimmed);
void DrawPlacementPreview(SDL_Renderer* ren, GameBoard* b, int ox, int oy,
                          int hR, int hC, int size, bool isV);
void DrawPlacementUI(SDL_Renderer* ren, TTF_Font* font, int placed,
                     Button* bRand, Button* bReset, Button* bPret, Button* bBack);
void DrawGameUI     (SDL_Renderer* ren, TTF_Font* font, GameBoard* p, GameBoard* e,
                     bool playerTurn, int moves, Uint32 startTick, int coins);
void DrawGameOver   (SDL_Renderer* ren, TTF_Font* font, bool playerWon,
                     int moves, int coinsEarned, Button* btnR, Button* btnM);
void DrawRecords    (SDL_Renderer* ren, TTF_Font* font, GameStats* s, Button* btnBack);

#endif
