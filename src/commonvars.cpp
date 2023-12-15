#include <stdlib.h>
#include <stdint.h>
#include "commonvars.h"
#include "dynamate_base.h"
#include "SFont.h"

unsigned int prevLogTime;
unsigned int FrameTime, Frames;
float CurrentMs;
bool BatteryMonitoring;
bool showFPS = false;

LCDBitmap *LevelData,
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

float menuAlpha = 0.75f;
int Selected = 0;
unsigned int Time1 = 0;
int IntroScreenNr = 1;
CInput * Input;
SFont_Font *FontDark, *FontLight;
CCursor * Cursor;
int GameState = GSINTROINIT, NextGameState = -1;
int HighScores[MAXSTATICLEVELS];
int lvl = 0, MaxLevels, UserLevelCount = 0, MusicCount = 0, SelectedMusic = 0, Volume = 128, StartScreenX = 0, StartScreenY = 0, InitialStartScreenX = 20, InitialStartScreenY = 24;
bool StaticLevels, SaveEnabled = false, TvOutMode = false;
char GetNameText[256];
const char *printMsgText;
const char *printMsgTitle;
bool GetNameResult;
bool GetNameDone;
bool printMsgDone;
int GetNameX;
int GetNameY;

//TTF_Font *Font, *SmallFont;
LCDFont *roobert;
//SDL_Color Brown = { 0, 0, 0, 255
//}, DarkBlue = { 0, 0, 0, 255
//}, Color = { 0, 0, 0, 255 };

char UnlockData[30];
dm_user_lev UserLevel;
char UserLevelsFilenames[MAXUSERLEVELS][256];
int Sounds[NROFSOUNDS];
int Music[MAXMUSICFILES];
int Selection = 0;
bool done = false;
dm_u8 SelectedPiece = 0x80 | DM_P_VERT;
