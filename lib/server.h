#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
#define MAX_PATH_LEN 256

// Các lệnh từ client
typedef enum {
    CMD_START_RECORDING = 1,
    CMD_STOP_RECORDING = 2,
    CMD_REPLAY = 3,              // Phát lại với tham số đầy đủ
    CMD_EXIT = 0
} ServerCommand;

// Cấu trúc để gửi tham số replay
typedef struct {
    char mouse_log_file[MAX_PATH_LEN];
    char keyboard_log_file[MAX_PATH_LEN];
    int mode;  // 0: chỉ chuột, 1: chỉ bàn phím, 2: cả hai
} ReplayParams;

// Khởi động server
int SERVER_start();

// Xử lý client
void SERVER_handle_client(SOCKET clientSocket);

// Dừng server
void SERVER_stop();

#endif // SERVER_H
