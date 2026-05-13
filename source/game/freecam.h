#ifndef __MGS_GAME_FREECAM_H__
#define __MGS_GAME_FREECAM_H__

#include "game/camera.h"

void FreeCam_Init(void);
void FreeCam_OnStageChange(void);
int  FreeCam_IsActive(void);
void FreeCam_Tick(void);
short FreeCam_GetYaw(void);

#endif
