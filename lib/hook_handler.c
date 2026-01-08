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
HOOK_ring_buffer_event ringData;
HANDLE mouseThread;
HANDLE keyThread;
HOOK_Logger mylog;
volatile BOOL is_log_realtime = FALSE;





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
    HOOK_FUNC_RingData_INIT_event(&ringData);
    HOOK_log_INIT(&mylog);
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
    is_log_realtime = FALSE;
}



LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || !g_running) return CallNextHookEx(NULL, nCode, wParam, lParam);
    
    MSLLHOOKSTRUCT* mouseInfo = (MSLLHOOKSTRUCT*)lParam;
    
    // Sao chép trực tiếp vào ring buffer (không dùng malloc)
    HOOK_MouseEvent event;
    event.index = 0;
    event.MsgID = (uint32_t)wParam;
    event.time = mouseInfo->time;
    event.mouseData = mouseInfo->mouseData;
    event.pt = mouseInfo->pt;
    
    // Đẩy vào ring buffer (lock-free)
    HOOK_FUNC_Write_RingData_eventMouse(&ringData, event);
    if(is_log_realtime) HOOK_FUNC_Write_RingData_eventMouseBoth(&ringData, event);
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0 || !g_running) return CallNextHookEx(NULL, nCode, wParam, lParam);
    
    KBDLLHOOKSTRUCT* keyboardInfo = (KBDLLHOOKSTRUCT*)lParam;
    
    // Sao chép trực tiếp vào ring buffer (không dùng malloc)
    HOOK_KeyboardEvent event;
    event.index = 0;
    event.MsgID = (uint32_t)wParam;
    event.time = keyboardInfo->time;
    event.vkCode = keyboardInfo->vkCode;
    event.scanCode = keyboardInfo->scanCode;
    event.flags = keyboardInfo->flags;
    
    // Đẩy vào ring buffer (lock-free)
    HOOK_FUNC_Write_RingData_eventKey(&ringData, event);
    if(is_log_realtime) HOOK_FUNC_Write_RingData_eventKeyBoth(&ringData, event);
    
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Thread ghi file mouse - đọc từ ring buffer và ghi file
DWORD WINAPI HOOK_writeMouseLogThread(LPVOID param) {
    (void)param;
    
    while (g_running) {
        HOOK_MouseEvent event;
        
        if (HOOK_FUNC_Read_RingData_eventMouse(&ringData, &event) != 0) {
            Sleep(1);
            continue;
        }

        EnterCriticalSection(&csMouse);
        
        // Tạo file mới nếu chưa có hoặc đã đủ số sự kiện
        if (!g_logFileMouse || g_eventCountMouse >= MAX_EVENTS_PER_FILE) HOOK_InitLogFile();
        
        if (g_logFileMouse) {
            event.index = g_eventCountMouse;
            if(is_log_realtime) HOOK_log_filter_processing_mouse(&mylog, (HOOK_log_MouseEvent *)&event);
            fprintf(g_logFileMouse, "%u,%x,%lu,%ld,%ld,%x\n",
                g_eventCountMouse,
                event.MsgID,
                event.time,
                event.pt.x,
                event.pt.y,
                event.mouseData
            );
            g_eventCountMouse++;
            fflush(g_logFileMouse);
        }
        
        LeaveCriticalSection(&csMouse);
    }
    
    return 0;
}

// Thread ghi file keyboard - đọc từ ring buffer và ghi file
DWORD WINAPI HOOK_writeKeyLogThread(LPVOID param) {
    (void)param;
    
    while (g_running) {
        HOOK_KeyboardEvent event;
        
        if (HOOK_FUNC_Read_RingData_eventKey(&ringData, &event) != 0) {
            Sleep(1);
            continue;
        }

        EnterCriticalSection(&csKeyboard);
        
        // Tạo file mới nếu chưa có hoặc đã đủ số sự kiện
        if (!g_logFileKeyboard || g_eventCountKeyboard >= MAX_EVENTS_PER_FILE) HOOK_InitLogFile();
        
        if (g_logFileKeyboard) {
            event.index = g_eventCountKeyboard;
             if(is_log_realtime)HOOK_log_filter_processing_key(&mylog, (HOOK_log_KeyboardEvent *)&event);
            fprintf(g_logFileKeyboard, "%u,%x,%lu,%x,%x,%x\n",
                g_eventCountKeyboard,
                event.MsgID,
                event.time,
                event.vkCode,
                event.scanCode,
                event.flags
            );
            g_eventCountKeyboard++;
            fflush(g_logFileKeyboard);
        }
        
        LeaveCriticalSection(&csKeyboard);
    }
    
    return 0;
}

DWORD WINAPI HOOK_filterProcessingBothKeyAndMouseThread(LPVOID param) {
    (void)param;
    // Sử dụng cờ hasData để biết biến đang giữ dữ liệu hợp lệ hay không
    bool hasMouse = false;
    bool hasKey = false;
    HOOK_KeyboardEvent keyEvent = {0};
    HOOK_MouseEvent mouseEvent = {0};

    while (g_running) {
        // 1. Cố gắng nạp dữ liệu nếu đang thiếu
        if (!hasMouse) {
            if (HOOK_FUNC_Read_RingData_eventMouseBoth(&ringData, &mouseEvent) == 0) {
                hasMouse = true;
            }
        }

        if (!hasKey) {
            if (HOOK_FUNC_Read_RingData_eventKeyBoth(&ringData, &keyEvent) == 0) {
                hasKey = true;
            }
        }


        if(hasMouse && !hasKey) {
            HOOK_log_filter_processing_both_mouse_and_key(&mylog, (HOOK_log_KeyboardEvent *)&keyEvent, (HOOK_log_MouseEvent *)&mouseEvent,1);
            hasMouse = false;
            continue;
        } else if (!hasMouse && hasKey) {
            HOOK_log_filter_processing_both_mouse_and_key(&mylog, (HOOK_log_KeyboardEvent *)&keyEvent, (HOOK_log_MouseEvent *)&mouseEvent,0);
            hasKey = false;
            continue;
        } else if (!hasMouse && !hasKey)
        {
            Sleep(1);
            continue;
        }

        // 2. Xử lý sự kiện dựa trên thời gian
        if (mouseEvent.time <= keyEvent.time) {
            HOOK_log_filter_processing_both_mouse_and_key(&mylog, (HOOK_log_KeyboardEvent *)&keyEvent, (HOOK_log_MouseEvent *)&mouseEvent,1);
            hasMouse = false;
        } else {
            HOOK_log_filter_processing_both_mouse_and_key(&mylog, (HOOK_log_KeyboardEvent *)&keyEvent, (HOOK_log_MouseEvent *)&mouseEvent,0);
            hasKey = false;
        }
        
    }
    return 0;
}

int HOOK_start_recording(int islogrealtime) {
    if(islogrealtime)is_log_realtime = TRUE;
    else is_log_realtime = FALSE;
    printf("=== Bat dau ghi log chuot va ban phim===\n");
    
    InitializeCriticalSection(&csMouse);
    InitializeCriticalSection(&csKeyboard);
    
    // Khởi tạo file log đầu tiên
    HOOK_InitLogFile();

    // Tạo thread ghi file - đọc từ ring buffer
    mouseThread = CreateThread(NULL, 0, HOOK_writeMouseLogThread, NULL, 0, NULL); 
    keyThread = CreateThread(NULL, 0, HOOK_writeKeyLogThread, NULL, 0, NULL);
    HANDLE bothThread = NULL;
    if(is_log_realtime) bothThread = CreateThread(NULL, 0, HOOK_filterProcessingBothKeyAndMouseThread, NULL, 0, NULL);
    
    if (!mouseThread || !keyThread || (is_log_realtime && !bothThread)) {
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

uint8_t HOOK_FUNC_RingData_INIT_event(HOOK_ring_buffer_event* ring){
    if(!ring) return 1;
    ring->pReadMouse = 0;
    ring->pWriteMouse = 0;
    ring->pReadKeyboard = 0;
    ring->pWriteKeyboard = 0;
    ring->hSemaphoreMouse = CreateSemaphore(NULL, 0, BUFFER_SIZE_EVENT, NULL);
    ring->hSemaphoreKeyboard = CreateSemaphore(NULL, 0, BUFFER_SIZE_EVENT, NULL);

    ring->pReadMouseBoth = 0;
    ring->pWriteMouseBoth = 0;
    ring->pReadKeyboardBoth = 0;
    ring->pWriteKeyboardBoth = 0;
    ring->hSemaphoreMouseBoth = CreateSemaphore(NULL, 0, BUFFER_SIZE_EVENT, NULL);
    ring->hSemaphoreKeyboardBoth = CreateSemaphore(NULL, 0, BUFFER_SIZE_EVENT, NULL);
    if(!ring->hSemaphoreMouse || !ring->hSemaphoreKeyboard || !ring->hSemaphoreMouseBoth || !ring->hSemaphoreKeyboardBoth) return 1;
    return 0;
}

uint8_t HOOK_FUNC_Read_RingData_eventMouse(HOOK_ring_buffer_event* ring, HOOK_MouseEvent* dataOut){
    if(!ring || !dataOut) return 1;
    
    // Chờ có dữ liệu (non-blocking với timeout 10ms)
    DWORD result = WaitForSingleObject(ring->hSemaphoreMouse, 10);
    if(result != WAIT_OBJECT_0) return 1;
    
    LONG readPos = ring->pReadMouse;
    LONG nextReadPos = (readPos + 1) % BUFFER_SIZE_EVENT;
    
    // Sao chép dữ liệu trực tiếp từ buffer
    *dataOut = ring->bufferMouse[readPos];
    
    // Cập nhật read pointer (atomic)
    InterlockedExchange(&ring->pReadMouse, nextReadPos);
    
    return 0;
}

uint8_t HOOK_FUNC_Write_RingData_eventMouse(HOOK_ring_buffer_event* ring, HOOK_MouseEvent data){
    if(!ring) return 1;
    
    LONG writePos = ring->pWriteMouse;
    LONG nextWritePos = (writePos + 1) % BUFFER_SIZE_EVENT;
    
    // Kiểm tra buffer đầy
    if(nextWritePos == ring->pReadMouse) return 1;
    
    // Sao chép dữ liệu trực tiếp vào buffer
    ring->bufferMouse[writePos] = data;
    
    // Cập nhật write pointer (atomic)
    InterlockedExchange(&ring->pWriteMouse, nextWritePos);
    
    // Báo hiệu có dữ liệu mới
    ReleaseSemaphore(ring->hSemaphoreMouse, 1, NULL);
    
    return 0;
}

uint8_t HOOK_FUNC_Read_RingData_eventKey(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent* dataOut){
    if(!ring || !dataOut) return 1;
    
    // Chờ có dữ liệu (non-blocking với timeout 10ms)
    DWORD result = WaitForSingleObject(ring->hSemaphoreKeyboard, 10);
    if(result != WAIT_OBJECT_0) return 1;
    
    LONG readPos = ring->pReadKeyboard;
    LONG nextReadPos = (readPos + 1) % BUFFER_SIZE_EVENT;
    
    // Sao chép dữ liệu trực tiếp từ buffer
    *dataOut = ring->bufferKeyboard[readPos];
    
    // Cập nhật read pointer (atomic)
    InterlockedExchange(&ring->pReadKeyboard, nextReadPos);
    
    return 0;
}

uint8_t HOOK_FUNC_Write_RingData_eventKey(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent data){
    if(!ring) return 1;
    
    LONG writePos = ring->pWriteKeyboard;
    LONG nextWritePos = (writePos + 1) % BUFFER_SIZE_EVENT;
    
    // Kiểm tra buffer đầy
    if(nextWritePos == ring->pReadKeyboard) return 1;
    
    // Sao chép dữ liệu trực tiếp vào buffer
    ring->bufferKeyboard[writePos] = data;
    
    // Cập nhật write pointer (atomic)
    InterlockedExchange(&ring->pWriteKeyboard, nextWritePos);
    
    // Báo hiệu có dữ liệu mới
    ReleaseSemaphore(ring->hSemaphoreKeyboard, 1, NULL);
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////


uint8_t HOOK_FUNC_Read_RingData_eventMouseBoth(HOOK_ring_buffer_event* ring, HOOK_MouseEvent* dataOut){
    if(!ring || !dataOut) return 1;
    
    // Chờ có dữ liệu (non-blocking với timeout 10ms)
    DWORD result = WaitForSingleObject(ring->hSemaphoreMouseBoth, 10);
    if(result != WAIT_OBJECT_0) return 1;
    
    LONG readPos = ring->pReadMouseBoth;
    LONG nextReadPos = (readPos + 1) % BUFFER_SIZE_EVENT;
    
    // Sao chép dữ liệu trực tiếp từ buffer
    *dataOut = ring->bufferMouseBoth[readPos];
    
    // Cập nhật read pointer (atomic)
    InterlockedExchange(&ring->pReadMouseBoth, nextReadPos);
    
    return 0;
}

uint8_t HOOK_FUNC_Write_RingData_eventMouseBoth(HOOK_ring_buffer_event* ring, HOOK_MouseEvent data){
    if(!ring) return 1;
    
    LONG writePos = ring->pWriteMouseBoth;
    LONG nextWritePos = (writePos + 1) % BUFFER_SIZE_EVENT;
    
    // Kiểm tra buffer đầy
    if(nextWritePos == ring->pReadMouseBoth) return 1;
    
    // Sao chép dữ liệu trực tiếp vào buffer
    ring->bufferMouseBoth[writePos] = data;
    
    // Cập nhật write pointer (atomic)
    InterlockedExchange(&ring->pWriteMouseBoth, nextWritePos);
    
    // Báo hiệu có dữ liệu mới
    ReleaseSemaphore(ring->hSemaphoreMouseBoth, 1, NULL);
    
    return 0;
}

uint8_t HOOK_FUNC_Read_RingData_eventKeyBoth(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent* dataOut){
    if(!ring || !dataOut) return 1;
    
    // Chờ có dữ liệu (non-blocking với timeout 10ms)
    DWORD result = WaitForSingleObject(ring->hSemaphoreKeyboardBoth, 10);
    if(result != WAIT_OBJECT_0) return 1;
    
    LONG readPos = ring->pReadKeyboardBoth;
    LONG nextReadPos = (readPos + 1) % BUFFER_SIZE_EVENT;
    
    // Sao chép dữ liệu trực tiếp từ buffer
    *dataOut = ring->bufferKeyboardBoth[readPos];
    
    // Cập nhật read pointer (atomic)
    InterlockedExchange(&ring->pReadKeyboardBoth, nextReadPos);
    
    return 0;
}

uint8_t HOOK_FUNC_Write_RingData_eventKeyBoth(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent data){
    if(!ring) return 1;
    
    LONG writePos = ring->pWriteKeyboardBoth;
    LONG nextWritePos = (writePos + 1) % BUFFER_SIZE_EVENT;
    
    // Kiểm tra buffer đầy
    if(nextWritePos == ring->pReadKeyboardBoth) return 1;
    
    // Sao chép dữ liệu trực tiếp vào buffer
    ring->bufferKeyboardBoth[writePos] = data;
    
    // Cập nhật write pointer (atomic)
    InterlockedExchange(&ring->pWriteKeyboardBoth, nextWritePos);
    
    // Báo hiệu có dữ liệu mới
    ReleaseSemaphore(ring->hSemaphoreKeyboardBoth, 1, NULL);
    
    return 0;
}