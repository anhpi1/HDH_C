#include <windows.h>
#include <stdio.h>
#include <string.h>

// Đảm bảo tên PIPE này KHỚP CHÍNH XÁC với PIPE_NAME trong server.h của bạn
#define PIPE_NAME "\\\\.\\pipe\\MyPipe" 
#define BUFFER_SIZE 128

void print_menu() {
    printf("\n--- CLIENT MENU ---\n");
    printf("1. START (Ghi lai thao tac)\n");
    printf("2. STOP (Dung ghi)\n");
    printf("3. REPLAY (Phat lai thao tac)\n");
    printf("4. EXIT\n");
    printf("5. Sap_xep (Xu ly log)\n");
    printf("6. Tach_va_dich (Tach va dich log)\n");
    printf("7. Phan_tich (Phan tich va thong ke)\n");
    printf("Chon chuc nang: ");
}

int main() {
    HANDLE hPipe;
    char buffer[BUFFER_SIZE];
    char response[256];
    DWORD dwWritten, dwRead;
    int choice;
    
    // Các biến để chứa thông tin gửi đi
    char cmd[32];
    char mouse_file[64];
    char key_file[64];
    int mode = 0;

    printf("Dang ket noi den Server...\n");

    // 1. Kết nối đến Named Pipe
    // Server phải đang chạy trước khi Client kết nối
    while (1) {
        hPipe = CreateFile(
            PIPE_NAME,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        if (GetLastError() != ERROR_PIPE_BUSY) {
            printf("Khong the ket noi den Server. Hay dam bao Server dang chay.\n");
            return -1;
        }

        // Nếu Pipe bận, chờ 2 giây rồi thử lại
        if (!WaitNamedPipe(PIPE_NAME, 2000)) {
            printf("Time out khi cho ket noi Pipe.\n");
            return -1;
        }
    }

    printf("Ket noi thanh cong!\n");

    // 2. Vòng lặp xử lý lệnh
    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); // Xóa bộ nhớ đệm nếu nhập sai
            continue;
        }

        // Đặt giá trị mặc định để tránh lỗi parse bên server
        strcpy(mouse_file, "none");
        strcpy(key_file, "none");
        mode = 0;
        int should_send = 1;

        switch (choice) {
            case 1:
                strcpy(cmd, "START");
                // Mặc định tên file nếu người dùng không muốn nhập
                printf("Nhap ten file chuot (vd: mouse.log): ");
                scanf("%s", mouse_file);
                printf("Nhap ten file phim (vd: key.log): ");
                scanf("%s", key_file);
                break;
            case 2:
                strcpy(cmd, "STOP");
                // Lệnh STOP không cần file, nhưng server vẫn parse nên ta để mặc định
                break;
            case 3:
                strcpy(cmd, "REPLAY");
                printf("Nhap ten file chuot de phat: ");
                scanf("%s", mouse_file);
                printf("Nhap ten file phim de phat: ");
                scanf("%s", key_file);
                printf("Nhap che do (mode 0/1...): ");
                scanf("%d", &mode);
                break;
            case 4:
                should_send = 0;
                break;
            case 5:
                strcpy(cmd, "Sap_xep");
                break;
            case 6:
                strcpy(cmd, "Tach_va_dich");
                break;
            case 7:
                strcpy(cmd, "Phan_tich");
                break;
            default:
                printf("Lua chon khong hop le.\n");
                should_send = 0;
        }

        if (choice == 4) break;

        if (should_send) {
            // 3. Định dạng dữ liệu theo cấu trúc Server mong đợi:
            // "%31s %255s %255s %d" -> CMD MOUSE_FILE KEY_FILE MODE
            sprintf(buffer, "%s %s %s %d", cmd, mouse_file, key_file, mode);

            // 4. Gửi lệnh đến Server
            BOOL writeSuccess = WriteFile(
                hPipe,
                buffer,
                strlen(buffer), // Gửi độ dài chuỗi thực tế
                &dwWritten,
                NULL);

            if (!writeSuccess) {
                printf("Loi khi gui du lieu. Server co the da dong.\n");
                break;
            }

            // 5. Đọc phản hồi từ Server
            BOOL readSuccess = ReadFile(
                hPipe,
                response,
                sizeof(response) - 1,
                &dwRead,
                NULL);

            if (readSuccess && dwRead > 0) {
                response[dwRead] = '\0'; // Kết thúc chuỗi
                printf("\nServer phan hoi: %s\n", response);
            } else {
                printf("Khong nhan duoc phan hoi tu Server.\n");
            }
        }
    }

    // 6. Đóng kết nối
    CloseHandle(hPipe);
    printf("Da ngat ket noi.\n");
    return 0;
}