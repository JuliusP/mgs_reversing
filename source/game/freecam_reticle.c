#include "freecam.h"

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

#include "common.h"
#include "libgv/libgv.h"
#include "libdg/libdg.h"
#include "game/game.h"

/* Static four-dot "+" reticle for OTS aim mode.
 *
 * Allocates one DG_PRIM_TILE prim with 4 packs at actor creation; positions
 * and colors are fixed for the prim's lifetime. Each frame's Act flips
 * visibility based on FreeCam_IsAimActive() — no per-frame primitive churn.
 *
 *   ·
 *  · ·
 *   ·
 *
 * Spacing and dot size are tunable below. Future: dynamic spread on
 * walk/shoot if desired. */

#define EXEC_LEVEL          GV_ACTOR_AFTER

#define RETICLE_CENTER_X    (SCREEN_WIDTH  / 2)
#define RETICLE_CENTER_Y    (SCREEN_HEIGHT / 2)
#define RETICLE_OFFSET      10
#define RETICLE_DOT_SIZE    2
#define RETICLE_R           255
#define RETICLE_G           255
#define RETICLE_B           255

typedef struct _Work
{
    GV_ACT   actor;
    DG_PRIM *prim;
} Work;

static SVECTOR s_reticle_vecs[4]; /* zero — per-pack world offsets unused */

static void PlaceDot(TILE *tile, int cx, int cy)
{
    setTile(tile);
    tile->x0 = (short)(cx - RETICLE_DOT_SIZE / 2);
    tile->y0 = (short)(cy - RETICLE_DOT_SIZE / 2);
    tile->w  = RETICLE_DOT_SIZE;
    tile->h  = RETICLE_DOT_SIZE;
    setRGB0(tile, RETICLE_R, RETICLE_G, RETICLE_B);
}

static void Act(Work *work)
{
    if (work->prim == NULL) { return; }

    if (FreeCam_IsAimActive())
    {
        DG_VisiblePrim(work->prim);
    }
    else
    {
        DG_InvisiblePrim(work->prim);
    }
}

static void Die(Work *work)
{
    if (work->prim != NULL)
    {
        GM_FreePrim(work->prim);
        work->prim = NULL;
    }
}

void FreeCamReticle_NewActor(void)
{
    Work *work = GV_NewActor(EXEC_LEVEL, sizeof(Work));
    if (work == NULL) { return; }

    GV_SetNamedActor(&work->actor, &Act, &Die, "freecam_reticle.c");

    work->prim = GM_MakePrim(DG_PRIM_OFFSET | DG_PRIM_TILE, 4, s_reticle_vecs, NULL);
    if (work->prim == NULL) { return; }

    work->prim->world = DG_ZeroMatrix;

    /* + arrangement around screen center. */
    PlaceDot((TILE *)work->prim->packs[0], RETICLE_CENTER_X,                  RETICLE_CENTER_Y - RETICLE_OFFSET);
    PlaceDot((TILE *)work->prim->packs[1], RETICLE_CENTER_X,                  RETICLE_CENTER_Y + RETICLE_OFFSET);
    PlaceDot((TILE *)work->prim->packs[2], RETICLE_CENTER_X - RETICLE_OFFSET, RETICLE_CENTER_Y);
    PlaceDot((TILE *)work->prim->packs[3], RETICLE_CENTER_X + RETICLE_OFFSET, RETICLE_CENTER_Y);

    DG_InvisiblePrim(work->prim);
}
