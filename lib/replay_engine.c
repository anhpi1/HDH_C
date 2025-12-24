#include "replay_engine.h"


// Định nghĩa struct nội bộ nếu chưa có trong header


// Hàm giải mã flags chuột (Giữ nguyên logic của bạn nhưng làm gọn)
int decode_mouse_flags(const uint32_t msgID, DWORD *dwFlags) {
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

// Hàm bỏ qua header của file log
void skip_log_header(FILE* f) {
    if (!f) return;
    char buffer[512];
    // Dòng 1: Version/Info
    fgets(buffer, sizeof(buffer), f);
    // Dòng 2: Header cột (Index, MsgID...)
    fgets(buffer, sizeof(buffer), f);
}

int replay_events(const char* mouse_log_file, const char* keyboard_log_file, int mode) {
    ReplayContext ctx = {0};
    char buffer[512];

    // 1. Mở file dựa trên mode
    // Mode: 0=Mouse, 1=Keyboard, 2=Both
    if (mode == 0 || mode == 2) {
        ctx.mouse_log_file = fopen(mouse_log_file, "r");
        if (!ctx.mouse_log_file) printf("Error: Cannot open mouse log %s\n", mouse_log_file);
    }
    if (mode == 1 || mode == 2) {
        ctx.keyboard_log_file = fopen(keyboard_log_file, "r");
        if (!ctx.keyboard_log_file) printf("Error: Cannot open keyboard log %s\n", keyboard_log_file);
    }

    if (!ctx.mouse_log_file && !ctx.keyboard_log_file) return -1;

    // 2. Đọc thông tin màn hình từ file hợp lệ đầu tiên
    FILE* headerFile = ctx.mouse_log_file ? ctx.mouse_log_file : ctx.keyboard_log_file;
    
    // Đọc dòng đầu tiên để lấy screen info
    fscanf(headerFile,
        "version,1,startTime,%04hd-%02hd-%02hd %02hd:%02hd:%02hd.%03hd,screenWidth,%d,screenHeight,%d\n",
        &ctx.st.wYear, &ctx.st.wMonth, &ctx.st.wDay, &ctx.st.wHour, &ctx.st.wMinute, &ctx.st.wSecond, &ctx.st.wMilliseconds,
        &ctx.screenWidth, &ctx.screenHeight
    );

    // Bỏ qua dòng tiêu đề cột cho cả 2 file (nếu mở)
    // Lưu ý: File headerFile đã đọc dòng 1 bằng fscanf, nên chỉ cần fgets thêm 1 lần để qua dòng tiêu đề cột.
    // Các file khác cần skip full header.
    fgets(buffer, sizeof(buffer), headerFile); 

    if (ctx.mouse_log_file && ctx.mouse_log_file != headerFile) skip_log_header(ctx.mouse_log_file);
    if (ctx.keyboard_log_file && ctx.keyboard_log_file != headerFile) skip_log_header(ctx.keyboard_log_file);

    // 3. Biến lưu trữ dữ liệu tạm
    MSLLHOOKSTRUCT mouseData = {0};
    uint32_t mouseMsgId = 0, mouseIndex = 0;
    
    KBDLLHOOKSTRUCT kbdData = {0};
    uint32_t kbdMsgId = 0, kbdIndex = 0;

    // Cờ trạng thái: 1 = Đã tải dữ liệu vào biến và đang chờ xử lý, 0 = Cần đọc mới
    int has_mouse_event = 0;
    int has_kbd_event = 0;
    
    // Kết quả fscanf
    int res_mouse = 0;
    int res_kbd = 0;

    // Khởi tạo thời gian gốc (sẽ được set bằng thời gian của sự kiện đầu tiên)
    int is_first_event = 1;

    printf("Replay Start... Screen: %dx%d\n", ctx.screenWidth, ctx.screenHeight);

    // 4. Vòng lặp chính (Merge Loop)
    while (1) {
        // --- BƯỚC A: Nạp dữ liệu nếu đang thiếu ---
        
        // Đọc Mouse nếu chưa có và file đang mở
        if (!has_mouse_event && ctx.mouse_log_file) {
            res_mouse = fscanf(ctx.mouse_log_file, "%u,%x,%lu,%ld,%ld,%x",
                &mouseIndex, &mouseMsgId, &mouseData.time, 
                &mouseData.pt.x, &mouseData.pt.y, &mouseData.mouseData);
            if (res_mouse == 6) has_mouse_event = 1;
            else { fclose(ctx.mouse_log_file); ctx.mouse_log_file = NULL; } // Hết file hoặc lỗi
        }

        // Đọc Keyboard nếu chưa có và file đang mở
        if (!has_kbd_event && ctx.keyboard_log_file) {
            res_kbd = fscanf(ctx.keyboard_log_file, "%u,%x,%lu,%x,%x,%x",
                &kbdIndex, &kbdMsgId, &kbdData.time, 
                &kbdData.vkCode, &kbdData.scanCode, &kbdData.flags);
            if (res_kbd == 6) has_kbd_event = 1;
            else { fclose(ctx.keyboard_log_file); ctx.keyboard_log_file = NULL; } // Hết file hoặc lỗi
        }

        // Nếu cả 2 đều không còn dữ liệu -> Kết thúc
        if (!has_mouse_event && !has_kbd_event) break;

        // --- BƯỚC B: Chọn sự kiện để thực thi ---
        // 0: Mouse, 1: Keyboard
        int choice = -1; 
        uint32_t current_event_time = 0;

        if (has_mouse_event && has_kbd_event) {
            if (mouseData.time <= kbdData.time) choice = 0;
            else choice = 1;
        } else if (has_mouse_event) {
            choice = 0;
        } else if (has_kbd_event) {
            choice = 1;
        }

        // --- BƯỚC C: Xử lý thời gian chờ (Sleep) ---
        if (choice == 0) current_event_time = mouseData.time;
        else current_event_time = kbdData.time;

        if (is_first_event) {
            ctx.last_event_time = current_event_time;
            is_first_event = 0;
        }

        // Tính delta time để sleep
        if (current_event_time > ctx.last_event_time) {
            DWORD sleep_time = current_event_time - ctx.last_event_time;
            // Giới hạn sleep để tránh treo nếu log bị lỗi timestamp quá lớn
            if (sleep_time > 5000) sleep_time = 5000; 
            if (sleep_time > 0) Sleep(sleep_time);
        }
        ctx.last_event_time = current_event_time;

        // --- BƯỚC D: Gửi sự kiện (SendInput) ---
        INPUT input = {0};

        if (choice == 0) { // MOUSE
            input.type = INPUT_MOUSE;
            // Chuyển đổi toạ độ sang Absolute (0-65535)
            input.mi.dx = (mouseData.pt.x * 65535) / (ctx.screenWidth - 1);
            input.mi.dy = (mouseData.pt.y * 65535) / (ctx.screenHeight - 1);
            input.mi.mouseData = mouseData.mouseData;
            input.mi.time = 0; // Để Windows tự gán timestamp hiện tại
            decode_mouse_flags(mouseMsgId, &input.mi.dwFlags);
            
            SendInput(1, &input, sizeof(INPUT));
            has_mouse_event = 0; // Đánh dấu đã dùng xong sự kiện này
        } 
        else if (choice == 1) { // KEYBOARD
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = 0; // Dùng scan code để chính xác hơn
            input.ki.wScan = (WORD)kbdData.scanCode;
            input.ki.dwFlags = KEYEVENTF_SCANCODE;
            input.ki.time = 0;

            if (kbdData.flags & LLKHF_UP) input.ki.dwFlags |= KEYEVENTF_KEYUP;
            if (kbdData.flags & LLKHF_EXTENDED) input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;

            SendInput(1, &input, sizeof(INPUT));
            has_kbd_event = 0; // Đánh dấu đã dùng xong sự kiện này
        }
    }

    // Cleanup
    if (ctx.mouse_log_file) fclose(ctx.mouse_log_file);
    if (ctx.keyboard_log_file) fclose(ctx.keyboard_log_file);
    printf("Replay finished.\n");

    return 0;
}