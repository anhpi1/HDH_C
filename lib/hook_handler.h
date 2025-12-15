
#ifndef HOOK_HANDLER_H
#define HOOK_HANDLER_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <direct.h>

#define MAX_EVENTS_PER_FILE 10000  // Số sự kiện tối đa mỗi file
#define DEBUG 0

extern FILE* g_logFile;
extern uint32_t g_fileIndex;
extern uint32_t g_eventCount;
extern CRITICAL_SECTION cs;
extern volatile BOOL g_running;


void HOOK_InitLogFile();
void HOOK_CloseLogFile();
LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);


// Thư viện hook_handler - Khai báo các hàm xử lý hook (chưa có hàm nào)

// TODO: Thêm prototype các hàm xử lý hook tại đây

#endif // HOOK_HANDLER_H
