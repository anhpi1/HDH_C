#include "main.h" 
#define THREAD_max_sevice 3 
DWORD WINAPI ThreadFunc1(LPVOID lpParam) { 
    HOOK_start_recording(); 
    return 0; 
} 
DWORD WINAPI ThreadFunc2(LPVOID lpParam) { 
    HOOK_stop_recording(); 
    return 0; 
} 
DWORD WINAPI ThreadFunc3(LPVOID lpParam) { 
    HOOK_replay_events("log/mouse_log0.csv", "log/keyboard_log0.csv", 2); 
    return 0; 
} 
int main() { 
    HANDLE hThreads[THREAD_max_sevice]; 
    while(1){ 
        printf("1. Bat dau ghi\n"); 
        printf("2. Dung ghi\n"); 
        printf("3. Phat lai\n"); 
        printf("0. Thoat\n"); 
        printf("Chon: "); 
        int choice; 
        scanf("%d", &choice); 
        getchar(); // clear '\n' 
        switch (choice) { 
            case 1: hThreads[0] = CreateThread(NULL, 0, ThreadFunc1, NULL, 0, NULL); break; 
            case 2: hThreads[1] = CreateThread(NULL, 0, ThreadFunc2, NULL, 0, NULL); break; 
            case 3: hThreads[2] = CreateThread(NULL, 0, ThreadFunc3, NULL, 0, NULL); break; 
            case 0: printf("Thoat chuong trinh.\n"); return 0; 
            default: printf("Lua chon khong hop le. Vui long thu lai.\n"); break; 
        } 
    } 
    return 0; 
}

// dựa vào hàm main hãu viết sever.c và sever.h để khởi động sever 
// lưu ý
// DWORD WINAPI ThreadFunc3(LPVOID lpParam)
// {
// HOOK_replay_events("log/mouse_log0.csv", "log/keyboard_log0.csv", 2);
// return 0;
// }
// cái này truyền vào có tham số tham số cuối dùng là mode 0 
// là chỉ phát chuột 1 là chỉ phát bàn phím 2 là phát cả hai viết client.c có menu để test