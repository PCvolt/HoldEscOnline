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

HWND getSokuHandle() {
    return FindWindowEx(nullptr, nullptr, "th123_110a", nullptr);
}

bool isSokuFocused() {
    return getSokuHandle() == GetForegroundWindow();
}

bool isFocused;

int __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This) {
    (This->*ogBattleMgrOnRender)();
    if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000 && isFocused == true &&
        (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER))
        cog.draw();

    isFocused = false; // Prevents from rendering even if the previous esc hold was valid
    return 0;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This) {
    if (!init) {
        cog.texture.loadFromGame("data/menu/gear/2L-front_M.cv2");
        cog.setPosition(SokuLib::Vector2i{10, 10});
        cog.setSize(cog.texture.getSize().to<unsigned>());
        cog.rect.width = cog.texture.getSize().x;
        cog.rect.height = cog.texture.getSize().y;
        init = true;
    }

    if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000 && isFocused == true &&
        (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER))
        cog.setRotation(cog.getRotation() + 0.01f);

    return (This->*ogBattleMgrOnProcess)();
}

int timeHeld = 0;

bool isEscapeHeld() {
    if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000) {
        if (!isSokuFocused()) {
            isFocused = false;
            timeHeld = 0;
            return false;
        } else {
            isFocused = true;
            if (timeHeld < 120) {
                ++timeHeld;
                return false;
            } else {
                timeHeld = 0;
                return true;
            }
        }

    }

    timeHeld = 0;

    return false;
}

bool mCheckFKey_VSNetwork(byte arg1, bool arg2, bool arg3, bool arg4) {
    return isEscapeHeld();
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
    return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
    DWORD old;

#ifdef _DEBUG
    FILE *_;

    AllocConsole();
    freopen_s(&_, "CONOUT$", "w", stdout);
    freopen_s(&_, "CONOUT$", "w", stderr);
#endif

    VirtualProtect((PVOID) RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
    ogBattleMgrOnRender = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender, CBattleManager_OnRender);
    ogBattleMgrOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
    VirtualProtect((PVOID) RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

    VirtualProtect((PVOID) TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
    og_mCheckFKey = SokuLib::TamperNearJmpOpr(0x482591, mCheckFKey_VSNetwork);
    VirtualProtect((PVOID) TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

    FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
    return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
    return TRUE;
}