
#ifndef REPLAY_ENGINE_H
#define REPLAY_ENGINE_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "setting.h"
#include "ring_buffer.h"


#define REPLAY_START_MOUSE "resources\\replaystart_mouse.csv"
#define REPLAY_START_KEY "resources\\replaystart_.keyboard.csv"

typedef struct {
    int  screenWidth;
    int  screenHeight;
    uint32_t last_event_time; // Thời điểm của sự kiện vừa thực thi xong
    FILE* mouse_log_file;
    FILE* keyboard_log_file;
    SYSTEMTIME st;
    HOOK_ring_buffer ringData;
    int mode; 
} ReplayContext;

typedef struct {
    DWORD time;
    INPUT input;
} ReplayEvent;

int HOOK_decode_replay_events(const uint32_t *MsgID, DWORD *dwFlags);
int HOOK_replay_start(ReplayContext* replay);
DWORD WINAPI HOOK_replay_replay_ing(LPVOID lpParam);
DWORD WINAPI HOOK_replay_load_file(LPVOID lpParam);
int HOOK_replay_event_INIT(ReplayContext* replay,const char* mouse_log_file, const char* keyboard_log_file, int mode);

#endif // REPLAY_ENGINE_H
