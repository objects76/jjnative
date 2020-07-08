
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <memory>

#include <chrono>
#include <future>
#include "log.hpp"

using namespace std::chrono_literals;

class LLHook
{
public:
    LLHook(HWND hWnd)
    {
        LLHook::_this = this;
        LLHook::hTargetWnd = hWnd;
        loopFuture = std::async(std::launch::async, &LLHook::messageLoop, this);
    }
    ~LLHook()
    {
        ::PostThreadMessageW(loopThreadId, WM_QUIT, 0, 0);
        if (loopFuture.valid())
        {
            loopFuture.wait_for(5s);
            loopFuture.get();
        }
        LLHook::_this = nullptr;
    }
    void Pause() { pause_hook = true; }
    void Resume() { pause_hook = false; }

private:
    LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode == HC_ACTION && pause_hook == false)
        {
            auto keybdhs = (KBDLLHOOKSTRUCT *)lParam;
            switch (wParam)
            {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (keybdhs->vkCode == VK_LWIN || keybdhs->vkCode == VK_RWIN)
                {
                    log("winkey block %s\n", ((wParam & 1) ? "UP" : "DOWN"));
                    //if (::GetForegroundWindow() == hTargetWnd)
                    {
                        assert(hTargetWnd);
                        const bool keyup = !!(wParam & 1);
                        ::PostMessageA(hTargetWnd, wParam, keybdhs->vkCode, keyup ? 0xc15b0001 : 0x415b0001);
                        return TRUE;
                    }
                }
            }
        }
        return ::CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    std::future<int> loopFuture;
    uint32_t loopThreadId = 0;
    bool pause_hook = false;
    HWND hTargetWnd = nullptr;
    inline static LLHook *_this = nullptr;

    int messageLoop()
    {
        HMODULE hInstance = ::GetModuleHandleA(TOSTR(NODE_GYP_MODULE_NAME) ".node");
        auto hookHandle = ::SetWindowsHookExW(
            WH_KEYBOARD_LL, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
                return _this->LLKeyboardProc(nCode, wParam, lParam);
            },
            hInstance, 0);
        assert(hookHandle);

        MSG msg;
        loopThreadId = ::GetCurrentThreadId();

        for (;;)
        {
            auto result = GetMessageW(&msg, nullptr, 0, 0);
            if (result <= 0)
                break;
        }

        ::UnhookWindowsHookEx(hookHandle);
        return 0;
    }
};

static std::unique_ptr<LLHook> _keybdMonitor;
bool startKeybdMonitor(int64_t hwndNumber)
{
    HWND hTargetWnd = (HWND)hwndNumber;

    if (!::IsWindow(hTargetWnd))
    {
        log("[E] handle is not window: 0x%x\n", hTargetWnd);
        return false;
    }

    _keybdMonitor = std::make_unique<LLHook>(hTargetWnd);
    log("install for hwnd.%p: %d(1:ok, 0:fail)\n", hTargetWnd, !!_keybdMonitor);
    return !!_keybdMonitor;
}

bool stopKeybdMonitor()
{
    if (_keybdMonitor)
        _keybdMonitor.reset();

    return true;
}

bool pauseResumeKeybdMonitor(bool resume)
{
    if (_keybdMonitor)
    {
        if (resume)
            _keybdMonitor->Resume();
        else
            _keybdMonitor->Pause();
        return true;
    }
    return false;
}

// BOOL WINAPI DllMain(
//     __in HINSTANCE hinstDLL,
//     __in DWORD fdwReason,
//     __in LPVOID lpvReserved)
// {
//     log("dllmain...: dllhandle=%p\n", hinstDLL);
//     return TRUE;
// }
#endif