#include "main.h"

HOOK_ringMouseData RingMouseData;
CRITICAL_SECTION cs;



int main() {
    InitializeCriticalSection(&cs);
    HHOOK myhook = SetWindowsHookExA(WH_MOUSE_LL, &HOOK_LowLevelMouseProc, NULL, 0);
    HANDLE myThread = CreateThread(NULL, 0, &HOOK_writeMouseLogThread, NULL, 0, NULL);   

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(myhook);
    CloseHandle(myThread);
    DeleteCriticalSection(&cs);
}


