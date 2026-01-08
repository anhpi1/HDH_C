#include "logger_real_time.h"
#include <direct.h>
#include <string.h>




int HOOK_log_new_mouse_event(HOOK_Logger *logger, HOOK_log_MouseEvent *mouseEvent, int mode) {
    HOOK_log_MouseEvent *newEvent = (HOOK_log_MouseEvent *)malloc(sizeof(HOOK_log_MouseEvent));
    if (newEvent == NULL) {
        printf("Loi logger: Khong the cap phat bo nho cho su kien chuot moi!\n");
        return 1;
    }
    *newEvent = *mouseEvent; 
    if(mode==1) push((void *)newEvent, &logger->stackMouseLeft);
    else if(mode==0) push((void *)newEvent, &logger->stackMouseRight);
    else if(mode==-1) push((void *)newEvent, &logger->stackMouseMove);
    else if(mode == -2) push((void *)newEvent, &logger->stackWheel);
    else {
        printf("Loi logger: Tham so mode mouse khong hop le mouse: %d\n", mode);
        return 1;
    }
    return 0;
}

int HOOK_log_new_key_event(HOOK_Logger *logger, HOOK_log_KeyboardEvent *keyEvent,int mode) {
    HOOK_log_KeyboardEvent *newEvent = (HOOK_log_KeyboardEvent *)malloc(sizeof(HOOK_log_KeyboardEvent));
    if (newEvent == NULL) {
        printf("Loi: Khong the cap phat bo nho cho su kien phim moi!\n");
        return 1;
    }
    *newEvent = *keyEvent; 
    if(mode == 0) push((void *)newEvent, &logger->stackKey);
    else if(mode == 1) push((void *)newEvent, &logger->stackKeySequence);
    else {
        printf("Loi logger: Tham so mode key khong hop le phim: %d\n", mode);
        free(newEvent);
        return 1;
    }
    return 0;
}

int HOOK_log_new_both_mouse_and_key_event(HOOK_Logger *logger, HOOK_log_KeyboardEvent *keyEvent,HOOK_log_MouseEvent *mouseEvent,int mode) {
    
}

void HOOK_log_write_real_time(const char* event_code, const char* content) {
    _mkdir("log_real_time");
    FILE* f = fopen("log_real_time/log_real_time.csv", "a");
    if (f) {
        fprintf(f, "%s,%s\n", event_code, content);
        fclose(f);
    }
}



int HOOK_log_filter_processing_mouse(HOOK_Logger *log, HOOK_log_MouseEvent *mouseEvent) {
    if(mouseEvent == NULL) return 1;
    if(log == NULL) return 1;

    HOOK_log_MouseEvent* lastLeft = NULL; // Initialize last to NULL
    HOOK_log_MouseEvent* lastRight = NULL;
    HOOK_log_MouseEvent* lastMove = NULL;
    HOOK_log_MouseEvent* lastWheel = NULL;
    char buffer[256];

    switch (mouseEvent->MsgID) {
        case WM_LBUTTONDOWN:
            #if DEBUG
                printf("Detected WM_LBUTTONDOWN\n");
            #endif
            switch (getSize(&log->stackMouseLeft))
            {
            case 0:
                HOOK_log_new_mouse_event(log, mouseEvent, 1);
                break;
            case 2:
                lastLeft = (HOOK_log_MouseEvent*)pop(&log->stackMouseLeft);
                if (lastLeft && mouseEvent->time - lastLeft->time <= TIME_CLICK &&
                        abs(mouseEvent->pt.x - lastLeft->pt.x) <= 5 &&
                        abs(mouseEvent->pt.y - lastLeft->pt.y) <= 5) {
                    HOOK_log_new_mouse_event(log, lastLeft, 1);
                    HOOK_log_new_mouse_event(log, mouseEvent, 1);
                    
                } else {
                    pop(&log->stackMouseLeft);
                    HOOK_log_new_mouse_event(log, mouseEvent, 1);
                }
                break;
            default:
                while ((pop(&log->stackMouseLeft)!= NULL)); 
                break;
            }
            
            break;

        case WM_LBUTTONUP:
            #if DEBUG
                printf("Detected WM_LBUTTONUP\n");
            #endif
            
            switch (getSize(&log->stackMouseLeft))
            {
            case 1:
                lastLeft = (HOOK_log_MouseEvent*)pop(&log->stackMouseLeft);
                if (lastLeft && mouseEvent->time - lastLeft->time <= TIME_CLICK){
                    if(abs(mouseEvent->pt.x - lastLeft->pt.x) <= 5 &&
                            abs(mouseEvent->pt.y - lastLeft->pt.y) <= 5) {
                        sprintf(buffer, "Time=%ld, X=%d, Y=%d", lastLeft->time, lastLeft->pt.x, lastLeft->pt.y);
                        HOOK_log_write_real_time(EVENT_MOUSE_L_CLICK, buffer);
                        HOOK_log_new_mouse_event(log, lastLeft, 1);
                        HOOK_log_new_mouse_event(log, mouseEvent, 1);
                    } 
                }else{
                    if(abs(mouseEvent->pt.x - lastLeft->pt.x) <= 5 &&
                            abs(mouseEvent->pt.y - lastLeft->pt.y) <= 5){
                        sprintf(buffer, "X=%d, Y=%d", mouseEvent->pt.x, mouseEvent->pt.y);
                        HOOK_log_write_real_time(EVENT_MOUSE_L_HOLD, buffer);
                    } else {
                        sprintf(buffer, "from X=%d, Y=%d to X=%d, Y=%d", lastLeft->pt.x, lastLeft->pt.y, mouseEvent->pt.x, mouseEvent->pt.y);
                        HOOK_log_write_real_time(EVENT_MOUSE_L_DRAG_DROP, buffer);
                    }
                }
                    
                break;
            case 3:
                lastLeft = (HOOK_log_MouseEvent*)pop(&log->stackMouseLeft);
                if (lastLeft && mouseEvent->time - lastLeft->time <= TIME_CLICK &&
                        abs(mouseEvent->pt.x - lastLeft->pt.x) <= 5 &&
                        abs(mouseEvent->pt.y - lastLeft->pt.y) <= 5) {
                    sprintf(buffer, "Time=%ld, X=%d, Y=%d", lastLeft->time, lastLeft->pt.x, lastLeft->pt.y);
                    HOOK_log_write_real_time(EVENT_MOUSE_L_DOUBLE_CLICK, buffer);
                    HOOK_log_new_mouse_event(log, lastLeft, 1);
                    HOOK_log_new_mouse_event(log, mouseEvent, 1);
                } else{
                    pop(&log->stackMouseLeft);
                    pop(&log->stackMouseLeft);
                }
            default:
                while ((pop(&log->stackMouseLeft)!= NULL)); 
                break;
            }
            break;
        case WM_RBUTTONDOWN:
            #if DEBUG
                printf("Detected WM_RBUTTONDOWN\n");
            #endif
            switch (getSize(&log->stackMouseRight))
            {
            case 0:
                HOOK_log_new_mouse_event(log, mouseEvent, 0);
                break;
            case 2:
                lastRight = (HOOK_log_MouseEvent*)pop(&log->stackMouseRight);
                if (lastRight && mouseEvent->time - lastRight->time <= TIME_CLICK &&
                        abs(mouseEvent->pt.x - lastRight->pt.x) <= 5 &&
                        abs(mouseEvent->pt.y - lastRight->pt.y) <= 5) {
                    HOOK_log_new_mouse_event(log, lastRight, 0);
                    HOOK_log_new_mouse_event(log, mouseEvent, 0);
                    
                } else {
                    pop(&log->stackMouseRight);
                    HOOK_log_new_mouse_event(log, mouseEvent, 0);
                }
                break;
            default:
                while ((pop(&log->stackMouseRight)!= NULL)); 
                break;
            }
            
            break;

        case WM_RBUTTONUP:
            #if DEBUG
                printf("Detected WM_RBUTTONUP\n");
            #endif
            
            switch (getSize(&log->stackMouseRight))
            {
            case 1:
                lastRight = (HOOK_log_MouseEvent*)pop(&log->stackMouseRight);
                if (lastRight && mouseEvent->time - lastRight->time <= TIME_CLICK){
                    if(abs(mouseEvent->pt.x - lastRight->pt.x) <= 5 && abs(mouseEvent->pt.y - lastRight->pt.y) <= 5) {
                        sprintf(buffer, "Time=%ld, X=%d, Y=%d", lastRight->time, lastRight->pt.x, lastRight->pt.y);
                        HOOK_log_write_real_time(EVENT_MOUSE_R_CLICK, buffer);
                        HOOK_log_new_mouse_event(log, lastRight, 0);
                        HOOK_log_new_mouse_event(log, mouseEvent, 0);
                    } 
                }else{
                    if(abs(mouseEvent->pt.x - lastRight->pt.x) <= 5 && abs(mouseEvent->pt.y - lastRight->pt.y) <= 5){
                        sprintf(buffer, "X=%d, Y=%d", mouseEvent->pt.x, mouseEvent->pt.y);
                        HOOK_log_write_real_time(EVENT_MOUSE_R_HOLD, buffer);
                    } else {
                        sprintf(buffer, "from X=%d, Y=%d to X=%d, Y=%d", lastRight->pt.x, lastRight->pt.y, mouseEvent->pt.x, mouseEvent->pt.y);
                        HOOK_log_write_real_time(EVENT_MOUSE_R_DRAG_DROP, buffer);
                    }
                }
                    
                break;
            case 3:
                lastRight = (HOOK_log_MouseEvent*)pop(&log->stackMouseRight);
                if (lastRight && mouseEvent->time - lastRight->time <= TIME_CLICK && abs(mouseEvent->pt.x - lastRight->pt.x) <= 5 && abs(mouseEvent->pt.y - lastRight->pt.y) <= 5) {
                    sprintf(buffer, "Time=%ld, X=%d, Y=%d", lastRight->time, lastRight->pt.x, lastRight->pt.y);
                    HOOK_log_write_real_time(EVENT_MOUSE_R_DOUBLE_CLICK, buffer);
                    HOOK_log_new_mouse_event(log, lastRight, 0);
                    HOOK_log_new_mouse_event(log, mouseEvent, 0);
                } else{
                    pop(&log->stackMouseRight);
                    pop(&log->stackMouseRight);
                }
            default:
                while ((pop(&log->stackMouseRight)!= NULL)); 
                break;
            }
            break;
        case WM_MOUSEMOVE:
            if(!getSize(&log->stackMouseMove)){
                HOOK_log_new_mouse_event(log, mouseEvent, -1);
                break;
            }
            lastMove = (HOOK_log_MouseEvent*)pop(&log->stackMouseMove);
            if(mouseEvent->index - lastMove->index != 1){
                while(getSize(&log->stackMouseMove) > 0){
                    free(lastMove);
                    lastMove = (HOOK_log_MouseEvent*)pop(&log->stackMouseMove);
                }
                sprintf(buffer, "from X=%d, Y=%d to X=%d, Y=%d", lastMove->pt.x, lastMove->pt.y, mouseEvent->pt.x, mouseEvent->pt.y);
                HOOK_log_write_real_time(EVENT_MOUSE_MOVE, buffer);
            }
            HOOK_log_new_mouse_event(log, mouseEvent, -1);
            break;
        case WM_MOUSEWHEEL:
            {
                int32_t sumWheel = 0;
                if(getSize(&log->stackWheel) == 0){
                    HOOK_log_new_mouse_event(log, mouseEvent, -2);
                    break;
                }
                lastWheel = (HOOK_log_MouseEvent*)pop(&log->stackWheel);
                
                if((mouseEvent->index - lastWheel->index != 1)||((short)HIWORD(mouseEvent->mouseData) * (short)HIWORD(lastWheel->mouseData) < 0)){
                    sumWheel += (short)HIWORD(lastWheel->mouseData);
                    free(lastWheel);
                    while(getSize(&log->stackWheel) > 0){
                        lastWheel = (HOOK_log_MouseEvent*)pop(&log->stackWheel);
                        sumWheel += (short)HIWORD(lastWheel->mouseData);
                        free(lastWheel);
                    }
                    lastWheel = NULL;
                    sprintf(buffer, "Total delta=%ld", sumWheel);
                    HOOK_log_write_real_time(EVENT_MOUSE_WHEEL, buffer);
                } else {
                    HOOK_log_new_mouse_event(log, lastWheel, -2);
                }
                HOOK_log_new_mouse_event(log, mouseEvent, -2);
            }
            break;
        default:
            break;
    } 
    

    free(lastLeft);
    free(lastRight);
    free(lastMove);
    free(lastWheel);
    return 0;
}

int HOOK_vkcode_to_text(DWORD vk, char* buff, int size)
{
    UINT scan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    if (scan == 0)
        return FALSE;

    LONG lParam = scan << 16;

    return GetKeyNameTextA(lParam, buff, size) > 0;
}


int HOOK_log_filter_processing_key(HOOK_Logger *log, HOOK_log_KeyboardEvent *keyEvent) {
    if (keyEvent == NULL || log == NULL) return 1;

    HOOK_log_KeyboardEvent *last = NULL;
    static bool is_combo_key_2= false;
    static char keyName1[64] = {0};
    static char keyName2[64] = {0};
    char buffer[256];

    last = (HOOK_log_KeyboardEvent*)pop(&log->stackKey);

    if (last != NULL) {
        // Check if the current key event is a repeat of the last one
        if (last->MsgID == keyEvent->MsgID && last->scanCode == keyEvent->scanCode) {
            // Free the last event and return to avoid logging duplicates
            HOOK_log_new_key_event(log, last,0);
            free(last);
            return 0;
        }
        // If it's not a repeat, push the last event back onto the stack
        HOOK_log_new_key_event(log, last,0);
        free(last);
        
    }

    switch (keyEvent->MsgID) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            switch (getSize(&log->stackKey)) {
                case 0:
                    HOOK_log_new_key_event(log, keyEvent,0);
                    break;
                case 1:{
                    last = (HOOK_log_KeyboardEvent*)pop(&log->stackKey);
                    if(last -> MsgID == WM_KEYDOWN || last -> MsgID == WM_SYSKEYDOWN){
                        
                        
                        is_combo_key_2 = true;
                        if (!HOOK_vkcode_to_text(last->vkCode, keyName1, sizeof(keyName1))) sprintf(keyName1, "0x%X", last->vkCode);
                        if (!HOOK_vkcode_to_text(keyEvent->vkCode, keyName2, sizeof(keyName2))) sprintf(keyName2, "0x%X", keyEvent->vkCode);
                        

                        HOOK_log_new_key_event(log, last,0);
                        HOOK_log_new_key_event(log, keyEvent,0);
                        
                    } else {
                        sprintf(buffer, "unknown event detected for MsgID: %x", keyEvent->MsgID);
                        HOOK_log_write_real_time(EVENT_ERROR, buffer);
                    }
                    free(last);
                    break;
                }
                case 2:{

                    last = (HOOK_log_KeyboardEvent*)pop(&log->stackKey);
                    HOOK_log_KeyboardEvent* first = (HOOK_log_KeyboardEvent*)pop(&log->stackKey);

                    if(last -> MsgID == WM_KEYDOWN || last -> MsgID == WM_SYSKEYDOWN){
                        is_combo_key_2 = false;
                        char keyName1[64] = {0};
                        char keyName2[64] = {0};
                        char keyName3[64] = {0};
                        if (first && !HOOK_vkcode_to_text(first->vkCode, keyName1, sizeof(keyName1))) sprintf(keyName1, "0x%X", first->vkCode);
                        if (!HOOK_vkcode_to_text(last->vkCode, keyName2, sizeof(keyName2))) sprintf(keyName2, "0x%X", last->vkCode);
                        if (!HOOK_vkcode_to_text(keyEvent->vkCode, keyName3, sizeof(keyName3))) sprintf(keyName3, "0x%X", keyEvent->vkCode);
                        sprintf(buffer, "%s + %s + %s", keyName1, keyName2, keyName3);
                        HOOK_log_write_real_time(EVENT_KEY_COMBO_3, buffer);

                        if(first) HOOK_log_new_key_event(log, first,0);
                        HOOK_log_new_key_event(log, last,0);
                        HOOK_log_new_key_event(log, keyEvent,0);
                    } else {
                        sprintf(buffer, "unknown event detected for MsgID: %x", keyEvent->MsgID);
                        HOOK_log_write_real_time(EVENT_ERROR, buffer);
                        if(first) HOOK_log_new_key_event(log, first,0);
                    }
                    free(last);
                    free(first);
                    break;
                }
                default:
                    break;
            }   
            
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:{
            bool flag_has_event_been_processing = false;
            Node *tempStack = NULL;
            char keyName[64];

            if (is_combo_key_2)
            {
                sprintf(buffer, "%s + %s", keyName1, keyName2);
                HOOK_log_write_real_time(EVENT_KEY_COMBO_2, buffer);
                is_combo_key_2 = false;
            }
            
            

            // Iterate through the stack to find a matching keydown event
            while (getSize(&log->stackKey) > 0) {
                last = (HOOK_log_KeyboardEvent*)pop(&log->stackKey);
                if (last == NULL) continue; // defensive programming

                if (keyEvent->scanCode == last->scanCode) {
                    if (HOOK_vkcode_to_text(keyEvent->vkCode, keyName, sizeof(keyName))) {
                        if ((keyEvent->time - last->time) < TIME_KEY_PRESS) HOOK_log_write_real_time(EVENT_KEY_PRESS, keyName);
                        else HOOK_log_write_real_time(EVENT_KEY_HOLD, keyName);
                    } else {
                        if ((keyEvent->time - last->time) < TIME_KEY_PRESS) { sprintf(buffer, "%x", keyEvent->vkCode); HOOK_log_write_real_time(EVENT_KEY_PRESS, buffer); }
                        else { sprintf(buffer, "%x", keyEvent->vkCode); HOOK_log_write_real_time(EVENT_KEY_HOLD, buffer); }
                    }

                    flag_has_event_been_processing = true;
                    free(last); // Free the matched keydown event
                    break;
                } else {
                    // Push the event onto the temporary stack
                    push((void *)last, &tempStack);
                }
            }

            if (!flag_has_event_been_processing) {
                if (HOOK_vkcode_to_text(keyEvent->vkCode, keyName, sizeof(keyName))) {
                    sprintf(buffer, "error key, current key %s", keyName);
                    HOOK_log_write_real_time(EVENT_ERROR, buffer);
                } else {
                    sprintf(buffer, "error key, current key %x", keyEvent->vkCode);
                    HOOK_log_write_real_time(EVENT_ERROR, buffer);
                }
            }

            // Repopulate the original stack with the events from the temporary stack
            while (getSize(&tempStack) > 0) {
                last = (HOOK_log_KeyboardEvent*)pop(&tempStack);
                HOOK_log_new_key_event(log, last,0); // push last back to main stack
                free(last); // free temp stack
            }
            break;
        }

            break;
    }
    return 0;
}


int HOOK_log_filter_processing_both_mouse_and_key(HOOK_Logger *log,HOOK_log_KeyboardEvent *keyEvent, HOOK_log_MouseEvent *mouseEvent, int is_mouse) {
    static bool is_recording = false;
    static bool first_interaction = true;
    bool is_toggle_event = false;

    if (is_mouse) {
        if (mouseEvent == NULL || log == NULL) return 1;
        if (mouseEvent->MsgID == WM_LBUTTONDOWN) {
            is_toggle_event = true;
        }
    } else {
        if (keyEvent == NULL || log == NULL) return 1;
        // Check for Enter key down (VK_RETURN)
        if ((keyEvent->MsgID == WM_KEYDOWN || keyEvent->MsgID == WM_SYSKEYDOWN) && keyEvent->vkCode == VK_RETURN) {
            is_toggle_event = true;
        }
    }

    if (is_toggle_event) {
        if (first_interaction) {
            // First time MUST be a click to start
            if (is_mouse) {
                is_recording = true;
                first_interaction = false;
                // Clear stack just in case
                while (getSize(&log->stackKeySequence) > 0) {
                    free(pop(&log->stackKeySequence));
                }
            }
        } else {
            // Subsequent times: Click or Enter toggles
            if (is_recording) {
                // Stop recording -> Process sequence
                is_recording = false;
                int size = getSize(&log->stackKeySequence);
                char sequenceBuffer[1024] = {0};
                if(size){
                    // Buffer to accumulate the sequence string
                    
                    reverseStack(&log->stackKeySequence); // Reverse to print in chronological order
                }
                
                while (getSize(&log->stackKeySequence) > 0) {
                    HOOK_log_KeyboardEvent* lastKey = (HOOK_log_KeyboardEvent*)pop(&log->stackKeySequence);
                    if (lastKey) {
                        if (!HOOK_log_is_continue_keySequence(lastKey)) {
                            if (lastKey->vkCode == VK_BACK) {
                                size_t len = strlen(sequenceBuffer);
                                if (len > 0) sequenceBuffer[len - 1] = '\0';
                            } else {
                                char keyName[64] = {0};
                                if (lastKey->vkCode == VK_SPACE) {
                                    strcpy(keyName, " ");
                                } else {
                                    
                                    if (!HOOK_vkcode_to_text(lastKey->vkCode, keyName, sizeof(keyName))) {
                                        #if DEBUG
                                            sprintf(keyName, "?%x?", lastKey->vkCode);
                                        #endif
                                    }
                                    
                                    
                                }
                                if (strlen(sequenceBuffer) + strlen(keyName) >= sizeof(sequenceBuffer) - 1) {
                                    HOOK_log_write_real_time(EVENT_KEY_SEQUENCE, sequenceBuffer);
                                    memset(sequenceBuffer, 0, sizeof(sequenceBuffer));
                                }
                                strcat(sequenceBuffer, keyName);
                            }
                        }
                        free(lastKey);
                    }
                }
                if(strlen(sequenceBuffer) > 0) HOOK_log_write_real_time(EVENT_KEY_SEQUENCE, sequenceBuffer);
            } else {
                // Start recording
                is_recording = true;
            }
        }
    } else {
        // If recording and it's a key press (not the toggle Enter), log it
        if (is_recording && !is_mouse) {
            if (keyEvent->MsgID == WM_KEYDOWN || keyEvent->MsgID == WM_SYSKEYDOWN) {
                HOOK_log_new_key_event(log, keyEvent, 1);
            }
        }
    }
    return 0;
}

bool HOOK_log_is_continue_keySequence(HOOK_log_KeyboardEvent* keyEvent) {

    if(keyEvent == NULL) {
        printf("Loi: keyEvent la NULL trong HOOK_log_is_control_key\n");
        return false;
    }

    if(keyEvent->flags & LLKHF_EXTENDED) return false; // Loại bỏ các phím mở rộng

    DWORD vkCode = keyEvent->vkCode;

    if(vkCode & 0xE00) return false;
    switch (vkCode) {
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_MENU: // Alt key
        case VK_LMENU:
        case VK_RMENU:
        case VK_LWIN:
        case VK_RWIN:
            return true;
        default:
            return false;
    }
}

int HOOK_log_INIT(HOOK_Logger* logger) {
    logger->stackMouseLeft = NULL;
    logger->stackMouseRight = NULL;
    logger->is_event_right = 0;
    logger->is_event_left = 0;
    logger->stackKey = NULL;
    return 0;
}
