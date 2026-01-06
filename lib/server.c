#include "server.h"


DWORD WINAPI ThreadFunc1(LPVOID lpParam) { 
    HOOK_start_recording();
    return 0; 
} 
DWORD WINAPI ThreadFunc2(LPVOID lpParam) { 
    HOOK_stop_recording(); 
    return 0; 
} 
DWORD WINAPI ThreadFunc3(LPVOID lpParam) { 
    ServerHandle *Server = (ServerHandle *)lpParam;
    HOOK_replay_events(Server->mouse_file, Server->key_file, Server->mode); 
    return 0; 
} 
DWORD WINAPI ThreadFunc4(LPVOID lpParam) { 
    ServerHandle *Server = (ServerHandle *)lpParam;
    Sap_xep();
    return 0; 
} 
DWORD WINAPI ThreadFunc5(LPVOID lpParam) { 
    ServerHandle *Server = (ServerHandle *)lpParam;
    Tach_va_dich();
    return 0; 
} 
DWORD WINAPI ThreadFunc6(LPVOID lpParam) { 
    ServerHandle *Server = (ServerHandle *)lpParam;
    Phan_tich ();
    return 0; 
} 


int HOOK_Server_thread_open(ServerHandle *Server){
    WaitForSingleObject(Server->hMutexNumThreads, INFINITE);
    if(Server->num_threads >= THREAD_max_sevice){
        ReleaseMutex(Server->hMutexNumThreads);
        return 1;
    }
    Server->num_threads++;
    ReleaseMutex(Server->hMutexNumThreads);
    return 0;
}

int HOOK_Server_thread_close(ServerHandle *Server){
    WaitForSingleObject(Server->hMutexNumThreads, INFINITE);
    Server->num_threads--;
    ReleaseMutex(Server->hMutexNumThreads);
    return 0;
}

int HOOK_Server_init(ServerHandle *Server){
    Server->num_threads = 0;
    Server->hMutexNumThreads = CreateMutex(NULL, FALSE, NULL);
    if (Server->hMutexNumThreads == NULL) {
        printf("CreateMutex failed\n");
        return 1;
    }
    Server->hPipe = CreateNamedPipeA(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,                 
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,                           
        1,                                   
        1024,                                
        1024,                                
        0,
        NULL
    );
    if (Server->hPipe == INVALID_HANDLE_VALUE) {
        printf("CreateNamedPipe failed\n");
        return 1;
    }

    printf("Waiting for client...\n");
    BOOL connected = ConnectNamedPipe(Server->hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        CloseHandle(Server->hPipe);
        return 1;
    }
    printf("Client connected!\n");
    return 0;
}

int HOOK_Server_start(ServerHandle *Server){
    // Đọc lệnh từ client và xử lý
    char buffer[128];
    DWORD bytesRead;
    char cmd[32];

    //phản hồi tới client
    char response[256];
    DWORD bytesWritten;

    // mã báo lỗi
    int error;

    while (1) {
        BOOL ok = ReadFile(
            Server->hPipe,
            buffer,
            sizeof(buffer) - 1,
            &bytesRead,
            NULL
        );

        if (!ok || bytesRead == 0) break;
        buffer[bytesRead] = '\0';
        printf("Received: %s\n", buffer);
        sscanf(buffer, "%31s %255s %255s %d", cmd, Server->mouse_file, Server->key_file, &Server->mode);

        if (strcmp(cmd, "START") == 0) {
            error = HOOK_Server_thread_open(Server);
            if(error) strcpy(response, "FULL");
            else strcpy(response, "OK START");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
            printf("err: %d\n", error);
            if(error) continue;
            Server->hThreads[Server->num_threads-1] = CreateThread(NULL, 0, ThreadFunc1, NULL, 0, NULL); 
            HOOK_Server_thread_close(Server);

        }
        else if (strcmp(cmd, "STOP") == 0) {
            error = HOOK_Server_thread_open(Server);
            if(error) strcpy(response, "FULL");
            else strcpy(response, "OK STOP");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
            
            if(error) continue;
            Server->hThreads[Server->num_threads-1] = CreateThread(NULL, 0, ThreadFunc2, NULL, 0, NULL); 
            HOOK_Server_thread_close(Server);
        }
        else if (strcmp(cmd, "REPLAY") == 0) {
            error = HOOK_Server_thread_open(Server);
            if(error) strcpy(response, "FULL");
            else strcpy(response, "OK REPLAY");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
            
            if(error) continue;
            Server->hThreads[Server->num_threads-1] = CreateThread(NULL, 0, ThreadFunc3, (LPVOID) Server, 0, NULL);
            HOOK_Server_thread_close(Server);
        }else if (strcmp(cmd, "Sap_xep") == 0) {
            error = HOOK_Server_thread_open(Server);
            if(error) strcpy(response, "FULL");
            else strcpy(response, "OK Sap_xep");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
            
            if(error) continue;
            Server->hThreads[Server->num_threads-1] = CreateThread(NULL, 0, ThreadFunc4, NULL, 0, NULL);
            HOOK_Server_thread_close(Server);
        }else if (strcmp(cmd, "Tach_va_dich") == 0) {
            error = HOOK_Server_thread_open(Server);
            if(error) strcpy(response, "FULL");
            else strcpy(response, "OK Tach_va_dich");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
            if(error) continue;
            Server->hThreads[Server->num_threads-1] = CreateThread(NULL, 0, ThreadFunc5, NULL, 0, NULL);
            HOOK_Server_thread_close(Server);
        }else if (strcmp(cmd, "Phan_tich") == 0) {
            error = HOOK_Server_thread_open(Server);
            if(error) strcpy(response, "FULL");
            else strcpy(response, "OK Phan_tich");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
            if(error) continue;
            Server->hThreads[Server->num_threads-1] = CreateThread(NULL, 0, ThreadFunc6, NULL, 0, NULL);
            HOOK_Server_thread_close(Server);
        }else{
            printf("Unknown command: %s\n", cmd);
            strcpy(response, "UNKNOWN COMMAND");
            WriteFile(Server->hPipe, response, strlen(response), &bytesWritten, NULL);
        }
    }
    return 0;
}