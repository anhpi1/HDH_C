#include "replay_engine.h"


// Định nghĩa struct nội bộ nếu chưa có trong header


// Hàm giải mã flags chuột (Giữ nguyên logic của bạn nhưng làm gọn)
int HOOK_decode_mouse_flags(const uint32_t msgID, DWORD *dwFlags) {
    *dwFlags = MOUSEEVENTF_ABSOLUTE; // Luôn dùng toạ độ tuyệt đối
    switch (msgID) {
        case WM_LBUTTONDOWN: *dwFlags |= MOUSEEVENTF_LEFTDOWN; break;
        case WM_LBUTTONUP:   *dwFlags |= MOUSEEVENTF_LEFTUP; break;
        case WM_MOUSEMOVE:   *dwFlags |= MOUSEEVENTF_MOVE; break;
        case WM_MOUSEWHEEL:  *dwFlags |= MOUSEEVENTF_WHEEL; break;
        case WM_RBUTTONDOWN: *dwFlags |= MOUSEEVENTF_RIGHTDOWN; break;
        case WM_RBUTTONUP:   *dwFlags |= MOUSEEVENTF_RIGHTUP; break;
        default: return 1; // Unknown event
    }
    return 0;
}

int HOOK_replay_event_INIT(ReplayContext* replay,const char* mouse_log_file, const char* keyboard_log_file,int mode) {
    if(!replay) return 1;
    char buffer[512];

    // 1. Mở file dựa trên mode
    // Mode: 0=Mouse, 1=Keyboard, 2=Both
    if (mode == 0 || mode == 2) {
        replay->mouse_log_file = fopen(mouse_log_file, "r");
        if (!replay->mouse_log_file) printf("Error: Cannot open mouse log %s\n", mouse_log_file);
    }
    if (mode == 1 || mode == 2) {
        replay->keyboard_log_file = fopen(keyboard_log_file, "r");
        if (!replay->keyboard_log_file) printf("Error: Cannot open keyboard log %s\n", keyboard_log_file);
    }

    if (!replay->mouse_log_file && !replay->keyboard_log_file) return -1;

    // 2. Đọc thông tin màn hình từ file hợp lệ đầu tiên
    FILE* headerFile = replay->mouse_log_file ? replay->mouse_log_file : replay->keyboard_log_file;
    
    // Đọc dòng đầu tiên để lấy screen info
    fscanf(headerFile,
        "version,1,startTime,%04hd-%02hd-%02hd %02hd:%02hd:%02hd.%03hd,screenWidth,%d,screenHeight,%d\n",
        &replay->st.wYear, &replay->st.wMonth, &replay->st.wDay, &replay->st.wHour, &replay->st.wMinute, &replay->st.wSecond, &replay->st.wMilliseconds,
        &replay->screenWidth, &replay->screenHeight
    );

    // Bỏ qua dòng tiêu đề cột cho cả 2 file (nếu mở)
    // Lưu ý: File headerFile đã đọc dòng 1 bằng fscanf, nên chỉ cần fgets thêm 1 lần để qua dòng tiêu đề cột.
    // Các file khác cần skip full header.
    fgets(buffer, sizeof(buffer), headerFile); 

    if (replay->mouse_log_file && replay->mouse_log_file != headerFile) {
        fgets(buffer, sizeof(buffer), replay->mouse_log_file);
        fgets(buffer, sizeof(buffer), replay->mouse_log_file);
    }
    if (replay->keyboard_log_file && replay->keyboard_log_file != headerFile) {
        fgets(buffer, sizeof(buffer), replay->keyboard_log_file);
        fgets(buffer, sizeof(buffer), replay->keyboard_log_file);
    }
    replay->mode = mode;
    return 0;
}



DWORD WINAPI HOOK_replay_load_file(LPVOID lpParam) { 
    ReplayContext* replay = (ReplayContext*)lpParam;
    MSLLHOOKSTRUCT mouseData = {0};
    uint32_t mouseMsgId = 0, mouseIndex = 0;
    
    KBDLLHOOKSTRUCT kbdData = {0};
    uint32_t kbdMsgId = 0, kbdIndex = 0;

    int has_mouse_event = 0;
    int has_kbd_event = 0;
    
    int res_mouse = 0;
    int res_kbd = 0;

    if(replay->mode == 1){
        FILE* fStartMouse = fopen(REPLAY_START_MOUSE, "r");
        FILE* fStartKey = fopen(REPLAY_START_KEY, "r");

        if (fStartMouse || fStartKey) {
            char tempBuf[512];
            // Skip headers
            if (fStartMouse) { fgets(tempBuf, sizeof(tempBuf), fStartMouse); fgets(tempBuf, sizeof(tempBuf), fStartMouse); }
            if (fStartKey) { fgets(tempBuf, sizeof(tempBuf), fStartKey); fgets(tempBuf, sizeof(tempBuf), fStartKey); }

            MSLLHOOKSTRUCT s_mouseData = {0};
            uint32_t s_mouseMsgId = 0, s_mouseIndex = 0;
            KBDLLHOOKSTRUCT s_kbdData = {0};
            uint32_t s_kbdMsgId = 0, s_kbdIndex = 0;
            int s_has_mouse = 0, s_has_kbd = 0;
            int s_res_mouse = 0, s_res_kbd = 0;

            while (1) {
                if (!s_has_mouse && fStartMouse) {
                    s_res_mouse = fscanf(fStartMouse, "%u,%x,%lu,%ld,%ld,%x",
                        &s_mouseIndex, &s_mouseMsgId, &s_mouseData.time, 
                        &s_mouseData.pt.x, &s_mouseData.pt.y, &s_mouseData.mouseData);
                    if (s_res_mouse == 6) s_has_mouse = 1;
                    else { fclose(fStartMouse); fStartMouse = NULL; }
                }

                if (!s_has_kbd && fStartKey) {
                    s_res_kbd = fscanf(fStartKey, "%u,%x,%lu,%x,%x,%x",
                        &s_kbdIndex, &s_kbdMsgId, &s_kbdData.time, 
                        &s_kbdData.vkCode, &s_kbdData.scanCode, &s_kbdData.flags);
                    if (s_res_kbd == 6) s_has_kbd = 1;
                    else { fclose(fStartKey); fStartKey = NULL; }
                }

                if (!s_has_mouse && !s_has_kbd) break;

                int s_choice = -1;
                if (s_has_mouse && s_has_kbd) {
                    if (s_mouseData.time <= s_kbdData.time) s_choice = 0;
                    else s_choice = 1;
                } else if (s_has_mouse) s_choice = 0;
                else if (s_has_kbd) s_choice = 1;

                ReplayEvent* evt = (ReplayEvent*)malloc(sizeof(ReplayEvent));
                if (!evt) break;
                evt->input = (INPUT){0};

                if (s_choice == 0) { // MOUSE
                    evt->time = s_mouseData.time;
                    evt->input.type = INPUT_MOUSE;
                    evt->input.mi.dx = (s_mouseData.pt.x * 65535) / (replay->screenWidth - 1);
                    evt->input.mi.dy = (s_mouseData.pt.y * 65535) / (replay->screenHeight - 1);
                    evt->input.mi.mouseData = s_mouseData.mouseData;
                    HOOK_decode_mouse_flags(s_mouseMsgId, &evt->input.mi.dwFlags);
                    s_has_mouse = 0;
                } else { // KEYBOARD
                    evt->time = s_kbdData.time;
                    evt->input.type = INPUT_KEYBOARD;
                    evt->input.ki.wScan = (WORD)s_kbdData.scanCode;
                    evt->input.ki.dwFlags = KEYEVENTF_SCANCODE;
                    if (s_kbdData.flags & LLKHF_UP) evt->input.ki.dwFlags |= KEYEVENTF_KEYUP;
                    if (s_kbdData.flags & LLKHF_EXTENDED) evt->input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
                    s_has_kbd = 0;
                }

                while (HOOK_FUNC_Write_RingData(&replay->ringData, evt) != 0) {
                    Sleep(1);
                }
            }
            if (fStartMouse) fclose(fStartMouse);
            if (fStartKey) fclose(fStartKey);
        }
    }


    while (1) {
        if (!has_mouse_event && replay->mouse_log_file) {
            res_mouse = fscanf(replay->mouse_log_file, "%u,%x,%lu,%ld,%ld,%x",
                &mouseIndex, &mouseMsgId, &mouseData.time, 
                &mouseData.pt.x, &mouseData.pt.y, &mouseData.mouseData);
            if (res_mouse == 6) has_mouse_event = 1;
            else { fclose(replay->mouse_log_file); replay->mouse_log_file = NULL; }
        }

        if (!has_kbd_event && replay->keyboard_log_file) {
            res_kbd = fscanf(replay->keyboard_log_file, "%u,%x,%lu,%x,%x,%x",
                &kbdIndex, &kbdMsgId, &kbdData.time, 
                &kbdData.vkCode, &kbdData.scanCode, &kbdData.flags);
            if (res_kbd == 6) has_kbd_event = 1;
            else { fclose(replay->keyboard_log_file); replay->keyboard_log_file = NULL; }
        }

        if (!has_mouse_event && !has_kbd_event) break;

        int choice = -1; 
        if (has_mouse_event && has_kbd_event) {
            if (mouseData.time <= kbdData.time) choice = 0;
            else choice = 1;
        } else if (has_mouse_event) {
            choice = 0;
        } else if (has_kbd_event) {
            choice = 1;
        }

        ReplayEvent* evt = (ReplayEvent*)malloc(sizeof(ReplayEvent));
        if (!evt) break;
        evt->input = (INPUT){0};

        if (choice == 0) { // MOUSE
            evt->time = mouseData.time;
            evt->input.type = INPUT_MOUSE;
            evt->input.mi.dx = (mouseData.pt.x * 65535) / (replay->screenWidth - 1);
            evt->input.mi.dy = (mouseData.pt.y * 65535) / (replay->screenHeight - 1);
            evt->input.mi.mouseData = mouseData.mouseData;
            HOOK_decode_mouse_flags(mouseMsgId, &evt->input.mi.dwFlags);
            has_mouse_event = 0; 
        } 
        else if (choice == 1) { // KEYBOARD
            evt->time = kbdData.time;
            evt->input.type = INPUT_KEYBOARD;
            evt->input.ki.wScan = (WORD)kbdData.scanCode;
            evt->input.ki.dwFlags = KEYEVENTF_SCANCODE;
            if (kbdData.flags & LLKHF_UP) evt->input.ki.dwFlags |= KEYEVENTF_KEYUP;
            if (kbdData.flags & LLKHF_EXTENDED) evt->input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            has_kbd_event = 0; 
        }

        while (HOOK_FUNC_Write_RingData(&replay->ringData, evt) != 0) {
            Sleep(1);
        }
    }

    while (HOOK_FUNC_Write_RingData(&replay->ringData, NULL) != 0) {
        Sleep(1);
    }
    return 0; 
} 

DWORD WINAPI HOOK_replay_replay_ing(LPVOID lpParam) { 
    ReplayContext* replay = (ReplayContext*)lpParam;
    ReplayEvent* evt = NULL;
    int is_first_event = 1;

    while (1) {
        HOOK_FUNC_Read_RingData(&replay->ringData, (void**)&evt);
        if (evt == NULL) break;

        if (is_first_event) {
            replay->last_event_time = evt->time;
            is_first_event = 0;
        }

        if (evt->time > replay->last_event_time) {
            DWORD sleep_time = evt->time - replay->last_event_time;
            if (sleep_time > 5000) sleep_time = 5000; 
            if (sleep_time > 0) Sleep(sleep_time);
        }
        replay->last_event_time = evt->time;

        SendInput(1, &evt->input, sizeof(INPUT));
        free(evt);
    }
    return 0; 
} 


int HOOK_replay_start(ReplayContext* replay)  { // hàm này về sau dùng để khởi động HOOK_replay_load_file HOOK_replay_load_file
    printf("Replay Start... Screen: %dx%d\n", replay->screenWidth, replay->screenHeight);
    HOOK_FUNC_RingData_INIT(&replay->ringData);
    
    HANDLE hThreads[2];
    hThreads[0] = CreateThread(NULL, 0, HOOK_replay_load_file, replay, 0, NULL);
    hThreads[1] = CreateThread(NULL, 0, HOOK_replay_replay_ing, replay, 0, NULL);

    WaitForMultipleObjects(2, hThreads, TRUE, INFINITE);

    CloseHandle(hThreads[0]);
    CloseHandle(hThreads[1]);

    if (replay->mouse_log_file) fclose(replay->mouse_log_file);
    if (replay->keyboard_log_file) fclose(replay->keyboard_log_file);
    printf("Replay finished.\n");

    return 0;
}
