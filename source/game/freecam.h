#ifndef __MGS_GAME_FREECAM_H__
#define __MGS_GAME_FREECAM_H__

#include "game/camera.h"

void FreeCam_Init(void);
void FreeCam_OnStageChange(void);
int  FreeCam_IsActive(void);
void FreeCam_Tick(void);
short FreeCam_GetYaw(void);

/* OTS free-look: if OTS is active this frame, populates *out_heading with
 * the right-stick-driven heading (libgte units, 0..0x0FFF) and returns 1.
 * Returns 0 when OTS is not active — callers fall back to vanilla logic. */
int FreeCam_GetAimOverride(short *out_heading);

#endif
