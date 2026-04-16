#include "common.h"
#include "logic.h"
#include "render.h"
#include <time.h>


/**
 * @brief Initialise une structure de bouton (Button).
 * * Cette fonction remplit les coordonnées, les dimensions, la couleur 
 * et l'étiquette (label) de l'objet bouton passé en paramètre.
 * * @param b Pointeur vers la structure Button à initialiser.
 * @param x Coordonnée X du coin supérieur gauche.
 * @param y Coordonnée Y du coin supérieur gauche.
 * @param w Largeur du bouton.
 * @param h Hauteur du bouton.
 * @param r Composante rouge de la couleur (0-255).
 * @param g Composante verte (0-255).
 * @param bl Composante bleue (0-255).
 * @param lbl Texte à afficher sur le bouton.
 */

static void MkBtn(Button* b,int x,int y,int w,int h,
                  Uint8 r,Uint8 g,Uint8 bl,const char* lbl){
    b->rect=(SDL_Rect){x,y,w,h}; 
    b->color=(SDL_Color){r,g,bl,255};
    b->isHovered=false; 
    strncpy(b->label,lbl,sizeof(b->label)-1);
    b->label[sizeof(b->label)-1]='\0';
}

/**
 * @brief Vérifie si la souris survole le bouton.
 * * Met à jour l'état interne `isHovered` du bouton en fonction 
 * des coordonnées actuelles de la souris.
 * * @param b Pointeur vers le bouton à tester.
 * @param mx Position X actuelle de la souris.
 * @param my Position Y actuelle de la souris.
 */

static void Hover(Button* b,int mx,int my){
    b->isHovered=IsPointInRect(mx,my,b->rect);
}

/**
 * @brief Calcule les pièces gagnées par partie.
 * * La récompense évolue en fonction du résultat (victoire ou défaite), 
 * du nombre de navires coulés et du niveau de difficulté de l'IA.
 * * @param win Indique si la partie est gagnée (true).
 * @param sunkCount Nombre total de navires coulés durant la partie.
 * @param diff Niveau de difficulté de l'IA (FACILE, NORMAL, EXPERT).
 * @return int Le nombre total de pièces à attribuer au joueur.
 */
static int CalcCoins(bool win,int sunkCount,AIDifficulty diff){
    int base=0;
    if(win){
        if(diff==DIFF_FACILE)  
          base=50;
        else if(diff==DIFF_NORMAL) 
          base=100;
        else                       
          base=200;
    } 
    else {
        /* Loss: tiny coins, still scale a bit */
        if(diff==DIFF_FACILE)  
          base=5;
        else if(diff==DIFF_NORMAL) 
          base=10;
        else                       
          base=20;
    }
    /* Bonus per ship sunk */
    int perSunk=(diff==DIFF_FACILE)?5:(diff==DIFF_NORMAL)?10:20;
    return base + sunkCount*perSunk;
}

int main(int argc,char* argv[]){
  
    (void)argc;
    (void)argv;
  
    SDL_Init(SDL_INIT_VIDEO); TTF_Init();
    srand((unsigned)time(NULL));
    SDL_Window*   win=SDL_CreateWindow("Point Zéro",
                         SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_W,WINDOW_H,
                         SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* ren=SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(ren, WINDOW_W, WINDOW_H);  /* scale to any screen size */
    TTF_Font*     font=TTF_OpenFont("lib/font.ttf",24);

    GameBoard player,enemy; 
    CPUState cpu;
    GameMode  mode=MODE_SPLASH;
    bool  quit=false,playerTurn=true,playerWon=false,isV=false;
    int   ships[]={4,3,3,2,2,2,1,1,1,1};
    int   curIdx=0,playerMoves=0,sunkThisGame=0,lastCoinsEarned=0;
    Uint32 gameStart=0;
    AIDifficulty diff=DIFF_NORMAL;
    GridSkin sk=SKIN_OCEAN;
  
    GameStats stats; 
    LoadStats(&stats);
  
    /* Restore active skin */
    for(int i=0;i<NUM_SKINS;i++) 
      if(stats.unlocked[i]) 
        sk=(GridSkin)i;
    sk=SKIN_OCEAN; /* default to ocean on start */
    SetSkin(sk);

    const char* dnames[]={"FACILE","NORMAL","EXPERT"};
    const char* snames[]={"Océan","Nuit","Arctique","Rouge","Or","Forêt","Plasma","Glace"};

    int bx=658,bw=430,bh=64,bg2=16;
    Button bPlay,bShop,bSettings,bRecords,bQuit;
    MkBtn(&bPlay,    bx,210+0*(bh+bg2),bw,bh, 30,110, 45,"  JOUER");
    MkBtn(&bShop,    bx,210+1*(bh+bg2),bw,bh, 90, 60,  5,"  BOUTIQUE");
    MkBtn(&bSettings,bx,210+2*(bh+bg2),bw,bh, 40, 55,140,"  REGLAGES");
    MkBtn(&bRecords, bx,210+3*(bh+bg2),bw,bh,100, 80, 25,"  RECORDS");
    MkBtn(&bQuit,    bx,210+4*(bh+bg2),bw,bh,120, 30, 30,"  QUITTER");

    int spx=WINDOW_W/2-220,spy=200;
    Button bDiff,bSkin,bSettBack;
    MkBtn(&bDiff,    spx,spy,    440,58, 45,55,140,"Difficulté: NORMAL");
    MkBtn(&bSkin,    spx,spy+110,440,58, 80,55,120,"Thème actif: Océan");
    MkBtn(&bSettBack,spx,spy+320,440,58, 60,50,110,"< RETOUR");

    static const int SKIN_PRICES[NUM_SKINS]={0,0,150,200,300,400,450,500};
    /* Shop card buttons: 4 cols x 2 rows, matching render.c layout */
    /* render.c: px=80, py=60, cw=168, ch=175, smx=px+20=100, smy=py+60=120, smg=12 */
    int scw=210,sch=200,ssmx=120,ssmy=135,ssmg=14;
    Button btnBuy[NUM_SKINS],btnSel[NUM_SKINS];
    Button* pBuy[NUM_SKINS],*pSel[NUM_SKINS];
    for(int i=0;i<NUM_SKINS;i++){
        int col=i%4, row=i/4;
        int cx=ssmx+col*(scw+ssmg);
        int cy=ssmy+row*(sch+ssmg);
        MkBtn(&btnBuy[i],cx+8,cy+158,120,32, 180,140,10,"ACHETER");
        MkBtn(&btnSel[i],cx+8,cy+158,120,32,  30,100,45,"CHOISIR");
        pBuy[i]=&btnBuy[i]; 
        pSel[i]=&btnSel[i];
    }
    Button bShopBack;
    MkBtn(&bShopBack,WINDOW_W/2-110,680,220,50, 60,50,110,"< RETOUR");

    int px2=E_OFFSET_X;
    Button bRand,bReset,bPret,bPlaceBack;
    MkBtn(&bRand,     px2+5,  OFFSET_Y+GRID_SIZE*CELL_SIZE+18,195,48, 35, 80,140,"ALEATOIRE");
    MkBtn(&bReset,    px2+5,  OFFSET_Y+GRID_SIZE*CELL_SIZE+72,195,48,120, 50, 30,"REINITIALISER");
    MkBtn(&bPret,     px2+205,OFFSET_Y+GRID_SIZE*CELL_SIZE+18,200,102, 30,100, 50,"PRET !");
    MkBtn(&bPlaceBack,P_OFFSET_X,OFFSET_Y+GRID_SIZE*CELL_SIZE+18,160,48, 80,40,40,"< MENU");

    Button bReplay,bMenu2;
    MkBtn(&bReplay,WINDOW_W/2-230,WINDOW_H/2+80,210,54, 35, 80,150,"REJOUER");
    MkBtn(&bMenu2, WINDOW_W/2+ 20,WINDOW_H/2+80,210,54,100, 50, 30,"MENU");

    Button bExit;
    MkBtn(&bExit, WINDOW_W-140, 10, 130, 38, 110, 25, 25, "< MENU");

    Button bRecBack;
    MkBtn(&bRecBack,WINDOW_W/2-110,WINDOW_H/2+160,220,54, 60,55,130,"RETOUR");

    int hR=-1,hC=-1;

    while(!quit){
        SDL_Event ev;
        int mx,my;
        {
            int wx,wy; 
            SDL_GetMouseState(&wx,&wy);
          
            float lx,ly; 
            SDL_RenderWindowToLogical(ren,wx,wy,&lx,&ly);
          
            mx=(int)lx; 
            my=(int)ly;
        }
        if(mode==MODE_PLACEMENT){
          hR=(my-OFFSET_Y)/CELL_SIZE;
          hC=(mx-P_OFFSET_X)/CELL_SIZE;
        }

        while(SDL_PollEvent(&ev)){
            if(ev.type==SDL_QUIT){
              quit=true;
              break;
            }

            if(mode==MODE_SPLASH){
                if(ev.type==SDL_KEYDOWN||ev.type==SDL_MOUSEBUTTONDOWN) 
                  mode=MODE_MAINMENU;

            } 

            else if(mode==MODE_MAINMENU){
                Hover(&bPlay,mx,my);
                Hover(&bShop,mx,my);
                Hover(&bSettings,mx,my);
              
                Hover(&bRecords,mx,my);
                Hover(&bQuit,mx,my);
              
                if(ev.type==SDL_MOUSEBUTTONDOWN){
                    if(bPlay.isHovered){
                        ClearBoard(&player);
                        ClearBoard(&enemy);
                      
                        InitCPUState(&cpu);
                        curIdx=0;
                        isV=false;
                      
                        playerMoves=0;
                        sunkThisGame=0;
                        mode=MODE_PLACEMENT;
                    }
                    if(bShop.isHovered)     
                      mode=MODE_SHOP;
                    if(bSettings.isHovered) 
                      mode=MODE_SETTINGS;
                    if(bRecords.isHovered)  
                      mode=MODE_RECORDS;
                    if(bQuit.isHovered)     
                      quit=true;
                }
                if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE) 
                  quit=true;

            } 
            else if(mode==MODE_SETTINGS){
                Hover(&bDiff,mx,my);
                Hover(&bSkin,mx,my);
                Hover(&bSettBack,mx,my);
              
                if(ev.type==SDL_MOUSEBUTTONDOWN){
                    if(bDiff.isHovered){
                        diff=(AIDifficulty)((diff+1)%3);
                        snprintf(bDiff.label,sizeof(bDiff.label),"Difficulté: %s",dnames[diff]);
                    }
                    if(bSkin.isHovered){
                        /* cycle only through unlocked skins */
                        int next=(sk+1)%NUM_SKINS;
                        while(!stats.unlocked[next]) 
                          next=(next+1)%NUM_SKINS;
                      
                        sk=(GridSkin)next; 
                        SetSkin(sk);
                        snprintf(bSkin.label,sizeof(bSkin.label),"Thème actif: %s",snames[sk]);
                    }
                    if(bSettBack.isHovered) 
                      mode=MODE_MAINMENU;
                }
                if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE) 
                  mode=MODE_MAINMENU;

            } 
            else if(mode==MODE_SHOP){
                for(int i=0;i<NUM_SKINS;i++){
                    Hover(&btnBuy[i],mx,my); 
                    Hover(&btnSel[i],mx,my);
                }
                Hover(&bShopBack,mx,my);
                if(ev.type==SDL_MOUSEBUTTONDOWN){
                    for(int i=0;i<NUM_SKINS;i++){
                        if(!stats.unlocked[i]&&btnBuy[i].isHovered){
                            if(stats.coins>=SKIN_PRICES[i]){
                                stats.coins-=SKIN_PRICES[i];
                                stats.unlocked[i]=true;
                                SaveStats(&stats);
                            }
                        }
                        if(stats.unlocked[i] && btnSel[i].isHovered && (GridSkin)i != sk){
                            sk=(GridSkin)i; 
                            SetSkin(sk);
                            snprintf(bSkin.label,sizeof(bSkin.label),"Thème actif: %s",snames[sk]);
                        }
                    }
                    if(bShopBack.isHovered) 
                      mode=MODE_MAINMENU;
                }
                if(ev.type==SDL_KEYDOWN && ev.key.keysym.sym==SDLK_ESCAPE) 
                  mode=MODE_MAINMENU;
            } 
            else if(mode==MODE_PLACEMENT){
                Hover(&bRand,mx,my);
                Hover(&bReset,mx,my);
                Hover(&bPret,mx,my);
                Hover(&bPlaceBack,mx,my);
              
                if(ev.type==SDL_MOUSEWHEEL)  
                  isV=!isV;
              
                if(ev.type==SDL_MOUSEBUTTONDOWN){
                    if(ev.button.button==SDL_BUTTON_RIGHT) isV=!isV;
                    else if(ev.button.button==SDL_BUTTON_LEFT){
                        if(bRand.isHovered){
                          RandomPlaceFull(&player);
                          curIdx=10;
                        }
                        else if(bReset.isHovered){
                          ClearBoard(&player);
                          curIdx=0;
                        }
                        else if(bPret.isHovered&&curIdx>=10){
                            RandomPlaceFull(&enemy);
                            playerTurn=true;
                            playerMoves=0;
                            sunkThisGame=0;
                            gameStart=SDL_GetTicks();
                            mode=MODE_GAME;
                        } else if(curIdx<10){
                            int r=(my-OFFSET_Y)/CELL_SIZE,c=(mx-P_OFFSET_X)/CELL_SIZE;
                            if(r>=0&&r<GRID_SIZE&&c>=0&&c<GRID_SIZE){
                                if(CanPlaceShip(&player,r,c,ships[curIdx],isV)){
                                    PlaceShip(&player,r,c,ships[curIdx],isV);
                                    curIdx++;
                                }
                            }
                        }
                    }
                }
                if(bPlaceBack.isHovered && ev.type==SDL_MOUSEBUTTONDOWN) 
                  mode=MODE_MAINMENU;
                if(ev.type==SDL_KEYDOWN && ev.key.keysym.sym==SDLK_ESCAPE)
                  mode=MODE_MAINMENU;

            } else if(mode==MODE_GAME){
                Hover(&bExit,mx,my);
                if(playerTurn && ev.type==SDL_MOUSEBUTTONDOWN){
                    if(bExit.isHovered){ 
                      mode=MODE_MAINMENU;
                      break; 
                    }
                    int c=(mx-E_OFFSET_X)/CELL_SIZE,r=(my-OFFSET_Y)/CELL_SIZE;
                    if(mx>=E_OFFSET_X&&r>=0&&r<GRID_SIZE&&c>=0&&c<GRID_SIZE){
                        CellState cs=enemy.cells[r][c];
                        if(cs==CELL_SHIP){
                            enemy.cells[r][c]=CELL_HIT; 
                            enemy.shipsLeft--;
                            int sz=CheckSunkShips(&enemy); 
                            playerMoves++;
                            if(sz>0) 
                              sunkThisGame++;
                            if(enemy.shipsLeft<=0){
                                playerWon=true;
                                stats.wins++;
                                if(playerMoves<stats.bestMoves) 
                                  stats.bestMoves=playerMoves;
                              
                                lastCoinsEarned=CalcCoins(true,sunkThisGame,diff);
                                stats.coins+=lastCoinsEarned;
                                SaveStats(&stats); 
                              mode=MODE_GAMEOVER;
                            }
                        } else if(cs==CELL_EMPTY){
                            enemy.cells[r][c]=CELL_MISS;
                            playerMoves++; 
                            playerTurn=false;
                        }
                    }
                }
                if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE) 
                  mode=MODE_MAINMENU;

            } else if(mode==MODE_GAMEOVER){
                Hover(&bReplay,mx,my);
                Hover(&bMenu2,mx,my);
                if((ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_SPACE)||
                   (ev.type==SDL_MOUSEBUTTONDOWN&&bReplay.isHovered)){
                    ClearBoard(&player);
                    ClearBoard(&enemy);
                    InitCPUState(&cpu);
                  
                    curIdx=0;
                    isV=false;
                    playerMoves=0;
                    sunkThisGame=0;
                    mode=MODE_PLACEMENT;
                }
                if(ev.type==SDL_MOUSEBUTTONDOWN&&bMenu2.isHovered) 
                  mode=MODE_MAINMENU;

            } else if(mode==MODE_RECORDS){
                Hover(&bRecBack,mx,my);
                if(ev.type==SDL_MOUSEBUTTONDOWN&&bRecBack.isHovered) 
                  mode=MODE_MAINMENU;
                if(ev.type==SDL_KEYDOWN&&ev.key.keysym.sym==SDLK_ESCAPE) 
                  mode=MODE_MAINMENU;
            }
        }

        if(mode==MODE_GAME&&!playerTurn){
            SDL_Delay(diff==DIFF_FACILE?700:diff==DIFF_NORMAL?450:250);
            ProcessCPUTurn(&player,&playerTurn,&cpu,diff);
            if(player.shipsLeft<=0){
                playerWon=false; 
                stats.losses++;
              
                lastCoinsEarned=0;
              
                SaveStats(&stats); 
                mode=MODE_GAMEOVER;
            }
        }

        SDL_SetRenderDrawColor(ren,8,12,24,255); 
        SDL_RenderClear(ren);

        if(mode==MODE_SPLASH){
            DrawSplash(ren,font,SDL_GetTicks());
        } 
        else if(mode==MODE_MAINMENU){
            DrawMainMenu(ren,font,&bPlay,&bShop,&bSettings,&bRecords,&bQuit,stats.coins);
        } 
        else if(mode==MODE_SETTINGS){
            DrawMainMenu(ren,font,&bPlay,&bShop,&bSettings,&bRecords,&bQuit,stats.coins);
            DrawSettings(ren,font,&bDiff,&bSkin,&bSettBack);
        } 
        else if(mode==MODE_SHOP){
            DrawMainMenu(ren,font,&bPlay,&bShop,&bSettings,&bRecords,&bQuit,stats.coins);
            DrawShop(ren,font,&stats,sk,pBuy,pSel,&bShopBack);
        } 
        else if(mode==MODE_PLACEMENT){
            SDL_SetRenderDrawColor(ren,12,16,28,255); SDL_RenderClear(ren);
            DrawText(ren,font,"PLACEMENT DES NAVIRES",P_OFFSET_X+30,6,(SDL_Color){200,200,200,255});
            DrawBoard(ren,font,&player,P_OFFSET_X,OFFSET_Y,false,false);
            if(curIdx<10)
                DrawPlacementPreview(ren,&player,P_OFFSET_X,OFFSET_Y,hR,hC,ships[curIdx],isV);
            DrawPlacementUI(ren,font,curIdx,&bRand,&bReset,&bPret,&bPlaceBack);
        } 
        else if(mode==MODE_GAME){
            /* Dim the board that is NOT active */
            DrawBoard(ren,font,&player,P_OFFSET_X,OFFSET_Y,false,!playerTurn);
            DrawBoard(ren,font,&enemy, E_OFFSET_X,OFFSET_Y,true,  false);
            DrawGameUI(ren,font,&player,&enemy,playerTurn,playerMoves,gameStart,stats.coins);
            DrawButton(ren,font,&bExit);
        } 
        else if(mode==MODE_GAMEOVER){
            DrawBoard(ren,font,&player,P_OFFSET_X,OFFSET_Y,false,false);
            DrawBoard(ren,font,&enemy, E_OFFSET_X,OFFSET_Y,false,false);
            DrawGameUI(ren,font,&player,&enemy,playerTurn,playerMoves,gameStart,stats.coins);
            DrawGameOver(ren,font,playerWon,playerMoves,lastCoinsEarned,&bReplay,&bMenu2);
        } 
        else if(mode==MODE_RECORDS){
            DrawMainMenu(ren,font,&bPlay,&bShop,&bSettings,&bRecords,&bQuit,stats.coins);
            DrawRecords(ren,font,&stats,&bRecBack);
        }
        SDL_RenderPresent(ren);
    }
  
    TTF_CloseFont(font); 
    TTF_Quit(); 
    SDL_Quit();
    return 0;
}
