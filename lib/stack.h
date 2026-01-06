#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>


// 1. Định nghĩa cấu trúc cho một Node (phần tử)
typedef struct Node {
    void *data;           // Dữ liệu chứa trong node (số nguyên)
    struct Node* next;  // Con trỏ trỏ đến node tiếp theo bên dưới
} Node;

void push(void *data, Node** top);
void* pop(Node** top);
void reverseStack(Node** top);
void printStack(Node** top);
int getSize(Node** top);


#endif // STACK_H