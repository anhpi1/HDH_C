#ifndef LOGGER_REAL_TIME_H
#define LOGGER_REAL_TIME_H

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>
#include "stack.h"
#include <stdio.h>
#include "setting.h"
#include "ring_buffer.h"


#define TIME_CLICK 400
#define TIME_KEY_PRESS 400

// Event types
#define EVENT_MOUSE_L_CLICK "MOUSE_L_CLICK"
#define EVENT_MOUSE_L_HOLD "MOUSE_L_HOLD"
#define EVENT_MOUSE_L_DRAG_DROP "MOUSE_L_DRAG_DROP"
#define EVENT_MOUSE_L_DOUBLE_CLICK "MOUSE_L_DOUBLE_CLICK"
#define EVENT_MOUSE_R_CLICK "MOUSE_R_CLICK"
#define EVENT_MOUSE_R_HOLD "MOUSE_R_HOLD"
#define EVENT_MOUSE_R_DRAG_DROP "MOUSE_R_DRAG_DROP"
#define EVENT_MOUSE_R_DOUBLE_CLICK "MOUSE_R_DOUBLE_CLICK"
#define EVENT_MOUSE_MOVE "MOUSE_MOVE"
#define EVENT_MOUSE_WHEEL "MOUSE_WHEEL"
#define EVENT_ERROR "ERROR"
#define EVENT_KEY_COMBO_3 "KEY_COMBO_3"
#define EVENT_KEY_COMBO_2 "KEY_COMBO_2"
#define EVENT_KEY_PRESS "KEY_PRESS"
#define EVENT_KEY_HOLD "KEY_HOLD"
#define EVENT_KEY_SEQUENCE "KEY_SEQUENCE"

// Thư viện logger - Khai báo các hàm ghi log (chưa có hàm nào)
typedef struct{
    uint32_t index;
    uint32_t MsgID;
    DWORD time;
    DWORD mouseData;
    POINT pt;
}HOOK_log_MouseEvent;

typedef struct{
    uint32_t index;
    uint32_t MsgID;
    DWORD time;
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
}HOOK_log_KeyboardEvent;

typedef struct{
    bool is_mouse;
    union{
        HOOK_log_MouseEvent mouseEvent;
        HOOK_log_KeyboardEvent keyEvent;
    }data;
}HooK_log_BothEvent;

typedef struct{
    Node *stackMouseLeft;
    Node *stackMouseRight;
    Node *stackMouseMove;
    Node *stackKey;
    Node *stackWheel;
    Node *stackKeySequence;
    bool is_event_right;
    bool is_event_left;
}HOOK_Logger;

int HOOK_log_INIT(HOOK_Logger* logger);

// TODO: Thêm prototype các hàm ghi log tại đây
int HOOK_log_new_mouse_event(HOOK_Logger *logger, HOOK_log_MouseEvent *mouseEvent, int mode);
int HOOK_log_new_key_event(HOOK_Logger *logger, HOOK_log_KeyboardEvent *keyEvent,int modes);
int HOOK_log_new_both_mouse_and_key_event(HOOK_Logger *logger, HOOK_log_KeyboardEvent *keyEvent,HOOK_log_MouseEvent *mouseEvent,int mode);
int HOOK_log_filter_processing_mouse(HOOK_Logger *log, HOOK_log_MouseEvent *mouseEvent);
int HOOK_log_filter_processing_key(HOOK_Logger *log, HOOK_log_KeyboardEvent *keyEvent);
int HOOK_log_filter_processing_both_mouse_and_key(HOOK_Logger *log,HOOK_log_KeyboardEvent *keyEvent, HOOK_log_MouseEvent *mouseEvent, int is_mouse);
bool HOOK_log_is_continue_keySequence(HOOK_log_KeyboardEvent* keyEvent);

#endif // LOGGER_H
