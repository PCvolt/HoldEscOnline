//
// Created by PC_volt on 10/07/2023.
//

#include <SokuLib.hpp>

static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static bool (*og_mCheckFKey)(byte arg1, bool arg2, bool arg3, bool arg4);

static bool init = false;
static SokuLib::DrawUtils::Sprite cog;

constexpr int ESC_KEYCODE = 0x1B;

int __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
    (This->*ogBattleMgrOnRender)();
    if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000)
        cog.draw();
    return 0;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
    if (!init) {
        SokuLib::Vector2i realSize;

        cog.texture.loadFromFile("modules/HoldEscOnline/assets/gear.png");
        cog.setPosition(SokuLib::Vector2i{35, 35});
        cog.setSize(realSize.to<unsigned>());
        cog.rect.width = realSize.x;
        cog.rect.height = realSize.y;
        init = true;
    }

    if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000)
        cog.setRotation(cog.getRotation() + 0.01f);
    return (This->*ogBattleMgrOnProcess)();
}

int timeHeld = 0;
bool isEscapeHeld()
{
    if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000)
    {
        if (timeHeld < 120)
        {
            ++timeHeld;
            return false;
        }
        else
        {
            timeHeld = 0;
            return true;
        }
    }

    timeHeld = 0;

    return false;
}

bool mCheckFKey_VSNetwork(byte arg1, bool arg2, bool arg3, bool arg4)
{
    return isEscapeHeld();
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
    return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
    DWORD old;

#ifdef _DEBUG
    FILE *_;

    AllocConsole();
    freopen_s(&_, "CONOUT$", "w", stdout);
    freopen_s(&_, "CONOUT$", "w", stderr);
#endif

    VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
    ogBattleMgrOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  CBattleManager_OnRender);
    ogBattleMgrOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
    VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

    VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
    og_mCheckFKey = SokuLib::TamperNearJmpOpr(0x482591, mCheckFKey_VSNetwork);
    VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

    FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
    return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    return TRUE;
}