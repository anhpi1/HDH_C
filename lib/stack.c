#include "stack.h" // Giả sử struct Node được định nghĩa ở đây


void push(void *data, Node** top) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("FROM stack.c:Lỗi: Không thể cấp phát bộ nhớ!\n");
        return;
    }
    
    newNode->data = data; 
    newNode->next = *top;   
    *top = newNode;       
    
    // SỬA: Ép kiểu sang int* để lấy giá trị in ra (nếu data là con trỏ int)
    if (data != NULL) {
        #if DEBUG
            printf("FROM stack.c: Da push ");
        #endif
    } else {
        printf("Da push: NULL \n");
    }
}

void* pop(Node** top) {
    if (*top == NULL) {
        #if DEBUG
            printf("FROM stack.c: Stack dang rong, khong the pop!\n");
        #endif
        return NULL; 
    }
    
    Node* temp = *top;      
    void *poppedValue = (*top)->data; 
    *top = (*top)->next;       
    free(temp);    
    #if DEBUG     
        printf("FROM stack.c: Da pop "); 
    #endif
    return poppedValue;
}

void reverseStack(Node** top) {
    Node* prev = NULL;     
    Node* current = *top;   
    Node* next = NULL;     
    
    //  
    // Logic này của bạn đã chính xác
    while (current != NULL) {
        next = current->next;  
        current->next = prev;  
        prev = current;
        current = next;
    }
    
    *top = prev; 
}


int getSize(Node** top) {
    int count = 0;
    Node* current = *top; 
    
    while (current != NULL) {
        count++;
        current = current->next;
    }
    
    return count;
}