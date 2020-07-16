
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <memory>

#include <chrono>
#include <future>
#include <cstdlib>
#include "log.h"

#include <initializer_list>

using namespace std::chrono_literals;


class LLHook
{
public:
    LLHook() = default;
    ~LLHook()
    {
        if (loopThreadId)
            LogOnFalse(::PostThreadMessageW(loopThreadId, WM_QUIT, 0, 0));

        if (loopFuture.valid())
        {
            auto state = loopFuture.wait_for(5s);
            if (state == std::future_status::ready)
                loopFuture.get();
            else {
                const char* name[] = {"ready", "timeout", "deffered"};
                log("wait failed: %s\n", name[(int)state]);
            }
        }
        
        if (hKeyHook)
            LogOnFalse(::UnhookWindowsHookEx(hKeyHook));
    }
    void Pause()
    {
        pause_hook = true;
    }
    void Resume()
    {
        pause_hook = false;
    }

    bool hook(HWND hWnd) 
    {
        static auto _this = this;
        HMODULE hInstance = ::GetModuleHandleA(TOSTR(NODE_GYP_MODULE_NAME) ".node");
        hKeyHook = ::SetWindowsHookExW(
            WH_KEYBOARD_LL, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
                return _this->LLKeyboardProc(nCode, wParam, lParam);
            },
            hInstance, 0);

        if (hKeyHook) {
            hTargetWnd = hWnd;
            loopFuture = std::async(std::launch::async, [this]{
                loopThreadId = ::GetCurrentThreadId();
                for (MSG msg; GetMessageW(&msg, nullptr, 0, 0) > 0; );
                LOG("monitor stopped");
                return 0;
            });
            return true;
        }

        log("keyhook failed: errno=%d\n", GetLastError());
        return false;
    }

private:
    LRESULT CALLBACK LLKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode == HC_ACTION && pause_hook == false)
        {
            switch (wParam)
            {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
                switch (auto vkCode = ((KBDLLHOOKSTRUCT *)lParam)->vkCode) {
                case VK_LWIN: case VK_RWIN:
                case VK_LMENU: case VK_RMENU:
                case VK_LCONTROL: case VK_RCONTROL:
                    const bool keyup = !!(wParam & 1);
                    if (isDev)
                        log("key.%x block %s\n", vkCode, (keyup ? "UP" : "DOWN"));
                    assert(hTargetWnd);
                    LogOnFalse(PostMessageW(hTargetWnd, wParam, vkCode, keyup ? 0xc15b0001 : 0x415b0001));
                    return TRUE;
                }
            }
        }
        return ::CallNextHookEx(NULL, nCode, wParam, lParam);
    }

    std::future<int> loopFuture;
    uint32_t loopThreadId = 0;
    bool pause_hook = false;
    HWND hTargetWnd = nullptr;
    HHOOK hKeyHook = nullptr;
};




static std::unique_ptr<LLHook> _keybdMonitor;
bool startKeybdMonitor(int64_t hwndNumber)
{
    HWND hTargetWnd = (HWND)hwndNumber;
    if (!::IsWindow(hTargetWnd))
    {
        throw_error(format("invalid window handle: %p", hTargetWnd), __FUNCTION__);
        return false;
    }

    _keybdMonitor = std::make_unique<LLHook>();
    if (!_keybdMonitor->hook(hTargetWnd))
    {
        throw_error(format("setup failed : errno=%d", GetLastError()), __FUNCTION__);
        _keybdMonitor.reset();
        return false;
    }

    log("setup ok at %s\n", __FUNCTION__);
    return true;
}

bool stopKeybdMonitor()
{
    if (_keybdMonitor)
    {
        _keybdMonitor.reset();
        return true;
    }

    log("No keybd monitor object at %s\n", __FUNCTION__);
    return false;
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

    log("No keybd monitor object at %s\n", __FUNCTION__);
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
