#include <criterion/criterion.h>
#include "c2.h"

/**
 * @brief Setup function to initialize the clients array before each test.
 *        It resets all client sockets to 0 and clears their addresses.
 */
void setup(void) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = 0;
        memset(&clients[i].address, 0, sizeof(clients[i].address));
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Define the test suite and associate the setup function to run before each test
TestSuite(client_tests, .init = setup);

/**
 * @brief Test to verify that a client can be added successfully.
 */
Test(client_tests, should_add_client_successfully) {
    struct sockaddr_in test_addr = { .sin_family = AF_INET, .sin_port = htons(8080) };
    int test_socket = 10;

    int index = add_client(test_socket, test_addr);
    
    cr_assert_neq(index, -1, "Client should be added successfully.");
    cr_assert_eq(clients[index].socket, test_socket, "Socket should be set correctly.");
    cr_assert_eq(clients[index].address.sin_port, htons(8080), "Port should match.");
}

/**
 * @brief Test to ensure no client can be added when the client list is full.
 */
Test(client_tests, should_not_add_client_when_full) {
    struct sockaddr_in test_addr = { .sin_family = AF_INET, .sin_port = htons(8080) };

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int socket = i + 1;
        int index = add_client(socket, test_addr);
        cr_assert_neq(index, -1, "Client should be added successfully at index %d.", i);
    }

    int index = add_client(999, test_addr);

    cr_assert_eq(index, -1, "Should not add client when list is full.");
}

/**
 * @brief Test to verify that a client is correctly removed.
 */
Test(client_tests, should_remove_client_successfully) {
    struct sockaddr_in test_addr = { .sin_family = AF_INET, .sin_port = htons(8080) };
    int test_socket = 10;
    
    int index = add_client(test_socket, test_addr);
    cr_assert_neq(index, -1, "Client should be added successfully.");
    
    remove_client(test_socket);
    
    cr_assert_eq(clients[index].socket, 0, "Socket should be cleared after removal.");
}

/**
 * @brief Test to check if clients are listed correctly after being added.
 */
Test(client_tests, should_list_clients_correctly) {
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(8080) };
    int socket = 10;
  
    int index = add_client(socket, addr);
    
    cr_assert_neq(index, -1, "First client should be added successfully.");
    cr_assert_eq(clients[index].socket, socket, "First client should be listed correctly.");
}

/**
 * @brief Test to ensure that attempting to remove a non-existent client does not affect existing clients.
 */
Test(client_tests, should_ignore_removal_of_nonexistent_client) {
    remove_client(99);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        cr_assert_neq(clients[i].socket, 99, "No client should have the nonexistent socket.");
    }
}