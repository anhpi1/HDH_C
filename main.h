#ifndef MAIN_H
#define MAIN_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>

// Đường dẫn tới các thư viện trong thư mục lib
#include "lib/hook_handler.h"
#include "lib/logger.h"
#include "lib/replay_engine.h"
#include "lib/setting.h"
#include "lib/server.h"

// các cài đặt chính
#define DEUG_MODE             1               // Bật chế độ debug (1: Bật, 0: Tắt)


#endif // MAIN_H
