# Hướng dẫn compile và chạy Server-Client

## Compile

### Compile Server (Hai_Anh_main.c)
```bash
gcc Hai_Anh_main.c lib/server.c lib/hook_handler.c lib/logger.c lib/replay_engine.c -o server.exe -lws2_32 -lgdi32 -luser32
```

### Compile Client
```bash
gcc client.c -o client.exe -lws2_32
```

## Chạy chương trình

### Bước 1: Khởi động Server
Mở terminal thứ nhất và chạy:
```bash
.\server.exe
```

Server sẽ chạy trên port 8888 và đợi client kết nối.

### Bước 2: Chạy Client
Mở terminal thứ hai và chạy:
```bash
.\client.exe
```

## Menu Client

```
========================================
       MENU CLIENT - TEST SERVER
========================================
1. Bat dau ghi su kien
2. Dung ghi su kien
3. Phat lai voi tham so tu nhap
4. Phat lai (mac dinh log/mouse_log0.csv, log/keyboard_log0.csv, mode=2)
0. Thoat
========================================
```

## Chức năng các lệnh

- **Lệnh 1**: Bắt đầu ghi các sự kiện chuột và bàn phím
- **Lệnh 2**: Dừng ghi sự kiện
- **Lệnh 3**: Phát lại với tham số TỰ NHẬP (nhập đường dẫn file và mode)
- **Lệnh 4**: Phát lại với tham số MẶC ĐỊNH
- **Lệnh 0**: Thoát và ngắt kết nối

### Chi tiết lệnh 3 (Phát lại với tham số tự nhập)

Khi chọn lệnh 3, client sẽ yêu cầu nhập:
1. **Đường dẫn file mouse log**: VD: `log/mouse_log0.csv`
2. **Đường dẫn file keyboard log**: VD: `log/keyboard_log0.csv`
3. **Mode**:
   - `0` = Chỉ phát lại sự kiện chuột
   - `1` = Chỉ phát lại sự kiện bàn phím
   - `2` = Phát lại cả hai loại sự kiện

Client sẽ gửi đầy đủ 3 tham số này đến server, server sẽ gọi:
```c
HOOK_replay_events(mouse_log_file, keyboard_log_file, mode);
```

## Lưu ý

- Server phải chạy trước khi client kết nối
- Server được khởi động từ Hai_Anh_main.c (nằm trong thư mục lib/)
- Client gửi đầy đủ tham số của hàm HOOK_replay_events()
- Server hỗ trợ 1 client tại một thời điểm
- Nhấn Enter trong terminal server để dừng server

## Kiến trúc

```
Hai_Anh_main.c (khởi động server)
    ↓
lib/server.c (xử lý kết nối và lệnh)
    ↓
lib/hook_handler.c (ghi/phát lại sự kiện)
lib/replay_engine.c (phát lại với tham số)
```
