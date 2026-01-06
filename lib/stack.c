#include "stack.h" // Giả sử struct Node được định nghĩa ở đây

void push(void *data, Node** top) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Lỗi: Không thể cấp phát bộ nhớ!\n");
        return;
    }
    
    newNode->data = data; 
    newNode->next = *top;   
    *top = newNode;       
    
    // SỬA: Ép kiểu sang int* để lấy giá trị in ra (nếu data là con trỏ int)
    if (data != NULL) {
        printf("Da push: %d\n", *(int*)data);
    } else {
        printf("Da push: NULL\n");
    }
}

void* pop(Node** top) {
    if (*top == NULL) {
        printf("Stack dang rong, khong the pop!\n");
        return NULL; 
    }
    
    Node* temp = *top;      
    void *poppedValue = (*top)->data; 
    *top = (*top)->next;       
    free(temp);            
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
    printf(">> Da dao nguoc Stack!\n");
}

void printStack(Node** top) {
    Node* temp = *top;
    printf("Stack hien tai (Dinh -> Day): ");
    while (temp != NULL) {
        // SỬA: Ép kiểu void* sang int* rồi lấy giá trị
        if (temp->data != NULL) {
            printf("%d -> ", *(int*)(temp->data));
        } else {
            printf("NULL -> ");
        }
        temp = temp->next;
    }
    printf("NULL\n");
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