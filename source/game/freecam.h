#ifndef __MGS_GAME_FREECAM_H__
#define __MGS_GAME_FREECAM_H__

#include "game/camera.h"

void FreeCam_Init(void);
void FreeCam_OnStageChange(void);
int  FreeCam_IsActive(void);
void FreeCam_Tick(void);
short FreeCam_GetYaw(void);

/* OTS free-look entry snap. Returns 1 ONCE on the rising edge of OTS aim,
 * populates *out_heading with the absolute heading Snake should be set to
 * (cam-look direction). Returns 0 on all other frames. Caller writes
 * out_heading to work->control.turn.vy. */
int FreeCam_ConsumeAimSnap(short *out_heading);

/* OTS free-look per-frame yaw nudge. Returns 1 each frame OTS is active,
 * populates *out_inc with the signed RX-derived delta (libgte units) to
 * ADD to Snake's current turn.vy. Resets to 0 after consume so a single
 * frame's input is applied exactly once. */
int FreeCam_ConsumeAimYawInc(short *out_inc);

#endif
