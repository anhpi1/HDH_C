#include "logger.h"

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
    HOOK_Header header;
    FILE *Mouse;
    FILE *Keyboard;
    HOOK_MouseEvent lastMouseEventLeft;
    HOOK_MouseEvent lastMouseEventRight;
    HOOK_KeyboardEvent lastKeyEvent[2];
}HOOK_Logger;

