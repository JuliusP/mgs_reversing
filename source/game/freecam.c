#include "freecam.h"
#include "freecam_rooms.h"

#include <sys/types.h>
#include <libgte.h>

#include "common.h"
#include "libgv/libgv.h"
#include "game/game.h"
#include "linkvar.h"
#include <stddef.h>

/* Over-the-shoulder placement (active while Snake is aiming a weapon).
 * The OTS path is a direct translation in world space, not a spherical orbit:
 *   eye    = Snake + right * SHOULDER_DIST + up * EYE_HEIGHT - forward * BEHIND_DIST
 *   center = Snake + forward * LOOK_FORWARD + up * LOOK_HEIGHT
 * Look-at sits far ahead so the camera looks where Snake aims, not at Snake. */
#define OTS_SHOULDER_DIST   500    /* lateral world units to Snake's right */
#define OTS_BEHIND_DIST     400    /* world units behind Snake's facing */
#define OTS_EYE_HEIGHT      500    /* eye above feet (PSX -Y is up) */
#define OTS_LOOK_FORWARD    8000   /* far-ahead target along Snake's facing */
#define OTS_LOOK_HEIGHT     500    /* aim point above feet */

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
    GV_PAD *pad;
    int     ots_active;
    SVECTOR eye;
    SVECTOR center;

    if (room == NULL) { return; }

    pad = &GV_PadData[0];
    /* "Shoot mode" = Square held with a weapon equipped. Mirrors sna_8005009C's
     * aim branch in sna_init.c so the camera engages exactly when Snake's
     * weapon animation does. */
    ots_active = ((pad->status & PAD_SQUARE) != 0) && (GM_CurrentWeaponId != WP_None);

    if (ots_active)
    {
        int sin_h = rsin(GM_PlayerHeading);
        int cos_h = rcos(GM_PlayerHeading);

        /* Eye: lateral right + slight behind, raised. */
        eye.vx = GM_PlayerPosition.vx
               + (short)((cos_h * OTS_SHOULDER_DIST) >> 12)
               - (short)((sin_h * OTS_BEHIND_DIST)   >> 12);
        eye.vz = GM_PlayerPosition.vz
               - (short)((sin_h * OTS_SHOULDER_DIST) >> 12)
               - (short)((cos_h * OTS_BEHIND_DIST)   >> 12);
        eye.vy = GM_PlayerPosition.vy - OTS_EYE_HEIGHT;
        eye.pad = 0;

        /* Look-at: far ahead of Snake along his facing. */
        center.vx = GM_PlayerPosition.vx + (short)((sin_h * OTS_LOOK_FORWARD) >> 12);
        center.vz = GM_PlayerPosition.vz + (short)((cos_h * OTS_LOOK_FORWARD) >> 12);
        center.vy = GM_PlayerPosition.vy - OTS_LOOK_HEIGHT;
        center.pad = 0;
    }
    else
    {
        int cos_p, sin_p, cos_y, sin_y;
        int horiz;

        /* Auto-rotate orbit: ~one revolution per ~8.5s @ 60fps. */
        g_yaw = (short)((g_yaw + 8) & 0x0FFF);

        cos_y = rcos(g_yaw);
        sin_y = rsin(g_yaw);
        cos_p = rcos(g_pitch);
        sin_p = rsin(g_pitch);

        horiz = (g_distance * cos_p) >> 12;

        eye.vx = GM_PlayerPosition.vx + (short)((horiz * sin_y) >> 12);
        eye.vz = GM_PlayerPosition.vz + (short)((horiz * cos_y) >> 12);
        eye.vy = GM_PlayerPosition.vy - (short)((g_distance * sin_p) >> 12);
        eye.pad = 0;

        center = GM_PlayerPosition;
        center.vy -= 200;
        center.pad = 0;
    }

    if (eye.vx < room->bounds_min.vx) { eye.vx = room->bounds_min.vx; }
    if (eye.vx > room->bounds_max.vx) { eye.vx = room->bounds_max.vx; }
    if (eye.vy < room->bounds_min.vy) { eye.vy = room->bounds_min.vy; }
    if (eye.vy > room->bounds_max.vy) { eye.vy = room->bounds_max.vy; }
    if (eye.vz < room->bounds_min.vz) { eye.vz = room->bounds_min.vz; }
    if (eye.vz > room->bounds_max.vz) { eye.vz = room->bounds_max.vz; }

    gUnkCameraStruct2_800B7868.eye    = eye;
    gUnkCameraStruct2_800B7868.center = center;
    gUnkCameraStruct2_800B7868.zoom   = 320;
}
