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
#define WRITE_BUFFER_SIZE    128           // Kích thước buffer ghi file log


/**
 * @brief  Loại sự kiện người dùng.
 */
typedef enum {
	EVT_KEYBOARD = 1,
	EVT_MOUSE    = 2
} MAIN_EventType;

/**
 * @brief  Dữ liệu thô của sự kiện bàn phím.
 *
 * @param vkCode   Mã phím ảo (Virtual-Key Code).
 * @param scanCode Mã scan phần cứng.
 * @param flags    Cờ sự kiện: KeyUp, KeyDown, Extended Key.
 */
typedef struct {
	DWORD vkCode;       // Virtual Key Code
	DWORD scanCode;     // Hardware Scan Code
	DWORD flags;        // KeyUp, KeyDown, Extended Key
} MAIN_RawKeyData;

/**
 * @brief  Dữ liệu thô của sự kiện chuột.
 *
 * @param x         Tọa độ X trên màn hình.
 * @param y         Tọa độ Y trên màn hình.
 * @param mouseData Scroll delta hoặc giá trị XButton.
 * @param flags     Cờ sự kiện: Click, Move, Absolute...
 */
typedef struct {
	LONG  x;            // Tọa độ X
	LONG  y;            // Tọa độ Y
	DWORD mouseData;    // Scroll delta hoặc XButton
	DWORD flags;        // Cờ sự kiện (Click, Absolute...)
} MAIN_RawMouseData;

/**
 * @brief  Sự kiện người dùng được chuẩn hóa dùng cho hệ thống logging.
 *
 * @param type       Loại sự kiện (MAIN_EventType).
 * @param timestamp  Thời điểm xảy ra (GetTickCount64 hoặc QPC).
 * @param msgId      Windows Message ID (WM_KEYDOWN, WM_MOUSEMOVE...).
 * @param data       Union chứa dữ liệu chuột hoặc bàn phím.
 */
typedef struct {
	uint8_t   type;        // 1 byte: Loại sự kiện (EventType)
	uint64_t  timestamp;   // 8 bytes: Thời gian (dùng GetTickCount64 hoặc QueryPerformanceCounter)
	uint32_t  msgId;       // 4 bytes: Windows Message ID (WM_KEYDOWN, WM_MOUSEMOVE...)
	union {
		MAIN_RawKeyData   key;
		MAIN_RawMouseData mouse;
	} data;                // Union giúp tiết kiệm bộ nhớ
} MAIN_UserEvent;

/**
 * @brief  Header mô tả metadata của file log.
 *
 * @param signature     Chuỗi nhận diện ("LOGX").
 * @param version       Phiên bản cấu trúc dữ liệu.
 * @param startTime     Thời điểm bắt đầu ghi log.
 * @param screenWidth   Độ phân giải ngang.
 * @param screenHeight  Độ phân giải dọc.
 * @param reserved      Dự phòng cho mở rộng.
 */
typedef struct {
	char      signature[4]; // "LOGX"
	uint16_t  version;      // Phiên bản cấu trúc dữ liệu
	uint64_t  startTime;    // Thời gian bắt đầu ghi
	uint32_t  screenWidth;  // Độ phân giải màn hình
	uint32_t  screenHeight; // Độ phân giải màn hình
	char      reserved[12]; // Dự phòng
} MAIN_LogFileHeader;

/**
 * @brief  Cấu trúc quản lý toàn bộ trạng thái ứng dụng ghi log.
 *
 * @param hKeyboardHook  Handle hook bàn phím.
 * @param hMouseHook     Handle hook chuột.
 * @param hInstance      Instance của module hiện tại.
 *
 * @param logFile        Con trỏ đến file log đang mở.
 * @param filePath       Đường dẫn file log.
 * @param isRecording    Cờ đang ghi hay không.
 *
 * @param buffer         Bộ đệm tạm sự kiện (malloc cấp phát).
 * @param bufferCount    Số lượng phần tử trong buffer.
 *
 * @param csLock         Critical section để đồng bộ hóa.
 */
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
