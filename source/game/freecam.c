#include "freecam.h"
#include "freecam_rooms.h"

#include <sys/types.h>
#include <libgte.h>

#include "common.h"
#include "libgv/libgv.h"
#include "game/game.h"
#include <stddef.h>

/* Input source for camera yaw.
 *   0 = auto-rotate (no controller required, ships as v1 default)
 *   1 = right stick (requires analog controller / PCSX-Redux analog mapping)
 */
#define FREECAM_YAW_SOURCE 0

extern UnkCameraStruct2 gUnkCameraStruct2_800B7868;
extern short            area_name;

static const FreeCamRoomConfig *g_current_room = NULL;
static short g_yaw      = 0;
static short g_pitch    = 0x0200;
static int   g_distance = 1200;

void FreeCam_Init(void)
{
    g_current_room = NULL;
    g_yaw      = 0;
    g_pitch    = 0x0200;
    g_distance = 1200;
}

void FreeCam_OnStageChange(void)
{
    g_current_room = FreeCam_LookupRoom((int)area_name);
}

int FreeCam_IsActive(void)
{
    return g_current_room != NULL;
}

short FreeCam_GetYaw(void)
{
    return g_yaw;
}

void FreeCam_Tick(void)
{
    const FreeCamRoomConfig *room = g_current_room;
    SVECTOR eye;
    SVECTOR center;
    int     cos_p, sin_p, cos_y, sin_y;
    int     horiz;

    if (room == NULL) { return; }

#if FREECAM_YAW_SOURCE == 0
    /* Auto-rotate: ~one revolution per ~8.5s @ 60fps. */
    g_yaw = (short)((g_yaw + 8) & 0x0FFF);
#else
    {
        GV_PAD *pad = &GV_PadData[0];
        int     rx, ry;

        rx = (int)pad->right_dx - 0x80;
        ry = (int)pad->right_dy - 0x80;
        if (rx > -8 && rx < 8) { rx = 0; }
        if (ry > -8 && ry < 8) { ry = 0; }

        g_yaw   = (short)((g_yaw + (rx >> 2)) & 0x0FFF);
        g_pitch = (short)(g_pitch + (ry >> 3));
        if (g_pitch < room->min_pitch) { g_pitch = room->min_pitch; }
        if (g_pitch > room->max_pitch) { g_pitch = room->max_pitch; }
    }
#endif

    cos_y = rcos(g_yaw);
    sin_y = rsin(g_yaw);
    cos_p = rcos(g_pitch);
    sin_p = rsin(g_pitch);

    horiz = (g_distance * cos_p) >> 12;

    eye.vx = GM_PlayerPosition.vx + (short)((horiz * sin_y) >> 12);
    eye.vz = GM_PlayerPosition.vz + (short)((horiz * cos_y) >> 12);
    eye.vy = GM_PlayerPosition.vy - (short)((g_distance * sin_p) >> 12);
    eye.pad = 0;

    if (eye.vx < room->bounds_min.vx) { eye.vx = room->bounds_min.vx; }
    if (eye.vx > room->bounds_max.vx) { eye.vx = room->bounds_max.vx; }
    if (eye.vy < room->bounds_min.vy) { eye.vy = room->bounds_min.vy; }
    if (eye.vy > room->bounds_max.vy) { eye.vy = room->bounds_max.vy; }
    if (eye.vz < room->bounds_min.vz) { eye.vz = room->bounds_min.vz; }
    if (eye.vz > room->bounds_max.vz) { eye.vz = room->bounds_max.vz; }

    center = GM_PlayerPosition;
    center.vy -= 200;
    center.pad = 0;

    gUnkCameraStruct2_800B7868.eye    = eye;
    gUnkCameraStruct2_800B7868.center = center;
    gUnkCameraStruct2_800B7868.zoom   = 320;
}
