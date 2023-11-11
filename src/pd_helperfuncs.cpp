#include <string.h>
#include <pd_api.h>
#include "SDL_HelperTypes.h"
#include "pd_helperfuncs.h"


PlaydateAPI* pd;
LCDBitmap *Buffer = NULL;

typedef struct DrawTextColorBitmapCacheItem DrawTextColorBitmapCacheItem;
struct DrawTextColorBitmapCacheItem
{
	char Text[DrawTextColorBitmapCacheMaxTextSize+1];
	LCDBitmap* Bitmap;
	LCDColor color;
	LCDFont *font;
};

DrawTextColorBitmapCacheItem DrawTextColorBitmapCache[DrawTextColorBitmapCacheCount];
int DrawTextColorBitmapCacheItems = 0;
long int DrawTextColorBitmapCacheMisses = 0;

LCDPattern kColorGrey = {
	// Bitmap
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,
	0b10101010,
	0b01010101,

	// Mask
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
};

void drawTextColor(bool IgnoreBitmapContext, LCDBitmap* BitmapContext, LCDFont* font, const char* text, size_t len, PDStringEncoding encoding, int x, int y, LCDColor color, bool inverted)
{
	//grab width & height of our to be rendered text
	if (!IgnoreBitmapContext)
		pd->graphics->pushContext(BitmapContext);
	
	//check if we previously drawn this text
	int bcacheditemindex = -1;
	for (int i = 0; i < DrawTextColorBitmapCacheItems; i++)
	{
		if ((strcmp(DrawTextColorBitmapCache[i].Text, text) == 0) && 
			(DrawTextColorBitmapCache[i].color == color) &&
			(DrawTextColorBitmapCache[i].font == font))
		{
			bcacheditemindex = i;
			break;
		}
	}
	
	//text & bitmap was found in cache draw from cache
	if (bcacheditemindex != -1)
	{
		pd->graphics->drawBitmap(DrawTextColorBitmapCache[bcacheditemindex].Bitmap, x, y, kBitmapUnflipped);
	}
	else
	{		
		//not found in check, if we have max items erase the 1st item we had used
		//and move the others down
		if (DrawTextColorBitmapCacheItems == DrawTextColorBitmapCacheCount)
		{
			pd->graphics->freeBitmap(DrawTextColorBitmapCache[0].Bitmap);
			for (int i = 1 ; i < DrawTextColorBitmapCacheCount; i++)
			{
				DrawTextColorBitmapCache[i - 1] = DrawTextColorBitmapCache[i];
			}
			DrawTextColorBitmapCacheItems--;
		}
		int Lines = 1;
		size_t chars = 0;
		const char* p = text;
		while ((*p != '\0') && (chars < len))
		{
			if (*p == '\n')
				Lines++;
			p++;
			chars++;
		}

		int h = Lines * pd->graphics->getFontHeight(font);
		pd->graphics->setFont(font);
		int w = pd->graphics->getTextWidth(font, text, len, encoding, 0);
		//create new bitmap and fillrect with our color / pattern
		DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Bitmap = pd->graphics->newBitmap(w, h, kColorClear);
		if (inverted)
			pd->graphics->setDrawMode(kDrawModeInverted);
		pd->graphics->pushContext(DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Bitmap);
		pd->graphics->fillRect(0, 0, w, h, color);
		pd->graphics->popContext();

		//create mask with black background and draw text in white on the mask 
		LCDBitmap* bitmapmask = pd->graphics->newBitmap(w,h, kColorBlack);
		pd->graphics->pushContext(bitmapmask);
		pd->graphics->setDrawMode(kDrawModeFillWhite);
		pd->graphics->setFont(font);
		pd->graphics->drawText(text, len, encoding, 0, 0);
		pd->graphics->popContext();

		//set the mask to the bitmap with our pattern, this will make sure only the text
		//part (white in mask is drawn from the bitmap)
		pd->graphics->setBitmapMask(DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Bitmap, bitmapmask);
		pd->graphics->freeBitmap(bitmapmask);
		//now draw the bitmap containing our text to the x & y position
		pd->graphics->drawBitmap(DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Bitmap, x, y, kBitmapUnflipped);
		//ignore debug text for the cache
		if (strstr(text, "DTC Misses") == NULL)
		{
			if (DrawTextColorBitmapCacheItems > 0)
				DrawTextColorBitmapCacheMisses++;
			DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].color = color;
			DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].font = font;
			size_t textlen = (len < DrawTextColorBitmapCacheMaxTextSize ? len : DrawTextColorBitmapCacheMaxTextSize);
			strncpy(DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Text, text, textlen);
			DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Text[textlen] = '\0';
			DrawTextColorBitmapCacheItems++;
			if (textlen < len)
				pd->system->logToConsole("Warning DrawTextColorBitmapCache text truncated given size:%d maxsize:%d", len, textlen);
		}
		else
			pd->graphics->freeBitmap(DrawTextColorBitmapCache[DrawTextColorBitmapCacheItems].Bitmap);
	}

	if (!IgnoreBitmapContext)
		pd->graphics->popContext();
}


LCDBitmap* loadImageAtPath(const char* path)
{
	const char* outErr = NULL;
	LCDBitmap* img = pd->graphics->loadBitmap(path, &outErr);
	return img;
}

LCDBitmapTable* loadBitmapTableAtPath(const char* path)
{
	const char* outErr = NULL;
	LCDBitmapTable* table = pd->graphics->loadBitmapTable(path, &outErr);
	return table;
}

LCDFont* loadFontAtPath(const char* path)
{
	const char* outErr = NULL;
	LCDFont* fnt = pd->graphics->loadFont(path, &outErr);
	return fnt;
}

void DrawBitmapSrcRec(LCDBitmap* Bitmap, int dstX, int dstY, int srcX, int srcY, int srcW, int srcH, LCDBitmapFlip FlipMode)
{
	pd->graphics->setClipRect(dstX, dstY, srcW, srcH);
	pd->graphics->setDrawOffset((-srcX) + dstX, (-srcY) + dstY);
	pd->graphics->drawBitmap(Bitmap, 0, 0, FlipMode);
	pd->graphics->setDrawOffset(0, 0);
	pd->graphics->clearClipRect();
}

void DrawBitmapScaledSrcRec(LCDBitmap* Bitmap, float scalex, float scaley, int dstX, int dstY, int srcX, int srcY, int srcW, int srcH)
{
	pd->graphics->setClipRect(dstX, dstY, (int)(srcW * scalex), (int)(srcH * scaley));
	pd->graphics->setDrawOffset((int)(-srcX * scalex) + dstX, (int)(-srcY * scaley) + dstY);
	pd->graphics->drawScaledBitmap(Bitmap, 0, 0, scalex, scaley);
	pd->graphics->setDrawOffset(0, 0);
	pd->graphics->clearClipRect();
}

unsigned int logPower(const char* filename, unsigned int logIntervalSeconds, unsigned int prevLogTime)
{
	unsigned int s = pd->system->getSecondsSinceEpoch(NULL);
	if(s - prevLogTime >= logIntervalSeconds)
	{
		float p = pd->system->getBatteryPercentage();
		float v = pd->system->getBatteryVoltage();
		SDFile* file = pd->file->open(filename, kFileAppend);
		if (file)
		{
			char* line;
			pd->system->formatString(&line, "%d, %f, %f\n", s, p, v);
			pd->file->write(file, line, (unsigned int)strlen(line));
			pd->system->realloc(line, 0);
			//simulator crashes on windows when calling flush
#ifndef _WIN32
			pd->file->flush(file);
#endif
			pd->file->close(file);
		}
		return s;
	}
	return prevLogTime;
}


void setPDPtr(PlaydateAPI* playdate)
{
	pd = playdate;
}

void pdDelay(unsigned int milliseconds)
{
	unsigned int start = pd->system->getCurrentTimeMilliseconds();
	while (start + milliseconds > pd->system->getCurrentTimeMilliseconds());
}

bool pdFileExists(char* Path)
{
	bool result = false;
	SDFile* File = pd->file->open(Path, kFileRead);
	if (File)
	{
		result = true;
		pd->file->close(File);
	}

	return result;
}