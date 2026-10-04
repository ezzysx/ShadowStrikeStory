#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "state.h"
#include "MinHook.h"

/* Supported Steam executable: 1053.9.0623.1807. Never use these RVAs on another build. */
#define SUPPORTED_TIMESTAMP 1782230973u
#define SUPPORTED_IMAGE_SIZE 58273792u
#define TICK_RVA 0x6DAAE0
#define SET_RVA 0xB16410
#define PLAYER_RVA 0x34C6F20
#define MODE_FUNC_RVA 0x59A800
void *ShadowTrampoline;
extern void ShadowHook(void);
static uintptr_t base,modeAddress,registryArrayAddress,registryCountAddress;
static HMODULE self;
static char configPath[MAX_PATH],logPath[MAX_PATH];
static SwapState state;
static bool enabled, toggleDown;
static DWORD hotkey;
static ULONGLONG lastTick,lastConfig;
static volatile LONG busy;
typedef void * (__fastcall *SetFn)(void*,unsigned,uint64_t,unsigned);
static bool Read(uintptr_t p,void *out,SIZE_T size) {
    SIZE_T got=0;
    return p && ReadProcessMemory(GetCurrentProcess(),(void*)p,out,size,&got) && got==size;
}
static uintptr_t Ptr(uintptr_t p) { uintptr_t v=0;Read(p,&v,8);return v; }
static unsigned U32(uintptr_t p) { unsigned v=0;Read(p,&v,4);return v; }
static uintptr_t Rip(uintptr_t p,int offset,int length) {
    int32_t disp=0;if(!Read(p+offset,&disp,4))return 0;return p+length+disp;
}
static void Log(const char *msg) {
    FILE *f=fopen(logPath,"a");if(f){fprintf(f,"[%llu] %s\n",GetTickCount64(),msg);fclose(f);}
}
static bool Registered(unsigned ref,uint64_t hash) {
    unsigned n=U32(registryCountAddress);uintptr_t a=Ptr(registryArrayAddress);
    if(!a || !n || n>100000)return false;
    for(unsigned i=0;i<n;i++) {
        struct { uintptr_t object; unsigned ref,pad; uint64_t hash; } e;
        if(!Read(a+i*24,&e,24))return false;
        if(e.ref==ref && e.hash==hash && e.object)return true;
    }
    return false;
}
static uint64_t CurrentUltimate(uintptr_t manager) {
    unsigned n=U32(manager+0x48);uintptr_t a=Ptr(manager+0x40);
    if(!a || !n || n>=256)return 0;
    for(unsigned i=0;i<n;i++) {
        uintptr_t e=Ptr(a+i*8),md=Ptr(e+0x28);
        if(md && U32(md+0x330)==0)return Ptr(md+0x338);
    }
    return 0;
}
typedef struct {uintptr_t player,manager;} ApplyContext;
static bool Set(void *opaque,uint64_t hash) {
    ApplyContext *c=opaque;
    if(Ptr(base+PLAYER_RVA)!=c->player || Ptr(c->player+0x9C8)!=c->manager || !Registered(0,hash))return false;
    void *value=((SetFn)(base+SET_RVA))((void*)c->manager,0,hash,0);
    return value && CurrentUltimate(c->manager)==hash;
}
void ShadowTick(void) {
    if(InterlockedCompareExchange(&busy,1,0))return;
    ULONGLONG now=GetTickCount64();
    if(now-lastTick<100)goto done;
    lastTick=now;
    if(now-lastConfig>=1000) {
        lastConfig=now;
        enabled=GetPrivateProfileIntA("ShadowStrike","Enabled",1,configPath)!=0;
        hotkey=GetPrivateProfileIntA("ShadowStrike","ToggleKey",VK_F8,configPath);
    }
    bool down=hotkey>0 && hotkey<256 && (GetAsyncKeyState((int)hotkey)&0x8000)!=0;
    if(down && !toggleDown) {
        enabled=!enabled;
        WritePrivateProfileStringA("ShadowStrike","Enabled",enabled?"1":"0",configPath);
        Log(enabled?"Enabled by hotkey":"Disabled by hotkey; restoration requested");
    }
    toggleDown=down;
    BYTE legends=1;
    if(!Read(modeAddress,&legends,1))goto done;
    uintptr_t p=Ptr(base+PLAYER_RVA),m=p?Ptr(p+0x9C8):0;
    ApplyContext c={p,m};
    uint64_t current=m?CurrentUltimate(m):0;
    bool available=legends==0 && enabled && Registered(0,SHADOW_HASH);
    int outcome=UpdateSwap(&state,p,m,legends==0,enabled,current,available,Set,&c);
    if(outcome==1)Log("Shadow Strike applied and slot verified");
    if(outcome==2)Log("Original Ultimate restored and slot verified");
    if(outcome<0)Log("Swap failed; recovery attempted; retries blocked for this entity");
done:
    InterlockedExchange(&busy,0);
}
static bool Bytes(uintptr_t at,const unsigned char *bytes,size_t n) {
    unsigned char actual[64];return n<=64 && Read(at,actual,n) && memcmp(actual,bytes,n)==0;
}
static DWORD WINAPI Start(void *unused) {
    (void)unused;
    char own[MAX_PATH];GetModuleFileNameA(self,own,MAX_PATH);
    char *dot=strrchr(own,'.');if(dot)*dot=0;
    snprintf(configPath,MAX_PATH,"%s.ini",own);
    snprintf(logPath,MAX_PATH,"%s.log",own);
    base=(uintptr_t)GetModuleHandleA("GhostOfTsushima.exe");
    if(!base){Log("Not GhostOfTsushima.exe; inactive");return 0;}
    IMAGE_DOS_HEADER *dos=(void*)base;
    IMAGE_NT_HEADERS64 *pe=(void*)(base+dos->e_lfanew);
    if(pe->FileHeader.TimeDateStamp!=SUPPORTED_TIMESTAMP || pe->OptionalHeader.SizeOfImage!=SUPPORTED_IMAGE_SIZE) {
        Log("Unsupported executable; inactive");return 0;
    }
    const unsigned char tick[]={0x40,0x57,0x48,0x83,0xEC,0x40,0x48,0x8B,0x05};
    const unsigned char setter[]={0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x6C,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x57,0x48,0x83,0xEC,0x20,0x48,0x8B,0x41,0x40};
    const unsigned char mode[]={0x80,0x3D};
    const unsigned char regcount[]={0x48,0x63,0x0D},regarray[]={0x48,0x8B,0x05};
    if(!Bytes(base+TICK_RVA,tick,sizeof(tick)) || !Bytes(base+SET_RVA,setter,sizeof(setter)) ||
       !Bytes(base+MODE_FUNC_RVA,mode,2) || !Bytes(base+0xB1647A,regcount,3) || !Bytes(base+0xB16481,regarray,3)) {
        Log("Code signatures differ or another mod owns hook; inactive");return 0;
    }
    modeAddress=Rip(base+MODE_FUNC_RVA,2,7);
    registryCountAddress=Rip(base+0xB1647A,3,7);
    registryArrayAddress=Rip(base+0xB16481,3,7);
    enabled=GetPrivateProfileIntA("ShadowStrike","Enabled",1,configPath)!=0;
    MH_STATUS status=MH_Initialize();
    if(status!=MH_OK && status!=MH_ERROR_ALREADY_INITIALIZED){Log("MinHook init failed");return 0;}
    status=MH_CreateHook((void*)(base+TICK_RVA),ShadowHook,&ShadowTrampoline);
    if(status!=MH_OK){Log("Hook creation failed");return 0;}
    /* Keep this module resident: callback and trampoline must not be unloaded. */
    HMODULE pinned;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,(const char*)self,&pinned);
    status=MH_EnableHook((void*)(base+TICK_RVA));
    if(status!=MH_OK){Log("Hook activation failed");return 0;}
    Log("Shadow Strike Story beta 0.1.0 loaded; campaign only; F8 toggles");
    return 0;
}
BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved) {
    (void)reserved;
    if(reason==DLL_PROCESS_ATTACH) {
        self=module;DisableThreadLibraryCalls(module);
        HANDLE thread=CreateThread(NULL,0,Start,NULL,0,NULL);
        if(thread)CloseHandle(thread);
    }
    return TRUE;
}
