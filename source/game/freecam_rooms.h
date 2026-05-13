#ifndef __MGS_GAME_FREECAM_ROOMS_H__
#define __MGS_GAME_FREECAM_ROOMS_H__

#include <sys/types.h>
#include <libgte.h>

typedef struct FreeCamRoomConfig
{
    int      stage_id;
    SVECTOR  bounds_min;
    SVECTOR  bounds_max;
    short    min_distance;
    short    max_distance;
    short    min_pitch;
    short    max_pitch;
} FreeCamRoomConfig;

const FreeCamRoomConfig *FreeCam_LookupRoom(int stage_id);

#endif
