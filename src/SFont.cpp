/*  SFont: a simple font-library that uses special .pngs as fonts
    Copyright (C) 2003 Karl Bartel

    License: GPL or LGPL (at your choice)
    WWW: http://www.linux-games.com/sfont/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Karl Bartel
    Cecilienstr. 14
    12307 Berlin
    GERMANY
    karlb@gmx.net
*/

//playdate api conversion by joyrider3774 on 29/09/2023

#include <pd_api.h>
#include <assert.h>
#include <stdlib.h>
#include "SFont.h"
#include "SDL_HelperTypes.h"

PlaydateAPI *spd;

void SFont_SetPdApi(PlaydateAPI *pd_api)
{
	spd = pd_api;
}

SFont_Font* SFont_InitFont(LCDBitmap* Surface)
{
    int x = 0, i = 0;

    SFont_Font* Font;

    if (Surface == NULL)
	return NULL;

    Font = (SFont_Font *) spd->system->realloc(NULL, sizeof(SFont_Font));
    Font->Surface = Surface;

	int rowbytes;
	uint8_t *data;
	spd->graphics->getBitmapData(Surface, &Font->sw, &Font->sh, &rowbytes, &data, NULL);

	spd->system->logToConsole("%rb:%d, 1st:%d 2nd:%d", rowbytes, data[0], data[rowbytes]);
    while (x < Font->sw) {
	if (((data[(0)*rowbytes+(x)/8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0)) {
    	    Font->CharPos[i++]=x;
    	    while((x < Font->sw) && (((data[(0)*rowbytes+(x)/8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0)))
		x++;
	    Font->CharPos[i++]=x;
	}
	x++;
    }
    Font->MaxPos = x-1;

    return Font;
}

void SFont_FreeFont(SFont_Font* FontInfo)
{
    spd->graphics->freeBitmap(FontInfo->Surface);
    free(FontInfo);
}

void SFont_Write(LCDBitmap *Surface, const SFont_Font *Font,
		 int x, int y, const char *text)
{
    const char* c;
    int charoffset;
    SDL_Rect srcrect, dstrect;

    if(text == NULL)
	return;

    // these values won't change in the loop
    srcrect.y = 1;
    dstrect.y = y;
    srcrect.h = dstrect.h = Font->sh - 1;

	int w;
	spd->graphics->getBitmapData(Surface, &w, NULL, NULL, NULL, NULL);

    for(c = text; *c != '\0' && x <= w ; c++) {
	charoffset = ((int) (*c - 33)) * 2 + 1;
	// skip spaces and nonprintable characters
	if (*c == ' ' || charoffset < 0 || charoffset > Font->MaxPos) {
	    x += Font->CharPos[2]-Font->CharPos[1];
	    continue;
	}

	srcrect.w = dstrect.w =
	    (Font->CharPos[charoffset+2] + Font->CharPos[charoffset+1])/2 -
	    (Font->CharPos[charoffset] + Font->CharPos[charoffset-1])/2;
	srcrect.x = (Font->CharPos[charoffset]+Font->CharPos[charoffset-1])/2;
	dstrect.x = x - (float)(Font->CharPos[charoffset]
			      - Font->CharPos[charoffset-1])/2;

	spd->graphics->pushContext(Surface);
	spd->graphics->setClipRect(dstrect.x, dstrect.y, srcrect.w, srcrect.h);
	spd->graphics->setDrawOffset((-srcrect.x) + dstrect.x, (-srcrect.y) + dstrect.y);
	spd->graphics->drawBitmap(Font->Surface, 0, 0, kBitmapUnflipped);
	spd->graphics->setDrawOffset(0, 0);
	spd->graphics->clearClipRect();
	spd->graphics->popContext();

	x += Font->CharPos[charoffset+1] - Font->CharPos[charoffset];
    }
}

int SFont_TextWidth(const SFont_Font *Font, const char *text)
{
    const char* c;
    int charoffset=0;
    int width = 0;

    if(text == NULL)
	return 0;

    for(c = text; *c != '\0'; c++) {
	charoffset = ((int) *c - 33) * 2 + 1;
	// skip spaces and nonprintable characters
        if (*c == ' ' || charoffset < 0 || charoffset > Font->MaxPos) {
            width += Font->CharPos[2]-Font->CharPos[1];
	    continue;
	}

	width += Font->CharPos[charoffset+1] - Font->CharPos[charoffset];
    }

    return width;
}

int SFont_TextHeight(const SFont_Font* Font)
{
    return Font->sh - 1;
}

void SFont_WriteCenter(LCDBitmap *Surface, const SFont_Font *Font,
		       int y, const char *text)
{
    int w;
	spd->graphics->getBitmapData(Surface, &w, NULL, NULL, NULL, NULL);
	SFont_Write(Surface, Font, w/2 - SFont_TextWidth(Font, text)/2, y, text);
}

