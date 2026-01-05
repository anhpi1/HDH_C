#include "hook_handler.h"


FILE* g_logFileMouse = NULL;
FILE* g_logFileKeyboard = NULL;
uint32_t g_fileIndexMouse = 0;
uint32_t g_fileIndexKeyboard = 0;
uint32_t g_eventCountMouse = 0;
uint32_t g_eventCountKeyboard = 0;
volatile BOOL g_running = TRUE;
CRITICAL_SECTION csMouse;
CRITICAL_SECTION csKeyboard;
HHOOK mouseHook = NULL;
HHOOK keyboardHook = NULL;
HOOK_ring_buffer ringMouse;
HOOK_ring_buffer ringKeyboard;
HANDLE mouseThread;
HANDLE keyThread;



void HOOK_InitLogFile() {
    g_logFileMouse = NULL;
    g_logFileKeyboard = NULL;
    g_fileIndexMouse = 0;
    g_fileIndexKeyboard = 0;
    g_eventCountMouse = 0;
    g_eventCountKeyboard = 0;
    g_running = TRUE;
    InitializeCriticalSection(&csMouse);
    InitializeCriticalSection(&csKeyboard);
    HOOK_FUNC_RingData_INIT(&ringMouse);
    HOOK_FUNC_RingData_INIT(&ringKeyboard);

    if (g_logFileMouse) fclose(g_logFileMouse);
    if (g_logFileKeyboard) fclose(g_logFileKeyboard);
    
    _mkdir("log");
    char filenameMouse[64];
    char filenameKeyboard[64];
    FILE* testFileMouse = NULL;
    FILE* testFileKeyboard = NULL;
    do {
        sprintf(filenameMouse, "log/mouse_log%u.csv", g_fileIndexMouse++);
        sprintf(filenameKeyboard, "log/keyboard_log%u.csv", g_fileIndexKeyboard++);
        testFileMouse = fopen(filenameMouse, "r");
        testFileKeyboard = fopen(filenameKeyboard, "r");
        if (testFileMouse) {
            fclose(testFileMouse);
        }
        if (testFileKeyboard) {
            fclose(testFileKeyboard);
        }
    } while (testFileMouse != NULL || testFileKeyboard != NULL);

    g_logFileMouse = fopen(filenameMouse, "w");
    g_logFileKeyboard = fopen(filenameKeyboard, "w");

    if (!g_logFileMouse) {
        printf("Loi: Khong the mo file %s\n", filenameMouse);
        return;
    }
    if (!g_logFileKeyboard) {
        printf("Loi: Khong the mo file %s\n", filenameKeyboard);
        return;
    }

    int screenWidth;
    int screenHeight ;
    DEVMODE dm = {0};
    dm.dmSize = sizeof(dm);

    // Lấy độ phân giải màn hình hiện tại, KHÔNG ảnh hưởng scale
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm)) {
        screenWidth = dm.dmPelsWidth;   // pixel theo Display Resolution
        screenHeight = dm.dmPelsHeight;
    } else {
        printf("Cannot get display settings!\n");
        return ;
    }
    SYSTEMTIME st;
    GetLocalTime(&st);

    fprintf(g_logFileMouse,
        "version,1,startTime,%04d-%02d-%02d %02d:%02d:%02d.%03d,screenWidth,%d,screenHeight,%d\n",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        screenWidth, screenHeight
    );
    fprintf(g_logFileKeyboard,
        "version,1,startTime,%04d-%02d-%02d %02d:%02d:%02d.%03d,screenWidth,%d,screenHeight,%d\n",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        screenWidth, screenHeight
    );
    fprintf(g_logFileMouse, "Event(uint),MsgID(hex),Time(uint),X(int),Y(int),MouseData(hex)\n");
    fprintf(g_logFileKeyboard, "Event(uint),MsgID(hex),Time(uint),VkCode(hex),ScanCode(hex),Flags(hex)\n");
    fflush(g_logFileMouse);
    fflush(g_logFileKeyboard);
    g_eventCountMouse = 0;
    g_eventCountKeyboard = 0;

    printf("Da tao file log: %s\n", filenameMouse);
    printf("Da tao file log: %s\n", filenameKeyboard);
}


void HOOK_stop_recording(void) {
    printf("Dang thoat chuong trinh...\n");
    g_running = FALSE;
    PostQuitMessage(0);
    if (g_logFileMouse) {
        fclose(g_logFileMouse);
        g_logFileMouse = NULL;
    }
    if (g_logFileKeyboard) {
        fclose(g_logFileKeyboard);
        g_logFileKeyboard = NULL;
    }
    printf("Dang don dep...\n");
    
    
    UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);
    DeleteCriticalSection(&csMouse);
    DeleteCriticalSection(&csKeyboard);
    
    printf("Chuong trinh da ket thuc\n");
}



LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || !g_running) return CallNextHookEx(NULL, nCode, wParam, lParam);
    
    MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
    
    // Allocate và copy data để đẩy vào ring buffer
    HOOK_MouseEvent* event = (HOOK_MouseEvent*)malloc(sizeof(HOOK_MouseEvent));
    if (event) {
        event->index = 0; // Sẽ được gán lại trong thread ghi file
        event->MsgID = (uint32_t)wParam;
        event->time = mouseInfo->time;
        event->mouseData = mouseInfo->mouseData;
        event->pt = mouseInfo->pt;
        
        // Đẩy vào ring buffer (lock-free)
        if (HOOK_FUNC_Write_RingData(&ringMouse, event) != 0) {
            // Buffer đầy, giải phóng memory
            free(event);
        }
    }
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || !g_running) return CallNextHookEx(NULL, nCode, wParam, lParam);
    
    KBDLLHOOKSTRUCT* keyboardInfo = (KBDLLHOOKSTRUCT*)lParam;
    
    // Allocate và copy data để đẩy vào ring buffer
    HOOK_KeyboardEvent* event = (HOOK_KeyboardEvent*)malloc(sizeof(HOOK_KeyboardEvent));
    if (event) {
        event->index = 0; // Sẽ được gán lại trong thread ghi file
        event->MsgID = (uint32_t)wParam;
        event->time = keyboardInfo->time;
        event->vkCode = keyboardInfo->vkCode;
        event->scanCode = keyboardInfo->scanCode;
        event->flags = keyboardInfo->flags;
        
        // Đẩy vào ring buffer (lock-free)
        if (HOOK_FUNC_Write_RingData(&ringKeyboard, event) != 0) {
            // Buffer đầy, giải phóng memory
            free(event);
        }
    }
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Thread ghi file mouse - đọc từ ring buffer và ghi file
DWORD WINAPI HOOK_writeMouseLogThread(LPVOID param) {
    (void)param;
    
    while (g_running) {
        HOOK_MouseEvent* event = NULL;
        
        if (!(HOOK_FUNC_Read_RingData(&ringMouse, (void**)&event) == 0 && event))return 0;

        EnterCriticalSection(&csMouse);
        
        // Tạo file mới nếu chưa có hoặc đã đủ số sự kiện
        if (!g_logFileMouse || g_eventCountMouse >= MAX_EVENTS_PER_FILE) HOOK_InitLogFile();
        
        if (g_logFileMouse) {
            fprintf(g_logFileMouse, "%u,%x,%lu,%ld,%ld,%x\n",
                g_eventCountMouse,
                event->MsgID,
                event->time,
                event->pt.x,
                event->pt.y,
                event->mouseData
            );
            g_eventCountMouse++;
            fflush(g_logFileMouse);
        }
        
        LeaveCriticalSection(&csMouse);
        free(event);
    }
    
    return 0;
}

// Thread ghi file keyboard - đọc từ ring buffer và ghi file
DWORD WINAPI HOOK_writeKeyLogThread(LPVOID param) {
    (void)param;
    
    while (g_running) {
        HOOK_KeyboardEvent* event = NULL;
        
        if (!(HOOK_FUNC_Read_RingData(&ringKeyboard, (void**)&event) == 0 && event))return 0;

        EnterCriticalSection(&csKeyboard);
        
        // Tạo file mới nếu chưa có hoặc đã đủ số sự kiện
        if (!g_logFileKeyboard || g_eventCountKeyboard >= MAX_EVENTS_PER_FILE) HOOK_InitLogFile();
            
            if (g_logFileKeyboard) {
                fprintf(g_logFileKeyboard, "%u,%x,%lu,%x,%x,%x\n",
                    g_eventCountKeyboard,
                    event->MsgID,
                    event->time,
                    event->vkCode,
                    event->scanCode,
                    event->flags
                );
                g_eventCountKeyboard++;
                fflush(g_logFileKeyboard);
            }
            
            LeaveCriticalSection(&csKeyboard);
            free(event);
        }
    
    return 0;
}

int HOOK_start_recording(void) {
    printf("=== Bat dau ghi log chuot va ban phim===\n");
    
    InitializeCriticalSection(&csMouse);
    InitializeCriticalSection(&csKeyboard);
    
    // Khởi tạo file log đầu tiên
    HOOK_InitLogFile();

    // Tạo thread ghi file - đọc từ ring buffer
    mouseThread = CreateThread(NULL, 0, HOOK_writeMouseLogThread, NULL, 0, NULL); 
    keyThread = CreateThread(NULL, 0, HOOK_writeKeyLogThread, NULL, 0, NULL);
    
    if (!mouseThread || !keyThread) {
        printf("Loi: Khong the tao thread ghi file!\n");
        return 1;
    } 
    
    // Cài đặt hook chuột và bàn phím
    mouseHook = SetWindowsHookExA(WH_MOUSE_LL, &HOOK_LowLevelMouseProc, NULL, 0);
    keyboardHook = SetWindowsHookExA(WH_KEYBOARD_LL, &HOOK_LowLevelKeyboardProc, NULL, 0);
    
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
}