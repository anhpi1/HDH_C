
#ifndef HOOK_HANDLER_H
#define HOOK_HANDLER_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <direct.h>
#include "setting.h"



void HOOK_InitLogFile();
void HOOK_CloseLogFile();
LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

extern CRITICAL_SECTION csMouse;
extern CRITICAL_SECTION csKeyboard;

// Thư viện hook_handler - Khai báo các hàm xử lý hook (chưa có hàm nào)

// TODO: Thêm prototype các hàm xử lý hook tại đây

#endif // HOOK_HANDLER_H
