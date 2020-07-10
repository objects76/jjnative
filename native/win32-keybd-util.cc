
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <memory>

#include <chrono>
#include <future>
#include <cstdlib>
#include "log.h"

using namespace std::chrono_literals;

class LLHook
{
public:
    LLHook(HWND hWnd)
    {
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
    }
    void Pause()
    {
        pause_hook = true;
        DisableLockWorkstation(false);
    }
    void Resume()
    {
        pause_hook = false;
        DisableLockWorkstation(true);
    }

    // Windows Registry Editor Version 5.00
    // [HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Policies\System]
    // "DisableLockWorkstation"=dword:00000000

private:
    void DisableLockWorkstation(bool disable)
    {
        // STARTUPINFO si = {sizeof(STARTUPINFO)};
        // si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
        // si.wShowWindow = SW_HIDE;

        // char cmdbuf[512];
        // strcpy_s(cmdbuf, sizeof(cmdbuf), disable ? R"(reg add HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\System /v DisableLockWorkstation /t REG_DWORD /d 1 /f)" : R"(reg delete HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\System /v DisableLockWorkstation /f)");

        // PROCESS_INFORMATION procInfo = {0};
        // uint32_t createFlags = CREATE_NEW_CONSOLE * 0 | CREATE_NO_WINDOW; // without window.
        // const char *curDir = nullptr;
        // if (!CreateProcessA(nullptr, cmdbuf, nullptr, nullptr, TRUE, createFlags, nullptr, curDir, &si, &procInfo))
        // {
        //     log("registry op: errno=%d\n", GetLastError());
        // }
        // ::WaitForSingleObject(procInfo.hProcess, INFINITE);
        // ::CloseHandle(procInfo.hThread);
        // ::CloseHandle(procInfo.hProcess);
    }

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
                    if (isDev)
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

    int messageLoop()
    {
        static auto _this = this;
        HMODULE hInstance = ::GetModuleHandleA(TOSTR(NODE_GYP_MODULE_NAME) ".node");
        auto hookHandle = ::SetWindowsHookExW(
            WH_KEYBOARD_LL, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
                return _this->LLKeyboardProc(nCode, wParam, lParam);
            },
            hInstance, 0);
        Assert1(hookHandle, format("errno=%d", GetLastError));

        devlog("monitor started: module=%p, hook=%p \n", hInstance, hookHandle);
        Resume();

        MSG msg;
        loopThreadId = ::GetCurrentThreadId();

        for (;;)
        {
            auto result = GetMessageW(&msg, nullptr, 0, 0);
            if (result <= 0)
                break;
        }

        ::UnhookWindowsHookEx(hookHandle);

        devlog("monitor stopped \n");
        return 0;
    }
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

    _keybdMonitor = std::make_unique<LLHook>(hTargetWnd);
    devlog("install for hwnd.%p: %d(1:ok, 0:fail)\n", hTargetWnd, !!_keybdMonitor);
    return !!_keybdMonitor;
}

bool stopKeybdMonitor()
{
    if (_keybdMonitor)
    {
        _keybdMonitor.reset();
        return true;
    }

    devlog("No keybd monitor object at %s\n", __FUNCTION__);
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

    devlog("No keybd monitor object at %s\n", __FUNCTION__);
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