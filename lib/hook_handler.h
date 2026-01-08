
#ifndef HOOK_HANDLER_H
#define HOOK_HANDLER_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <direct.h>
#include "logger_real_time.h"
#include "stack.h"
#include "ring_buffer.h"
#include "setting.h"

#define BUFFER_SIZE_EVENT 10000
#define TIME_BOTH 1000

typedef struct{
    uint32_t index;
    uint32_t MsgID;
    DWORD time;
    DWORD mouseData;
    POINT pt;
}HOOK_MouseEvent;


typedef struct{
    uint32_t index;
    uint32_t MsgID;
    DWORD time;
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
}HOOK_KeyboardEvent;

typedef struct{
    volatile LONG pReadMouse;
    volatile LONG pWriteMouse;
    volatile LONG pWriteKeyboard;
    volatile LONG pReadKeyboard;
    HANDLE hSemaphoreMouse;
    HANDLE hSemaphoreKeyboard;
    HOOK_MouseEvent bufferMouse[BUFFER_SIZE_EVENT];
    HOOK_KeyboardEvent bufferKeyboard[BUFFER_SIZE_EVENT];

    //keybroad and mouse both processing
    volatile LONG pReadMouseBoth;
    volatile LONG pWriteMouseBoth;
    volatile LONG pWriteKeyboardBoth;
    volatile LONG pReadKeyboardBoth;
    HANDLE hSemaphoreMouseBoth;
    HANDLE hSemaphoreKeyboardBoth;
    HOOK_MouseEvent bufferMouseBoth[BUFFER_SIZE_EVENT];
    HOOK_KeyboardEvent bufferKeyboardBoth[BUFFER_SIZE_EVENT];

}HOOK_ring_buffer_event;

uint8_t HOOK_FUNC_RingData_INIT_event(HOOK_ring_buffer_event* ring);

uint8_t HOOK_FUNC_Read_RingData_eventMouse(HOOK_ring_buffer_event* ring, HOOK_MouseEvent* dataOut);
uint8_t HOOK_FUNC_Write_RingData_eventMouse(HOOK_ring_buffer_event* ring, HOOK_MouseEvent data);
uint8_t HOOK_FUNC_Read_RingData_eventKey(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent* dataOut);
uint8_t HOOK_FUNC_Write_RingData_eventKey(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent data);

uint8_t HOOK_FUNC_Read_RingData_eventMouseBoth(HOOK_ring_buffer_event* ring, HOOK_MouseEvent* dataOut);
uint8_t HOOK_FUNC_Write_RingData_eventMouseBoth(HOOK_ring_buffer_event* ring, HOOK_MouseEvent data);
uint8_t HOOK_FUNC_Read_RingData_eventKeyBoth(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent* dataOut);
uint8_t HOOK_FUNC_Write_RingData_eventKeyBoth(HOOK_ring_buffer_event* ring, HOOK_KeyboardEvent data);


void HOOK_InitLogFile();
LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HOOK_LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
int HOOK_start_recording(int islogrealtime);
void HOOK_stop_recording(void);
DWORD WINAPI HOOK_writeMouseLogThread(LPVOID param);
DWORD WINAPI HOOK_writeKeyLogThread(LPVOID param);

#endif // HOOK_HANDLER_H
