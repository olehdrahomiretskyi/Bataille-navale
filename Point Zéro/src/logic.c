#include "logic.h"

static const int SCHEMA[10] = {4,3,3,2,2,2,1,1,1,1};


void ClearBoard(GameBoard* b) {
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            b->cells[r][c] = CELL_EMPTY;
    b->shipsLeft = 0;
}

bool CanPlaceShip(GameBoard* b, int r, int c, int size, bool vertical) {
    for (int i = 0; i < size; i++) {
        int cr = vertical ? r+i : r;
        int cc = vertical ? c   : c+i;
        if (cr < 0 || cr >= GRID_SIZE || cc < 0 || cc >= GRID_SIZE) 
            return false;
        for (int dr = -1; dr <= 1; dr++){
            for (int dc = -1; dc <= 1; dc++) {
                int nr = cr+dr, nc = cc+dc;
                if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE)
                    if (b->cells[nr][nc] == CELL_SHIP) 
                        return false;
            }
        }
    }
    return true;
}

void PlaceShip(GameBoard* b, int r, int c, int size, bool vertical) {
    for (int i = 0; i < size; i++) {
        b->cells[vertical ? r+i : r][vertical ? c : c+i] = CELL_SHIP;
        b->shipsLeft++;
    }
}

void RandomPlaceFull(GameBoard* b) {
    ClearBoard(b);
    for (int i = 0; i < 10; i++) {
        bool placed = false;
        while (!placed) {
            int r = rand() % GRID_SIZE;
            int c = rand() % GRID_SIZE;
            bool v = rand() % 2;
            if (CanPlaceShip(b, r, c, SCHEMA[i], v)) {
                PlaceShip(b, r, c, SCHEMA[i], v);
                placed = true;
            }
        }
    }
}

bool IsPointInRect(int x, int y, SDL_Rect rect) {
    return x >= rect.x && x <= rect.x+rect.w &&
           y >= rect.y && y <= rect.y+rect.h;
}


static bool ShipIsSunk(GameBoard* b, int r, int c) {
    int dr[] = {0,0,1,-1};
    int dc[] = {1,-1,0,0};
    for (int d = 0; d < 4; d++) {
        int nr = r+dr[d], nc = c+dc[d];
        while (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
            if (b->cells[nr][nc] == CELL_SHIP) 
                return false;
            if (b->cells[nr][nc] != CELL_HIT)  
                break;
            
            nr += dr[d]; 
            nc += dc[d];
        }
    }
    return true;
}

static int MarkSunk(GameBoard* b, int r, int c) {
    int dr[] = {0,0,1,-1};
    int dc[] = {1,-1,0,0};
    int cells[20][2]; 
    int cnt = 0;

    cells[cnt][0] = r; 
    cells[cnt][1] = c; 
    cnt++;
    
    b->cells[r][c] = CELL_SUNK;
    
    for (int d = 0; d < 4; d++) {
        int nr = r+dr[d], nc = c+dc[d];
        while (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE
               && b->cells[nr][nc] == CELL_HIT) {
            b->cells[nr][nc] = CELL_SUNK;
            cells[cnt][0] = nr; 
            cells[cnt][1] = nc; 
            
            cnt++;
            
            nr += dr[d]; 
            nc += dc[d];
        }
    }
    int ar[] = {-1,-1,-1, 0, 0, 1, 1, 1};
    int ac[] = {-1, 0, 1,-1, 1,-1, 0, 1};
    for (int i = 0; i < cnt; i++) {
        int sr = cells[i][0], sc = cells[i][1];
        for (int d = 0; d < 8; d++) {
            int nr = sr+ar[d], nc = sc+ac[d];
            if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE
                && b->cells[nr][nc] == CELL_EMPTY)
                b->cells[nr][nc] = CELL_NEAR_SUNK;
        }
    }
    return cnt;
}

int CheckSunkShips(GameBoard* b) {
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            if (b->cells[r][c] == CELL_HIT && ShipIsSunk(b, r, c))
                return MarkSunk(b, r, c);
    return 0;
}

void InitCPUState(CPUState* cpu) {
    cpu->size        = 0;
    cpu->remainCount = 10;
    for (int i = 0; i < 10; i++) 
        cpu->remain[i] = SCHEMA[i];
}

static void RemoveRemain(CPUState* cpu, int sz) {
    for (int i = 0; i < cpu->remainCount; i++) {
        if (cpu->remain[i] == sz) {
            cpu->remain[i] = cpu->remain[--cpu->remainCount];
            return;
        }
    }
}

/* A cell is "unshot" if it has never been fired at */
static bool Unshot(CellState s) {
    return s == CELL_EMPTY || s == CELL_SHIP;
}

/* Push a cell onto the target stack only if it is unshot and not a duplicate */
static void Push(CPUState* cpu, GameBoard* b, int r, int c) {
    if (r < 0 || r >= GRID_SIZE || c < 0 || c >= GRID_SIZE) 
        return;
    if (!Unshot(b->cells[r][c])) 
        return;
    
    for (int i = 0; i < cpu->size; i++){
        if (cpu->stack[i][0] == r && cpu->stack[i][1] == c) 
            return;
    }
    
    cpu->stack[cpu->size][0] = r;
    cpu->stack[cpu->size][1] = c;
    cpu->size++;
}

/* Pop the next valid (unshot) cell from the stack.
   Returns true and sets outR/outC if found, false if stack exhausted. */
static bool PopValid(CPUState* cpu, GameBoard* b, int* r, int* c) {
    while (cpu->size > 0) {
        cpu->size--;
        int tr = cpu->stack[cpu->size][0];
        int tc = cpu->stack[cpu->size][1];
        if (Unshot(b->cells[tr][tc])) {
            *r = tr; 
            *c = tc;
            return true;
        }
    }
    return false;
}

/* After a hit at (r,c): push follow-up targets.
   If we already have alignment info from adjacent CELL_HIT cells,
   extend only along that axis. Otherwise push all 4 neighbours. */
static void PushTargets(CPUState* cpu, GameBoard* b, int r, int c) {
    bool upHit    = (r > 0           && b->cells[r-1][c] == CELL_HIT);
    bool downHit  = (r < GRID_SIZE-1 && b->cells[r+1][c] == CELL_HIT);
    bool leftHit  = (c > 0           && b->cells[r][c-1] == CELL_HIT);
    bool rightHit = (c < GRID_SIZE-1 && b->cells[r][c+1] == CELL_HIT);

    if (upHit || downHit) {
        /* Vertical alignment confirmed — walk to ends of chain and push beyond */
        int top = r; 
        
        while (top > 0 && b->cells[top-1][c] == CELL_HIT) 
            top--;
        
        int bot = r;
        
        while (bot < GRID_SIZE-1 && b->cells[bot+1][c] == CELL_HIT) 
            bot++;
        
        Push(cpu, b, top-1, c);
        Push(cpu, b, bot+1, c);
    } 
    else if (leftHit || rightHit) {
        /* Horizontal alignment confirmed */
        int lft = c; 
        while (lft > 0 && b->cells[r][lft-1] == CELL_HIT) 
            lft--;
        
        int rgt = c; 
        while (rgt < GRID_SIZE-1 && b->cells[r][rgt+1] == CELL_HIT) 
            rgt++;
        
        Push(cpu, b, r, lft-1);
        Push(cpu, b, r, rgt+1);
    } 
    else {
        /* First hit on this ship — no alignment yet, push all 4 */
        Push(cpu, b, r-1, c);
        Push(cpu, b, r+1, c);
        Push(cpu, b, r,   c-1);
        Push(cpu, b, r,   c+1);
    }
}

/* Safety net: if stack is empty but HIT cells remain on board,
   rebuild stack from their unshot neighbours. */
static void RecoverFromOrphanedHits(CPUState* cpu, GameBoard* b) {
    cpu->size = 0;
    for (int r = 0; r < GRID_SIZE; r++){
        for (int c = 0; c < GRID_SIZE; c++){
            if (b->cells[r][c] == CELL_HIT) {
                Push(cpu, b, r-1, c);
                Push(cpu, b, r+1, c);
                Push(cpu, b, r,   c-1);
                Push(cpu, b, r,   c+1);
            }
        }
    }
}

/* Probability density map for EXPERT hunt phase.
   For each remaining ship, slide it over every valid position.
   Count how many times each cell is covered → higher = better target.
   A position is valid if it contains no MISS/SUNK/NEAR_SUNK cells.
   NOTE: CELL_SHIP counts as a valid (unshot) cell for the sliding window. */
static void BestHuntCell(GameBoard* b, CPUState* cpu, int* outR, int* outC) {
    int map[GRID_SIZE][GRID_SIZE];
    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            map[r][c] = 0;

    for (int si = 0; si < cpu->remainCount; si++) {
        int sz = cpu->remain[si];

        /* Horizontal */
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c <= GRID_SIZE - sz; c++) {
                bool ok = true;
                for (int k = 0; k < sz && ok; k++) {
                    CellState cs = b->cells[r][c+k];
                    /* A slot is blocked by any previously-shot miss or confirmed sunk */
                    if (cs == CELL_MISS || cs == CELL_SUNK || cs == CELL_NEAR_SUNK)
                        ok = false;
                }
                if (ok)
                    for (int k = 0; k < sz; k++)
                        if (Unshot(b->cells[r][c+k]))   /* only count unshot cells */
                            map[r][c+k]++;
            }
        }

        /* Vertical */
        for (int r = 0; r <= GRID_SIZE - sz; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                bool ok = true;
                for (int k = 0; k < sz && ok; k++) {
                    CellState cs = b->cells[r+k][c];
                    if (cs == CELL_MISS || cs == CELL_SUNK || cs == CELL_NEAR_SUNK)
                        ok = false;
                }
                if (ok)
                    for (int k = 0; k < sz; k++)
                        if (Unshot(b->cells[r+k][c]))
                            map[r+k][c]++;
            }
        }
    }

    /* Checkerboard parity when smallest remaining ship >= 2.
       Two-pass so we never get stuck: preferred parity first, fallback second. */
    int minShip = 99;
    for (int i = 0; i < cpu->remainCount; i++){
        if (cpu->remain[i] < minShip) 
            minShip = cpu->remain[i];
    }
    
    bool useParity = (minShip >= 2);

    int best0 = -1, br0 = 0, bc0 = 0;   /* even (r+c)%2==0 */
    int best1 = -1, br1 = 0, bc1 = 0;   /* odd  (r+c)%2==1 */

    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            if (!Unshot(b->cells[r][c])) 
                continue;
            int score = map[r][c];
            if (score <= 0) 
                continue;
            if (!useParity || (r+c) % 2 == 0) {
                if (score > best0) { 
                    best0 = score; 
                    br0 = r; 
                    bc0 = c; 
                }
            } 
            else {
                if (score > best1) { 
                    best1 = score; 
                    br1 = r; 
                    bc1 = c; 
                }
            }
        }
    }

    if (best0 >= 0) { 
        *outR = br0; 
        *outC = bc0; 
    }
        
    else if (best1 >= 0) { 
        *outR = br1; 
        *outC = bc1; 
    }
    else {
        /* Absolute fallback: first unshot cell (should never happen mid-game) */
        for (int r = 0; r < GRID_SIZE; r++){
            for (int c = 0; c < GRID_SIZE; c++){
                if (Unshot(b->cells[r][c])) { 
                    *outR = r; 
                    *outC = c; 
                    return; 
                }
            }
        }
    }
}

/* Pick a random unshot cell for FACILE/NORMAL hunt */
static void RandomUnshot(GameBoard* b, int* outR, int* outC) {
    int pool[GRID_SIZE*GRID_SIZE][2]; 
    int pn = 0;
    for (int r = 0; r < GRID_SIZE; r++){
        for (int c = 0; c < GRID_SIZE; c++){
            if (Unshot(b->cells[r][c])){ 
                pool[pn][0] = r; 
                pool[pn][1] = c; 
                pn++; 
            }
        }
    }

    if (pn > 0) {
        int idx = rand() % pn;
        *outR = pool[idx][0];
        *outC = pool[idx][1];
    }
}

void ProcessCPUTurn(GameBoard* player, bool* playerTurn,
                    CPUState* cpu, AIDifficulty diff) {
    int r = 0, c = 0;

    if (diff == DIFF_FACILE) {
        /* ── FACILE: pure random ── */
        RandomUnshot(player, &r, &c);

    } 
    else {
        bool found = PopValid(cpu, player, &r, &c);

        if (!found) {
            /* Check if any HIT cells remain (damaged but un-sunk ship) */
            bool hitExists = false;
            
            for (int pr = 0; pr < GRID_SIZE && !hitExists; pr++){
                for (int pc = 0; pc < GRID_SIZE && !hitExists; pc++){
                    if (player->cells[pr][pc] == CELL_HIT) 
                        hitExists = true;
                }
            }

            if (hitExists) {
                RecoverFromOrphanedHits(cpu, player);
                found = PopValid(cpu, player, &r, &c);
            }
        }

        if (!found) {
            /* Hunt mode */
            if (diff == DIFF_EXPERT)
                BestHuntCell(player, cpu, &r, &c);
            else
                RandomUnshot(player, &r, &c);
        }
    }

    
    CellState target = player->cells[r][c];

    if (target == CELL_SHIP) {
        player->cells[r][c] = CELL_HIT;
        player->shipsLeft--;

        int sunkSz = CheckSunkShips(player);
        if (sunkSz > 0) {
            RemoveRemain(cpu, sunkSz);
            cpu->size = 0;          /* ship sunk — clear stale targets */
        } 
        else if (diff != DIFF_FACILE) {
            PushTargets(cpu, player, r, c);
        }
        /* Hit: CPU keeps its turn — playerTurn stays false */

    } 
    else {
        /* CELL_EMPTY → miss */
        player->cells[r][c] = CELL_MISS;
        *playerTurn = true;         /* pass turn back to player */
    }
}


void LoadStats(GameStats* s) {
    s->wins = 0; 
    s->losses = 0; 
    s->bestMoves = 9999; 
    s->coins = 0;
    s->unlocked[0] = true; 
    s->unlocked[1] = true;
    
    for (int i = 2; i < NUM_SKINS; i++) 
        s->unlocked[i] = false;
    
    FILE* f = fopen("records.dat", "r");
    
    if (!f) { 
        SaveStats(s); 
        return; 
    }
    
    int u[NUM_SKINS] = {0};
    fscanf(f, "wins=%d\nlosses=%d\nbest=%d\ncoins=%d\n"
              "unlocked=%d %d %d %d %d %d %d %d\n",
           &s->wins, &s->losses, &s->bestMoves, &s->coins,
           &u[0],&u[1],&u[2],&u[3],&u[4],&u[5],&u[6],&u[7]);
    
    for (int i = 0; i < NUM_SKINS; i++) 
        s->unlocked[i] = (u[i] != 0);
    
    s->unlocked[0] = true;
    s->unlocked[1] = true;
    fclose(f);
}

void SaveStats(GameStats* s) {
    FILE* f = fopen("records.dat", "w"); 
    if (!f) 
        return;
    fprintf(f, "wins=%d\nlosses=%d\nbest=%d\ncoins=%d\n"
               "unlocked=%d %d %d %d %d %d %d %d\n",
            s->wins, s->losses, s->bestMoves, s->coins,
            s->unlocked[0],s->unlocked[1],s->unlocked[2],s->unlocked[3],
            s->unlocked[4],s->unlocked[5],s->unlocked[6],s->unlocked[7]);
    fclose(f);
}
