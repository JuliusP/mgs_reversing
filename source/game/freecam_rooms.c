#include "freecam_rooms.h"
#include "strcode.h"
#include <stddef.h>

static const FreeCamRoomConfig freecam_rooms[] = {
    /* Loading dock (s00a). Bounds are placeholders; tuned in Task 10. */
    {
        STAGE_s00a,
        { -8000, -2000, -8000, 0 },
        {  8000,  2000,  8000, 0 },
        640, 2400,
        -0x0200, 0x0300,
    },
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
