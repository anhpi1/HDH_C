
#ifndef HOOK_HANDLER_H
#define HOOK_HANDLER_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <direct.h>
#include "setting.h"



void HOOK_InitLogFile();
LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
int HOOK_start_recording(void);
void HOOK_stop_recording(void);


#endif // HOOK_HANDLER_H
