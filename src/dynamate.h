#ifndef DYNAMATE_H
#define DYNAMATE_H

#include <pd_api.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WINDLL
__declspec(dllexport)
#endif

int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg);

#ifdef __cplusplus
}
#endif

#endif