#ifndef SERVER_H
#define SERVER_H

#include <windows.h>
#include <stdio.h>
#include "hook_handler.h"
#include "replay_engine.h"

#define THREAD_max_sevice 3 
#define PIPE_NAME "\\\\.\\pipe\\MyPipe"

typedef struct {
    HANDLE hPipe;
    HANDLE hThreads[THREAD_max_sevice]; 
    uint8_t num_threads;
    HANDLE hMutexNumThreads;
    char mouse_file[256];
    char key_file[256];
    int mode;
} ServerHandle;

DWORD WINAPI ThreadFunc1(LPVOID lpParam);
DWORD WINAPI ThreadFunc2(LPVOID lpParam);
DWORD WINAPI ThreadFunc3(LPVOID lpParam);
int HOOK_Server_thread_open(ServerHandle *Server);
int HOOK_Server_thread_close(ServerHandle *Server);
int HOOK_Server_init(ServerHandle *Server);
int HOOK_Server_start(ServerHandle *Server);

#endif // SERVER_H
