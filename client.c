#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
#define MAX_PATH_LEN 256

typedef enum {
    CMD_START_RECORDING = 1,
    CMD_STOP_RECORDING = 2,
    CMD_REPLAY = 3,              // Phát lại với tham số đầy đủ
    CMD_EXIT = 0
} ClientCommand;

// Cấu trúc để gửi tham số replay
typedef struct {
    char mouse_log_file[MAX_PATH_LEN];
    char keyboard_log_file[MAX_PATH_LEN];
    int mode;  // 0: chỉ chuột, 1: chỉ bàn phím, 2: cả hai
} ReplayParams;

SOCKET clientSocket = INVALID_SOCKET;

int connect_to_server() {
    WSADATA wsaData;
    struct sockaddr_in serverAddr;
    
    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[CLIENT] Loi khoi tao Winsock. Error: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Tạo socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("[CLIENT] Loi tao socket. Error: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    
    // Cấu hình địa chỉ server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);
    
    // Kết nối đến server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[CLIENT] Khong the ket noi den server. Error: %d\n", WSAGetLastError());
        printf("[CLIENT] Hay dam bao server dang chay!\n");
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }
    
    printf("[CLIENT] Ket noi thanh cong den server %s:%d\n", SERVER_IP, SERVER_PORT);
    return 0;
}

int send_replay_command(const char* mouse_log, const char* keyboard_log, int mode) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int recvSize;
    
    // Gửi lệnh CMD_REPLAY
    sprintf(buffer, "%d", CMD_REPLAY);
    if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
        printf("[CLIENT] Loi gui lenh replay. Error: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Nhận phản hồi READY
    recvSize = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
    if (recvSize > 0) {
        response[recvSize] = '\0';
        printf("[SERVER RESPONSE] %s\n", response);
    }
    
    // Gửi tham số ReplayParams
    ReplayParams params;
    strncpy(params.mouse_log_file, mouse_log, MAX_PATH_LEN - 1);
    params.mouse_log_file[MAX_PATH_LEN - 1] = '\0';
    strncpy(params.keyboard_log_file, keyboard_log, MAX_PATH_LEN - 1);
    params.keyboard_log_file[MAX_PATH_LEN - 1] = '\0';
    params.mode = mode;
    
    if (send(clientSocket, (char*)&params, sizeof(ReplayParams), 0) == SOCKET_ERROR) {
        printf("[CLIENT] Loi gui tham so replay. Error: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Nhận phản hồi cuối
    recvSize = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
    if (recvSize > 0) {
        response[recvSize] = '\0';
        printf("[SERVER RESPONSE] %s\n", response);
    }
    
    return 0;
}

int send_command(int command) {
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int recvSize;
    
    // Gửi lệnh
    sprintf(buffer, "%d", command);
    if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
        printf("[CLIENT] Loi gui lenh. Error: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Nhận phản hồi
    recvSize = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
    if (recvSize > 0) {
        response[recvSize] = '\0';
        printf("[SERVER RESPONSE] %s\n", response);
    } else if (recvSize == 0) {
        printf("[CLIENT] Server da dong ket noi.\n");
        return -1;
    } else {
        printf("[CLIENT] Loi nhan phan hoi. Error: %d\n", WSAGetLastError());
        return -1;
    }
    
    return 0;
}

void disconnect_from_server() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    WSACleanup();
    printf("[CLIENT] Da ngat ket noi khoi server.\n");
}

void show_menu() {
    printf("\n");
    printf("========================================\n");
    printf("       MENU CLIENT - TEST SERVER\n");
    printf("========================================\n");
    printf("1. Bat dau ghi su kien\n");
    printf("2. Dung ghi su kien\n");
    printf("3. Phat lai voi tham so tu nhap\n");
    printf("4. Phat lai (mac dinh: mode=2)\n");
    printf("0. Thoat\n");
    printf("========================================\n");
    printf("Chon: ");
}

int main() {
    int choice;
    
    // Kết nối đến server
    if (connect_to_server() != 0) {
        printf("[CLIENT] Khong the ket noi. Thoat chuong trinh.\n");
        return 1;
    }
    
    // Menu chính
    while (1) {
        show_menu();
        
        if (scanf("%d", &choice) != 1) {
            printf("[CLIENT] Lua chon khong hop le!\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        getchar(); // Clear '\n'
        
        if (choice == 0) {
            printf("[CLIENT] Gui lenh thoat den server...\n");
            send_command(CMD_EXIT);
            break;
        }
        
        switch (choice) {
            case 1:
                printf("[CLIENT] Gui lenh bat dau ghi...\n");
                send_command(CMD_START_RECORDING);
                break;
                
            case 2:
                printf("[CLIENT] Gui lenh dung ghi...\n");
                send_command(CMD_STOP_RECORDING);
                break;
                
            case 3: {
                char mouse_file[MAX_PATH_LEN];
                char keyboard_file[MAX_PATH_LEN];
                int mode;
                
                printf("Nhap duong dan file mouse log: ");
                fgets(mouse_file, MAX_PATH_LEN, stdin);
                mouse_file[strcspn(mouse_file, "\n")] = '\0';
                
                printf("Nhap duong dan file keyboard log: ");
                fgets(keyboard_file, MAX_PATH_LEN, stdin);
                keyboard_file[strcspn(keyboard_file, "\n")] = '\0';
                
                printf("Nhap mode (0=chi chuot, 1=chi ban phim, 2=ca hai): ");
                scanf("%d", &mode);
                getchar();
                
                printf("[CLIENT] Gui lenh phat lai voi tham so:\n");
                printf("  Mouse log: %s\n", mouse_file);
                printf("  Keyboard log: %s\n", keyboard_file);
                printf("  Mode: %d\n", mode);
                
                send_replay_command(mouse_file, keyboard_file, mode);
                break;
            }
                
            case 4:
                printf("[CLIENT] Gui lenh phat lai voi tham so mac dinh...\n");
                send_replay_command("log/mouse_log0.csv", "log/keyboard_log0.csv", 2);
                break;
                
            default:
                printf("[CLIENT] Lua chon khong hop le. Vui long thu lai.\n");
                break;
        }
    }
    
    // Ngắt kết nối
    disconnect_from_server();
    
    printf("[CLIENT] Thoat chuong trinh.\n");
    return 0;
}
