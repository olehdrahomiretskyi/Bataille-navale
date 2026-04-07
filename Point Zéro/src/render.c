#include "render.h"
#include "logic.h"

typedef struct { 
    SDL_Color bg,water,grid,ship,hit,sunk,miss,near,accent,text,dim; 
} Skin;

static const Skin SKINS[NUM_SKINS]={
    /* 0 OCEAN */
    {{10,15,30,255},{18,36,62,255},{40,65,110,255},{80,110,160,255},
     {255,165,0,255},{160,25,25,255},{28,42,72,255},{22,32,55,255},
     {80,150,255,255},{255,255,255,255},{130,150,185,255}},
    /* 1 NUIT */
    {{5,5,18,255},{14,10,28,255},{55,30,80,255},{100,55,130,255},
     {255,70,120,255},{130,0,65,255},{22,14,38,255},{18,10,30,255},
     {180,100,255,255},{230,220,255,255},{120,90,160,255}},
    /* 2 ARCTIQUE */
    {{195,210,228,255},{175,190,210,255},{130,150,175,255},{80,110,155,255},
     {220,80,40,255},{160,45,30,255},{155,170,190,255},{145,160,182,255},
     {50,95,180,255},{20,25,45,255},{60,80,115,255}},
    /* 3 ROUGE */
    {{25,5,5,255},{50,10,10,255},{100,30,30,255},{160,50,50,255},
     {255,220,0,255},{80,0,0,255},{40,8,8,255},{32,6,6,255},
     {220,60,60,255},{255,220,220,255},{160,80,80,255}},
    /* 4 OR */
    {{20,15,5,255},{40,30,8,255},{90,70,20,255},{160,130,40,255},
     {220,80,40,255},{120,30,10,255},{32,24,6,255},{26,20,4,255},
     {220,180,40,255},{255,245,200,255},{160,140,80,255}},
    /* 5 FORET (jungle green) */
    {{8,18,8,255},{12,35,12,255},{30,70,30,255},{55,115,45,255},
     {255,200,0,255},{100,20,10,255},{10,28,10,255},{8,22,8,255},
     {80,200,80,255},{210,240,210,255},{70,130,70,255}},
    /* 6 PLASMA (neon cyan) */
    {{5,15,20,255},{8,28,35,255},{20,80,100,255},{30,140,160,255},
     {255,80,200,255},{0,80,120,255},{8,20,28,255},{6,16,22,255},
     {0,220,230,255},{200,245,255,255},{60,160,180,255}},
    /* 7 GLACE (icy white/steel blue) */
    {{210,228,240,255},{190,210,230,255},{150,180,210,255},{100,150,200,255},
     {255,100,50,255},{180,60,40,255},{170,185,205,255},{160,178,200,255},
     {90,160,230,255},{15,20,40,255},{80,110,150,255}}
};
static GridSkin g_skin=SKIN_OCEAN;
void SetSkin(GridSkin s){
    g_skin=s;
}
static const Skin* SK(void){
    return &SKINS[g_skin];
}

static const char* SKIN_NAMES[NUM_SKINS]={"Ocean","Nuit","Arctique","Rouge","Or","Foret","Plasma","Glace"};
static const int   SKIN_PRICES[NUM_SKINS]={0,0,150,300,500,200,400,350};

static Particle g_p[MAX_PARTICLES]; 
static bool g_pInit=false;

static void InitParts(void){
    for(int i=0;i<MAX_PARTICLES;i++){
        g_p[i].x=(float)(rand()%WINDOW_W); 
        g_p[i].y=(float)(rand()%WINDOW_H);
        
        float a=(float)(rand()%628)/100.f,sp=0.15f+(rand()%25)/100.f;
        g_p[i].vx=cosf(a)*sp; 
        g_p[i].vy=sinf(a)*sp;
        g_p[i].r=60+rand()%60; 
        g_p[i].g=100+rand()%80; 
        g_p[i].b=180+rand()%75;
        g_p[i].alpha=(float)(40+rand()%40)/100.f;
    } 
    g_pInit=true;
}
void UpdateParticles(SDL_Renderer* ren){
    if(!g_pInit) InitParts();
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    for(int i=0;i<MAX_PARTICLES;i++){
        Particle* p=&g_p[i];
        p->x+=p->vx; p->y+=p->vy;
        if(p->x < 0)       p->x += WINDOW_W;
        if(p->x > WINDOW_W) p->x -= WINDOW_W;
        if(p->y < 0)       p->y += WINDOW_H;
        if(p->y > WINDOW_H) p->y -= WINDOW_H;
        for(int j=i+1;j<MAX_PARTICLES;j++){
            float dx=g_p[j].x-p->x,dy=g_p[j].y-p->y,d=sqrtf(dx*dx+dy*dy);
            if(d<85.f){Uint8 a=(Uint8)((1.f-d/85.f)*45.f);
                SDL_SetRenderDrawColor(ren,(Uint8)p->r,(Uint8)p->g,(Uint8)p->b,a);
                SDL_RenderDrawLine(ren,(int)p->x,(int)p->y,(int)g_p[j].x,(int)g_p[j].y);}
        }
        SDL_SetRenderDrawColor(ren,(Uint8)p->r,(Uint8)p->g,(Uint8)p->b,(Uint8)(p->alpha*200.f));
        SDL_Rect dot={(int)p->x-2,(int)p->y-2,4,4}; SDL_RenderFillRect(ren,&dot);
    }
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
}

static void Fill(SDL_Renderer* r,int x,int y,int w,int h,SDL_Color c){
    SDL_Rect rc={x,y,w,h};
    SDL_SetRenderDrawColor(r,c.r,c.g,c.b,c.a?c.a:255); SDL_RenderFillRect(r,&rc);
}
static void Box(SDL_Renderer* r,int x,int y,int w,int h,SDL_Color c){
    SDL_Rect rc={x,y,w,h};
    SDL_SetRenderDrawColor(r,c.r,c.g,c.b,255); SDL_RenderDrawRect(r,&rc);
}
void DrawText(SDL_Renderer* ren,TTF_Font* font,const char* text,int x,int y,SDL_Color col){
    SDL_Surface* s=TTF_RenderUTF8_Blended(font,text,col); if(!s) return;
    SDL_Texture* t=SDL_CreateTextureFromSurface(ren,s);
    SDL_Rect dst={x,y,s->w,s->h}; SDL_RenderCopy(ren,t,NULL,&dst);
    SDL_FreeSurface(s); SDL_DestroyTexture(t);
}

void DrawButton(SDL_Renderer* ren,TTF_Font* font,Button* b){
    int x=b->rect.x, y=b->rect.y, w=b->rect.w, h=b->rect.h;
    int hi=b->isHovered?55:0;
    SDL_Color bc={(Uint8)SDL_min(b->color.r+hi,255),
                  (Uint8)SDL_min(b->color.g+hi,255),
                  (Uint8)SDL_min(b->color.b+hi,255),255};

    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);

    /* Outer glow on hover */
    if(b->isHovered){
        for(int g2=6;g2>=1;g2--){
            Uint8 ga=(Uint8)(18*(7-g2));
            SDL_SetRenderDrawColor(ren,bc.r,bc.g,bc.b,ga);
            SDL_Rect gl={x-g2,y-g2,w+g2*2,h+g2*2};
            SDL_RenderDrawRect(ren,&gl);
        }
    }

    /* Drop shadow */
    SDL_SetRenderDrawColor(ren,0,0,0, b->isHovered?50:90);
    SDL_Rect sh={x+4,y+5,w,h}; SDL_RenderFillRect(ren,&sh);

    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

    /* Base fill */
    Fill(ren,x,y,w,h,bc);

    /* Gradient: lighter top strip */
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    int bands=h/2;
    for(int i=0;i<bands;i++){
        Uint8 a=(Uint8)(60*(1.f-(float)i/bands));
        SDL_SetRenderDrawColor(ren,255,255,255,a);
        SDL_RenderDrawLine(ren,x+1,y+i,x+w-2,y+i);
    }
    /* Darker bottom strip */
    for(int i=0;i<h/4;i++){
        Uint8 a=(Uint8)(40*((float)i/(h/4)));
        SDL_SetRenderDrawColor(ren,0,0,0,a);
        SDL_RenderDrawLine(ren,x+1,y+h-1-i,x+w-2,y+h-1-i);
    }
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);

    /* Border: bright top+left, dim bottom+right */
    SDL_SetRenderDrawColor(ren,
        (Uint8)SDL_min((int)bc.r+90,255),
        (Uint8)SDL_min((int)bc.g+90,255),
        (Uint8)SDL_min((int)bc.b+90,255),255);
    SDL_RenderDrawLine(ren,x,y,x+w-1,y);
    SDL_RenderDrawLine(ren,x,y,x,y+h-1);
    SDL_SetRenderDrawColor(ren,
        (Uint8)SDL_max((int)bc.r-60,0),
        (Uint8)SDL_max((int)bc.g-60,0),
        (Uint8)SDL_max((int)bc.b-60,0),255);
    SDL_RenderDrawLine(ren,x+w-1,y,x+w-1,y+h-1);
    SDL_RenderDrawLine(ren,x,y+h-1,x+w-1,y+h-1);

    /* Label with text shadow */
    SDL_Surface* s=TTF_RenderUTF8_Blended(font,b->label,(SDL_Color){255,255,255,255});
    if(s){
        SDL_Texture* t=SDL_CreateTextureFromSurface(ren,s);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(t,80);
        SDL_Rect ds2={x+(w-s->w)/2+1,y+(h-s->h)/2+1,s->w,s->h};
        SDL_RenderCopy(ren,t,NULL,&ds2);
        SDL_SetTextureAlphaMod(t,255);
        SDL_Rect dst={x+(w-s->w)/2,y+(h-s->h)/2,s->w,s->h};
        SDL_RenderCopy(ren,t,NULL,&dst);
        SDL_FreeSurface(s); SDL_DestroyTexture(t);
    }
}


static void DrawCoins(SDL_Renderer* ren,TTF_Font* font,int coins,int x,int y){
    char buf[32]; snprintf(buf,sizeof(buf),"$ %d",coins);
    Fill(ren,x,y,150,36,(SDL_Color){40,30,5,255});
    Box(ren, x,y,150,36,(SDL_Color){220,180,40,255});
    DrawText(ren,font,buf,x+12,y+7,(SDL_Color){220,180,40,255});
}


void DrawSplash(SDL_Renderer* ren,TTF_Font* font,Uint32 ticks){
    SDL_SetRenderDrawColor(ren,8,12,24,255); SDL_RenderClear(ren);
    UpdateParticles(ren);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    for(int w=0;w<3;w++){
        float off=(float)ticks/1000.f*(0.8f+w*0.2f);
        for(int x=0;x<WINDOW_W;x++){
            int y2=WINDOW_H-80-w*25+(int)(12.f*sinf((float)x/60.f+off));
            SDL_SetRenderDrawColor(ren,(Uint8)(40+w*10),(Uint8)(80+w*20),(Uint8)(160+w*20),(Uint8)(55-w*15));
            SDL_Rect wv={x,y2,1,WINDOW_H}; SDL_RenderFillRect(ren,&wv);
        }
    }
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    int tx=WINDOW_W/2-310,ty=WINDOW_H/2-145;
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,10,20,50,200);
    SDL_Rect tb={tx,ty,620,155}; SDL_RenderFillRect(ren,&tb);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    Box(ren,tx,ty,620,155,(SDL_Color){80,150,255,255});
    Box(ren,tx+2,ty+2,616,151,(SDL_Color){30,60,120,255});
    DrawText(ren,font,"POINT ZERO",WINDOW_W/2-130,ty+18,(SDL_Color){255,255,255,255});
    DrawText(ren,font,"~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~",WINDOW_W/2-162,ty+62,(SDL_Color){80,150,255,255});
    if((ticks/600)%2==0)
        DrawText(ren,font,"[ APPUYEZ SUR UNE TOUCHE ]",
                 WINDOW_W/2-168,WINDOW_H/2+60,(SDL_Color){200,220,255,255});
    DrawText(ren,font,"SDL2  |  C11",WINDOW_W/2-58,WINDOW_H-32,(SDL_Color){50,65,95,255});
}

void DrawMainMenu(SDL_Renderer* ren,TTF_Font* font,
                  Button* play,Button* shop,Button* settings,Button* records,Button* quit,
                  int coins){
    SDL_Color bg=SK()->bg;
    SDL_SetRenderDrawColor(ren,bg.r,bg.g,bg.b,255); SDL_RenderClear(ren);
    UpdateParticles(ren);
    SDL_Color acc=SK()->accent;
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,acc.r,acc.g,acc.b,18);
    SDL_Rect tp={WINDOW_W/2-240,38,480,80}; SDL_RenderFillRect(ren,&tp);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    Box(ren,tp.x,tp.y,tp.w,tp.h,acc);
    DrawText(ren,font,"POINT ZERO",WINDOW_W/2-150,52,SK()->text);
    DrawText(ren,font,"~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~",WINDOW_W/2-153,88,acc);

    /* Left panel: mini grid */
    int lx=60,ly=130;
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,acc.r,acc.g,acc.b,18);
    SDL_Rect lp={lx,ly,520,560}; SDL_RenderFillRect(ren,&lp);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    Box(ren,lx,ly,520,560,acc);
    DrawText(ren,font,"POINT ZERO",lx+42,ly+20,SK()->text);
    SDL_SetRenderDrawColor(ren,acc.r,acc.g,acc.b,255);
    SDL_RenderDrawLine(ren,lx+10,ly+56,lx+380,ly+56);
    static const char PAT[8][8]={{0,0,1,1,1,0,0,0},{0,0,0,0,0,0,1,0},
        {0,1,0,0,0,0,1,0},{0,1,0,2,0,0,1,0},{0,1,0,0,0,0,0,0},
        {0,0,0,0,1,1,0,0},{0,0,3,0,0,0,0,0},{0,0,0,0,0,0,0,1}};
    int gx=lx+30,gy=ly+80,cs=36;
    for(int r=0;r<8;r++) for(int c=0;c<8;c++){
        int x=gx+c*cs,y=gy+r*cs;
        Fill(ren,x+1,y+1,cs-2,cs-2,SK()->water);
        if(PAT[r][c]==1) Fill(ren,x+2,y+2,cs-4,cs-4,SK()->ship);
        else if(PAT[r][c]==2){Fill(ren,x+1,y+1,cs-2,cs-2,SK()->hit);
            SDL_SetRenderDrawColor(ren,255,255,255,255);
            SDL_RenderDrawLine(ren,x+5,y+5,x+cs-6,y+cs-6);
            SDL_RenderDrawLine(ren,x+cs-6,y+5,x+5,y+cs-6);}
        else if(PAT[r][c]==3){Fill(ren,x+1,y+1,cs-2,cs-2,SK()->miss);
            Fill(ren,x+cs/2-2,y+cs/2-2,4,4,SK()->dim);}
        Box(ren,x,y,cs,cs,SK()->grid);
    }
    Box(ren,gx,gy,8*cs,8*cs,acc);
    DrawText(ren,font,"Apercu de partie...",lx+68,gy+8*cs+10,SK()->dim);

    /* Right panel */
    int rx=638,ry=120;
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,acc.r,acc.g,acc.b,14);
    SDL_Rect rp={rx-10,ry,500,580}; SDL_RenderFillRect(ren,&rp);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    Box(ren,rx-10,ry,500,580,SK()->dim);

    /* Buttons — draw them first, then icon decorations on top-left corner */
    Button* btns[5]={play,shop,settings,records,quit};
    for(int i=0;i<5;i++) DrawButton(ren,font,btns[i]);

    /* Icon accent squares on left edge of each button */
    SDL_Color icols[5]={{30,110,50,255},{100,65,10,255},{50,65,160,255},{110,85,20,255},{130,30,30,255}};
    for(int i=0;i<5;i++){
        int bx2=btns[i]->rect.x, by2=btns[i]->rect.y, bh2=btns[i]->rect.h;
        Fill(ren,bx2,by2,46,bh2,icols[i]);
        /* Lighter inner edge */
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren,255,255,255,35);
        SDL_Rect hl={bx2,by2,46,bh2}; SDL_RenderFillRect(ren,&hl);
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    }
    /* Play icon - triangle */
    {int bx2=play->rect.x,by2=play->rect.y,bh2=play->rect.h;
     SDL_SetRenderDrawColor(ren,150,255,150,255);
     int cy2=by2+bh2/2;
     SDL_RenderDrawLine(ren,bx2+10,cy2-10,bx2+10,cy2+10);
     SDL_RenderDrawLine(ren,bx2+10,cy2-10,bx2+30,cy2);
     SDL_RenderDrawLine(ren,bx2+30,cy2,bx2+10,cy2+10);}
    /* Shop icon - bag */
    {int bx2=shop->rect.x,by2=shop->rect.y,bh2=shop->rect.h;
     SDL_SetRenderDrawColor(ren,255,220,80,255);
     int cy2=by2+bh2/2;
     SDL_RenderDrawLine(ren,bx2+8,cy2-4,bx2+38,cy2-4);
     SDL_RenderDrawLine(ren,bx2+8,cy2-4,bx2+6,cy2+8);
     SDL_RenderDrawLine(ren,bx2+38,cy2-4,bx2+40,cy2+8);
     SDL_RenderDrawLine(ren,bx2+6,cy2+8,bx2+40,cy2+8);
     SDL_RenderDrawLine(ren,bx2+16,cy2-4,bx2+13,cy2-10);
     SDL_RenderDrawLine(ren,bx2+30,cy2-4,bx2+33,cy2-10);}
    /* Settings icon - gear cross */
    {int bx2=settings->rect.x,by2=settings->rect.y,bh2=settings->rect.h;
     SDL_SetRenderDrawColor(ren,160,185,255,255);
     int cy2=by2+bh2/2;
     SDL_RenderDrawLine(ren,bx2+6,cy2,bx2+40,cy2);
     SDL_RenderDrawLine(ren,bx2+23,cy2-12,bx2+23,cy2+12);
     SDL_RenderDrawLine(ren,bx2+10,cy2-8,bx2+36,cy2+8);
     SDL_RenderDrawLine(ren,bx2+36,cy2-8,bx2+10,cy2+8);}
    /* Records icon - trophy */
    {int bx2=records->rect.x,by2=records->rect.y,bh2=records->rect.h;
     SDL_SetRenderDrawColor(ren,255,220,60,255);
     int cy2=by2+bh2/2;
     SDL_RenderDrawLine(ren,bx2+8,cy2-10,bx2+38,cy2-10);
     SDL_RenderDrawLine(ren,bx2+8,cy2-10,bx2+8,cy2+2);
     SDL_RenderDrawLine(ren,bx2+38,cy2-10,bx2+38,cy2+2);
     SDL_RenderDrawLine(ren,bx2+16,cy2+2,bx2+30,cy2+2);
     SDL_RenderDrawLine(ren,bx2+23,cy2+2,bx2+23,cy2+9);}
    /* Quit icon - X */
    {int bx2=quit->rect.x,by2=quit->rect.y,bh2=quit->rect.h;
     SDL_SetRenderDrawColor(ren,255,100,100,255);
     int cy2=by2+bh2/2;
     SDL_RenderDrawLine(ren,bx2+10,cy2-10,bx2+36,cy2+10);
     SDL_RenderDrawLine(ren,bx2+36,cy2-10,bx2+10,cy2+10);}
    DrawCoins(ren,font,coins,WINDOW_W-165,WINDOW_H-50);
}

void DrawSettings(SDL_Renderer* ren,TTF_Font* font,Button* diff,Button* skin,Button* back){
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,0,0,10,210);
    SDL_Rect ov={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(ren,&ov);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    int px=WINDOW_W/2-260,py=80;
    Fill(ren,px,py,520,540,SK()->bg); Box(ren,px,py,520,540,SK()->accent);
    Box(ren,px+2,py+2,516,536,SK()->dim);
    DrawText(ren,font,"REGLAGES",px+185,py+15,SK()->text);
    SDL_Color ac=SK()->accent;
    SDL_SetRenderDrawColor(ren,ac.r,ac.g,ac.b,255);
    SDL_RenderDrawLine(ren,px+10,py+54,px+510,py+54);
    DrawText(ren,font,"Difficulte IA :",px+30,diff->rect.y-26,SK()->dim);
    DrawText(ren,font,"Theme actif  :",px+30,skin->rect.y-26,SK()->dim);
    DrawButton(ren,font,diff); DrawButton(ren,font,skin); DrawButton(ren,font,back);
}

void DrawShop(SDL_Renderer* ren,TTF_Font* font,
              GameStats* stats,GridSkin activeSkin,
              Button* btnBuy[NUM_SKINS],Button* btnSelect[NUM_SKINS],Button* back){
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,0,0,10,210);
    SDL_Rect ov={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(ren,&ov);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    int px=100,py=70,pw=1080,ph=640;
    Fill(ren,px,py,pw,ph,SK()->bg); Box(ren,px,py,pw,ph,SK()->accent);
    DrawText(ren,font,"BOUTIQUE DES THEMES",px+380,py+12,SK()->text);
    SDL_Color ac=SK()->accent;
    SDL_SetRenderDrawColor(ren,ac.r,ac.g,ac.b,255);
    SDL_RenderDrawLine(ren,px+10,py+50,px+pw-10,py+50);
    DrawCoins(ren,font,stats->coins,px+pw-155,py+10);

    /* 8 skin cards — 4 per row, 2 rows */
    int cw=210,ch=200,smx=px+20,smy=py+65,smg=14;
    for(int i=0;i<NUM_SKINS;i++){
        int col=i%4, row=i/4;
        int cx=smx+col*(cw+smg);
        int cy=smy+row*(ch+smg);
        bool owned=stats->unlocked[i];
        bool active=(activeSkin==(GridSkin)i);
        SDL_Color border=active?(SDL_Color){220,180,40,255}:owned?(SDL_Color){80,150,255,255}:SK()->dim;
        Fill(ren,cx,cy,cw,ch,SKINS[i].bg);
        Box(ren, cx,cy,cw,ch,border);
        if(active){ Box(ren,cx+2,cy+2,cw-4,ch-4,border); }
        /* Mini preview of water+ship colours */
        for(int r2=0;r2<3;r2++) for(int c2=0;c2<5;c2++){
            int xx=cx+12+c2*38,yy=cy+12+r2*28;
            SDL_Color wc=(r2==1&&c2>=1&&c2<=3)?SKINS[i].ship:SKINS[i].water;
            Fill(ren,xx,yy,35,25,wc); Box(ren,xx,yy,35,25,SKINS[i].grid);
        }
        DrawText(ren,font,SKIN_NAMES[i],cx+12,cy+100,SKINS[i].text);
        if(!owned){
            char pbuf[32]; snprintf(pbuf,sizeof(pbuf),"$ %d",SKIN_PRICES[i]);
            DrawText(ren,font,pbuf,cx+12,cy+128,(SDL_Color){220,180,40,255});
            DrawButton(ren,font,btnBuy[i]);
        } else if(active){
            DrawText(ren,font,"ACTIF",cx+12,cy+128,(SDL_Color){60,220,80,255});
        } else {
            DrawButton(ren,font,btnSelect[i]);
        }
    }
    DrawButton(ren,font,back);
}

void DrawBoard(SDL_Renderer* ren,TTF_Font* font,GameBoard* b,
               int ox,int oy,bool hide,bool dimmed){
    const char* COLS="ABCDEFGHIJ"; char buf[4];
    SDL_Color labelCol=dimmed?(SDL_Color){60,70,90,255}:SK()->dim;
    for(int c=0;c<GRID_SIZE;c++){buf[0]=COLS[c];buf[1]=0;
        DrawText(ren,font,buf,ox+c*CELL_SIZE+CELL_SIZE/2-6,oy-26,labelCol);}
    for(int r=0;r<GRID_SIZE;r++){snprintf(buf,4,"%d",r+1);
        DrawText(ren,font,buf,ox-26,oy+r*CELL_SIZE+CELL_SIZE/2-10,labelCol);}
    for(int r=0;r<GRID_SIZE;r++) for(int c=0;c<GRID_SIZE;c++){
        int x=ox+c*CELL_SIZE, y=oy+r*CELL_SIZE;
        CellState cs=b->cells[r][c];
        Fill(ren,x+1,y+1,CELL_SIZE-2,CELL_SIZE-2,SK()->water);
        if(cs==CELL_SHIP&&!hide){
            Fill(ren,x+2,y+2,CELL_SIZE-4,CELL_SIZE-4,SK()->ship);
            Box(ren, x+3,y+3,CELL_SIZE-6,CELL_SIZE-6,(SDL_Color){200,215,240,255});
        } else if(cs==CELL_HIT){
            /* Bright flame orange — clearly different from sunk */
            Fill(ren,x+1,y+1,CELL_SIZE-2,CELL_SIZE-2,SK()->hit);
            /* Bold white X */
            SDL_SetRenderDrawColor(ren,255,255,255,255);
            SDL_RenderDrawLine(ren,x+6,y+6,x+CELL_SIZE-7,y+CELL_SIZE-7);
            SDL_RenderDrawLine(ren,x+CELL_SIZE-7,y+6,x+6,y+CELL_SIZE-7);
            SDL_RenderDrawLine(ren,x+7,y+6,x+CELL_SIZE-6,y+CELL_SIZE-7);
            SDL_RenderDrawLine(ren,x+CELL_SIZE-6,y+6,x+7,y+CELL_SIZE-7);
        } else if(cs==CELL_SUNK){
            /* Dark crimson — clearly "dead" */
            Fill(ren,x+1,y+1,CELL_SIZE-2,CELL_SIZE-2,SK()->sunk);
            /* Skull-ish: two diagonal lines + horizontal */
            SDL_SetRenderDrawColor(ren,255,80,80,255);
            SDL_RenderDrawLine(ren,x+5,y+5,x+CELL_SIZE-6,y+CELL_SIZE-6);
            SDL_RenderDrawLine(ren,x+CELL_SIZE-6,y+5,x+5,y+CELL_SIZE-6);
            SDL_SetRenderDrawColor(ren,200,50,50,255);
            SDL_RenderDrawLine(ren,x+5,y+CELL_SIZE/2,x+CELL_SIZE-6,y+CELL_SIZE/2);
        } else if(cs==CELL_MISS){
            Fill(ren,x+1,y+1,CELL_SIZE-2,CELL_SIZE-2,SK()->miss);
            /* Small dot */
            Fill(ren,x+CELL_SIZE/2-3,y+CELL_SIZE/2-3,6,6,SK()->dim);
        } else if(cs==CELL_NEAR_SUNK){
            /* Subtle dark safe zone — darker than water */
            Fill(ren,x+1,y+1,CELL_SIZE-2,CELL_SIZE-2,SK()->near);
            /* Tiny diamond to indicate "scanned safe" */
            int mx3=x+CELL_SIZE/2, my3=y+CELL_SIZE/2;
            SDL_SetRenderDrawColor(ren,SK()->dim.r,SK()->dim.g,SK()->dim.b,180);
            SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
            SDL_RenderDrawLine(ren,mx3,my3-4,mx3+4,my3);
            SDL_RenderDrawLine(ren,mx3+4,my3,mx3,my3+4);
            SDL_RenderDrawLine(ren,mx3,my3+4,mx3-4,my3);
            SDL_RenderDrawLine(ren,mx3-4,my3,mx3,my3-4);
            SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
        }
        Box(ren,x,y,CELL_SIZE,CELL_SIZE,SK()->grid);
    }
    Box(ren,ox,oy,GRID_SIZE*CELL_SIZE,GRID_SIZE*CELL_SIZE,SK()->accent);

    /* Board dimming overlay — only player board dims when waiting for CPU */
    if(dimmed){
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
        /* Soft blue-black tint, not pure black */
        SDL_SetRenderDrawColor(ren,5,10,30,100);
        SDL_Rect dimR={ox,oy,GRID_SIZE*CELL_SIZE,GRID_SIZE*CELL_SIZE};
        SDL_RenderFillRect(ren,&dimR);
        /* Subtle "waiting" grid lines to indicate inactive state */
        SDL_SetRenderDrawColor(ren,80,100,140,25);
        for(int i=0;i<=GRID_SIZE;i++){
            SDL_RenderDrawLine(ren,ox,oy+i*CELL_SIZE,ox+GRID_SIZE*CELL_SIZE,oy+i*CELL_SIZE);
            SDL_RenderDrawLine(ren,ox+i*CELL_SIZE,oy,ox+i*CELL_SIZE,oy+GRID_SIZE*CELL_SIZE);
        }
        SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    }
}

void DrawPlacementPreview(SDL_Renderer* ren,GameBoard* b,int ox,int oy,
                           int hR,int hC,int size,bool isV){
    if(hR<0||hR>=GRID_SIZE||hC<0||hC>=GRID_SIZE) return;
    bool valid=CanPlaceShip(b,hR,hC,size,isV);
    SDL_Color col=valid?(SDL_Color){50,220,90,170}:(SDL_Color){220,50,50,170};
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    for(int i=0;i<size;i++){
        int r=isV?hR+i:hR, c=isV?hC:hC+i;
        if(r<0||r>=GRID_SIZE||c<0||c>=GRID_SIZE) break;
        SDL_SetRenderDrawColor(ren,col.r,col.g,col.b,col.a);
        SDL_Rect rc={ox+c*CELL_SIZE+1,oy+r*CELL_SIZE+1,CELL_SIZE-2,CELL_SIZE-2};
        SDL_RenderFillRect(ren,&rc);
        SDL_SetRenderDrawColor(ren,255,255,255,200); SDL_RenderDrawRect(ren,&rc);
    }
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
}

void DrawPlacementUI(SDL_Renderer* ren,TTF_Font* font,int placed,
                     Button* bRand,Button* bReset,Button* bPret,Button* bBack){
    static const int schema[10]={4,3,3,2,2,2,1,1,1,1};
    /* Panel geometry */
    int sx=E_OFFSET_X-8, sy=OFFSET_Y-30;
    int pw=GRID_SIZE*CELL_SIZE+18;
    int ph=GRID_SIZE*CELL_SIZE+150;
    /* Button area at bottom of panel — lay them out inside */
    int btn_y1 = sy+ph-120;  /* row 1: MENU | ALEATOIRE | REINITIALISER */
    int btn_y2 = sy+ph-62;   /* row 2: PRET! */
    int bw3 = (pw-24)/3;     /* width for three equal buttons in row1 */
    /* Update button rects to stay inside panel */
    bBack->rect  = (SDL_Rect){sx+6,        btn_y1, bw3,    46};
    bRand->rect  = (SDL_Rect){sx+8+bw3,    btn_y1, bw3,    46};
    bReset->rect = (SDL_Rect){sx+10+bw3*2, btn_y1, bw3,    46};
    bPret->rect  = (SDL_Rect){sx+6,        btn_y2, pw-12,  46};

    SDL_Color pb=SK()->bg;
    pb.r=(Uint8)SDL_min((int)pb.r+12,255);
    pb.g=(Uint8)SDL_min((int)pb.g+12,255);
    pb.b=(Uint8)SDL_min((int)pb.b+22,255); pb.a=255;
    Fill(ren,sx,sy,pw,ph,pb);
    Box(ren,sx,sy,pw,ph,SK()->accent);

    /* Title */
    DrawText(ren,font,"FLOTTE A PLACER",sx+45,sy+7,SK()->text);
    SDL_Color ac=SK()->accent;
    SDL_SetRenderDrawColor(ren,ac.r,ac.g,ac.b,255);
    SDL_RenderDrawLine(ren,sx+6,sy+36,sx+pw-6,sy+36);

    /* Ship list */
    int iy=OFFSET_Y;
    for(int i=0;i<10;i++){
        SDL_Color col;
        if(i<placed)       col=(SDL_Color){30,120,50,255};
        else if(i==placed) col=(SDL_Color){220,190,30,255};
        else               col=SK()->dim;
        for(int j=0;j<schema[i];j++){
            int cx=sx+12+j*(CELL_SIZE-4);
            Fill(ren,cx,iy+2,CELL_SIZE-5,CELL_SIZE-6,col);
            Box(ren, cx,iy+2,CELL_SIZE-5,CELL_SIZE-6,(SDL_Color){200,210,230,255});
        }
        if(i<placed)
            DrawText(ren,font,"OK",sx+14+schema[i]*(CELL_SIZE-4),iy+6,(SDL_Color){60,220,80,255});
        iy+=CELL_SIZE;
    }

    /* Hint text */
    DrawText(ren,font,"Clic droit souris = rotation",sx+200,btn_y1-50,SK()->dim);

    /* Buttons */
    DrawButton(ren,font,bBack);
    DrawButton(ren,font,bRand);
    DrawButton(ren,font,bReset);
    if(placed>=10) DrawButton(ren,font,bPret);
    else {
        /* Show greyed-out PRET hint */
        SDL_Color gc={(Uint8)(bPret->color.r/2),(Uint8)(bPret->color.g/2),(Uint8)(bPret->color.b/2),255};
        Button ghost=*bPret; ghost.color=gc; ghost.isHovered=false;
        DrawButton(ren,font,&ghost);
        /* Overlay text */
        char buf[32]; 
        snprintf(buf,sizeof(buf)," ");
        int tw=SDL_max(1,bPret->rect.w);
        DrawText(ren,font,buf,bPret->rect.x+tw/2-80,bPret->rect.y+13,SK()->dim);
    }
}

void DrawGameUI(SDL_Renderer* ren,TTF_Font* font,GameBoard* p,GameBoard* e,
                bool playerTurn,int moves,Uint32 startTick,int coins){
    DrawText(ren,font,"VOTRE FLOTTE",  P_OFFSET_X+50,8,SK()->dim);
    DrawText(ren,font,"FLOTTE ENNEMIE",E_OFFSET_X+35,8,SK()->dim);
    const char* msg=playerTurn?">>> VOTRE TOUR <<<":">>>  IA EN JEU  <<<";
    SDL_Color mc=playerTurn?(SDL_Color){60,215,80,255}:(SDL_Color){220,60,60,255};
    int bw=310,bx=WINDOW_W/2-bw/2,by=30;
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,mc.r,mc.g,mc.b,22);
    SDL_Rect bp={bx,by,bw,32}; SDL_RenderFillRect(ren,&bp);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    Box(ren,bx,by,bw,32,mc);
    DrawText(ren,font,msg,bx+26,by+6,mc);
    char buf[48];
    int el=startTick?(int)((SDL_GetTicks()-startTick)/1000):0;
    snprintf(buf,sizeof(buf),"Coups: %d   %02d:%02d",moves,el/60,el%60);
    DrawText(ren,font,buf,WINDOW_W/2-90,WINDOW_H-44,SK()->dim);
    DrawCoins(ren,font,coins,WINDOW_W-165,WINDOW_H-50);
    int by2=OFFSET_Y+GRID_SIZE*CELL_SIZE+10;
    char pb2[24],eb[24];
    snprintf(pb2,sizeof(pb2),"Vie: %d",p->shipsLeft);
    snprintf(eb, sizeof(eb),"Vie: %d",e->shipsLeft);
    DrawText(ren,font,pb2,P_OFFSET_X+60,by2,(SDL_Color){60,210,80,255});
    DrawText(ren,font,eb, E_OFFSET_X+60,by2,(SDL_Color){220,60,60,255});
    int mhp=20,pw2=p->shipsLeft*220/mhp,ew2=e->shipsLeft*220/mhp,by3=by2+28;
    Fill(ren,P_OFFSET_X+10,by3,220,12,SK()->water);
    Fill(ren,P_OFFSET_X+10,by3,pw2,12,(SDL_Color){50,200,70,255});
    Box(ren, P_OFFSET_X+10,by3,220,12,(SDL_Color){60,160,80,255});
    Fill(ren,E_OFFSET_X+10,by3,220,12,SK()->water);
    Fill(ren,E_OFFSET_X+10,by3,ew2,12,(SDL_Color){210,50,50,255});
    Box(ren, E_OFFSET_X+10,by3,220,12,(SDL_Color){160,60,60,255});
}

void DrawGameOver(SDL_Renderer* ren,TTF_Font* font,bool win,int moves,
                  int coinsEarned,Button* btnR,Button* btnM){
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,0,0,0,185);
    SDL_Rect ov={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(ren,&ov);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    int px=WINDOW_W/2-270,py=WINDOW_H/2-165;
    Fill(ren,px,py,540,350,SK()->bg); Box(ren,px,py,540,350,SK()->accent);
    SDL_Color tc=win?(SDL_Color){60,215,90,255}:(SDL_Color){220,60,60,255};
    DrawText(ren,font,win?"  VICTOIRE !":"  DEFAITE...",WINDOW_W/2-100,py+18,tc);
    char buf[64];
    if(win){snprintf(buf,sizeof(buf),"Coule en %d coups !",moves);
            DrawText(ren,font,buf,px+90,py+72,SK()->text);}
    else DrawText(ren,font,"Votre flotte a ete coulée...",px+50,py+72,SK()->text);
    if(coinsEarned>0){
        snprintf(buf,sizeof(buf),"+ %d pièces gagnées !",coinsEarned);
        DrawText(ren,font,buf,px+90,py+110,(SDL_Color){220,180,40,255});
    }
    DrawText(ren,font,"ESPACE = rejouer",px+140,py+155,SK()->dim);
    DrawButton(ren,font,btnR); DrawButton(ren,font,btnM);
}

void DrawRecords(SDL_Renderer* ren,TTF_Font* font,GameStats* s,Button* btnBack){
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren,0,0,0,195);
    SDL_Rect ov={0,0,WINDOW_W,WINDOW_H}; SDL_RenderFillRect(ren,&ov);
    SDL_SetRenderDrawBlendMode(ren,SDL_BLENDMODE_NONE);
    int px=WINDOW_W/2-260,py=WINDOW_H/2-190;
    Fill(ren,px,py,520,400,SK()->bg); Box(ren,px,py,520,400,SK()->accent);
    DrawText(ren,font,"TABLEAU DES RECORDS",px+100,py+15,SK()->text);
    SDL_Color ac=SK()->accent;
    SDL_SetRenderDrawColor(ren,ac.r,ac.g,ac.b,255);
    SDL_RenderDrawLine(ren,px+10,py+55,px+510,py+55);
    char buf[64];
    snprintf(buf,sizeof(buf),"Victoires :  %d",s->wins);
    DrawText(ren,font,buf,px+70,py+80,(SDL_Color){60,210,80,255});
    snprintf(buf,sizeof(buf),"Defaites  :  %d",s->losses);
    DrawText(ren,font,buf,px+70,py+130,(SDL_Color){220,60,60,255});
    int tot=s->wins+s->losses;
    if(tot>0){snprintf(buf,sizeof(buf),"Ratio     :  %d%%",s->wins*100/tot);
              DrawText(ren,font,buf,px+70,py+180,SK()->text);}
    if(s->bestMoves<9999){snprintf(buf,sizeof(buf),"Meilleur  :  %d coups",s->bestMoves);
                          DrawText(ren,font,buf,px+70,py+230,SK()->accent);}
    else DrawText(ren,font,"Meilleur  :  ---",px+70,py+210,SK()->dim);
    snprintf(buf,sizeof(buf),"Pieces    :  %d",s->coins);
    DrawText(ren,font,buf,px+70,py+280,(SDL_Color){220,180,40,255});
    DrawButton(ren,font,btnBack);
}
