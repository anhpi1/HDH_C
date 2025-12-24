#include "main.h"

CRITICAL_SECTION csMouse;
CRITICAL_SECTION csKeyboard;

int main() {
    printf("=== Chuong trinh ghi log chuot va ban phim===\n");
    printf("Nhan F12 de thoat chuong trinh\n\n");
    
    InitializeCriticalSection(&csMouse);
    InitializeCriticalSection(&csKeyboard);
    
    // Khởi tạo file log đầu tiên
    HOOK_InitLogFile();
    
    // Cài đặt hook chuột và bàn phím
    HHOOK mouseHook = SetWindowsHookExA(WH_MOUSE_LL, &HOOK_LowLevelMouseProc, NULL, 0);
    HHOOK keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, &HOOK_LowLevelKeyboardProc, NULL, 0);
    
    if (!mouseHook || !keyboardHook) {
        printf("Loi: Khong the cai dat hook!\n");
        return 1;
    }
    
    printf("Hook da duoc cai dat thanh cong\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    replay_events("log/mouse_log0.csv", "log/keyboard_log0.csv", 2);
    
    printf("Dang don dep...\n");
    
    // Dọn dẹp
    HOOK_CloseLogFile();
    UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);
    DeleteCriticalSection(&csMouse);
    DeleteCriticalSection(&csKeyboard);
    
    printf("Chuong trinh da ket thuc\n");
    return 0;
}


