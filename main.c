#include <winsock2.h>
#include <ws2tcpip.h>
#include "main.h"
#include "lib/server.h"

#define THREAD_max_sevice 3

DWORD WINAPI ServerThreadFunc(LPVOID lpParam) {
    printf("========================================\n");
    printf("    SERVER - Ghi va Phat Lai Su Kien\n");
    printf("========================================\n");
    printf("Dang khoi dong server...\n\n");
    SERVER_start();
    return 0;
}

DWORD WINAPI ThreadFunc1(LPVOID lpParam)
{
    HOOK_start_recording();
    return 0;
}
DWORD WINAPI ThreadFunc2(LPVOID lpParam)
{
    HOOK_stop_recording();
    return 0;
}
DWORD WINAPI ThreadFunc3(LPVOID lpParam)
{
    HOOK_replay_events("log/mouse_log0.csv", "log/keyboard_log0.csv", 2);
    return 0;
}


int main() {
    HANDLE hThreads[THREAD_max_sevice];
    
    // Khởi động server trong thread riêng
    printf("Khoi dong server...\n");
    HANDLE serverThread = CreateThread(NULL, 0, ServerThreadFunc, NULL, 0, NULL);
    
    if (serverThread == NULL) {
        printf("Loi: Khong the khoi dong server!\n");
        return 1;
    }
    
    printf("Server da duoc khoi dong trong background.\n");
    printf("Server dang lang nghe tren cong %d\n\n", SERVER_PORT);
    
    // Giữ chương trình chạy
    printf("Nhan Enter de dung server va thoat...\n");
    getchar();
    
    // Dừng server
    SERVER_stop();
    
    if (serverThread) {
        WaitForSingleObject(serverThread, 1000);
        CloseHandle(serverThread);
    }
    
    printf("Thoat chuong trinh.\n");
    return 0;
}

