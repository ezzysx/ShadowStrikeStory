#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include "state.h"
typedef struct {uint64_t hash;int calls;bool failShadow;} Mock;
static bool SetMock(void *ctx,uint64_t hash) {
    Mock *m=ctx;m->calls++;
    if(m->failShadow && hash==SHADOW_HASH){m->hash=0;return false;}
    m->hash=hash;return true;
}
int main(void) {
    SwapState s={0};Mock m={.hash=123};
    assert(UpdateSwap(&s,1,2,false,true,123,true,SetMock,&m)==0 && m.calls==0);
    assert(UpdateSwap(&s,1,2,true,true,123,false,SetMock,&m)==0 && m.calls==0);
    assert(UpdateSwap(&s,1,2,true,true,123,true,SetMock,&m)==1 && m.hash==SHADOW_HASH && s.original==123);
    assert(UpdateSwap(&s,1,2,true,true,m.hash,true,SetMock,&m)==0 && m.calls==1);
    assert(UpdateSwap(&s,1,2,true,false,m.hash,true,SetMock,&m)==2 && m.hash==123);
    assert(UpdateSwap(&s,1,2,true,true,m.hash,true,SetMock,&m)==1);
    /* Returning to Legends with the same entity restores what this plugin owned. */
    assert(UpdateSwap(&s,1,2,false,true,m.hash,true,SetMock,&m)==2 && m.hash==123);
    /* Reload/death: new identity receives its own backup. */
    m.hash=456;
    assert(UpdateSwap(&s,3,4,true,true,m.hash,true,SetMock,&m)==1 && s.original==456);
    assert(UpdateSwap(&s,3,4,true,false,m.hash,true,SetMock,&m)==2 && m.hash==456);
    /* Existing Shadow Strike installed elsewhere must not be claimed. */
    m.hash=SHADOW_HASH;
    assert(UpdateSwap(&s,5,6,true,true,m.hash,true,SetMock,&m)==0 && !s.applied);
    /* Engine resets same owner's slot: capture the replacement before applying. */
    m.hash=789;
    assert(UpdateSwap(&s,7,8,true,true,m.hash,true,SetMock,&m)==1);
    m.hash=999;
    assert(UpdateSwap(&s,7,8,true,true,m.hash,true,SetMock,&m)==1 && s.original==999);
    assert(UpdateSwap(&s,7,8,true,false,m.hash,true,SetMock,&m)==2 && m.hash==999);
    /* Native failure can clear slot; restore and stop repeated destructive attempts. */
    m.hash=555;m.failShadow=true;
    assert(UpdateSwap(&s,9,10,true,true,m.hash,true,SetMock,&m)==-1 && m.hash==555 && s.blocked);
    int calls=m.calls;
    assert(UpdateSwap(&s,9,10,true,true,m.hash,true,SetMock,&m)==0 && m.calls==calls);
    puts("State tests passed: apply, restore, Legends, reload, external ownership, slot reset, failure recovery.");
}
