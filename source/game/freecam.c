#include "freecam.h"
#include "freecam_rooms.h"

#include <sys/types.h>
#include <libgte.h>

#include "common.h"
#include "libgv/libgv.h"
#include "game/game.h"
#include "linkvar.h"
#include <stddef.h>

/* Over-the-shoulder placement (active while Snake is aiming a weapon). */
#define OTS_YAW_FLIP          0x800   /* 180 deg, put camera behind Snake (not in front) */
#define OTS_SHOULDER_OFFSET   0x100   /* ~45 deg shoulder displacement */
#define OTS_PITCH             0x0080  /* gentle downward tilt */
#define OTS_DISTANCE          1000    /* further from Snake for breathing room */
#define OTS_CENTER_VY        (-350)   /* look-at point near Snake's head */

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
    int     local_distance;
    int     local_pitch;
    int     local_center_vy;
    SVECTOR eye;
    SVECTOR center;
    int     cos_p, sin_p, cos_y, sin_y;
    int     horiz;

    if (room == NULL) { return; }

    pad = &GV_PadData[0];
    /* "Shoot mode" = Square held with a weapon equipped. Mirrors sna_8005009C's
     * aim/fire branch in sna_init.c so the camera engages exactly when Snake's
     * weapon animation does. */
    ots_active = ((pad->status & PAD_SQUARE) != 0) && (GM_CurrentWeaponId != WP_None);

    if (ots_active)
    {
        g_yaw           = (short)((GM_PlayerHeading + OTS_YAW_FLIP + OTS_SHOULDER_OFFSET) & 0x0FFF);
        local_pitch     = OTS_PITCH;
        local_distance  = OTS_DISTANCE;
        local_center_vy = OTS_CENTER_VY;
    }
    else
    {
        /* Auto-rotate orbit: ~one revolution per ~8.5s @ 60fps. */
        g_yaw           = (short)((g_yaw + 8) & 0x0FFF);
        local_pitch     = g_pitch;
        local_distance  = g_distance;
        local_center_vy = -200;
    }

    cos_y = rcos(g_yaw);
    sin_y = rsin(g_yaw);
    cos_p = rcos(local_pitch);
    sin_p = rsin(local_pitch);

    horiz = (local_distance * cos_p) >> 12;

    eye.vx = GM_PlayerPosition.vx + (short)((horiz * sin_y) >> 12);
    eye.vz = GM_PlayerPosition.vz + (short)((horiz * cos_y) >> 12);
    eye.vy = GM_PlayerPosition.vy - (short)((local_distance * sin_p) >> 12);
    eye.pad = 0;

    if (eye.vx < room->bounds_min.vx) { eye.vx = room->bounds_min.vx; }
    if (eye.vx > room->bounds_max.vx) { eye.vx = room->bounds_max.vx; }
    if (eye.vy < room->bounds_min.vy) { eye.vy = room->bounds_min.vy; }
    if (eye.vy > room->bounds_max.vy) { eye.vy = room->bounds_max.vy; }
    if (eye.vz < room->bounds_min.vz) { eye.vz = room->bounds_min.vz; }
    if (eye.vz > room->bounds_max.vz) { eye.vz = room->bounds_max.vz; }

    center = GM_PlayerPosition;
    center.vy += local_center_vy;
    center.pad = 0;

    gUnkCameraStruct2_800B7868.eye    = eye;
    gUnkCameraStruct2_800B7868.center = center;
    gUnkCameraStruct2_800B7868.zoom   = 320;
}
