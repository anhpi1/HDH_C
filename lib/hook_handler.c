#include "hook_handler.h"

FILE* g_logFile = NULL;
uint32_t g_fileIndex = 0;
uint32_t g_eventCount = 0;
volatile BOOL g_running = TRUE;


void HOOK_InitLogFile() {
    if (g_logFile) {
        fclose(g_logFile);
    }

    _mkdir("log");
    char filename[64];
    FILE* testFile = NULL;
    do {
        sprintf(filename, "log/mouse_log%u.csv", g_fileIndex++);
        testFile = fopen(filename, "r");
        if (testFile) {
            fclose(testFile);
        }
    } while (testFile != NULL);

    g_logFile = fopen(filename, "w");

    if (!g_logFile) {
        printf("Loi: Khong the mo file %s\n", filename);
        return;
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    SYSTEMTIME st;
    GetLocalTime(&st);

    fprintf(g_logFile,
        "version,1,startTime,%04d-%02d-%02d %02d:%02d:%02d.%03d,screenWidth,%d,screenHeight,%d\n",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        screenWidth, screenHeight
    );
    fprintf(g_logFile, "Event,MsgID,Time,X,Y,MouseData\n");
    fflush(g_logFile);
    g_eventCount = 0;

    printf("Da tao file log: %s\n", filename);
}


void HOOK_CloseLogFile() {
    if (g_logFile) {
        fclose(g_logFile);
        g_logFile = NULL;
    }
}

LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || !g_running) {
        return CallNextHookEx(NULL, nCode, wParam, lParam);
    }
    
    EnterCriticalSection(&cs);
    
    // Tạo file mới nếu chưa có hoặc đã đủ số sự kiện
    if (!g_logFile || g_eventCount >= MAX_EVENTS_PER_FILE) {
        HOOK_InitLogFile();
    }
    
    if (g_logFile) {
        MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
        
          fprintf(g_logFile, "%u,%ld,%lu,%ld,%ld,%lu\n",
              g_eventCount,
              (uint32_t)wParam,
              mouseInfo->time,
              mouseInfo->pt.x,
              mouseInfo->pt.y,
              mouseInfo->mouseData
          );
        
        g_eventCount++;
        
        // Flush mỗi 100 sự kiện để đảm bảo dữ liệu được ghi
        if (g_eventCount % 100 == 0) {
            fflush(g_logFile);
        }
        #if DEBUG
        printf("Mouse Event: MsgId=%u, X=%ld, Y=%ld, MouseData=%lu, Time=%lu\n",
            (uint32_t)wParam,
            mouseInfo->pt.x,
            mouseInfo->pt.y,
            mouseInfo->mouseData,
            mouseInfo->time
        );
        #endif
    }
    
    LeaveCriticalSection(&cs);
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbInfo = (KBDLLHOOKSTRUCT*)lParam;
        
        // Kiểm tra nếu phím F12 được nhấn
        if (wParam == WM_KEYDOWN && kbInfo->vkCode == VK_F12) {
            printf("\nNhan F12 - Dang thoat chuong trinh...\n");
            g_running = FALSE;
            PostQuitMessage(0);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
