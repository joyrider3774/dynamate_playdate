#include "CInput.h"

#include "pd_api.h"
#include "crank.h"

CInput::CInput(PlaydateAPI *pd_api, int UpdateCounterDelay) {
    Reset();
	pda = pd_api;
    PUpdateCounterDelay = UpdateCounterDelay;
    UpdateCounter = 0;
	setCrankMoveThreshold(45);
}

CInput::~CInput() {
}

void CInput::Update() {
    //printf("Cinput::update:0\n");
    if (UpdateCounter > 0)
	{
        UpdateCounter--;
	}
	 
	int ret = crankUpdate();
    KeyboardHeld[SDLK_PAGEDOWN] = ret == CRANKMOVELEFT;
	KeyboardHeld[SDLK_PAGEUP] = ret == CRANKMOVERIGHT;

	PDButtons buttons;
	pda->system->getButtonState(&buttons, NULL, NULL);
	KeyboardHeld[SDLK_a] = (buttons & kButtonA);
	KeyboardHeld[SDLK_b] = (buttons & kButtonB);
	KeyboardHeld[SDLK_UP] = (buttons & kButtonUp);
	KeyboardHeld[SDLK_DOWN] = (buttons & kButtonDown);
	KeyboardHeld[SDLK_LEFT] = (buttons & kButtonLeft);
	KeyboardHeld[SDLK_RIGHT] = (buttons & kButtonRight);
}

void CInput::Reset() {
    int x;
    for (x=0;x<SDLK_LAST;x++)
        KeyboardHeld[x] = false;

}

