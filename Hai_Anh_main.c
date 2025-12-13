#include "main.h"
#include <direct.h>


#define TIME_OUT 5000 // 5 giây
uint16_t HOOK_logIndex = 0;
volatile uint8_t HOOK_currentBuffer = 0;
volatile uint8_t HOOK_isBufferFull[2] = {0};
uint16_t HOOK_bufferCount[2] = {0};
uint32_t HOOK_bufferMsgId[WRITE_BUFFER_SIZE][2] = {0};
MSLLHOOKSTRUCT HOOK_buffermouse[WRITE_BUFFER_SIZE][2] = {0}; // Giả sử buffer có kích thước 1024 sự kiện
volatile uint8_t HOOK_is_running = 0;


LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if(nCode < 0) return CallNextHookEx(NULL, nCode, wParam, lParam);

    // nếu buffer đầy thì chuyển sang buffer dự phòng
    if(HOOK_bufferCount[HOOK_currentBuffer] >= WRITE_BUFFER_SIZE){
      HOOK_isBufferFull[HOOK_currentBuffer] = 1;
      HOOK_currentBuffer =HOOK_currentBuffer ? 0:1;
    } 
    uint8_t i = HOOK_currentBuffer;
    HOOK_bufferMsgId[HOOK_bufferCount[i]][i] = (uint32_t)wParam;
    HOOK_buffermouse[HOOK_bufferCount[i]][i] = *(MSLLHOOKSTRUCT *)lParam;
    HOOK_bufferCount[i]++;

    return CallNextHookEx(NULL, nCode, wParam, lParam);
};

DWORD WINAPI HOOK_writeMouseLogThread(LPVOID lpParameter){
  while(!HOOK_isBufferFull[0] && !HOOK_isBufferFull[1]) Sleep(10);

  int index;
  if(HOOK_isBufferFull[0]) index = 0;
  else if(HOOK_isBufferFull[1]) index = 1;
  else return 1;

  _mkdir("log");
  char filename[64];
  sprintf(filename, "log/mouse_log%d.log", HOOK_logIndex++);
  FILE* logFile = fopen(filename, "a");
  if (!logFile) {
      printf("Loi: Khong the mo file %s. Kiem tra thu muc 'log'.\n", filename);
      return 2;
  }

  int screenWidth  = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);
  SYSTEMTIME st;
  GetLocalTime(&st);
  fprintf(logFile,
      "version = 1\n"
      "startTime = %04d-%02d-%02d %02d:%02d:%02d.%03d\n"
      "screenWidth = %d\n"
      "screenHeight = %d\n"
      "----------------------------------------\n",
      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
      screenWidth,
      screenHeight
  );

  for(uint16_t i=0; i<WRITE_BUFFER_SIZE; i++){
    fprintf(logFile,
            "Event %d: MsgID=%u, Time=%lu, X=%ld, Y=%ld, MouseData=%lu, Flags=%lu, TimeStamp=%lu\n",
            i,
            HOOK_bufferMsgId[i][index],
            HOOK_buffermouse[i][index].time,
            HOOK_buffermouse[i][index].pt.x,
            HOOK_buffermouse[i][index].pt.y,
            HOOK_buffermouse[i][index].mouseData,
            HOOK_buffermouse[i][index].flags,
            HOOK_buffermouse[i][index].time
        );
  }
  fclose(logFile);
  return 0;
};


int main() {
    
    HHOOK myhook = SetWindowsHookExA(WH_MOUSE_LL, &HOOK_LowLevelMouseProc, NULL, 0); // Ví dụ gọi hàm Set
    HANDLE myThread = CreateThread(NULL, 0, &HOOK_writeMouseLogThread, NULL, 0, NULL);   

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(myhook);
    CloseHandle(myThread);
}


