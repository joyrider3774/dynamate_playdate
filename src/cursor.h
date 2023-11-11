#ifndef CURSOR_H
#define CURSOR_H
#include "defines.h"

extern int WINDOW_WIDTH,WINDOW_HEIGHT,TileWidth,TileHeight;

class CCursor {
    public:
        CCursor(PlaydateAPI *pd_api);
        ~CCursor(void);
        void Draw(LCDBitmap *Surface);
        int GetX(void) { return Px;};
        int GetY(void) { return Py;};
        void SetX(int x) {if(x < 0) Px = TILECOLUMNS -1; else if (x >= TILECOLUMNS) Px = 0; else Px = x;};
        void SetY(int y) {if( y < 0) Py = TILEROWS-1; else if (y >= TILEROWS) Py = 0; else Py = y;};
        void SetXY(int x, int y) { Px=x;Py=y;};
    private:
       int Px,Py,AnimCounter;
       LCDBitmap *PSurface;
	   PlaydateAPI *pda;
       int MaxAnims, NewFrame,Anim;
};

#endif
