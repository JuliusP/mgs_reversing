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
 * Look-at sits far ahead so the camera looks where Snake aims, not at Snake.
 * NOTE: empirically +Y is UP in this engine (cam was previously rendered
 * underneath Snake when subtracting heights — see commit history). */
#define OTS_SHOULDER_DIST  (-500)  /* negative = Snake's right side under this engine's axes */
#define OTS_BEHIND_DIST     1500   /* further back so Snake's shoulder sits in frame */
#define OTS_EYE_HEIGHT      700    /* eye above Snake (added to vy) */
#define OTS_LOOK_FORWARD    8000   /* far-ahead target along Snake's facing */
#define OTS_LOOK_HEIGHT     700    /* aim point above Snake, matches eye for level look */

/* OTS free-look tuning. Shifts match the orbit cam's live values so feel is
 * consistent between modes. Pitch limit ~22° each way; eye/center offsets
 * scale g_ots_pitch into world units via >> 4 in the read sites. */
#define OTS_YAW_SHIFT     1
#define OTS_PITCH_SHIFT   2
#define OTS_PITCH_LIMIT   0x100
#define OTS_PITCH_EYE_K   3
#define OTS_PITCH_LOOK_K  8

extern UnkCameraStruct2 gUnkCameraStruct2_800B7868;
extern short            area_name;

static const FreeCamRoomConfig *g_current_room = NULL;
static short g_yaw      = 0;
static short g_pitch    = 0x0200;
static int   g_distance = 3800;

/* OTS free-look state. g_ots_active is the "this frame" flag consumed by
 * FreeCam_GetAimOverride; the previous frame's value is captured into a
 * local at the head of FreeCam_Tick for edge detection. */
static short g_ots_yaw_delta    = 0;
static short g_ots_pitch        = 0;
static short g_ots_base_heading = 0;
static char  g_ots_active       = 0;

void FreeCam_Init(void)
{
    g_current_room = NULL;
    g_yaw      = 0;
    g_pitch    = 0x0200;
    g_distance = 3200;

    g_ots_yaw_delta    = 0;
    g_ots_pitch        = 0;
    g_ots_base_heading = 0;
    g_ots_active       = 0;
}

void FreeCam_OnStageChange(void)
{
    g_current_room = FreeCam_LookupRoom((int)area_name);
    g_ots_active   = 0;
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
    int     pad_origin;
    SVECTOR eye;
    SVECTOR center;

    if (room == NULL) { return; }

    pad = &GV_PadData[0];
    {
        int was_active;
        /* "Shoot mode" = Square held with a weapon equipped. Mirrors sna_8005009C's
         * aim branch in sna_init.c so the camera engages exactly when Snake's
         * weapon animation does. */
        was_active = (int)g_ots_active;
        ots_active = ((pad->status & PAD_SQUARE) != 0) && (GM_CurrentWeaponId != WP_None);
        g_ots_active = (char)ots_active;

        /* Rising edge: snap Snake's heading to where the orbit cam was
         * looking. (g_yaw + 2048) is the inverse of the OTS pad_origin
         * relation; it converts cam-yaw into Snake-facing. RE4 convention:
         * entering aim turns Snake to camera direction, cam stays put. */
        if (!was_active && ots_active)
        {
            g_ots_base_heading = (short)((g_yaw + 2048) & 0x0FFF);
            g_ots_yaw_delta    = 0;
            g_ots_pitch        = 0;
        }

        /* Falling edge: orbit cam returns already behind Snake's new facing.
         * +2048 matches the engine pad-dir convention used in the orbit branch. */
        if (was_active && !ots_active)
        {
            g_yaw = (short)((GM_PlayerHeading + 2048) & 0x0FFF);
        }

        /* Integrate right stick into yaw/pitch deltas while OTS is active.
         * Pitch is clamped here; Phase 4's eye/center reads use g_ots_pitch
         * directly. */
        if (ots_active)
        {
            int rx = (int)pad->right_dx - 0x80;
            int ry = (int)pad->right_dy - 0x80;
            if (rx > -8 && rx < 8) { rx = 0; }
            if (ry > -8 && ry < 8) { ry = 0; }

            g_ots_yaw_delta = (short)(g_ots_yaw_delta - (rx >> OTS_YAW_SHIFT));
            g_ots_pitch     = (short)(g_ots_pitch     + (ry >> OTS_PITCH_SHIFT));
            if (g_ots_pitch < -OTS_PITCH_LIMIT) { g_ots_pitch = -OTS_PITCH_LIMIT; }
            if (g_ots_pitch >  OTS_PITCH_LIMIT) { g_ots_pitch =  OTS_PITCH_LIMIT; }
        }
    }

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
        eye.vy = GM_PlayerPosition.vy + OTS_EYE_HEIGHT
               + (short)((g_ots_pitch * OTS_PITCH_EYE_K) >> 4);
        eye.pad = 0;

        /* Look-at: far ahead of Snake along his facing. */
        center.vx = GM_PlayerPosition.vx + (short)((sin_h * OTS_LOOK_FORWARD) >> 12);
        center.vz = GM_PlayerPosition.vz + (short)((cos_h * OTS_LOOK_FORWARD) >> 12);
        center.vy = GM_PlayerPosition.vy + OTS_LOOK_HEIGHT
                  - (short)((g_ots_pitch * OTS_PITCH_LOOK_K) >> 4);
        center.pad = 0;

        /* Eye is behind Snake along his heading; cam-forward = heading.
         * +2048 empirical: engine's pad-dir convention is opposite of cam-forward. */
        pad_origin = (GM_PlayerHeading + 2048) & 0x0FFF;
    }
    else
    {
        int cos_p, sin_p, cos_y, sin_y;
        int horiz;

        /* Right stick drives yaw and pitch. Fields are unsigned 0..255 centered
         * at 0x80; subtract to get signed deltas (-128..127), deadzone, then
         * integrate into g_yaw (wrap) and g_pitch (clamped to room range).
         * Sensitivity shifts: >> 1 yaw, >> 2 pitch (yaw twice as responsive).
         * Yaw negated so stick-right rotates cam right; pitch left + (stick-up
         * tilts cam down, MGSV / inverted-Y convention). */
        {
            int rx = (int)pad->right_dx - 0x80;
            int ry = (int)pad->right_dy - 0x80;
            if (rx > -8 && rx < 8) { rx = 0; }
            if (ry > -8 && ry < 8) { ry = 0; }

            g_yaw   = (short)((g_yaw - (rx >> 1)) & 0x0FFF);
            g_pitch = (short)(g_pitch + (ry >> 2));
            if (g_pitch < room->min_pitch) { g_pitch = room->min_pitch; }
            if (g_pitch > room->max_pitch) { g_pitch = room->max_pitch; }
        }

        cos_y = rcos(g_yaw);
        sin_y = rsin(g_yaw);
        cos_p = rcos(g_pitch);
        sin_p = rsin(g_pitch);

        horiz = (g_distance * cos_p) >> 12;

        eye.vx = GM_PlayerPosition.vx + (short)((horiz * sin_y) >> 12);
        eye.vz = GM_PlayerPosition.vz + (short)((horiz * cos_y) >> 12);
        eye.vy = GM_PlayerPosition.vy + (short)((g_distance * sin_p) >> 12);
        eye.pad = 0;

        center = GM_PlayerPosition;
        center.vy += 200;
        center.pad = 0;

        /* Eye sits at angle g_yaw around Snake; pad-dir uses opposite convention. */
        pad_origin = g_yaw & 0x0FFF;
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

    GV_OriginPadSystem(pad_origin);
}

int FreeCam_GetAimOverride(short *out_heading)
{
    if (!g_ots_active) { return 0; }
    *out_heading = (short)((g_ots_base_heading + g_ots_yaw_delta) & 0x0FFF);
    return 1;
}
