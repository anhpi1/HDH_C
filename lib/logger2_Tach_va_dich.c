#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h> // Thu vien tao thu muc (_mkdir) cho Windows

// --- CẤU HÌNH ---
#define INPUT_FILE "processed/merged_log.csv"
#define OUTPUT_DIR "processed_events"

// Định nghĩa Header cho file đầu ra (để Replay Engine hiểu cột)
#define HEADER_KB "Event(uint),MsgID(hex),Time(uint),VkCode(hex),ScanCode(hex),Flags(hex)\n"
#define HEADER_MS "Event(uint),MsgID(hex),Time(uint),X(int),Y(int),MouseData(hex)\n"

// Cấu trúc chứa dữ liệu 1 dòng
typedef struct {
    int type; // 1: KEYBOARD, 2: MOUSE
    unsigned int id;
    unsigned int msg;
    unsigned int time;
    unsigned int vk;
    unsigned int scan;
    unsigned int flags;
    int x;
    int y;
    unsigned int mouse_data;
} EventData;

// --- HÀM DỊCH MÃ SANG TIẾNG VIỆT (TỐI ƯU HÓA) ---

// --- PHIÊN BẢN ĐẦY ĐỦ CÁC PHÍM ---
const char* get_key_name_vi(unsigned int vk) {
    static char buffer[32];
    
    // 1. Chữ và Số (0-9, A-Z)
    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) {
        sprintf(buffer, "Nhan_phim_%c", vk);
        return buffer;
    }

    // 2. Tra cứu chi tiết
    switch (vk) {
        // --- Nhóm điều khiển cơ bản ---
        case 0x0D: return "Nhan_Enter";
        case 0x20: return "Nhan_Space";
        case 0x1B: return "Nhan_Esc";
        case 0x08: return "Nhan_Backspace";
        case 0x09: return "Nhan_Tab";
        case 0x14: return "Nhan_CapsLock";
        case 0x2C: return "Nhan_PrintScreen";
        case 0x91: return "Nhan_ScrollLock";
        case 0x13: return "Nhan_Pause";

        // --- Nhóm Modifier ---
        case 0x10: case 0xA0: case 0xA1: return "Nhan_Shift";
        case 0x11: case 0xA2: case 0xA3: return "Nhan_Ctrl";
        case 0x12: case 0xA4: case 0xA5: return "Nhan_Alt";
        case 0x5B: case 0x5C: return "Nhan_Win";
        case 0x5D: return "Nhan_Menu_Chuot_Phai"; // Phím Apps/Context Menu

        // --- Nhóm Điều hướng & Soạn thảo ---
        case 0x2D: return "Nhan_Insert";
        case 0x2E: return "Nhan_Delete";
        case 0x24: return "Nhan_Home";
        case 0x23: return "Nhan_End";
        case 0x21: return "Nhan_PageUp";
        case 0x22: return "Nhan_PageDown";
        case 0x25: return "Nhan_Mui_ten_Trai";
        case 0x26: return "Nhan_Mui_ten_Len";
        case 0x27: return "Nhan_Mui_ten_Phai";
        case 0x28: return "Nhan_Mui_ten_Xuong";

        // --- Nhóm Phím F (F1 - F12) ---
        case 0x70: return "Nhan_F1";
        case 0x71: return "Nhan_F2";
        case 0x72: return "Nhan_F3";
        case 0x73: return "Nhan_F4";
        case 0x74: return "Nhan_F5";
        case 0x75: return "Nhan_F6";
        case 0x76: return "Nhan_F7";
        case 0x77: return "Nhan_F8";
        case 0x78: return "Nhan_F9";
        case 0x79: return "Nhan_F10";
        case 0x7A: return "Nhan_F11";
        case 0x7B: return "Nhan_F12";

        // --- Nhóm Dấu câu (Trên bàn phím chuẩn US) ---
        case 0xBA: return "Nhan_Dau_Cham_Phay"; // ;
        case 0xBB: return "Nhan_Dau_Bang";      // =
        case 0xBC: return "Nhan_Dau_Phay";      // ,
        case 0xBD: return "Nhan_Dau_Tru";       // -
        case 0xBE: return "Nhan_Dau_Cham";      // .
        case 0xBF: return "Nhan_Dau_Gach_Cheo"; // /
        case 0xC0: return "Nhan_Dau_Huyen";     // ` (Backtick)
        case 0xDB: return "Nhan_Ngoac_Vuong_Mo";// [
        case 0xDC: return "Nhan_Dau_Gach_Nguoc";// \ (Backslash)
        case 0xDD: return "Nhan_Ngoac_Vuong_Dong";// ]
        case 0xDE: return "Nhan_Dau_Nhay_Don";  // '

        // --- Nhóm Numpad (Bàn phím số bên phải) ---
        case 0x90: return "Nhan_NumLock";
        case 0x60: return "Nhan_Num_0";
        case 0x61: return "Nhan_Num_1";
        case 0x62: return "Nhan_Num_2";
        case 0x63: return "Nhan_Num_3";
        case 0x64: return "Nhan_Num_4";
        case 0x65: return "Nhan_Num_5";
        case 0x66: return "Nhan_Num_6";
        case 0x67: return "Nhan_Num_7";
        case 0x68: return "Nhan_Num_8";
        case 0x69: return "Nhan_Num_9";
        case 0x6A: return "Nhan_Num_Sao"; // *
        case 0x6B: return "Nhan_Num_Cong"; // +
        case 0x6D: return "Nhan_Num_Tru";  // -
        case 0x6E: return "Nhan_Num_Cham"; // .
        case 0x6F: return "Nhan_Num_Chia"; // /

        // --- Đặc biệt ---
        case 0xE7: return "Ky_tu_Tieng_Viet";

        default: 
            sprintf(buffer, "Nhan_Key_0x%X", vk);
            return buffer;
    }
}

const char* get_mouse_action_vi(unsigned int msg) {
    switch (msg) {
        case 0x200: return "Di_chuyen_chuot";
        case 0x201: return "Nhan_chuot_trai";
        case 0x202: return "Nha_chuot_trai";
        case 0x204: return "Nhan_chuot_phai";
        case 0x205: return "Nha_chuot_phai";
        case 0x207: return "Nhan_chuot_giua"; // Con lăn
        case 0x208: return "Nha_chuot_giua";
        case 0x20A: return "Cuon_chuot";      // Lăn chuột
        default: return "Thao_tac_chuot_khac";
    }
}

// --- HÀM HỖ TRỢ FILES ---

// Parse dòng CSV thành Struct
int parse_line(const char* line, EventData* evt) {
    // Format: Type,ID,MsgID,Time,Vk,Scan,Flags,X,Y,MouseData
    // Dùng sscanf để parse nhanh
    int count = sscanf(line, "%d,%u,%x,%u,%x,%x,%x,%d,%d,%x",
        &evt->type, &evt->id, &evt->msg, &evt->time,
        &evt->vk, &evt->scan, &evt->flags,
        &evt->x, &evt->y, &evt->mouse_data);
    return (count >= 4); // Ít nhất phải parse được Type, ID, Msg, Time
}

// Tạo file rỗng (Dummy)
void create_dummy_file(const char* filepath) {
    FILE *fp = fopen(filepath, "w");
    if (fp) fclose(fp); // Tạo xong đóng ngay -> File 0 byte
}

// Ghi dữ liệu vào file Master
void write_event_to_file(FILE* fp, EventData* e) {
    if (e->type == 1) { // KEYBOARD
        fprintf(fp, "%u,%x,%u,%x,%x,%x\n", 
            e->id, e->msg, e->time, e->vk, e->scan, e->flags);
    } else { // MOUSE
        fprintf(fp, "%u,%x,%u,%d,%d,%x\n", 
            e->id, e->msg, e->time, e->x, e->y, e->mouse_data);
    }
}

// --- MAIN LOGIC ---

int Tach_va_dich (void) {
    printf("=== CHUONG TRINH 2: TACH FILE VA DICH MA ===\n");

    FILE *f_in = fopen(INPUT_FILE, "r");// để đọc file đầu vào
    if (!f_in) {
        printf("[Loi] Khong tim thay file '%s'. Hay chay Step 1 truoc!\n", INPUT_FILE);
        return 1;
    }

    // Tạo thư mục đầu ra
    _mkdir(OUTPUT_DIR);

    char line[1024];
    // Bỏ qua dòng Header của file merged_log.csv (Type,ID,...)
    fgets(line, sizeof(line), f_in);

    int stt = 0;
    EventData current_evt, pending_evt;
    int has_pending = 0; // Cờ đánh dấu có dòng đang chờ xử lý hay không

    // Vòng lặp chính xử lý từng dòng
    while (1) {
        // 1. Lấy dữ liệu (Từ file hoặc từ biến chờ Pending)
        if (has_pending) {// có dòng chờ xử lý thì thực hiện thao tác trong nhánh này để bên dưới biết để xử lý
            current_evt = pending_evt;
            has_pending = 0;
        } else {// không có thì đọc cái mới đọc mà hết thì out lỗi thì bỏ qua đọc tiếp
            if (fgets(line, sizeof(line), f_in) == NULL) break; // Hết file
            if (!parse_line(line, &current_evt)) continue;      // Bỏ qua dòng lỗi
        }

        stt++; // Tăng số thứ tự hành động
        char filename_master[256], filename_dummy[256];
        char action_name[128];

        // --- NHÁNH 1: DI CHUYỂN CHUỘT (GỘP NHÓM) ---
        if (current_evt.type == 2 && current_evt.msg == 0x200) {
            strcpy(action_name, "Di_chuyen_chuot");
            
            // Tạo tên file
            sprintf(filename_master, "%s/%05d_%s_mouse.csv", OUTPUT_DIR, stt, action_name);
            sprintf(filename_dummy,  "%s/%05d_%s_keyboard.csv", OUTPUT_DIR, stt, action_name);

            // Mở file Master để ghi (Append mode không cần, Write mode là đủ)
            FILE *fp_out = fopen(filename_master, "w");
            fprintf(fp_out, HEADER_MS); // Ghi Header
            write_event_to_file(fp_out, &current_evt); // Ghi dòng đầu tiên

            // Vòng lặp gom nhóm (Look-ahead)
            while (fgets(line, sizeof(line), f_in)) {
                EventData next_evt;
                if (!parse_line(line, &next_evt)) continue;

                if (next_evt.type == 2 && next_evt.msg == 0x200) {
                    // Vẫn là di chuyển chuột -> Ghi tiếp vào file đang mở
                    write_event_to_file(fp_out, &next_evt);
                } else {
                    // Gặp sự kiện KHÁC -> Ngừng gom
                    pending_evt = next_evt; // Lưu lại để vòng lặp chính xử lý sau
                    has_pending = 1;        // Bật cờ Pending
                    break;
                }
            }
            fclose(fp_out);
            create_dummy_file(filename_dummy); // Tạo file keyboard rỗng
        }
        
        // --- NHÁNH 2: CÁC SỰ KIỆN KHÁC (ĐƠN LẺ) ---
        else {
            // Xác định tên hành động và tạo file
            if (current_evt.type == 1) { // KEYBOARD
                // Cần check xem là Nhấn (WM_KEYDOWN = 0x100) hay Nhả (WM_KEYUP = 0x101)
                const char* key_text = get_key_name_vi(current_evt.vk);
                const char* prefix = (current_evt.msg == 0x101) ? "Nha" : "Nhan"; 
                // Lưu ý: Hàm get_key_name_vi đã có chữ "Nhan_" rồi, nên ta xử lý tên file chút
                if (current_evt.msg == 0x101) {
                    // Nếu là nhả phím, thay chữ "Nhan" thành "Nha" trong tên
                     sprintf(action_name, "Nha_%s", key_text + 5); // +5 để bỏ chữ "Nhan_" gốc
                } else {
                    strcpy(action_name, key_text);
                }

                sprintf(filename_master, "%s/%05d_%s_keyboard.csv", OUTPUT_DIR, stt, action_name);
                sprintf(filename_dummy,  "%s/%05d_%s_mouse.csv", OUTPUT_DIR, stt, action_name);

                FILE *fp_out = fopen(filename_master, "w");
                fprintf(fp_out, HEADER_KB);
                write_event_to_file(fp_out, &current_evt);
                fclose(fp_out);
                create_dummy_file(filename_dummy);
            } 
            else { // MOUSE (Click, Scroll...)
                
                
                if (current_evt.msg == 0x20A) { // 0x20A là WM_MOUSEWHEEL
                    // Ép kiểu sang (int) để máy hiểu số Hex lớn là số âm
                    if ((int)current_evt.mouse_data > 0) {
                        strcpy(action_name, "Cuon_chuot_len");
                    } else {
                        strcpy(action_name, "Cuon_chuot_xuong");
                    }
                } 
                else {
                    // Các hành động khác (Click trái, phải...) giữ nguyên logic cũ
                    strcpy(action_name, get_mouse_action_vi(current_evt.msg));
                }
                

                sprintf(filename_master, "%s/%05d_%s_mouse.csv", OUTPUT_DIR, stt, action_name);
                sprintf(filename_dummy,  "%s/%05d_%s_keyboard.csv", OUTPUT_DIR, stt, action_name);

                FILE *fp_out = fopen(filename_master, "w");
                fprintf(fp_out, HEADER_MS);
                write_event_to_file(fp_out, &current_evt);
                fclose(fp_out);
                create_dummy_file(filename_dummy);
            }
        }
    }

    fclose(f_in);
    printf("Hoan tat! Da tao %d cap file trong thu muc '%s'.\n", stt, OUTPUT_DIR);
    return 0;
}
