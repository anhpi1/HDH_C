#include "replay_engine.h"


typedef struct {
    SYSTEMTIME st;    // Thời gian bắt đầu ghi
    int  screenWidth;  // Độ phân giải màn hình
    int  screenHeight; // Độ phân giải màn hình
} LogFileHeader;



int replay_mouse_events(const char* mouse_log_file) {


    FILE* file = fopen(mouse_log_file, "r");
    if (!file) {
        printf("Error: Cannot open mouse log file %s\n", mouse_log_file);
        return ;
    }
    LogFileHeader header;
    
    fscanf(file, "version,1,startTime,%04d-%02d-%02d %02d:%02d:%02d.%03d,screenWidth,%d,screenHeight,%d",
        header.st.wYear, header.st.wMonth, header.st.wDay, header.st.wHour, header.st.wMinute, header.st.wSecond, header.st.wMilliseconds,
        header.screenWidth, header.screenHeight);
    char buffer[256];
    fgets(buffer, sizeof(buffer), file);

    while (!feof(file)){
        uint32_t event_index;
        uint32_t MsgID;
        MSLLHOOKSTRUCT mouse_event;
        fscanf(file, "%u,%x,%lu,%ld,%ld,%x",
                &event_index,
                &MsgID,
                &mouse_event.MsgID,
                &mouse_event.time,
                &mouse_event.pt.x,
                &mouse_event.pt.y,
                &mouse_event.mouseData);
    }
    


    fclose(file);

    return 0;
}
