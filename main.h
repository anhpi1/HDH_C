#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>

// Đường dẫn tới các thư viện trong thư mục lib
#include "lib/hook_handler.h"
#include "lib/logger.h"
#include "lib/replay_engine.h"
#include "lib/write_to_SQL.h"

// các cài đặt chính
#define DEUG_MODE             1               // Bật chế độ debug (1: Bật, 0: Tắt)
#define WRITE_BUFFER_SIZE    1024            // Kích thước buffer ghi file log

// Định nghĩa kiểu sự kiện
typedef enum {
	EVT_KEYBOARD = 1,
	EVT_MOUSE    = 2
} MAIN_EventType;

// Dữ liệu thô của bàn phím
typedef struct {
	DWORD vkCode;       // Virtual Key Code
	DWORD scanCode;     // Hardware Scan Code
	DWORD flags;        // KeyUp, KeyDown, Extended Key
} MAIN_RawKeyData;

// Dữ liệu thô của chuột
typedef struct {
	LONG  x;            // Tọa độ X
	LONG  y;            // Tọa độ Y
	DWORD mouseData;    // Scroll delta hoặc XButton
	DWORD flags;        // Cờ sự kiện (Click, Absolute...)
} MAIN_RawMouseData;

// Dữ liệu sự kiện người dùng đã xử lý thô
typedef struct {
	uint8_t   type;        // 1 byte: Loại sự kiện (EventType)
	uint64_t  timestamp;   // 8 bytes: Thời gian (dùng GetTickCount64 hoặc QueryPerformanceCounter)
	uint32_t  msgId;       // 4 bytes: Windows Message ID (WM_KEYDOWN, WM_MOUSEMOVE...)
	union {
		MAIN_RawKeyData   key;
		MAIN_RawMouseData mouse;
	} data;                // Union giúp tiết kiệm bộ nhớ
} MAIN_UserEvent;

// Header mô tả metadata của file log
typedef struct {
	char      signature[4]; // "LOGX"
	uint16_t  version;      // Phiên bản cấu trúc dữ liệu
	uint64_t  startTime;    // Thời gian bắt đầu ghi
	uint32_t  screenWidth;  // Độ phân giải màn hình
	uint32_t  screenHeight; // Độ phân giải màn hình
	char      reserved[12]; // Dự phòng
} MAIN_LogFileHeader;

// Quản lý toàn bộ trạng thái ứng dụng
typedef struct {
	// A. Quản lý Hooks
	HHOOK       hKeyboardHook;
	HHOOK       hMouseHook;
	HINSTANCE   hInstance;

	// B. Quản lý File
	FILE*       logFile;
	char        filePath[MAX_PATH];
	int         isRecording;

	// C. Quản lý Buffer
	MAIN_UserEvent   *buffer; // sử dụng malloc để cấp phát động
	int         bufferCount;

	// D. Đồng bộ hóa
	CRITICAL_SECTION csLock;
} MAIN_AppHandle;

#endif // MAIN_H
