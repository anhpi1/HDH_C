//#include "logger.h"
#include <stdio.h>
#include <stdlib.h> // thư viện dùng để cấp bộ nhớ động với malloc, realloc, free   
#include <string.h> // thư viện dùng để xử lý chuỗi ký tự
#include <direct.h> // thư viện dùng để tạo thư mục với _mkdir

// --- CẤU HÌNH ---
#define LOG_DIR "log" // thư mục chứa các file log
#define OUTPUT_FOLDER "processed" // thư mục đầu ra
#define OUTPUT_FILE "processed/merged_log.csv" // file output
#define INITIAL_CAPACITY 10000 // Sức chứa mảng ban đầu

// Định nghĩa loại sự kiện
#define TYPE_KEYBOARD 1
#define TYPE_MOUSE    2

// --- CẤU TRÚC DỮ LIỆU TỔNG HỢP (SUPER STRUCT) ---
// Chứa tất cả các trường có thể có của cả Bàn phím và Chuột
typedef struct {
    int type;           // 1: KEYBOARD, 2: MOUSE
    
    // Các trường chung
    unsigned int id;    // Event ID
    unsigned int msg;   // MsgID (Hex)
    unsigned int time;  // Time (Quan trọng nhất: dùng để sắp xếp)

    // Các trường riêng của Keyboard
    unsigned int vk;    // VkCode (Hex)
    unsigned int scan;  // ScanCode (Hex)
    unsigned int flags; // Flags (Hex)

    // Các trường riêng của Mouse
    int x;              // X (Dec)
    int y;              // Y (Dec)
    unsigned int mouse_data; // MouseData (Hex)
} UnifiedEvent;

// Biến toàn cục để quản lý bộ nhớ động
UnifiedEvent *g_events = NULL;
int g_count = 0;
int g_capacity = 0;

// --- HÀM QUẢN LÝ BỘ NHỚ ---

// Thêm một sự kiện vào mảng chung (Tự động mở rộng bộ nhớ)
void add_event(UnifiedEvent evt) {
    if (g_count >= g_capacity) {
        // Tăng gấp đôi kích thước mảng nếu đầy (Exponential Growth)
        g_capacity = (g_capacity == 0) ? INITIAL_CAPACITY : g_capacity * 2;
        UnifiedEvent *temp = (UnifiedEvent*)realloc(g_events, g_capacity * sizeof(UnifiedEvent));
        if (!temp) {
            printf("[Loi] Khong du bo nho RAM de tai them du lieu!\n");
            exit(1);
        }
        g_events = temp;
    }
    g_events[g_count++] = evt;
}

// --- HÀM ĐỌC DỮ LIỆU ---

void load_logs() {
    char filename[256];
    char line[1024];
    int file_index;
    FILE *fp;

    // --- 1. ĐỌC LOG BÀN PHÍM (keyboard_log0, log1, log2...) ---
    file_index = 0;
    while (1) {
        sprintf(filename, "%s/keyboard_log%d.csv", LOG_DIR, file_index);
        fp = fopen(filename, "r");
        
        if (!fp) break; // Không tìm thấy file tiếp theo -> Dừng vòng lặp
        printf("Dang doc file: %s ... ", filename);

        // Bỏ qua 2 dòng đầu (Version và Header columns)
        if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); break; }
        if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); break; }

        int count_in_file = 0;
        while (fgets(line, sizeof(line), fp)) {
            UnifiedEvent evt = {0}; // Khởi tạo bằng 0 hết
            evt.type = TYPE_KEYBOARD;

            // Format Keyboard: Event,MsgID,Time,Vk,Scan,Flags
            // Lưu ý: MsgID, Vk, Scan, Flags là Hex (%x)
            // Cắt chuỗi thủ công để an toàn hơn sscanf nếu có trường rỗng (nhưng sscanf ổn với format này)
            // Dùng sscanf cho gọn mã nguồn vì log máy sinh ra chuẩn
            
            // Tách từng phần bằng dấu phẩy
            char *token;
            token = strtok(line, ","); if(token) evt.id = atoi(token); // hàm atoi có tác dụng chuyển đổi chuỗi ký tự thành số nguyên
            token = strtok(NULL, ","); if(token) evt.msg = (unsigned int)strtoul(token, NULL, 16); // hàm strtoul có tác dụng chuyển đổi chuỗi ký tự thành số nguyên không dấu
            token = strtok(NULL, ","); if(token) evt.time = (unsigned int)strtoul(token, NULL, 10); // hàm strtoul có tác dụng chuyển đổi chuỗi ký tự thành số nguyên không dấu
            token = strtok(NULL, ","); if(token) evt.vk = (unsigned int)strtoul(token, NULL, 16); // hàm strtoul có tác dụng chuyển đổi chuỗi ký tự thành số nguyên không dấu
            token = strtok(NULL, ","); if(token) evt.scan = (unsigned int)strtoul(token, NULL, 16); // hàm strtoul có tác dụng chuyển đổi chuỗi ký tự thành số nguyên không dấu
            token = strtok(NULL, ","); if(token) evt.flags = (unsigned int)strtoul(token, NULL, 16); // hàm strtoul có tác dụng chuyển đổi chuỗi ký tự thành số nguyên không dấu

            add_event(evt);
            count_in_file++;
        }
        printf("-> Da them %d su kien.\n", count_in_file);
        fclose(fp);
        file_index++; // Chuyển sang file tiếp theo
    }

    // --- 2. ĐỌC LOG CHUỘT (mouse_log0, log1, log2...) ---
    file_index = 0;
    while (1) {
        sprintf(filename, "%s/mouse_log%d.csv", LOG_DIR, file_index);
        fp = fopen(filename, "r");
        
        if (!fp) break;
        printf("Dang doc file: %s ... ", filename);

        // Bỏ qua 2 dòng đầu
        if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); break; }
        if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); break; }

        int count_in_file = 0;
        while (fgets(line, sizeof(line), fp)) {
            UnifiedEvent evt = {0};
            evt.type = TYPE_MOUSE;

            // Format Mouse: Event,MsgID,Time,X,Y,MouseData
            char *token;
            token = strtok(line, ","); if(token) evt.id = atoi(token);
            token = strtok(NULL, ","); if(token) evt.msg = (unsigned int)strtoul(token, NULL, 16);
            token = strtok(NULL, ","); if(token) evt.time = (unsigned int)strtoul(token, NULL, 10);
            token = strtok(NULL, ","); if(token) evt.x = atoi(token);
            token = strtok(NULL, ","); if(token) evt.y = atoi(token);
            token = strtok(NULL, ","); if(token) evt.mouse_data = (unsigned int)strtoul(token, NULL, 16);

            add_event(evt);
            count_in_file++;
        }
        printf("-> Da them %d su kien.\n", count_in_file);
        fclose(fp);
        file_index++;
    }
}

// --- HÀM SẮP XẾP ---

// Hàm so sánh cho qsort (Sắp xếp theo thời gian tăng dần)
int compare_events(const void *a, const void *b) {
    UnifiedEvent *evtA = (UnifiedEvent *)a;
    UnifiedEvent *evtB = (UnifiedEvent *)b;
    
    if (evtA->time < evtB->time) return -1;
    if (evtA->time > evtB->time) return 1;
    return 0; // Thời gian bằng nhau (hiếm gặp, giữ nguyên thứ tự)
}

// --- HÀM XUẤT FILE ---

void export_merged_log() {
    _mkdir(OUTPUT_FOLDER); // Tạo thư mục đầu ra nếu chưa có
    FILE *fp = fopen(OUTPUT_FILE, "w");
    if (!fp) {
        printf("[Loi] Khong the tao file dau ra: %s\n", OUTPUT_FILE);
        return;
    }

    // Ghi Header thống nhất (Chứa tất cả các cột để Bước 2 dễ đọc)
    fprintf(fp, "Type,ID,MsgID(hex),Time,Vk(hex),Scan(hex),Flags(hex),X,Y,MouseData(hex)\n");

    for (int i = 0; i < g_count; i++) {
        UnifiedEvent *e = &g_events[i];
        
        // Ghi dữ liệu ra file
        // Nếu là Keyboard thì X,Y,Data = 0
        // Nếu là Mouse thì Vk,Scan,Flags = 0
        // Tất cả số Hex in ra định dạng %X (chữ hoa) hoặc %x (chữ thường)
        fprintf(fp, "%d,%u,%x,%u,%x,%x,%x,%d,%d,%x\n",
            e->type,
            e->id,
            e->msg,
            e->time,
            e->vk,
            e->scan,
            e->flags,
            e->x,
            e->y,
            e->mouse_data
        );
    }

    fclose(fp);
    printf("Da ghi %d su kien vao file '%s'.\n", g_count, OUTPUT_FILE);
}

// --- MAIN ---

int Sap_xep (void) {
    printf("=== CHUONG TRINH 1: HOP NHAT VA SAP XEP LOG ===\n");
    
    // 1. Đọc dữ liệu
    load_logs();
    
    if (g_count == 0) {
        printf("Khong tim thay du lieu nao trong thu muc '%s'.\n", LOG_DIR);
        return 0;
    }

    // 2. Sắp xếp
    printf("Dang sap xep %d su kien theo thoi gian...\n", g_count);
    qsort(g_events, g_count, sizeof(UnifiedEvent), compare_events);

    // 3. Xuất file
    export_merged_log();

    // Dọn dẹp
    if (g_events) free(g_events);
    
    printf("=== HOAN TAT ===\n");
    return 0;
}


