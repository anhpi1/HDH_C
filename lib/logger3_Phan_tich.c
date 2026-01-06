#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

// --- CẤU HÌNH ---
#define INPUT_DIR "processed_events"
#define REPORT_FILE "Bao_cao_thong_ke.txt"
#define CSV_DETAIL_FILE "Bang_chi_tiet_su_kien.csv" // <--- FILE MỚI BẠN YÊU CẦU

#define SRC_UNKNOWN 0
#define SRC_KEYBOARD 1
#define SRC_MOUSE 2

// Cấu trúc lưu thông tin Step
typedef struct {
    int stt;
    int src_type;
    char action_name[256];
    char full_path[512];
    unsigned int time_stamp;
} StepInfo;

// --- BIẾN TOÀN CỤC ---
long long count_keystrokes = 0;
long long count_lclick = 0;
long long count_rclick = 0;
long long count_scroll_up = 0;
long long count_scroll_down = 0;
char combo_log[20000] = ""; // Tăng bộ đệm log lên cho thoải mái

// Trạng thái Modifier
bool is_ctrl = false;
bool is_alt = false;
bool is_shift = false;
bool is_win = false;

// Biến chống Spam Combo
static unsigned int last_combo_time = 0;
static char last_combo_key[256] = "";

// --- CÁC HÀM HỖ TRỢ ---

int detect_source(const char *filename) {
    if (strstr(filename, "keyboard.csv")) return SRC_KEYBOARD;
    if (strstr(filename, "mouse.csv")) return SRC_MOUSE;
    return SRC_UNKNOWN;
}

void extract_action_name(const char *filename, char *buffer) {
    char temp[256];
    strcpy(temp, filename);
    char *start = strchr(temp, '_');
    if (!start) { strcpy(buffer, "Unknown"); return; }
    start++;
    char *end = strrchr(start, '_');
    if (!end) { strcpy(buffer, start); return; }
    *end = 0;
    strcpy(buffer, start);
}

int find_step_info(int stt, StepInfo *info) {
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char searchPattern[256];
    sprintf(searchPattern, "%s\\%05d_*.csv", INPUT_DIR, stt);
    hFind = FindFirstFile(searchPattern, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    int found = 0;
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (findData.nFileSizeLow > 0) { // File thật (> 0 byte)
            info->stt = stt;
            sprintf(info->full_path, "%s\\%s", INPUT_DIR, findData.cFileName);
            info->src_type = detect_source(findData.cFileName);
            extract_action_name(findData.cFileName, info->action_name);
            
            FILE *fp = fopen(info->full_path, "r");
            if (fp) {
                char line[512];
                fgets(line, sizeof(line), fp);
                if (fgets(line, sizeof(line), fp)) {
                    char *token = strtok(line, ",");
                    token = strtok(NULL, ",");
                    token = strtok(NULL, ",");
                    if (token) info->time_stamp = (unsigned int)strtoul(token, NULL, 10);
                }
                fclose(fp);
            }
            found = 1;
            break;
        }
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);
    return found;
}

// --- LOGIC PHÂN TÍCH (ĐÃ TỐI ƯU HÓA) ---

void analyze_step(StepInfo *step) {
    if (step->src_type == SRC_MOUSE) {
        if (strstr(step->action_name, "Nhan_chuot_trai")) count_lclick++;
        else if (strstr(step->action_name, "Nhan_chuot_phai")) count_rclick++;
        else if (strstr(step->action_name, "Cuon_chuot_len")) count_scroll_up++;
        else if (strstr(step->action_name, "Cuon_chuot_xuong")) count_scroll_down++;
        return; 
    }

    if (step->src_type == SRC_KEYBOARD) {
        bool is_down = (strncmp(step->action_name, "Nhan_", 5) == 0);
        char real_key_name[256];
        strcpy(real_key_name, step->action_name + (is_down ? 5 : 4));

        if (!is_down) {
            if (strcmp(real_key_name, "Ctrl") == 0) is_ctrl = false;
            else if (strcmp(real_key_name, "Alt") == 0) is_alt = false;
            else if (strcmp(real_key_name, "Shift") == 0) is_shift = false;
            else if (strcmp(real_key_name, "Win") == 0) is_win = false;
            last_combo_time = 0; 
            return;
        }

        count_keystrokes++; 

        bool current_is_modifier = false;
        if (strcmp(real_key_name, "Ctrl") == 0) { is_ctrl = true; current_is_modifier = true; }
        else if (strcmp(real_key_name, "Alt") == 0) { is_alt = true; current_is_modifier = true; }
        else if (strcmp(real_key_name, "Shift") == 0) { is_shift = true; current_is_modifier = true; }
        else if (strcmp(real_key_name, "Win") == 0) { is_win = true; current_is_modifier = true; }

        if (is_ctrl || is_alt || is_shift || is_win) {
            if (current_is_modifier) {
                int mod_count = (is_ctrl?1:0) + (is_alt?1:0) + (is_shift?1:0) + (is_win?1:0);
                if (mod_count < 2) return;
            }

            // Logic lọc Shift thông minh (Giữ lại số, ký tự đặc biệt, bỏ qua chữ cái)
            if (is_shift && !is_ctrl && !is_alt && !is_win) {
                char *ptr = strstr(step->action_name, "Nhan_phim_");
                if (ptr != NULL) {
                    char c = ptr[10]; 
                    if (c >= 'A' && c <= 'Z') return; // Bỏ qua chữ hoa
                }
                if (strstr(step->action_name, "Nhan_Space")) return; // Bỏ qua Shift + Space
            }

            char combo_str[256] = "";
            if (is_ctrl && strcmp(real_key_name, "Ctrl") != 0) strcat(combo_str, "Ctrl + ");
            if (is_alt && strcmp(real_key_name, "Alt") != 0) strcat(combo_str, "Alt + ");
            if (is_shift && strcmp(real_key_name, "Shift") != 0) strcat(combo_str, "Shift + ");
            if (is_win && strcmp(real_key_name, "Win") != 0) strcat(combo_str, "Win + ");
            strcat(combo_str, real_key_name);

            if (strcmp(combo_str, last_combo_key) == 0 && 
               (step->time_stamp - last_combo_time) < 200) {
                last_combo_time = step->time_stamp;
            } 
            else {
                char log_line[512];
                sprintf(log_line, "- Phat hien Combo: [%s] tai thoi diem %u (STT: %05d)\n", 
                        combo_str, step->time_stamp, step->stt);
                printf("%s", log_line);
                if (strlen(combo_log) < 19000) strcat(combo_log, log_line);
                strcpy(last_combo_key, combo_str);
                last_combo_time = step->time_stamp;
            }
        }
    }
}

// --- MAIN ---

int Phan_tich (void) {
    printf("=== CHUONG TRINH 3: PHAN TICH & THONG KE (FINAL) ===\n");
    printf("Output 1: %s (Bao cao)\n", REPORT_FILE);
    printf("Output 2: %s (Bang chi tiet)\n\n", CSV_DETAIL_FILE);

    int stt = 1;
    StepInfo current_step;

    // >>> MỞ FILE CSV ĐỂ GHI <<<
    FILE *f_csv = fopen(CSV_DETAIL_FILE, "w");
    if (f_csv) {
        // Ghi dòng tiêu đề (Header)
        fprintf(f_csv, "So_Thu_Tu,Ten_Su_Kien,Noi_Luu_File\n");
    } else {
        printf("[Canh bao] Khong the tao file CSV chi tiet!\n");
    }

    while (find_step_info(stt, &current_step)) {
        // 1. Phân tích thống kê & Combo
        analyze_step(&current_step);

        // 2. Ghi vào bảng CSV chi tiết (Theo yêu cầu của bạn)
        if (f_csv) {
            fprintf(f_csv, "%d,%s,%s\n", 
                current_step.stt,          // Số thứ tự
                current_step.action_name,  // Tên sự kiện
                current_step.full_path     // Đường dẫn file
            );
        }

        stt++;
    }

    // Đóng file CSV sau khi ghi xong
    if (f_csv) {
        fclose(f_csv);
        printf("\nDa tao xong file bang: %s\n", CSV_DETAIL_FILE);
    }

    // --- XUẤT BÁO CÁO TỔNG HỢP ---
    FILE *f_report = fopen(REPORT_FILE, "w");
    if (f_report) {
        fprintf(f_report, "=== BAO CAO THONG KE SU DUNG ===\n");
        // Đã xóa dòng "Step" theo yêu cầu của bạn
        fprintf(f_report, "- Tong so lan nhan phim: %lld\n", count_keystrokes);
        fprintf(f_report, "- Click chuot trai:      %lld\n", count_lclick);
        fprintf(f_report, "- Click chuot phai:      %lld\n", count_rclick);
        fprintf(f_report, "- Cuon chuot len:        %lld\n", count_scroll_up);
        fprintf(f_report, "- Cuon chuot xuong:      %lld\n", count_scroll_down);
        fprintf(f_report, "\n=== DANH SACH COMBO PHIM DA DUNG ===\n");
        fprintf(f_report, "%s", combo_log);
        fclose(f_report);
        printf("Da tao xong file bao cao: %s\n", REPORT_FILE);
    }

    printf("\nNhan Enter de thoat...");
    getchar();
    return 0;
}