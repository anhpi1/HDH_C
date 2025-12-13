#include "hook_handler.h"

uint8_t HOOK_FUNC_addRead_RingMouseData(HOOK_ringMouseData* ring, uint32_t *address) {
  if(ring->pRead == ring->pWrite) return 1; // empty
    *address = ring->pRead;
    ring->pRead = (ring->pRead + 1) % WRITE_BUFFER_SIZE;
    return 0;
}

uint8_t HOOK_FUNC_addWrite_RingMouseData(HOOK_ringMouseData* ring, uint32_t *address) {
  uint32_t next = (ring->pWrite + 1) % WRITE_BUFFER_SIZE;
  if(next == ring->pRead) return 1; // full
  *address = ring->pWrite;
  ring->pWrite = next;
  return 0;
}



LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);
    EnterCriticalSection(&cs);
    uint32_t index=0;
    uint8_t result = HOOK_FUNC_addWrite_RingMouseData(&RingMouseData, &index);
    if(!result){
      // Successfully added write index
      RingMouseData.HOOK_buffermouse[index].x = ((MSLLHOOKSTRUCT*)lParam)->pt.x;
      RingMouseData.HOOK_buffermouse[index].y = ((MSLLHOOKSTRUCT*)lParam)->pt.y;
      RingMouseData.HOOK_buffermouse[index].mouseData = ((MSLLHOOKSTRUCT*)lParam)->mouseData;
      RingMouseData.HOOK_buffermouse[index].time = ((MSLLHOOKSTRUCT*)lParam)->time;
      RingMouseData.HOOK_buffermouse[index].MsgId = (uint32_t)wParam;
    }
    LeaveCriticalSection(&cs);

    return CallNextHookEx(NULL, nCode, wParam, lParam);
};

DWORD WINAPI HOOK_writeMouseLogThread(LPVOID lpParameter){
  static uint32_t fileIndex = 0;
  uint16_t count = 10000;
  while(1){

    _mkdir("log");
    char filename[64];
    sprintf(filename, "log/mouse_log%u.csv", fileIndex++);
    FILE* logFile = fopen(filename, "w");
    if (!logFile) {
        printf("Loi: Khong the mo file %s. Kiem tra thu muc 'log'.\n", filename);
        return 2;
    }

    int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    SYSTEMTIME st;
    GetLocalTime(&st);
    fprintf(logFile,
        "version,1,startTime,%04d-%02d-%02d %02d:%02d:%02d.%03d,screenWidth,%d,screenHeight,%d\n",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        screenWidth,
        screenHeight
    );
    fprintf(logFile, "Event,MsgID,Time,X,Y,MouseData\n");
    uint16_t i = 0;
    uint32_t index = 0;
    uint8_t result = 1;
    while(i<count){
      EnterCriticalSection(&cs);
      result = HOOK_FUNC_addRead_RingMouseData(&RingMouseData, &index);
      LeaveCriticalSection(&cs);
      if(!result){
        // successfully read data
        fprintf(logFile,
              "%x,%x,%x,%x,%x,%x\n",
              i,
              RingMouseData.HOOK_buffermouse[index].MsgId,
              RingMouseData.HOOK_buffermouse[index].time,
              RingMouseData.HOOK_buffermouse[index].x,
              RingMouseData.HOOK_buffermouse[index].y,
              RingMouseData.HOOK_buffermouse[index].mouseData
              );
        i++;
      }else{
        // buffer empty
        Sleep(1);
      }
    }
    fclose(logFile);
  }

  return 0;
};
