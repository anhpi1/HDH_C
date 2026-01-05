#include "ring_buffer.h"




uint8_t HOOK_FUNC_Write_RingData(HOOK_ring_buffer* ring, void* data) {
    LONG currentWrite = ring->pWrite;
    LONG nextWrite = (currentWrite + 1) % BUFFER_SIZE;
    LONG currentRead = ring->pRead; 
    if (nextWrite == currentRead) {
        printf("Warning: Ring buffer is full, dropping data\n");
        return 1;
    }
    ring->buffer[currentWrite] = data;
    InterlockedExchange(&ring->pWrite, nextWrite);
    LONG count;
    ReleaseSemaphore(ring->hSemaphore, 0, &count);
    if (!count)ReleaseSemaphore(ring->hSemaphore,1,NULL);
    return 0; 
}

uint8_t HOOK_FUNC_Read_RingData(HOOK_ring_buffer* ring, void** dataOut) {
    LONG currentRead = ring->pRead;
    LONG currentWrite = ring->pWrite;
    if (currentRead == currentWrite) {
        WaitForSingleObject(ring->hSemaphore, INFINITE);
        WaitForSingleObject(ring->hSemaphore, INFINITE);
    }
    *dataOut = ring->buffer[currentRead];
    MemoryBarrier();
    LONG nextRead = (currentRead + 1) % BUFFER_SIZE;
    InterlockedExchange(&ring->pRead, nextRead);
    return 0; 
}

uint8_t HOOK_FUNC_RingData_INIT(HOOK_ring_buffer* ring) {
    ring->pRead = 0;
    ring->pWrite = 0;
    ring->hSemaphore = CreateSemaphore(
        NULL,   // security
        1,      // giá trị ban đầu
        1,      // giá trị tối đa
        NULL    // không đặt tên
    );

    return 0;
}