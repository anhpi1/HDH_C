
#ifndef REPLAY_ENGINE_H
#define REPLAY_ENGINE_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "setting.h"

typedef struct {
    int  screenWidth;
    int  screenHeight;
    uint32_t last_event_time; // Thời điểm của sự kiện vừa thực thi xong
    FILE* mouse_log_file;
    FILE* keyboard_log_file;
    SYSTEMTIME st;
} ReplayContext;

int decode_replay_events(const uint32_t *MsgID, DWORD *dwFlags);
DWORD WINAPI replay_mouse_events(LPVOID lpParam);
DWORD WINAPI replay_keyboard_events(LPVOID lpParam);
int replay_events(const char* mouse_log_file, const char* keyboard_log_file, int mode);

#endif // REPLAY_ENGINE_H
