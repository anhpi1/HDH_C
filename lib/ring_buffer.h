#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <direct.h>

#define BUFFER_SIZE    10000
#define BUFFER_SIZE_SMALL 100

typedef struct{
    volatile LONG pRead;
    volatile LONG pWrite;
    HANDLE hSemaphore;
    void* buffer[BUFFER_SIZE];
}HOOK_ring_buffer;

uint8_t HOOK_FUNC_RingData_INIT(HOOK_ring_buffer* ring);
uint8_t HOOK_FUNC_Read_RingData(HOOK_ring_buffer* ring, void** dataOut);
uint8_t HOOK_FUNC_Write_RingData(HOOK_ring_buffer* ring, void* data);

typedef struct{
    volatile LONG pRead;
    volatile LONG pWrite;
    HANDLE hSemaphore;
    void* buffer[BUFFER_SIZE_SMALL];
}HOOK_ring_buffer_small;

uint8_t HOOK_FUNC_RingData_INIT_small(HOOK_ring_buffer_small* ring);
uint8_t HOOK_FUNC_Read_RingData_small(HOOK_ring_buffer_small* ring, void** dataOut);
uint8_t HOOK_FUNC_Write_RingData_small(HOOK_ring_buffer_small* ring, void* data);



#endif // RING_BUFFER_H