#ifndef SDL2API
#include <pdcpp/pdnewlib.h>
#endif
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <pd_api.h>
#include "dynamate.h"
#include "dynamate_base.h"
#include "dynamate_levels.h"
#include "defines.h"
#include "cursor.h"
#include "CInput.h"
#include "Functions.h"
#include "pd_helperfuncs.h"
#include "SDL_HelperTypes.h"
#include "commonvars.h"
#include "CAudio.h"

void setNextGameState(int nextState, eFadeType fadeType, bool useWhite)
{
	if(NextGameState == -1)
	{
		NextGameState = nextState;
		startFade(fadeType, useWhite, 0.075f);	
		Input->Disable();
	}
}

bool HandleNextGameState()
{
	if(NextGameState == -1)
	{
		Input->Enable();
		return true;
	}

	if(handleFade() == fadeNone)
	{
		GameState = NextGameState;
		Input->Enable();
		NextGameState = -1;
		return true;
	}

	return false;
}

void LoadHighScores()
{
	SDFile *File;
	int Teller;
	File = pd->file->open("high.dat", kFileReadData);
	if (File)
	{
		for (Teller = 0; Teller < MAXSTATICLEVELS; Teller++)
			pd->file->read(File, &HighScores[Teller], sizeof(int));
		pd->file->close(File);
	}
	else
	{
		for (Teller = 0; Teller < MAXSTATICLEVELS; Teller++)
			HighScores[Teller] = 0;
	}
}

void SaveHighScores()
{
	SDFile * File;
	int Teller;
	File = pd->file->open("high.dat", kFileWrite);
	if (File)
	{
		for (Teller = 0; Teller < MAXSTATICLEVELS; Teller++)
			pd->file->write(File, &HighScores[Teller], sizeof(int));
		pd->file->close(File);
	}
}

void LoadSounds()
{
	Sounds[SND_MENU] = CAudio_LoadSound("menu");
	Sounds[SND_SELECT] = CAudio_LoadSound("select");
	Sounds[SND_ERROR] = CAudio_LoadSound("error");

	Sounds[SND_EXPLODE] = CAudio_LoadSound("explode");
	Sounds[SND_MOVE] = CAudio_LoadSound("move");
	Sounds[SND_TELEPORT] = CAudio_LoadSound("teleport");
	Sounds[SND_LEVELPASS] = CAudio_LoadSound("passlevel");
	Sounds[SND_LEVELFAIL] = CAudio_LoadSound("fail");

	Music[0] = CAudio_LoadMusic("title");
}

void UnLoadSounds()
{
	CAudio_StopSound();
	CAudio_StopMusic();
	int Teller;
	for (Teller = 0; Teller < NROFSOUNDS; Teller++)
 		CAudio_UnLoadSound(Sounds[Teller]);
		
	CAudio_UnLoadMusic(Music[0]);
}

dm_u8 NextPiece(dm_u8 piece)
{
	switch (piece)
	{
		case DM_P_EMPTY:
			return (0x80 | DM_P_VERT);
			//case S_GLUE3		:
			//case S_GLUE5		:
		case (0x80 | DM_P_VERT):
			return (0x80 | DM_P_HORI);
		case (0x80 | DM_P_HORI):
			return (0x80 | DM_P_TELE1);
		case (0x80 | DM_P_TELE1):
			return (0x80 | DM_P_TELE2);
		case (0x80 | DM_P_TELE2):
			return (0x80 | DM_P_ROTCOL);
		case (0x80 | DM_P_ROTCOL):
			return (0x80 | DM_P_ROT_CW);
		case (0x80 | DM_P_ROT_CW):
			return (0x80 | DM_P_ROT_CCW);
		case (0x80 | DM_P_ROT_CCW):
			return (0x80 | DM_P_GRAY);
		case (0x80 | DM_P_GRAY):
			return (0x80 | DM_P_DIE);
		case (0x80 | DM_P_DIE):
			return (0x80 | DM_P_BLACK);
		case (0x80 | DM_P_BLACK):
			return (0x80 | DM_P_YELLOW);
		case (0x80 | DM_P_YELLOW):
			return (0x80 | DM_P_RED);
		case (0x80 | DM_P_RED):
			return (0x80 | DM_P_GREEN);
		case (0x80 | DM_P_GREEN):
			return (0x80 | DM_P_BLUE);
		case (0x80 | DM_P_BLUE):
			return DM_P_VERT;

			//case M_GLUE3		:
			//case M_GLUE5		:
		case DM_P_VERT:
			return DM_P_HORI;
		case DM_P_HORI:
			return DM_P_TELE1;
		case DM_P_TELE1:
			return DM_P_TELE2;
		case DM_P_TELE2:
			return DM_P_ROTCOL;
		case DM_P_ROTCOL:
			return DM_P_ROT_CW;
		case DM_P_ROT_CW:
			return DM_P_ROT_CCW;
		case DM_P_ROT_CCW:
			return DM_P_GRAY;
		case DM_P_GRAY:
			return DM_P_DIE;
		case DM_P_DIE:
			return DM_P_BLACK;
		case DM_P_BLACK:
			return DM_P_YELLOW;
		case DM_P_YELLOW:
			return DM_P_RED;
		case DM_P_RED:
			return DM_P_GREEN;
		case DM_P_GREEN:
			return DM_P_BLUE;
		case DM_P_BLUE:
			return (DM_P_EMPTY);

			//case S_GLUE0		:
			//case M_GLUE0		:

		default:
			return DM_P_EMPTY;
	}
}

dm_u8 PreviousPiece(dm_u8 piece)
{
	dm_u8 prevPiece, newPiece = piece;

	do {
		prevPiece = newPiece;
		newPiece = NextPiece(newPiece);
	} while (newPiece != piece);

	return prevPiece;
}

void DrawPieces(LCDBitmap *Surface)
{	
	int x, y, piecenr;
	SDL_Rect Src, Dst;
	dm_u8 TmpPiece = NextPiece(DM_P_EMPTY);
	for (y = 0; y < 6; y++)
	{
		for (x = 0; x < 5; x++)
		{
			switch (L7(TmpPiece))
			{
				case DM_P_RED:
					if (SOLID(TmpPiece))
						piecenr = 14;
					else
						piecenr = 30;
					break;
				case DM_P_BLUE:
					if (SOLID(TmpPiece))
						piecenr = 16;
					else
						piecenr = 32;
					break;
				case DM_P_GREEN:
					if (SOLID(TmpPiece))
						piecenr = 15;
					else
						piecenr = 31;
					break;
				case DM_P_YELLOW:
					if (SOLID(TmpPiece))
						piecenr = 13;
					else
						piecenr = 29;
					break;

				case DM_P_BLACK:
					if (SOLID(TmpPiece))
						piecenr = 12;
					else
						piecenr = 28;
					break;

				case DM_P_HORI:
					if (SOLID(TmpPiece))
						piecenr = 4;
					else
						piecenr = 20;
					break;

				case DM_P_VERT:
					if (SOLID(TmpPiece))
						piecenr = 3;
					else
						piecenr = 19;
					break;

				case DM_P_TELE1:
					if (SOLID(TmpPiece))
						piecenr = 5;
					else
						piecenr = 21;
					break;

				case DM_P_TELE2:
					if (SOLID(TmpPiece))
						piecenr = 6;
					else
						piecenr = 22;
					break;

				case DM_P_ROTCOL:
					if (SOLID(TmpPiece))
						piecenr = 7;
					else
						piecenr = 23;
					break;

				case DM_P_ROT_CW:
					if (SOLID(TmpPiece))
						piecenr = 8;
					else
						piecenr = 24;
					break;

				case DM_P_ROT_CCW:
					if (SOLID(TmpPiece))
						piecenr = 9;
					else
						piecenr = 25;
					break;

				case DM_P_GRAY:
					if (SOLID(TmpPiece))
						piecenr = 10;
					else
						piecenr = 26;
					break;

				case DM_P_DIE:
					if (SOLID(TmpPiece))
						piecenr = 11;
					else
						piecenr = 27;
					break;
				case DM_P_EMPTY:
					piecenr = 1;
					break;
				default:
					piecenr = 0;
					break;
			}

			if (piecenr != 0)
			{
				pd->graphics->pushContext(Surface);
				if(piecenr != 1)
				{
					Src.x = 0;
					Src.y = piecenr * TileHeight;
					Src.w = TileWidth;
					Src.h = TileHeight;
					Dst.x = (x *TileWidth) + (243);
					Dst.y = (y *TileHeight) + (150);
					Dst.w = TileWidth;
					Dst.h = TileHeight;
					pd->graphics->pushContext(Surface);
					DrawBitmapSrcRec(LevelData, Dst.x, Dst.y, Src.x, Src.y, Src.w, Src.h, kBitmapUnflipped);
				}
				if (SelectedPiece == TmpPiece)
				{
					pd->graphics->drawRect((x *TileWidth) + (243)-1, (y *TileHeight) + (150)-1, TileWidth +2, TileHeight +2, kColorBlack);
				}
				pd->graphics->popContext();
			}

			TmpPiece = NextPiece(TmpPiece);
			if (((y) *5) + (x) >= 28)
				break;
		}

		if (((y) *5) + (x) >= 28)
			break;
	}
}

void DrawLevel(LCDBitmap *Surface)
{
	int x, y, tmp, piecenr;
	SDL_Rect Src, Dst;
	for (y = 0; y < TILEROWS; y++)
	{
		for (x = 0; x < TILECOLUMNS; x++)
		{
			tmp = dm_field()[(y *TILEROWS) + x];
			switch (L7(tmp))
			{
				case DM_P_RED:
					if (SOLID(tmp))
						piecenr = 14;
					else
						piecenr = 30;
					break;
				case DM_P_BLUE:
					if (SOLID(tmp))
						piecenr = 16;
					else
						piecenr = 32;
					break;
				case DM_P_GREEN:
					if (SOLID(tmp))
						piecenr = 15;
					else
						piecenr = 31;
					break;
				case DM_P_YELLOW:
					if (SOLID(tmp))
						piecenr = 13;
					else
						piecenr = 29;
					break;

				case DM_P_BLACK:
					if (SOLID(tmp))
						piecenr = 12;
					else
						piecenr = 28;
					break;

				case DM_P_HORI:
					if (SOLID(tmp))
						piecenr = 4;
					else
						piecenr = 20;
					break;

				case DM_P_VERT:
					if (SOLID(tmp))
						piecenr = 3;
					else
						piecenr = 19;
					break;

				case DM_P_TELE1:
					if (SOLID(tmp))
						piecenr = 5;
					else
						piecenr = 21;
					break;

				case DM_P_TELE2:
					if (SOLID(tmp))
						piecenr = 6;
					else
						piecenr = 22;
					break;

				case DM_P_ROTCOL:
					if (SOLID(tmp))
						piecenr = 7;
					else
						piecenr = 23;
					break;

				case DM_P_ROT_CW:
					if (SOLID(tmp))
						piecenr = 8;
					else
						piecenr = 24;
					break;

				case DM_P_ROT_CCW:
					if (SOLID(tmp))
						piecenr = 9;
					else
						piecenr = 25;
					break;

				case DM_P_GRAY:
					if (SOLID(tmp))
						piecenr = 10;
					else
						piecenr = 26;
					break;

				case DM_P_DIE:
					if (SOLID(tmp))
						piecenr = 11;
					else
						piecenr = 27;
					break;

				default:
					piecenr = 0;
					break;
			}

			if (piecenr != 0)
			{
				Src.x = 0;
				Src.y = piecenr * TileHeight;
				Src.w = TileWidth;
				Src.h = TileHeight;
				Dst.x = x * TileWidth;
				Dst.y = y * TileHeight;
				Dst.w = TileWidth;
				Dst.h = TileHeight;
				pd->graphics->pushContext(Surface);
				DrawBitmapSrcRec(LevelData, Dst.x, Dst.y, Src.x, Src.y, Src.w, Src.h, kBitmapUnflipped);
				pd->graphics->popContext();
			}
		}
	}
}

void LoadLevel(char *FileName)
{
	SDFile * f;
	f = pd->file->open(FileName, kFileReadData);
	if(!f)
		f = pd->file->open(FileName, kFileRead);
	if (f)

	{
		pd->file->read(f, &UserLevel, sizeof(dm_user_lev));
		pd->file->close(f);
		dm_init_level(UserLevel.field);
	}
}

void SaveLevel1(char *FileName)
{
	SDFile *f;
	f = pd->file->open(FileName, kFileWrite);
	if (f)
	{
		pd->file->write(f, &UserLevel, sizeof(dm_user_lev));
		pd->file->close(f);
	}
}

void SaveLevel(char *FileName)
{
	dm_user_lev tmp;
	SDFile * f;
	memset(tmp.hs, 0, 256);
	memcpy(&tmp.field, dm_field(), 256);
	strcpy(tmp.name, UserLevel.name);
	f = pd->file->open(FileName, kFileWrite);
	if (f)
	{
		pd->file->write(f, &tmp, sizeof(dm_user_lev));
		pd->file->close(f);
	}
}

void GetName(char *Text)
{
	strcpy(GetNameText, Text);
	GetNameResult = false;
	GetNameDone = false;
	GameState = GSGETNAMEINIT;
}

bool doGetNameInit()
{
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->SetUpdateDelay(3);
	Input->Reset();
	GetNameX = 0;
	GetNameY = 0;
	return true;
}

void doGetName()
{
	int x1, y1;
	LCDBitmap * Tmp1;
	if (GameState == GSGETNAMEINIT)
	{
		if(doGetNameInit())
			GameState -= GSINITDIFF;
	}

	if(GameState == GSGETNAME)
	{

		if (Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSLEVELEDITORMENUINIT;
			GetNameDone = true;
			GetNameResult = false;
		}


		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a]))
		{
			if (strlen(GetNameText) < 11)
			{
				if (((GetNameY *11) + GetNameX) < 65)
				{
					int l = strlen(GetNameText);
					if (l < 10)
					{
						GetNameText[l] = NameSymbols[(GetNameY *11) + GetNameX];
						GetNameText[l+1] = '\0';
					}
				}
				else
				{
					if (((GetNameY *11) + GetNameX) == 65)
					{
						pd->system->logToConsole("bleh");
						if (strlen(GetNameText) > 0)
						{
							GetNameText[strlen(GetNameText)-1] = '\0';
						}
					}
					else
					{
						if ((GetNameY == 6) && (GetNameX == 0))
						{
							if ((strlen(GetNameText) > 0) && GetNameText[0] != ' ')
							{
								CAudio_PlaySound(Sounds[SND_SELECT], 0);
								GameState = GSLEVELEDITORMENUINIT;
								GetNameResult = true;
								GetNameDone = true;
							}
						}
						else
						{
							CAudio_PlaySound(Sounds[SND_SELECT], 0);
							GameState = GSLEVELEDITORMENUINIT;
							GetNameResult = false;
							GetNameDone = true;
						}
					}
				}
			}

			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_RIGHT]))
		{
			GetNameX++;
			if ((GetNameY >= 0) && (GetNameY <= 5))
			{
				if (GetNameX > 10)
					GetNameX = 0;
			}
			else
			{
				if (GetNameX > 1)
					GetNameX = 0;
			}

			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_LEFT]))
		{
			GetNameX--;
			if ((GetNameY >= 0) && (GetNameY <= 5))
			{
				if (GetNameX < 0)
					GetNameX = 10;
			}
			else
			if (GetNameX < 0)
				GetNameX = 1;
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			if (GetNameY == 6)
				GetNameX = 0;
			GetNameY--;
			if (GetNameY < 0)
				GetNameY = 6;
			if (GetNameY == 6)
				GetNameX = 0;
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			if (GetNameY == 5)
				GetNameX = 0;
			GetNameY++;
			if (GetNameY > 6)
			{
				GetNameY = 0;
				GetNameX = 0;
			}

			Input->Delay();
		}
	}

	pd->graphics->drawBitmap(BackgroundLevelEditor, 0, 0, kBitmapUnflipped);

	Tmp1 = pd->graphics->copyBitmap(LevelTitle);
	
	
	int w;
	pd->graphics->getBitmapData(Tmp1, &w, NULL, NULL, NULL, NULL);
	char *Text2;
	pd->system->formatString(&Text2, "Level name: %s", GetNameText);
	pd->graphics->pushContext(Tmp1);
	pd->graphics->setFont(roobert);
	int w2 = pd->graphics->getTextWidth(roobert, Text2, strlen(Text2), kASCIIEncoding, 0);
	pd->graphics->drawText(Text2, strlen(Text2), kASCIIEncoding, w / 2 - w2 / 2 , 8);
	pd->graphics->popContext();
	pd->system->realloc(Text2, 0);

	pd->graphics->drawBitmap(Tmp1,120 - (w / 2), 15, kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);
	pd->graphics->pushContext(Tmp1);
	pd->graphics->setFont(roobert);
	for (y1 = 0; y1 < 6; y1++)
	{
		for (x1 = 0; x1 < 11; x1++)
		{
			if ((y1 *11) + x1 >= 65)
				break;
			pd->system->formatString(&Text2, "%c", NameSymbols[(y1 *11) + x1]);
			pd->graphics->drawText(Text2, strlen(Text2), kASCIIEncoding, (7 + (x1 *14)), (20 + (y1 *14)));
			pd->system->realloc(Text2, 0);
		}

		if ((y1 *11) + x1 >= 65)
			break;
	}

	pd->graphics->drawText("<", 1, kASCIIEncoding, (7 + (10 *14)), (20 + (5 *14)));
	pd->graphics->drawText("OK", 2, kASCIIEncoding, 50, (25 + (6 *14)));
	pd->graphics->drawText("CANCEL", 6, kASCIIEncoding, 75, (25 + (6 *14)));

	if (GetNameY < 6)
		pd->graphics->drawRect( (3 + (GetNameX *14)), (20 + (GetNameY *14)), 15, 15, kColorBlack);
	else
	{
		if (GetNameX == 0)
		{
			pd->graphics->drawRect(45, (25 + (GetNameY *14)), 25, 15, kColorBlack);
		}
		else
		{
			pd->graphics->drawRect(70, (25 + (GetNameY *14)), 67, 15, kColorBlack);
		}

	}

	pd->graphics->popContext();


	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);

	if(Tmp1)
		pd->graphics->freeBitmap(Tmp1);
	
	DrawPieces(NULL);
}


void PrintMsg(const char *Text, const char *Title)
{
	printMsgText = Text;
	printMsgTitle = Title;
	printMsgDone = false;
	GameState = GSPRINTMSGINIT;
}

bool doPrintMsgInit()
{
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->SetUpdateDelay(10);
	Input->Reset();
	return true;
}

void doPrintMsg()
{
	LCDBitmap * Tmp1;
	if(GameState == GSPRINTMSGINIT)
	{
		if(doPrintMsgInit())
			GameState -= GSINITDIFF;
	}

	if(GameState == GSPRINTMSG)
	{
	
	
		if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			GameState = GSLEVELEDITORMENUINIT;
			printMsgDone = true;
		}
	}

	pd->graphics->drawBitmap(BackgroundLevelEditor, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);
	DrawPieces(NULL);

	
	Tmp1 = pd->graphics->copyBitmap(Menu);
	int w, h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);
	SFont_WriteCenter(Tmp1, FontDark, 4, printMsgTitle);
	pd->graphics->pushContext(Tmp1);
	pd->graphics->setFont(roobert);
	pd->graphics->drawText(printMsgText, strlen(printMsgText), kASCIIEncoding, 8, 30);
	pd->graphics->popContext();
	
	pd->graphics->drawBitmap(Tmp1,120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);

	if(Tmp1)
		pd->graphics->freeBitmap(Tmp1);
}

bool LevelSelectMenuInit(void)
{
	Selected = 0;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	
	Input->SetUpdateDelay(10);
	Input->Reset();
	if (StaticLevels)
		dm_init_level(dml_levels[lvl].field);
	else
	{
		LoadLevel(UserLevelsFilenames[lvl]);
		dm_init_level(UserLevel.field);
	}
	return true;
}

void LevelSelectMenu(void)
{
	char *Text;
	LCDBitmap * Tmp1;
	if (GameState == GSLEVELSELECTMENUINIT)
	{
		if (LevelSelectMenuInit())
			GameState -= GSINITDIFF;
	}
	if (GameState == GSLEVELSELECTMENU)
	{
		if (Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSTITLESCREENINIT;
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (Selected > 3)
				Selected = 0;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (Selected < 0)
				Selected = 3;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a] ))
		{
			switch (Selected)
			{
				case 0:	// Next
					lvl++;
					if (lvl > MaxLevels - 1)
						lvl = 0;
					if (StaticLevels)
						dm_init_level(dml_levels[lvl].field);
					else
					{
						LoadLevel(UserLevelsFilenames[lvl]);
						dm_init_level(UserLevel.field);
					}

					CAudio_PlaySound(Sounds[SND_SELECT], 0);
					break;
				case 1:	// previous
					lvl--;
					if (lvl < 0)
						lvl = MaxLevels - 1;
					if (StaticLevels)
						dm_init_level(dml_levels[lvl].field);
					else
					{
						LoadLevel(UserLevelsFilenames[lvl]);
						dm_init_level(UserLevel.field);
					}

					CAudio_PlaySound(Sounds[SND_SELECT], 0);
					break;
				case 2:
					if (StaticLevels)
					{
						if (UnlockData[lvl] == '1')
						{
							GameState = GSGAMEINIT;
							Cursor->SetXY(8, 8);
							CAudio_PlaySound(Sounds[SND_SELECT], 0);
						}
						else
						{
							//play error sound;
							CAudio_PlaySound(Sounds[SND_ERROR], 0);
						}
					}
					else
					{
						GameState = GSGAMEINIT;
						Cursor->SetXY(8, 8);
						CAudio_PlaySound(Sounds[SND_SELECT], 0);
					}

					break;
				case 3:	//quit
					GameState = GSTITLESCREENINIT;
					CAudio_PlaySound(Sounds[SND_SELECT], 0);
					
					break;
			}

			Input->Delay();
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(Background, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);
	Tmp1 = pd->graphics->copyBitmap(LevelTitle);
	int w;
	pd->graphics->getBitmapData(Tmp1, &w, NULL, NULL, NULL, NULL);
	if (StaticLevels)
		pd->system->formatString(&Text, "Level %d: %s", lvl + 1, dml_levels[lvl].name);
	else
		pd->system->formatString(&Text, "Level %d: %s", lvl + 1, UserLevel.name);
	pd->graphics->pushContext(Tmp1);
	pd->graphics->setFont(roobert);
	int w2 = pd->graphics->getTextWidth(roobert, Text, strlen(Text), kASCIIEncoding, 0);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, w / 2 - w2 / 2 , 8);
	pd->graphics->popContext();
	
	pd->graphics->drawBitmap(Tmp1,120 - (w / 2), 15, kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);

	SFont_WriteCenter(Tmp1, FontDark, 4, "Select Level");
	if (Selected == 0)
	{
		SFont_WriteCenter(Tmp1, FontDark, 24, "Next");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 24, "Next");
	}

	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 44, "Previous");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 44, "Previous");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 64, "Play");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 64, "Play");
	}

	if (Selected == 3)
	{
		SFont_WriteCenter(Tmp1, FontDark, 84, "Quit");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 84, "Quit");
	}
	pd->graphics->pushContext(Tmp1);
	if (StaticLevels)
	{
		if (UnlockData[lvl] == '1')
		{
			int w2;
			pd->graphics->getBitmapData(Unlocked, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(Unlocked, (w / 2) - (w2 / 2), 108, kBitmapUnflipped);
		}
		else
		{
			int w2;
			pd->graphics->getBitmapData(Locked, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(Locked, (w / 2) - (w2 / 2), 108, kBitmapUnflipped);
		}
	}
	pd->graphics->popContext();
		
	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

	if (StaticLevels)
		pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
	else
		pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
	pd->graphics->pushContext(NULL);
	pd->graphics->setFont(roobert);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
	pd->graphics->popContext();
	pd->system->realloc(Text, 0);
}

bool WinMenuInit(void)
{
	if (lvl == MaxLevels - 1)
		Selected = 3;
	else
		Selected = 1;

	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->Reset();
	CAudio_PlaySound(Sounds[SND_LEVELPASS], 0);
	Input->SetUpdateDelay(10);
	return true;
}

void WinMenu(void)
{
	LCDBitmap * Tmp1;

	char *Text;
	if (GameState == GSWINMENUINIT)
	{
		if(WinMenuInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSWINMENU)
	{
		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (lvl == MaxLevels - 1)
			{
				if (Selected > 3)
					Selected = 2;
			}
			else
			{
				if (Selected > 3)
					Selected = 1;
			}

			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (lvl == MaxLevels - 1)
			{
				if (Selected < 2)
					Selected = 3;
			}
			else
			{
				if (Selected < 1)
					Selected = 3;
			}

			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 1:	// Next
					lvl++;
					if (StaticLevels)
						dm_init_level(dml_levels[lvl].field);
					else
					{
						LoadLevel(UserLevelsFilenames[lvl]);
						dm_init_level(UserLevel.field);
					}

					GameState = GSGAMEINIT;
					Cursor->SetXY(8, 8);
					break;
				case 2:	//level select
					GameState = GSLEVELSELECTMENUINIT;
					break;
				case 3:	//quit
					GameState = GSTITLESCREENINIT;
					break;
			}
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(Background, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int w, h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);
	SFont_WriteCenter(Tmp1, FontDark, 4, "You Win!");
	pd->graphics->pushContext(Tmp1);
	if (lvl == MaxLevels - 1)
	{
		int w2;
		pd->graphics->getBitmapData(AllLevelsCompleted, &w2, NULL, NULL, NULL, NULL);
		pd->graphics->drawBitmap(AllLevelsCompleted, (w / 2) - (w2 / 2), 24, kBitmapUnflipped);
	}
	else
	{
		int w2;
		pd->graphics->getBitmapData(LevelCompleted, &w2, NULL, NULL, NULL, NULL);
		pd->graphics->drawBitmap(LevelCompleted, (w / 2) - (w2 / 2), 24, kBitmapUnflipped);
	}
	pd->graphics->popContext();
	if (lvl < MaxLevels - 1)
	{
		if (Selected == 1)
		{
			SFont_WriteCenter(Tmp1, FontDark, 59, "Next");
		}
		else
		{
			SFont_WriteCenter(Tmp1, FontLight, 59, "Next");
		}
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 82, "Select Level");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 82, "Select Level");
	}

	if (Selected == 3)
	{
		SFont_WriteCenter(Tmp1, FontDark, 105, "Quit");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 105, "Quit");
	}

	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);
	if (StaticLevels)
		pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
	else
		pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
	pd->graphics->pushContext(NULL);
	pd->graphics->setFont(roobert);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
	pd->graphics->popContext();
	pd->system->realloc(Text, 0);
}

bool DiedMenuInit(void)
{
	Selected = 1;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->Reset();
	CAudio_PlaySound(Sounds[SND_LEVELFAIL], 0);
	Input->SetUpdateDelay(10);
	return true;
}

void DiedMenu(void)
{
	char *Text;
	LCDBitmap * Tmp1;

	if (GameState == GSDIEDMENUINIT)
	{
		if(DiedMenuInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSDIEDMENU)
	{
		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (Selected > 3)
				Selected = 1;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (Selected < 1)
				Selected = 3;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 1:	// retry
					if (StaticLevels)
						dm_init_level(dml_levels[lvl].field);
					else
					{
						LoadLevel(UserLevelsFilenames[lvl]);
						dm_init_level(UserLevel.field);
					}

					GameState = GSGAMEINIT;
					Cursor->SetXY(8, 8);
					break;
				case 2:	//level select
					GameState = GSLEVELSELECTMENUINIT;
					break;
				case 3:	//quit
					GameState = GSTITLESCREENINIT;
					break;
			}
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(Background, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int w, h, w2;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);
	SFont_WriteCenter(Tmp1, FontDark, 4, "You Died!");
	pd->graphics->pushContext(Tmp1);
	switch (dm_state())
	{
		case DM_OUT_OF_MOVES:
			pd->graphics->getBitmapData(OutOfMoves, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(OutOfMoves, (w / 2) - (w2 / 2), 24, kBitmapUnflipped);
			break;
		case DM_HIT_DIE_PIECE:
			pd->graphics->getBitmapData(DeathPiece, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(DeathPiece, (w / 2) - (w2 / 2), 24, kBitmapUnflipped);
			break;
		case DM_OUT_OF_FIELD:
			pd->graphics->getBitmapData(PieceOutsidePlayfield, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(PieceOutsidePlayfield, (w / 2) - (w2 / 2), 24, kBitmapUnflipped);
			break;
		case DM_INFINITE_LOOP:
			pd->graphics->getBitmapData(InfiniteLoop, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(InfiniteLoop, (w / 2) - (w2 / 2), 24, kBitmapUnflipped);
			break;
		default:
			break;
	}
	pd->graphics->popContext();
	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 59, "Retry");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 59, "Retry");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 82, "Select Level");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 82, "Select Level");
	}

	if (Selected == 3)
	{
		SFont_WriteCenter(Tmp1, FontDark, 105, "Quit");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 105, "Quit");
	}

	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);
	if (StaticLevels)
		pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
	else
		pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
	pd->graphics->pushContext(NULL);
	pd->graphics->setFont(roobert);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
	pd->graphics->popContext();
	pd->system->realloc(Text, 0);
}

bool IngameMenuInit(void)
{
	Selected = 0;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;

	Input->Reset();
	Input->SetUpdateDelay(10);
	return true;
}

void IngameMenu(void)
{
	LCDBitmap * Tmp1;
	char *Text;
	if (GameState == GSINGAMEMENUINIT)
	{
		if (IngameMenuInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSINGAMEMENU)
	{	
		if (Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSGAMEINIT;
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (Selected > 3)
				Selected = 0;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (Selected < 0)
				Selected = 3;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 0:	// Resume play
					GameState = GSGAMEINIT;
					break;
				case 1:	// retry
					if (StaticLevels)
						dm_init_level(dml_levels[lvl].field);
					else
					{
						LoadLevel(UserLevelsFilenames[lvl]);
						dm_init_level(UserLevel.field);
					}

					Cursor->SetXY(8, 8);
					GameState = GSGAMEINIT;
					break;
				case 2:	//level select
					GameState = GSLEVELSELECTMENUINIT;
					break;
				case 3:	//quit
					GameState = GSTITLESCREENINIT;
					break;
			}
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(Background, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int w, h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);
	SFont_WriteCenter(Tmp1, FontDark, 4, "Game Menu");
	if (Selected == 0)
	{
		SFont_WriteCenter(Tmp1, FontDark, 27, "Resume Play");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 27, "Resume Play");
	}

	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 50, "Retry");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 50, "Retry");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 73, "Select Level");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 73, "Select Level");
	}

	if (Selected == 3)
	{
		SFont_WriteCenter(Tmp1, FontDark, 96, "Quit");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 96, "Quit");
	}

	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);
	if (StaticLevels)
		pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
	else
		pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
	pd->graphics->pushContext(NULL);
	pd->graphics->setFont(roobert);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
	pd->graphics->popContext();
	pd->system->realloc(Text, 0);
}

int GetLastUnLocked()
{
	if (!StaticLevels)
		return 0;

	int teller = 0;
	while (UnlockData[teller] == '1')
	{
		teller++;
	}

	return teller - 1;
}

bool LevelTypeMenuInit(void)
{
	Selected = 0;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->SetUpdateDelay(10);

	Input->Reset();
	return true;
}

void LevelTypeMenu(void)
{
	LCDBitmap * Tmp1;
	if (GameState == GSLEVELTYPESELECTINIT)
	{
		if(LevelTypeMenuInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSLEVELTYPESELECT)
	{
		if (Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSTITLESCREENINIT;
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			if (UserLevelCount > 0)
			{
				Selected++;
				if (Selected > 2)
					Selected = 0;
			}
			else
			{
				if (Selected == 2)
					Selected = 0;
				else
				if (Selected == 0)
					Selected = 2;
			}

			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			if (UserLevelCount > 0)
			{
				Selected--;
				if (Selected < 0)
					Selected = 2;
			}
			else
			{
				if (Selected == 2)
					Selected = 0;
				else
				if (Selected == 0)
					Selected = 2;
			}

			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 0:	// static
					StaticLevels = true;
					//set to last unlocked level!
					lvl = GetLastUnLocked();
					dm_init_level(dml_levels[lvl].field);
					MaxLevels = MAXSTATICLEVELS;
					GameState = GSGAMEINIT;
					Cursor->SetXY(8, 8);

					break;
				case 1:	// user
					StaticLevels = false;
					lvl = 0;
					MaxLevels = UserLevelCount;
					LoadLevel(UserLevelsFilenames[lvl]);
					dm_init_level(UserLevel.field);
					GameState = GSLEVELSELECTMENUINIT;
					break;
				case 2:	// back
					GameState = GSTITLESCREENINIT;
					break;
			}
		}
	}
    pd->graphics->setBackgroundColor(kColorWhite);
	pd->graphics->clear(kColorWhite);
	pd->graphics->drawBitmap(TitleScreen, 0, 0, kBitmapUnflipped);


	Tmp1 = pd->graphics->copyBitmap(Menu);
	SFont_WriteCenter(Tmp1, FontDark, 3, "Leveltype");
	int w, h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	
	if (Selected == 0)
	{
		SFont_WriteCenter(Tmp1, FontDark, 27, "Static Levels");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 27, "Static Levels");
	}

	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 50, "User Levels");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 50, "User Levels");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 73, "Back");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 73, "Back");
	}

	pd->graphics->drawBitmap(Tmp1, (WINDOW_WIDTH / 2) - (w / 2), 150  - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

}

bool CreditsInit(void)
{
	Selected = 0;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->SetUpdateDelay(10);
	Input->Reset();
	return true;
}

void Credits(void)
{
	if (GameState == GSCREDITSINIT)
	{
		if (CreditsInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSCREDITS)
	{
		if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			GameState = GSTITLESCREENINIT;
		
		}
	}
	pd->graphics->setBackgroundColor(kColorWhite);
	pd->graphics->clear(kColorWhite);
	pd->graphics->drawBitmap(TitleScreen, 0, 0, kBitmapUnflipped);

	drawTextColor(true, NULL, roobert, "Dynamate for playdate is created by Willems Davy.\nIt is a port of my GP2X version of the game.\n\nDynamate Engine created by Bjorn Kalzen and\nJonas Nordberg.\n\nGraphics are a modified version of graphics made\nby Flavor for the gp32 version of the game.\n\nMusic by Don Skeeto.", 1000, kASCIIEncoding, 0,70, kColorBlack, false);
}


bool TitleScreenMenuInit(void)
{
	Selected = 0;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	if(!CAudio_IsMusicPlaying())
		CAudio_PlayMusic(Music[0], -1);
	Input->SetUpdateDelay(10);
	Input->Reset();
	return true;
}

void TitleScreenMenu(void)
{
	LCDBitmap *Tmp1 = NULL;

	if (GameState == GSTITLESCREENINIT)
	{
		if (TitleScreenMenuInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSTITLESCREEN)
	{
		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (Selected > 2)
				Selected = 0;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (Selected < 0)
				Selected = 2;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 0:	// play
					GameState = GSLEVELTYPESELECTINIT;
					break;
				case 1:	// Level editor
					memset(UserLevel.name, 0, 256);
					memset(UserLevel.hs, 0, 256);
					memset(UserLevel.field, 0, 256);
					dm_init_level(UserLevel.field);
					SaveEnabled = false;
					StaticLevels = false;
					Cursor->SetXY(8, 8);
					GameState = GSLEVELEDITORMENUINIT;
					break;
				case 2: // Credits
					GameState = GSCREDITSINIT;
					break;
				default:
					break;
			}
		}
	}
	pd->graphics->setBackgroundColor(kColorWhite);
	pd->graphics->clear(kColorWhite);
	pd->graphics->drawBitmap(TitleScreen, 0, 0, kBitmapUnflipped);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	SFont_WriteCenter(Tmp1, FontDark, 4, "Main Menu");
	if (Selected == 0)
	{
		SFont_WriteCenter(Tmp1, FontDark, 27, "Play");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 27, "Play");
	}

	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 50, "Level Editor");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 50, "Level Editor");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 73, "Credits");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 73, "Credits");
	}

	int w, h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);

	pd->graphics->drawBitmap(Tmp1, (WINDOW_WIDTH / 2) - (w / 2), 150 - (h / 2), kBitmapUnflipped);
	pd->graphics->freeBitmap(Tmp1);

}

void MovePieceIter()
{
	char *Text;
	if (dm_step() == 0)
	{
		if (dm_cmd() != DM_NOTHING)
		{
			switch (dm_cmd())
			{
				case DM_MOVE:
					//playsound
					CAudio_PlaySound(Sounds[SND_MOVE], 0);
					break;

				case DM_EXPL:
					//playsound
					CAudio_PlaySound(Sounds[SND_EXPLODE], 0);
					break;

				case DM_TELE:
					//playsound
					CAudio_PlaySound(Sounds[SND_TELEPORT], 0);
					break;

				case DM_NOTHING:
				default:
					break;
			}

			pd->graphics->drawBitmap(Background,0,0, kBitmapUnflipped);
			DrawLevel(NULL);
			if (StaticLevels)
				pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
			else
				pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
			pd->graphics->pushContext(NULL);
			pd->graphics->setFont(roobert);
			pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
			pd->graphics->popContext();
			pd->system->realloc(Text, 0);

		}
	}
	else
	{
		GameState = GSGAME;
		pd->graphics->drawBitmap(Background,0,0, kBitmapUnflipped);
		DrawLevel(NULL);
		if (StaticLevels)
			pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
		else
			pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
		pd->graphics->pushContext(NULL);
		pd->graphics->setFont(roobert);
		pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
		pd->graphics->popContext();
		pd->system->realloc(Text, 0);
		Cursor->SetXY(dm_srcx(), dm_srcy());

	}
}

//fixme
void MovePiece(int x, int y, dm_dir dir)
{
	dm_init_step(x, y, dir);
	GameState = GSMOVEPIECEITTER;
	//need to run once already because otherwise screen flickers
	MovePieceIter();
}

void SearchForLevelsCB(const char *path, void *userdata)
{
	//not a dir
	if (path[strlen(path)-1] != '/')
	{
		pd->system->logToConsole("%s", path);
		if (UserLevelCount < MAXUSERLEVELS)
		{
			char *tmp;
			pd->system->formatString(&tmp, "userlevels/%s", path);
			strcpy(UserLevelsFilenames[UserLevelCount], tmp);
			pd->system->realloc(tmp, 0);
			pd->system->logToConsole("%s", UserLevelsFilenames[UserLevelCount]);
			UserLevelCount++;
		}
	}	
}

void SearchForLevels()
{
	UserLevelCount = 0;
	pd->file->listfiles("userlevels", &SearchForLevelsCB, NULL, 0);
}

void LoadUnlockData(void)
{
	SDFile * f;
	f = pd->file->open("dynamate.dat", kFileReadData);
	if (f)
	{
		pd->file->read(f, &UnlockData, sizeof(UnlockData));
		pd->file->close(f);
	}
	else
	{
		for (int teller = 0; teller < MAXSTATICLEVELS; teller++)
			UnlockData[teller] = '0';
		UnlockData[0] = '1';
	}
}

void SaveUnlockData(void)
{
	SDFile * f;
	f = pd->file->open("dynamate.dat", kFileWrite);
	if(f)
	{
		pd->file->write(f, &UnlockData, sizeof(UnlockData));
		pd->file->close(f);
	}
}

bool GameInit(void)
{
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->SetUpdateDelay(5);

	Input->Reset();
	return true;
}

void Game(void)
{
	char *Text;
	if (GameState == GSGAMEINIT)
	{
		if (GameInit())
			GameState -= GSINITDIFF;
	}


	if(GameState == GSGAME)
	{
		if (Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSINGAMEMENUINIT;
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
		}


		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_UP]))
		{
			MovePiece(Cursor->GetX(), Cursor->GetY(), DM_UP);		
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			MovePiece(Cursor->GetX(), Cursor->GetY(), DM_DOWN);
			Input->Delay();
		}
		
		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_LEFT]))
		{
			MovePiece(Cursor->GetX(), Cursor->GetY(), DM_LEFT);
			Input->Delay();
		}

		
		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_RIGHT]))
		{
			MovePiece(Cursor->GetX(), Cursor->GetY(), DM_RIGHT);
			Input->Delay();
		}

		//diagonals
		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) &&
			(((Input->KeyboardHeld[SDLK_UP]) && (Input->KeyboardHeld[SDLK_LEFT]))))
		{
			Cursor->SetY(Cursor->GetY() - 1);
			Cursor->SetX(Cursor->GetX() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) &&
			(((Input->KeyboardHeld[SDLK_UP]) && (Input->KeyboardHeld[SDLK_RIGHT]))))
		{
			Cursor->SetY(Cursor->GetY() - 1);
			Cursor->SetX(Cursor->GetX() + 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) &&
			(((Input->KeyboardHeld[SDLK_DOWN]) && (Input->KeyboardHeld[SDLK_LEFT]))))
		{
			Cursor->SetY(Cursor->GetY() + 1);
			Cursor->SetX(Cursor->GetX() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) &&
			(((Input->KeyboardHeld[SDLK_DOWN]) && (Input->KeyboardHeld[SDLK_RIGHT]))))
		{
			Cursor->SetY(Cursor->GetY() + 1);
			Cursor->SetX(Cursor->GetX() + 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Cursor->SetY(Cursor->GetY() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Cursor->SetY(Cursor->GetY() + 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_LEFT]))
		{
			Cursor->SetX(Cursor->GetX() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && !(Input->KeyboardHeld[SDLK_a]) && (Input->KeyboardHeld[SDLK_RIGHT]))
		{
			Cursor->SetX(Cursor->GetX() + 1);
			Input->Delay();
		}

		if (dm_state() != DM_NORMAL)
			switch (dm_state())
			{
				case DM_FINISHED:
					if (StaticLevels)
					{
						if (HighScores[lvl] == 0)
							HighScores[lvl] = dm_movecount();
						else
						if (HighScores[lvl] > dm_movecount())
							HighScores[lvl] = dm_movecount();
						if (lvl + 1 < MAXSTATICLEVELS)
							UnlockData[lvl + 1] = '1';
						SaveUnlockData();
						SaveHighScores();
					}

					GameState = GSWINMENUINIT;
					break;
				case DM_OUT_OF_MOVES:
				case DM_HIT_DIE_PIECE:
				case DM_OUT_OF_FIELD:
				case DM_INFINITE_LOOP:
					GameState = GSDIEDMENUINIT;
					break;
				default:
					break;
			}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(Background,0,0, kBitmapUnflipped);
	DrawLevel(NULL);
	Cursor->Draw(NULL);
	if (StaticLevels)
		pd->system->formatString(&Text, "Level: %d\nBest: %d\nMoves: %d\nStones: %d", lvl + 1, HighScores[lvl], dm_movecount(), dm_stones_left());
	else
		pd->system->formatString(&Text, "Level: %d\nMoves: %d\nStones: %d", lvl + 1, dm_movecount(), dm_stones_left());
	pd->graphics->pushContext(NULL);
	pd->graphics->setFont(roobert);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, 242, 182);
	pd->graphics->popContext();
	pd->system->realloc(Text, 0);
}


bool LevelEditorLevelSelectInit(void)
{
	Selected = 0;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	
	SaveEnabled = false;
	Input->SetUpdateDelay(10);
	
	Input->Reset();
	return true;
}

void LevelEditorLevelSelect(void)
{
	LCDBitmap * Tmp1;
	char *Text;

	if (GameState == GSLEVELEDITORLEVELSELECTINIT)
	{
		if (LevelEditorLevelSelectInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSLEVELEDITORLEVELSELECT)
	{
		if (Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSLEVELEDITORMENUINIT;
			memset(UserLevel.field, 0, 256);
			memset(UserLevel.name, 0, 256);
			dm_init_level(UserLevel.field);		
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (Selected > 4)
				Selected = 0;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (Selected < 0)
				Selected = 4;
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_a]))
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 0:	// Next
					lvl++;
					if (lvl > UserLevelCount - 1)
						lvl = 0;
					LoadLevel(UserLevelsFilenames[lvl]);
					dm_init_level(UserLevel.field);
					break;
				case 1:	// previous
					lvl--;
					if (lvl < 0)
						lvl = UserLevelCount - 1;
					LoadLevel(UserLevelsFilenames[lvl]);
					dm_init_level(UserLevel.field);
					break;
				case 2:	//edit
					SaveEnabled = true;
					Cursor->SetXY(8, 8);
					GameState = GSLEVELEDITORINIT;
					break;
				case 3: //delete
					pd->file->unlink(UserLevelsFilenames[lvl], false);
					if(lvl < UserLevelCount -1)
					{
						for (int i = lvl; i < UserLevelCount-1; i++)
							strcpy(UserLevelsFilenames[i], UserLevelsFilenames[i+1]);
					}
					UserLevelCount--;
					if (UserLevelCount > 0)
					{
						while(lvl >= UserLevelCount)
							lvl--;
						LoadLevel(UserLevelsFilenames[lvl]);
						dm_init_level(UserLevel.field);
					}
					else
					{
						memset(UserLevel.field, 0, 256);
						memset(UserLevel.name, 0, 256);
						dm_init_level(UserLevel.field);
						GameState = GSLEVELEDITORMENUINIT;
					}
					break;
				case 4:	//back
					memset(UserLevel.field, 0, 256);
					memset(UserLevel.name, 0, 256);
					dm_init_level(UserLevel.field);
					GameState = GSLEVELEDITORMENUINIT;
					break;
			}

			Input->Delay();
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(BackgroundLevelEditor, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);

	Tmp1 = pd->graphics->copyBitmap(LevelTitle);
	int w;
	pd->graphics->getBitmapData(Tmp1, &w, NULL, NULL, NULL, NULL);
	pd->system->formatString(&Text, "Level %d: %s", lvl + 1, UserLevel.name);
	pd->graphics->pushContext(Tmp1);
	pd->graphics->setFont(roobert);
	int w2 = pd->graphics->getTextWidth(roobert, Text, strlen(Text), kASCIIEncoding, 0);
	pd->graphics->drawText(Text, strlen(Text), kASCIIEncoding, w / 2 - w2 / 2 , 8);
	pd->graphics->popContext();
	
	pd->graphics->drawBitmap(Tmp1,120 - (w / 2), 15, kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);

	SFont_WriteCenter(Tmp1, FontDark, 4, "Select Level");
	if (Selected == 0)
	{
		SFont_WriteCenter(Tmp1, FontDark, 24, "Next");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 24, "Next");
	}

	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 44, "Previous");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 44, "Previous");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 64, "Edit");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 64, "Edit");
	}

	if (Selected == 3)
	{
		SFont_WriteCenter(Tmp1, FontDark, 84, "Delete");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 84, "Delete");
	}


	if (Selected == 4)
	{
		SFont_WriteCenter(Tmp1, FontDark, 104, "Back");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 104, "Back");
	}

	pd->graphics->pushContext(Tmp1);
	if (StaticLevels)
	{
		if (UnlockData[lvl] == '1')
		{
			int w2;
			pd->graphics->getBitmapData(Unlocked, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(Unlocked, (w / 2) - (w2 / 2), 108, kBitmapUnflipped);
		}
		else
		{
			int w2;
			pd->graphics->getBitmapData(Locked, &w2, NULL, NULL, NULL, NULL);
			pd->graphics->drawBitmap(Locked, (w / 2) - (w2 / 2), 108, kBitmapUnflipped);
		}
	}
	pd->graphics->popContext();
	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

	DrawPieces(NULL);
}

bool LevelEditorInit(void)
{
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Input->SetUpdateDelay(5);
	Input->Reset();
	SelectedPiece = NextPiece(DM_P_EMPTY);
	return true;
}

void LevelEditor(void)
{
	if (GameState == GSLEVELEDITORINIT)
	{
		if (LevelEditorInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSLEVELEDITOR)
	{
		
		if (Input->KeyboardHeld[SDLK_b])
		{
			if(SaveEnabled)
			{
				GameState = GSLEVELEDITORMENUINIT;
				CAudio_PlaySound(Sounds[SND_SELECT], 0);
			}
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			UserLevel.field[(Cursor->GetY() *TILEROWS) + Cursor->GetX()] = SelectedPiece;
			dm_init_level(UserLevel.field);
		}

		//if (Input->KeyboardHeld[SDLK_BACKSPACE] || Input->KeyboardHeld[SDLK_x] || Input->KeyboardHeld[SDLK_y] || Input->KeyboardHeld[SDLK_DELETE])
		//{
		//	UserLevel.field[(Cursor->GetY() *TILEROWS) + Cursor->GetX()] = DM_P_EMPTY;
		//	dm_init_level(UserLevel.field);
		//}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_PAGEDOWN]))
		{
			SelectedPiece = PreviousPiece(SelectedPiece);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_PAGEUP]))
		{
			SelectedPiece = NextPiece(SelectedPiece);
			Input->Delay();
		}

		if ((Input->Ready()) && ((Input->KeyboardHeld[SDLK_UP]) && (Input->KeyboardHeld[SDLK_LEFT])))
		{
			Cursor->SetY(Cursor->GetY() - 1);
			Cursor->SetX(Cursor->GetX() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && ((Input->KeyboardHeld[SDLK_DOWN]) && (Input->KeyboardHeld[SDLK_LEFT])))
		{
			Cursor->SetY(Cursor->GetY() + 1);
			Cursor->SetX(Cursor->GetX() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && ((Input->KeyboardHeld[SDLK_RIGHT]) && (Input->KeyboardHeld[SDLK_UP])))
		{
			Cursor->SetY(Cursor->GetY() - 1);
			Cursor->SetX(Cursor->GetX() + 1);
			Input->Delay();
		}

		if ((Input->Ready()) && ((Input->KeyboardHeld[SDLK_RIGHT]) && (Input->KeyboardHeld[SDLK_DOWN])))
		{
			Cursor->SetY(Cursor->GetY() + 1);
			Cursor->SetX(Cursor->GetX() + 1);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Cursor->SetY(Cursor->GetY() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Cursor->SetY(Cursor->GetY() + 1);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_LEFT]))
		{
			Cursor->SetX(Cursor->GetX() - 1);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_RIGHT]))
		{
			Cursor->SetX(Cursor->GetX() + 1);
			Input->Delay();
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(BackgroundLevelEditor, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);
	Cursor->Draw(NULL);
	DrawPieces(NULL);
}

void UpdateUserLevelFilenames(char *filename)
{
	int teller;
	bool found = false;
	for (teller = 0; teller < UserLevelCount; teller++)
	{
		if (strcmp(filename, UserLevelsFilenames[teller]) == 0)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		pd->system->logToConsole("Not Found %s", filename);
		UserLevelCount++;
		strcpy(UserLevelsFilenames[UserLevelCount - 1], filename);
		lvl = UserLevelCount - 1;
	}
}

int CheckForLevelErrors()
{
	int x, y, NumTeleport1 = 0, NumTeleport2 = 0, NumColors = 0, NumObjects = 0, tmp;
	for (y = 0; y < TILEROWS; y++)
	{
		for (x = 0; x < TILECOLUMNS; x++)
		{
			tmp = dm_field()[(y *TILEROWS) + x];
			switch (L7(tmp))
			{
				case DM_P_DIE:
				case DM_P_GRAY:
				case DM_P_ROT_CCW:
				case DM_P_ROT_CW:
				case DM_P_ROTCOL:
				case DM_P_VERT:
				case DM_P_HORI:
					NumObjects++;
					break;

				case DM_P_BLACK:
				case DM_P_YELLOW:
				case DM_P_GREEN:
				case DM_P_BLUE:
				case DM_P_RED:
					if (SOLID(tmp))
						NumObjects++;
					else
					{
						NumObjects++;
						NumColors++;
					}

					break;
				case DM_P_TELE1:
					NumTeleport1++;
					NumObjects++;
					break;

				case DM_P_TELE2:
					NumTeleport2++;
					NumObjects++;
					break;
				default:
					break;
			}
		}
	}

	if (NumTeleport1 > 0)
		if (NumTeleport1 != 2)
			return 0;	//Teleport 1 invalid number
	if (NumTeleport2 > 0)
		if (NumTeleport2 != 2)
			return 1;	//teleport 2 invalid number
	if (NumObjects == 0)
		return 2;	// no pieces in the field
	if (NumColors < 2)
		return 3;	// there must be at least 2 colored pieces
	return -1;	//all good
}

bool LevelEditorMenuInit(void)
{
	Selected = 0;
	if(printMsgDone || GetNameDone)
		Selected = 2;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	
	Input->SetUpdateDelay(10);

	Input->Reset();
	return true;
}

void LevelEditorMenu(void)
{
	char Text[256];
	LCDBitmap * Tmp1;
	int LevelError = -1;

	if (GameState == GSLEVELEDITORMENUINIT)
	{
		if (LevelEditorMenuInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSLEVELEDITORMENU)
	{
		if(printMsgDone)
		{
			Selected = 2;
			printMsgDone = false;
		}

		if(GetNameDone)
		{
			Selected = 2;
			if(GetNameResult)
			{
				memset(UserLevel.name, 0, 256);
				strcpy(UserLevel.name, GetNameText);
				char *fname;
				pd->system->formatString(&fname, "userlevels/%s.lev", UserLevel.name);
				SaveLevel(fname);
				UpdateUserLevelFilenames(fname);
				pd->system->realloc(fname, 0);
			}
			GetNameDone = false;
		}

		if (Input->KeyboardHeld[SDLK_b])
		{
			if (SaveEnabled)
			{
				GameState = GSLEVELEDITORINIT;
				CAudio_PlaySound(Sounds[SND_SELECT], 0);
			}
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_DOWN]))
		{
			Selected++;
			if (Selected > 4)
			{
				Selected = 0;
			}

			if (Selected == 1)
			{
				if (UserLevelCount == 0)
					Selected = 2;
			}

			if (Selected == 2)
				if (!SaveEnabled)
					Selected = 4;
			
			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if ((Input->Ready()) && (Input->KeyboardHeld[SDLK_UP]))
		{
			Selected--;
			if (Selected < 0)
			{
				Selected = 4;
			}

			if (Selected == 3)
			{
				if (!SaveEnabled)
					Selected = 1;
			}

			if (UserLevelCount == 0)
			{
				if (Selected == 1)
				{
					Selected = 0;
				}
			}

			CAudio_PlaySound(Sounds[SND_MENU], 0);
			Input->Delay();
		}

		if (Input->KeyboardHeld[SDLK_a])
		{
			CAudio_PlaySound(Sounds[SND_SELECT], 0);
			switch (Selected)
			{
				case 0:	// new
					memset(UserLevel.name, 0, 256);
					memset(UserLevel.hs, 0, 256);
					memset(UserLevel.field, 0, 256);
					dm_init_level(UserLevel.field);
					SaveEnabled = true;
					GameState = GSLEVELEDITORINIT;
					Cursor->SetXY(8, 8);
					break;
				case 1:	//level select
					lvl = 0;
					LoadLevel(UserLevelsFilenames[lvl]);
					dm_init_level(UserLevel.field);
					GameState = GSLEVELEDITORLEVELSELECTINIT;
					break;
				case 2:
					LevelError = CheckForLevelErrors();
					if (LevelError >= 0)
					{
						switch (LevelError)
						{
							case 0:
								PrintMsg("When using the\nteleporters it needs\n2 teleporter pieces of\neach kind!\nPlease Fix this error\nbefore saving!", "Level Error");
								break;
							case 1:
								PrintMsg("When using the\nteleporters it needs\n2 teleporter pieces of\neach kind!\nPlease Fix this error\nbefore saving!", "Level Error");
								break;
							case 2:
								PrintMsg("There are no pieces in\nthis level!\n\nPlease Fix this error\nbefore saving!", "Level Error");
								break;
							case 3:
								PrintMsg("There should be at\nleast 2 number pieces\nin a level !\n\nPlease Fix this error\nbefore saving!", "Level Error");
								break;
						}
					}
					else
					{
						memset(Text, 0, 256);
						sprintf(Text, "%s", UserLevel.name);
						GetName(Text);
					}

					Input->Reset();
					break;
				case 3:
					GameState = GSLEVELEDITORINIT;
					break;
				case 4:	//Quit
					GameState = GSTITLESCREENINIT;
					break;
			}
		}
	}
	pd->graphics->setBackgroundColor(kColorBlack);
	pd->graphics->clear(kColorBlack);
	pd->graphics->drawBitmap(BackgroundLevelEditor, 0, 0, kBitmapUnflipped);
	DrawLevel(NULL);

	Tmp1 = pd->graphics->copyBitmap(Menu);
	int w, h;
	pd->graphics->getBitmapData(Tmp1, &w, &h, NULL, NULL, NULL);
	bitmap_set_alpha_rect(Tmp1, 3, 3, w-8, h-7, menuAlpha);
	SFont_WriteCenter(Tmp1, FontDark, 4, "Level Editor");
	if (Selected == 0)
	{
		SFont_WriteCenter(Tmp1, FontDark, 27, "New");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 27, "New");
	}

	if (Selected == 1)
	{
		SFont_WriteCenter(Tmp1, FontDark, 46, "Select Level");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 46, "Select Level");
	}

	if (Selected == 2)
	{
		SFont_WriteCenter(Tmp1, FontDark, 65, "Save");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 65, "Save");
	}

	if (Selected == 3)
	{
		SFont_WriteCenter(Tmp1, FontDark, 83, "Back");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 83, "Back");
	}

	if (Selected == 4)
	{
		SFont_WriteCenter(Tmp1, FontDark, 101, "Quit");
	}
	else
	{
		SFont_WriteCenter(Tmp1, FontLight, 101, "Quit");
	}

	pd->graphics->drawBitmap(Tmp1, 120 - (w / 2), (WINDOW_HEIGHT / 2) - (h / 2), kBitmapUnflipped);
	if (Tmp1)
		pd->graphics->freeBitmap(Tmp1);

	DrawPieces(NULL);
}

bool IntroInit()
{
	IntroScreenNr = 1;
	if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b] || Input->KeyboardHeld[SDLK_UP] ||
		Input->KeyboardHeld[SDLK_LEFT] || Input->KeyboardHeld[SDLK_DOWN] || Input->KeyboardHeld[SDLK_RIGHT] )
		return false;
	Time1 = pd->system->getCurrentTimeMilliseconds();
	Input->SetUpdateDelay(10);
	Input->Reset();
	startFade(fadeIn, true, 0.075f);
	return true;
}

void Intro()
{
	if (GameState == GSINTROINIT)
	{
		if (IntroInit())
			GameState -= GSINITDIFF;
	}

	if (GameState == GSINTRO)
	{
		if (Input->KeyboardHeld[SDLK_a] || Input->KeyboardHeld[SDLK_b])
		{
			GameState = GSTITLESCREENINIT;
		}
	}
	pd->graphics->setBackgroundColor(kColorWhite);
	pd->graphics->clear(kColorWhite);
	switch (IntroScreenNr)
	{
		case 1:
			pd->graphics->drawBitmap(IMGIntro1, 0, 0, kBitmapUnflipped);
			break;
		case 2:
			pd->graphics->drawBitmap(IMGIntro2, 0, 0, kBitmapUnflipped);
			break;
		case 3:
			pd->graphics->drawBitmap(IMGIntro3, 0, 0, kBitmapUnflipped);
			break;
		case 4:
			pd->graphics->drawBitmap(IMGIntro4, 0, 0, kBitmapUnflipped);
			break;
	}

	if ((Time1 + 3700 < pd->system->getCurrentTimeMilliseconds()))
	{
		startFade(fadeOut, true, 0.075f);
		Time1 = pd->system->getCurrentTimeMilliseconds();	
	}	

	if(handleFade() == fadeNone)
	{
		if (prevFadeType == fadeOut)
		{
			IntroScreenNr++;
			if (IntroScreenNr > 4)
				GameState = GSTITLESCREENINIT;	
			else
				startFade(fadeIn, true, 0.075f);
		}
	}
}

void LoadGraphics(void)
{
	LevelData = loadImageAtPath("graphics/pieces.png");
	Background = loadImageAtPath("graphics/background.png");
	BackgroundLevelEditor = loadImageAtPath("graphics/backgroundleveleditor.png");
	Menu = loadImageAtPath("graphics/menu.png");
	LevelTitle = loadImageAtPath("graphics/leveltitle.png");
	Unlocked = loadImageAtPath("graphics/unlocked.png");
	Locked = loadImageAtPath("graphics/locked.png");
	PieceOutsidePlayfield = loadImageAtPath("graphics/pieceoutsideplayfield.png");
	InfiniteLoop = loadImageAtPath("graphics/infiniteloop.png");
	DeathPiece = loadImageAtPath("graphics/deathpiece.png");
	OutOfMoves = loadImageAtPath("graphics/outofmoves.png");
	LevelCompleted = loadImageAtPath("graphics/levelcompleted.png");
	AllLevelsCompleted = loadImageAtPath("graphics/alllevelscompleted.png");
	SFontDark = loadImageAtPath("graphics/darkfont14pt.png");
	SFontLight = loadImageAtPath("graphics/lightfont14pt.png");
	TitleScreen = loadImageAtPath("graphics/titlescreen.png");
	IMGIntro1 = loadImageAtPath("graphics/intro1.png");
	IMGIntro2 = loadImageAtPath("graphics/intro2.png");
	IMGIntro3 = loadImageAtPath("graphics/intro3.png");
	IMGIntro4 = loadImageAtPath("graphics/intro4.png");
	FontDark = SFont_InitFont(SFontDark);
	FontLight = SFont_InitFont(SFontLight);
}

void UnLoadGraphics(void)
{
	SFont_FreeFont(FontDark);

	SFont_FreeFont(FontLight);

	if (IMGIntro1)
		pd->graphics->freeBitmap(IMGIntro1);
	if (IMGIntro2)
		pd->graphics->freeBitmap(IMGIntro2);
	if (IMGIntro3)
		pd->graphics->freeBitmap(IMGIntro3);
	if (IMGIntro4)
		pd->graphics->freeBitmap(IMGIntro4);

	if (LevelData)
		pd->graphics->freeBitmap(LevelData);
	if (Background)
		pd->graphics->freeBitmap(Background);

	if (BackgroundLevelEditor)
		pd->graphics->freeBitmap(BackgroundLevelEditor);

	if (Menu)
		pd->graphics->freeBitmap(Menu);

	if (Unlocked)
		pd->graphics->freeBitmap(Unlocked);

	if (Locked)
		pd->graphics->freeBitmap(Locked);

	if (PieceOutsidePlayfield)
		pd->graphics->freeBitmap(PieceOutsidePlayfield);

	if (InfiniteLoop)
		pd->graphics->freeBitmap(InfiniteLoop);

	if (DeathPiece)
		pd->graphics->freeBitmap(DeathPiece);

	if (OutOfMoves)
		pd->graphics->freeBitmap(OutOfMoves);

	if (LevelCompleted)
		pd->graphics->freeBitmap(LevelCompleted);

	if (AllLevelsCompleted)
		pd->graphics->freeBitmap(AllLevelsCompleted);

	if (LevelTitle)
		pd->graphics->freeBitmap(LevelTitle);

	if (TitleScreen)
		pd->graphics->freeBitmap(TitleScreen);

}

static void setupGame()
{
	/*Font = TTF_OpenFont("font.ttf", 12);
	SmallFont = TTF_OpenFont("font.ttf", 11);
	*/
	SFont_SetPdApi(pd);
	CAudio_Init(false);
	srand(pd->system->getCurrentTimeMilliseconds());
	pd->file->mkdir("userlevels");
	Cursor = new CCursor(pd);
	LoadSounds();
	LoadGraphics();
	LoadUnlockData();
	LoadHighScores();
	SearchForLevels();
	Input = new CInput(pd, 10);
	const char *err;
	roobert = pd->graphics->loadFont("fonts/Roobert-10-Bold", &err);
	showFPS = false;
	
	pd->graphics->setBackgroundColor(kColorWhite);
}

static void destroyGame()
{
	UnLoadGraphics();
	delete Input;
	delete Cursor;
	UnLoadSounds();
}

static int mainLoop(void* ud)
{
	Input->Update();
	switch (GameState)
	{
		case GSINTROINIT:
		case GSINTRO:
			Intro();
			break;
		case GSINGAMEMENUINIT:
		case GSINGAMEMENU:
			IngameMenu();
			break;
		case GSGAMEINIT:
		case GSGAME:
			Game();
			break;
		case GSDIEDMENUINIT:
		case GSDIEDMENU:
			DiedMenu();
			break;
		case GSWINMENUINIT:
		case GSWINMENU:
			WinMenu();
			break;
		case GSLEVELSELECTMENUINIT:
		case GSLEVELSELECTMENU:
			LevelSelectMenu();
			break;
		case GSTITLESCREENINIT:
		case GSTITLESCREEN:
			TitleScreenMenu();
			break;
		case GSLEVELTYPESELECTINIT:
		case GSLEVELTYPESELECT:
			LevelTypeMenu();
			break;
		case GSMOVEPIECEITTER:
			MovePieceIter();
			break;

		case GSLEVELEDITORINIT:
		case GSLEVELEDITOR:
			LevelEditor();
			break;
		case GSLEVELEDITORMENUINIT:
		case GSLEVELEDITORMENU:
			LevelEditorMenu();
			break;
		case GSLEVELEDITORLEVELSELECTINIT:
		case GSLEVELEDITORLEVELSELECT:
			LevelEditorLevelSelect();
			break;
		case GSCREDITSINIT:
		case GSCREDITS:
			Credits();
			break;
		case GSGETNAMEINIT:
		case GSGETNAME:
			doGetName();
			break;
		case GSPRINTMSGINIT:
		case GSPRINTMSG:
			doPrintMsg();
			break;	
		default:
			break;
	}
	
	if (showFPS)
	{
		pd->system->drawFPS(0,0);
		Frames++;
		if (pd->system->getCurrentTimeMilliseconds() - FrameTime >= 1000)
		{
			CurrentMs = (float)(1000.0f / Frames);
			Frames = 0;
			FrameTime += 1000;
		}

		char* TmpText;
		pd->system->formatString(&TmpText, "FPS: %.0f\n", 1000.0f / CurrentMs);
		pd->system->realloc(TmpText, 0);

	}

	return 1;
}

#ifdef __cplusplus
#ifndef SDL2API
extern "C" {
#endif
#endif

#ifdef _WINDLL
__declspec(dllexport)
#endif

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	if ( event == kEventInit )
	{
		#ifndef SDL2API
		eventHandler_pdnewlib(pd, event, arg);
		#endif
		setPDPtr(playdate);
		playdate->display->setRefreshRate(FRAMERATE);
		playdate->display->setOffset(40,0);
		playdate->system->setUpdateCallback(mainLoop, NULL);
		setupGame();
	}

	if (event == kEventTerminate)
	{
		destroyGame();
	}
	return 0;
}

#ifdef __cplusplus
#ifndef SDL2API
}
#endif
#endif