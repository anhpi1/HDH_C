#include "main.h" 


int main() { 
    ServerHandle Server;
    if (HOOK_Server_init(&Server) != 0) {
        return 1;
    }
    if (HOOK_Server_start(&Server) != 0) {
        return 1;
    }
    return 0; 
}
