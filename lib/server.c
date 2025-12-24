#include "server.h"
#include "../main.h"

static SOCKET serverSocket = INVALID_SOCKET;
static BOOL isServerRunning = FALSE;

// Thread để ghi sự kiện
static HANDLE recordingThread = NULL;

DWORD WINAPI RecordingThreadFunc(LPVOID lpParam) {
    HOOK_start_recording();
    return 0;
}

DWORD WINAPI StopRecordingThreadFunc(LPVOID lpParam) {
    HOOK_stop_recording();
    return 0;
}

DWORD WINAPI ReplayThreadFunc(LPVOID lpParam) {
    ReplayParams* params = (ReplayParams*)lpParam;
    printf("[SERVER] Phat lai: mouse='%s', keyboard='%s', mode=%d\n", 
           params->mouse_log_file, params->keyboard_log_file, params->mode);
    HOOK_replay_events(params->mouse_log_file, params->keyboard_log_file, params->mode);
    free(lpParam);
    return 0;
}

int SERVER_start() {
    WSADATA wsaData;
    struct sockaddr_in serverAddr;
    
    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[SERVER] Loi khoi tao Winsock. Error: %d\n", WSAGetLastError());
        return -1;
    }
    
    // Tạo socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("[SERVER] Loi tao socket. Error: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    
    // Cấu hình địa chỉ server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);
    
    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[SERVER] Loi bind socket. Error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    
    // Lắng nghe kết nối
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("[SERVER] Loi listen. Error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }
    
    isServerRunning = TRUE;
    printf("[SERVER] Server dang chay tren cong %d...\n", SERVER_PORT);
    
    // Chấp nhận kết nối từ client
    while (isServerRunning) {
        SOCKET clientSocket;
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            if (isServerRunning) {
                printf("[SERVER] Loi accept. Error: %d\n", WSAGetLastError());
            }
            continue;
        }
        
        printf("[SERVER] Client ket noi tu %s:%d\n", 
               inet_ntoa(clientAddr.sin_addr), 
               ntohs(clientAddr.sin_port));
        
        // Xử lý client
        SERVER_handle_client(clientSocket);
        
        closesocket(clientSocket);
        printf("[SERVER] Client ngat ket noi.\n");
    }
    
    return 0;
}

void SERVER_handle_client(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int recvSize;
    
    while ((recvSize = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[recvSize] = '\0';
        
        // Parse lệnh
        int command = atoi(buffer);
        char response[BUFFER_SIZE];
        
        printf("[SERVER] Nhan lenh: %d\n", command);
        
        switch (command) {
            case CMD_START_RECORDING:
                recordingThread = CreateThread(NULL, 0, RecordingThreadFunc, NULL, 0, NULL);
                if (recordingThread) {
                    sprintf(response, "OK: Bat dau ghi su kien");
                    printf("[SERVER] Bat dau ghi su kien\n");
                } else {
                    sprintf(response, "ERROR: Khong the bat dau ghi");
                }
                break;
                
            case CMD_STOP_RECORDING:
                CreateThread(NULL, 0, StopRecordingThreadFunc, NULL, 0, NULL);
                sprintf(response, "OK: Dung ghi su kien");
                printf("[SERVER] Dung ghi su kien\n");
                break;
                
            case CMD_REPLAY: {
                // Gửi xác nhận và đợi nhận tham số
                sprintf(response, "READY: Gui tham so replay");
                send(clientSocket, response, strlen(response), 0);
                
                // Nhận cấu trúc ReplayParams
                ReplayParams* params = (ReplayParams*)malloc(sizeof(ReplayParams));
                recvSize = recv(clientSocket, (char*)params, sizeof(ReplayParams), 0);
                
                if (recvSize == sizeof(ReplayParams)) {
                    CreateThread(NULL, 0, ReplayThreadFunc, params, 0, NULL);
                    sprintf(response, "OK: Phat lai voi tham so da nhan");
                } else {
                    free(params);
                    sprintf(response, "ERROR: Nhan tham so khong thanh cong");
                    printf("[SERVER] Loi nhan tham so replay\n");
                }
                break;
            }
                
            case CMD_EXIT:
                sprintf(response, "OK: Thoat");
                printf("[SERVER] Nhan lenh thoat\n");
                send(clientSocket, response, strlen(response), 0);
                return;
                
            default:
                sprintf(response, "ERROR: Lenh khong hop le");
                printf("[SERVER] Lenh khong hop le: %d\n", command);
                break;
        }
        
        // Gửi phản hồi cho client
        send(clientSocket, response, strlen(response), 0);
    }
}

void SERVER_stop() {
    isServerRunning = FALSE;
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    WSACleanup();
    printf("[SERVER] Server da dung.\n");
}
