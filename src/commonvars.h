#ifndef COMMONVARS_H
#define COMMONVARS_H

#include <pd_api.h>
#include <stdint.h>
#include <stdio.h>
#include "dynamate_base.h"
#include "dynamate_levels.h"
#include "cursor.h"
#include "CInput.h"
#include "SFont.h"

#define FRAMERATE 30
#define WINDOW_WIDTH 320 
#define WINDOW_HEIGHT 240
#define TileWidth 15
#define TileHeight 15

extern bool showFPS;
extern unsigned int prevLogTime;
extern unsigned int FrameTime, Frames;
extern float CurrentMs;
extern bool BatteryMonitoring;

extern LCDBitmap *LevelData,
	*Screen,
	*Background,
	*BackgroundLevelEditor,
	*Menu,
	*LevelTitle,
	*IMGIntro1,
	*IMGIntro2,
	*IMGIntro3,
	*IMGIntro4,
	*Unlocked,
	*Locked,
	*PieceOutsidePlayfield,
	*InfiniteLoop,
	*DeathPiece,
	*OutOfMoves,
	*LevelCompleted,
	*AllLevelsCompleted,
	*TitleScreen,
	*SFontDark,
	*SFontLight;

extern float menuAlpha;
extern int Selected;
extern uint32_t Time1;
extern int IntroScreenNr;
extern CInput * Input;
extern SFont_Font *FontDark, *FontLight;
extern CCursor * Cursor;
extern uint32_t NextTime;
extern int GameState, NextGameState;
extern int HighScores[MAXSTATICLEVELS];
extern int lvl, MaxLevels, UserLevelCount, MusicCount, SelectedMusic, Volume, StartScreenX, StartScreenY, InitialStartScreenX, InitialStartScreenY;
extern bool StaticLevels, SaveEnabled;
extern char GetNameText[256];
extern bool GetNameResult;
extern bool GetNameDone;
extern bool printMsgDone;
extern const char *printMsgText;
extern const char *printMsgTitle;
extern int GetNameX;
extern int GetNameY;
//extern TTF_Font *Font, *SmallFont;
//extern SDL_Color Brown = { 0, 0, 0, 255
//}, DarkBlue = { 0, 0, 0, 255
//}, Color = { 0, 0, 0, 255 };
extern LCDFont *roobert;
extern char UnlockData[30];
extern dm_user_lev UserLevel;
extern char UserLevelsFilenames[MAXUSERLEVELS][256];
extern uint32_t Time;
extern int Sounds[NROFSOUNDS];
extern int Music[MAXMUSICFILES];
extern int Selection;
extern dm_u8 SelectedPiece;

#endif