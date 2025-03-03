#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include "c2.h"

Test(C2, add_client_test) {
    struct sockaddr_in client_addr;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = htons(12345);
    
    int socket = 5; // Simulated socket descriptor
    int index = add_client(socket, client_addr);
    
    cr_expect(index >= 0 && index < MAX_CLIENTS, "Client should be added within valid range");
    cr_expect(clients[index].socket == socket, "Client socket should match assigned socket");
}

Test(C2, remove_client_test) {
    struct sockaddr_in client_addr;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = htons(12345);
    
    int socket = 6; // Simulated socket descriptor
    int index = add_client(socket, client_addr);
    remove_client(index);
    
    cr_expect(clients[index].socket == 0, "Client socket should be reset to 0 after removal");
}

Test(C2, list_clients_test) {
    struct sockaddr_in client_addr;
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    client_addr.sin_port = htons(12345);
    
    int socket = 7; // Simulated socket descriptor
    int index = add_client(socket, client_addr);
    
    FILE *fp = freopen("output.txt", "w", stdout);
    list_clients();
    fclose(fp);
    
    fp = fopen("output.txt", "r");
    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);
    
    cr_expect(strstr(buffer, "127.0.0.1"), "Output should contain the client's IP address");
}