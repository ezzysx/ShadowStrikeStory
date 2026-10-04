#pragma once
#include <stdint.h>
#include <stdbool.h>
#define SHADOW_HASH UINT64_C(0xE13C9D23D48DFF5E)
typedef struct { uintptr_t player, manager; uint64_t original; bool applied, blocked; } SwapState;
typedef bool (*SetAsset)(void *context, uint64_t hash);
/* Runs on the game thread. A new entity never inherits another entity's backup. */
static int UpdateSwap(SwapState *s, uintptr_t player, uintptr_t manager,
                      bool story, bool enabled, uint64_t current,
                      bool shadowAvailable, SetAsset set, void *ctx) {
    if (player!=s->player || manager!=s->manager) {
        *s=(SwapState){.player=player,.manager=manager};
    }
    if (!player || !manager || !current) return 0;
    if (!story || !enabled) {
        if (s->applied && current==SHADOW_HASH && s->original && set(ctx,s->original)) {
            s->applied=false; return 2;
        }
        return 0;
    }
    if (s->blocked) return 0;
    if (s->applied) {
        if (current==SHADOW_HASH) return 0;
        /* Engine rebuilt the slot without replacing the owner: capture its new value. */
        s->applied=false;
    }
    /* Do not claim ownership of an ability installed by another mod. */
    if (current==SHADOW_HASH || !shadowAvailable) return 0;
    s->original=current;
    if (set(ctx,SHADOW_HASH)) { s->applied=true; return 1; }
    /* Native setter may have removed the old entry before failing. */
    set(ctx,s->original);
    s->blocked=true;
    return -1;
}
