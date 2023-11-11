
#include <stdio.h>
#include <stdlib.h>
#include <pd_api.h>
#include "cursor.h"
#include "defines.h"
#include "Functions.h"
#include "pd_helperfuncs.h"
#include "SDL_HelperTypes.h"
#include "commonvars.h"

CCursor::CCursor(PlaydateAPI *pd_api) {
    pda = pd_api;
	PSurface = loadImageAtPath("graphics/cursor");
    MaxAnims = 4;
    NewFrame = 2;
    AnimCounter= 0;
    Anim = 0;
}

CCursor::~CCursor(void) {
    if(PSurface)
        pda->graphics->freeBitmap(PSurface);
}

void CCursor::Draw(LCDBitmap *Surface) {
    SDL_Rect Dst,Src;

    Dst.x = Px * TileWidth;
    Dst.y = Py * TileHeight;
    Dst.w = TileWidth;
    Dst.h = TileHeight;
    Src.x = Anim * TileWidth;
    Src.y = 0;
    Src.w = TileWidth;
    Src.h = TileHeight;
    AnimCounter++;
    if(AnimCounter>NewFrame)
    {
        AnimCounter = 0;
        Anim++;
        if(Anim >= MaxAnims)
            Anim = 0;
    }
	pda->graphics->pushContext(Surface);
	DrawBitmapSrcRec(PSurface, Dst.x, Dst.y, Src.x, Src.y, Src.w, Src.h, kBitmapUnflipped);
	pda->graphics->popContext();
}
