
#ifndef HOOK_HANDLER_H
#define HOOK_HANDLER_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <direct.h>
#define WRITE_BUFFER_SIZE    100000           // Kích thước buffer ghi file log

typedef struct {
  LONG  x;            // Tọa độ X
  LONG  y;            // Tọa độ Y
  DWORD mouseData;   // Scroll delta hoặc giá trị XButton
  DWORD time;
  uint32_t MsgId;
} HOOK_rawMouseData;

typedef struct {
  uint32_t pRead;
  uint32_t pWrite;
  HOOK_rawMouseData HOOK_buffermouse[WRITE_BUFFER_SIZE];
} HOOK_ringMouseData;

extern HOOK_ringMouseData RingMouseData;
extern CRITICAL_SECTION cs;

uint8_t HOOK_FUNC_addRead_RingMouseData(HOOK_ringMouseData* ring, uint32_t *address);
uint8_t HOOK_FUNC_addWrite_RingMouseData(HOOK_ringMouseData* ring, uint32_t *address);
LRESULT CALLBACK HOOK_LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI HOOK_writeMouseLogThread(LPVOID lpParameter);


// Thư viện hook_handler - Khai báo các hàm xử lý hook (chưa có hàm nào)

// TODO: Thêm prototype các hàm xử lý hook tại đây

#endif // HOOK_HANDLER_H
