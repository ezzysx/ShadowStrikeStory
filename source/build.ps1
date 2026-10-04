param([Parameter(Mandatory=$true)][string]$Zig)
$ErrorActionPreference='Stop'
Push-Location $PSScriptRoot
try {
    & $Zig cc -target x86_64-windows-gnu -O2 -Wall -Wextra test_state.c -o test_state.exe
    if ($LASTEXITCODE -ne 0) { throw 'Test build failed' }
    & .\test_state.exe
    if ($LASTEXITCODE -ne 0) { throw 'State tests failed' }
    & $Zig cc -target x86_64-windows-gnu -shared -O2 -Wall -Wextra -I third_party/minhook/include plugin.c bridge.s third_party/minhook/src/buffer.c third_party/minhook/src/hook.c third_party/minhook/src/trampoline.c third_party/minhook/src/hde/hde64.c -luser32 -o ShadowStrikeStory.asi
    if ($LASTEXITCODE -ne 0) { throw 'Plugin build failed' }
} finally { Pop-Location }
