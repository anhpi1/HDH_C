#include "logger.h"
#include "ring_buffer.h"
typedef struct{
    SYSTEMTIME st;
    int screenWidth;
    int screenHeight;
}HOOK_Header;

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
    HOOK_Header header;
    Node stackMouse;
    Node stackKey; 
}HOOK_Logger;


