#include <Windows.h>
#include <iostream>
#include <set>
#include <thread>
#include <atomic>

std::atomic<bool> g_running(true);
HHOOK g_hook = NULL;

const std::set<DWORD> ALLOWED_KEYS =
{   
    VK_F6     
};

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) 
    {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = kbd->vkCode;

        if (vkCode == VK_F6 && wParam == WM_KEYDOWN)
        {
            g_running = false;           
            PostQuitMessage(0);           
            return 1;                    
        }
        bool isAllowed = ALLOWED_KEYS.find(vkCode) != ALLOWED_KEYS.end();
        if (!isAllowed) return 1;
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

void MinimizeToBackground()
{
    HWND hwnd = GetConsoleWindow();
    ShowWindow(hwnd, SW_HIDE);
}

void MessageLoop() 
{
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    if (!g_hook)
    {
        std::cerr << "Failed to install hook!" << std::endl;
        return;
    }

    MSG msg;
    while (g_running && GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(g_hook); 
}

int main()
{
    std::thread hook_thread(MessageLoop);
    hook_thread.join();

    std::cout << "Program terminated by F6." << std::endl;
    Sleep(1000);
    return 0;
}