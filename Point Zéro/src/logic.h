#ifndef LOGIC_H
#define LOGIC_H
#include "common.h"

void ClearBoard     (GameBoard* b);
bool CanPlaceShip   (GameBoard* b, int r, int c, int size, bool vertical);
void PlaceShip      (GameBoard* b, int r, int c, int size, bool vertical);
void RandomPlaceFull(GameBoard* b);
bool IsPointInRect  (int x, int y, SDL_Rect rect);
void InitCPUState   (CPUState* cpu);
/* Returns size of newly-sunk ship (0 = none).
   Also marks CELL_NEAR_SUNK on adjacent empty cells. */
int  CheckSunkShips (GameBoard* b);
void ProcessCPUTurn (GameBoard* player, bool* playerTurn,
                     CPUState* cpu, AIDifficulty diff);
void LoadStats      (GameStats* s);
void SaveStats      (GameStats* s);

#endif
