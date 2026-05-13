#include "freecam_rooms.h"
#include "strcode.h"
#include <stddef.h>

/* Temporary: every overlay opted in with maximum SVECTOR bounds (~no clamp).
 * Per-room bounds will be tuned later. SHRT_MAX = 32767. */
#define FULL_ROOM(id) {                                         \
    (id),                                                       \
    { -32767, -32767, -32767, 0 },                              \
    {  32767,  32767,  32767, 0 },                              \
    640, 2400,                                                  \
    -0x0200, 0x0300                                             \
}

static const FreeCamRoomConfig freecam_rooms[] = {
    FULL_ROOM(STAGE_s00a),
    FULL_ROOM(STAGE_s01a),
    FULL_ROOM(STAGE_s02a),
    FULL_ROOM(STAGE_s02b),
    FULL_ROOM(STAGE_s02c),
    FULL_ROOM(STAGE_s02d),
    FULL_ROOM(STAGE_s02e),
    FULL_ROOM(STAGE_s03a),
    FULL_ROOM(STAGE_s03b),
    FULL_ROOM(STAGE_s03c),
    FULL_ROOM(STAGE_s03d),
    FULL_ROOM(STAGE_s03e),
    FULL_ROOM(STAGE_s04a),
    FULL_ROOM(STAGE_s04b),
    FULL_ROOM(STAGE_s04c),
    FULL_ROOM(STAGE_s05a),
    FULL_ROOM(STAGE_s06a),
    FULL_ROOM(STAGE_s07a),
    FULL_ROOM(STAGE_s07b),
    FULL_ROOM(STAGE_s07c),
    FULL_ROOM(STAGE_s08a),
    FULL_ROOM(STAGE_s08b),
    FULL_ROOM(STAGE_s08c),
    FULL_ROOM(STAGE_s09a),
    FULL_ROOM(STAGE_s10a),
    FULL_ROOM(STAGE_s11a),
    FULL_ROOM(STAGE_s11b),
    FULL_ROOM(STAGE_s11c),
    FULL_ROOM(STAGE_s11d),
    FULL_ROOM(STAGE_s11e),
    FULL_ROOM(STAGE_s11g),
    FULL_ROOM(STAGE_s11h),
    FULL_ROOM(STAGE_s11i),
    FULL_ROOM(STAGE_s12a),
    FULL_ROOM(STAGE_s12b),
    FULL_ROOM(STAGE_s12c),
    FULL_ROOM(STAGE_s13a),
    FULL_ROOM(STAGE_s14e),
    FULL_ROOM(STAGE_s15a),
    FULL_ROOM(STAGE_s15b),
    FULL_ROOM(STAGE_s15c),
    FULL_ROOM(STAGE_s16a),
    FULL_ROOM(STAGE_s16b),
    FULL_ROOM(STAGE_s16c),
    FULL_ROOM(STAGE_s16d),
    FULL_ROOM(STAGE_s17a),
    FULL_ROOM(STAGE_s18a),
    FULL_ROOM(STAGE_s19a),
    FULL_ROOM(STAGE_s19b),
    FULL_ROOM(STAGE_s20a),
};

#define FREECAM_ROOMS_COUNT (sizeof(freecam_rooms) / sizeof(freecam_rooms[0]))

const FreeCamRoomConfig *FreeCam_LookupRoom(int stage_id)
{
    int i;
    for (i = 0; i < (int)FREECAM_ROOMS_COUNT; ++i)
    {
        if (freecam_rooms[i].stage_id == stage_id)
        {
            return &freecam_rooms[i];
        }
    }
    return NULL;
}
